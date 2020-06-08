#include "HTTP.hpp"

HTTP::HTTP(std::vector<t_server> &srvs)
	: servers(srvs)
{
	// set server socket and bind() / listen()
	int reuse_port = 1;
	std::vector<t_server>::iterator it;
	for (it = servers.begin(); it != servers.end(); ++it)
	{
		it->addr_len = sizeof(it->addr);
		// create server socket
		if ((it->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0)
			throw FailToSetServerSocket();
		// make non blocking server socket
		if (fcntl(it->socket, F_SETFL, O_NONBLOCK) < 0)
			throw FailToSetServerSocket();
		// set reuse port
		if (setsockopt(it->socket, SOL_SOCKET, SO_REUSEADDR, &reuse_port, sizeof(reuse_port)) < 0)
			throw FailToSetServerSocket();
		// bind
		memset(&it->addr, 0, it->addr_len); // change to ft_memset
		it->addr.sin_family = AF_INET;
		it->addr.sin_addr.s_addr = htonl(INADDR_ANY);
		it->addr.sin_port = htons(it->listen);
		if (bind(it->socket, (struct sockaddr *)&it->addr, it->addr_len) < 0)
			throw FailToSetServerSocket();
		// listen
		if (listen(it->socket, 1000) < 0)
			throw FailToSetServerSocket();
	}
}

HTTP::~HTTP()
{
}

void HTTP::run(char **env)
{
	int fdmax;
	fd_set read_set, write_set, init_set;
	std::set<int> fds;
	std::vector<t_server>::iterator it;

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&init_set);
	// add server socket into init_set and init_fds
	for (it = servers.begin(); it != servers.end(); ++it)
	{
		FD_SET(it->socket, &init_set);
		fds.insert(it->socket);
	}
	// start server
	struct timeval timeout;
	while (1)
	{
		read_set = init_set;
		write_set = init_set;
		fdmax = *fds.rbegin();

		init_timeout(timeout, 3, 0); // sec, usec
		http_select(fdmax, read_set, write_set, timeout);
		manage_clients(read_set, write_set, init_set, fds, env);
		manage_servers(read_set, init_set, fds);
	}
}

void HTTP::skip_leading_empty_line(t_client &cli, char *buffer)
{
	int i = 0;
	if (!cli.req.req_line_parsed && !cli.req.req_header_parsed && !cli.req.req_body_parsed)
	{
		while (buffer[i] && is_newline_char(buffer[i])) //skip leading empty lines before the request
			i++;
		cli.req.raw += std::string(&buffer[i]);
	}
	else
		cli.req.raw += std::string(buffer);
}

void HTTP::manage_clients(fd_set &read_set, fd_set &write_set, fd_set &init_set, std::set<int> &fds, char **env)
{
	std::vector<t_client>::iterator it;
	char buffer[MAX_BUFFER_SIZE + 1];
	ssize_t nb_read = 0;
	struct timeval tv;

	for (it = clients.begin(); it != clients.end(); ++it)
	{
		// close client when timeout
		gettimeofday(&tv, NULL);
		if (tv.tv_sec - it->time_stamp > CLIENT_TIMEOUT_SEC)
		{
			printf("client timeout disconnect\n");
			disconnect(init_set, fds, it);
			continue;
		}

		// Server reject client connection with 503 respond
		if (it - clients.begin() > MAX_CLIENT)
		{
			respond_service_unavailable(*it);
			disconnect(init_set, fds, it);
		}

		// receive request
		if (!it->req_arrived && FD_ISSET(it->socket, &read_set))
		{
			nb_read = read(it->socket, buffer, MAX_BUFFER_SIZE);
			if (nb_read == 0)
				continue;
			// Client close connection unexpectly
			if (nb_read < 0)
			{
				disconnect(init_set, fds, it);
				continue;
			}
			buffer[nb_read] = '\0';
			renew_client_timestamp(*it);
			skip_leading_empty_line(*it, buffer);

			if (it->req.raw.find("\r\n") == std::string::npos)
				continue;
			// request line parsing
			if (it->req.req_line_parsed != 2)
			{
				it->req.req_line_parsed = 1;
				parse_request_line((char *)it->req.raw.substr(0, it->req.raw.find("\r\n")).c_str(), *it);
				it->req.req_line_parsed = 2;
				it->req.raw = it->req.raw.substr(it->req.raw.find("\r\n") + 2);
			}
			// request header parsing
			if (it->req.req_line_parsed == 2 && it->req.req_header_parsed != 2)
			{
				if (it->req.raw.empty())
					continue;
				it->req.req_header_parsed = 1;
				while (it->req.req_header_parsed != 2 && !it->req.raw.empty())
				{
					parse_request_header(*it, it->req.raw.substr(0, it->req.raw.find("\r\n") + 2));
					it->req.raw = it->req.raw.substr(it->req.raw.find("\r\n") + 2);
				}
			}
			// request body parsing
			if (it->req.req_line_parsed == 2 && it->req.req_header_parsed == 2 && it->req.req_body_parsed != 2)
			{
				if (it->req_arrived)
					it->req.req_body_parsed = 2;
				else
				{
					parse_request_body(*it);
				}
			}
		}


		/******************/
		/*     respond    */
		/******************/
		// make response head and body
		if (it->req_arrived && it->res.head.empty() && !it->res.sent_head)
		{
			// make head and keep fd
			if (it->res.status_code == 0)
			{
				// std::cout << "handle methods" << std::endl;
				handle_methods(*it, env);
			}
			else
			{
				// std::cout << "res gen" << std::endl;
				res_generator(*it);
			}
			// std::cout << it->res.head << std::endl;
			// std::cout << it->res.body << std::endl;
		}

		// send res head

		if (it->req_arrived && !it->res.sent_head && FD_ISSET(it->socket, &write_set))
		{
			if (!send_res_head(*it))
				disconnect(init_set, fds, it);
			printf("sent response head\n");
			continue;
		}
		
		// make res body if res.body not exist
		if (it->req_arrived && !it->res.sent_body && it->res.body.empty())
		{	// read()
			make_res_body_from_fd(*it);
			continue;
		}
		
		if (it->req_arrived && !it->res.sent_body && !it->res.head.empty() && FD_ISSET(it->socket, &write_set))
		{	// write()
			if (!send_res_body(*it))
				disconnect(init_set, fds, it);
			printf("sent response body\n");
			continue;
		}
		
		if (it->res.sent_body)
		{
			it->req.req_line_parsed = 0;
			it->req.req_header_parsed = 0;
			it->req.req_body_parsed = 0;
			it->req_arrived = false;
			it->res_sent = false;
			it->res.status_code = 0;
			it->res.sent_head = false;
			it->res.sent_body = false;
			it->res.head.clear();
			it->res.headers.clear();
			it->res.body.clear();
			it->req.headers.clear();
			it->req.method.clear();
			it->req.path.clear();
			it->req.chunk_size_read = -1;
			it->req.raw.clear();
			it->req.headers.clear();
			printf("reset\n");
		}
	}
}

void HTTP::manage_servers(fd_set &read_set, fd_set &init_set, std::set<int> &fds)
{
	t_client new_client;
	std::vector<t_server>::iterator it;

	init_client(new_client);
	for (it = servers.begin(); it != servers.end(); ++it)
	{
		if (FD_ISSET(it->socket, &read_set))
		{
			new_client.socket = accept(it->socket, (sockaddr *)&new_client.addr, &new_client.addr_len);
			if (new_client.socket < 0)
				throw FailToAccept();
			if (fcntl(new_client.socket, F_SETFL, O_NONBLOCK) < 0)
				throw FailToSetClientSocket();
			renew_client_timestamp(new_client);
			new_client.server = *it;
			clients.push_back(new_client);
			fds.insert(new_client.socket); //fdmax
			FD_SET(new_client.socket, &init_set);
			// std::cout << "FD1: "<< FD_ISSET(new_client.socket, &init_set) << std::endl;
			printf("client connected\n");
		}
	}
}

void HTTP::make_res_body_from_fd(t_client &cli)
{
	t_res &res = cli.res;
	char buff[MAX_BUFFER_SIZE + 1];
	int nb_read = read(res.fd, buff, MAX_BUFFER_SIZE);
	if (nb_read <= 0)
		res.body = "0\r\n\r\n";
	else
	{
		buff[nb_read] = 0;
		res.body += int_to_hexstr(nb_read) + "\r\n";
		res.body += std::string(buff) + "\r\n";
	}
}

bool HTTP::send_res_head(t_client &cli)
{
	t_res &res = cli.res;
	int ret = write(cli.socket, cli.res.head.c_str(), cli.res.head.size());
	if (ret < 0)
		return false; // disconnect
	if (ret == 0)
		return true;
	res.sent_head = true;
	return true;
}

bool HTTP::send_res_body(t_client &cli)
{
	t_res &res = cli.res;

	int ret = write(cli.socket, cli.res.body.c_str(), cli.res.body.size());
	if (ret < 0)
		return false; // disconnect
	if (ret == 0)
		return true;
	if (res.headers.find("Content-Length") != res.headers.end())
		res.sent_body = true;
	else if (res.headers.find("Transfer-Encoding") != res.headers.end() 
		&& res.headers["Transfer-Encoding"] == "chunked" && cli.res.body == "0\r\n\r\n")
	{
		close(res.fd);
		res.sent_body = true;
	}
	res.body.clear();
	renew_client_timestamp(cli);
	return true;
}

void HTTP::disconnect(fd_set &init_set, std::set<int> &fds, std::vector<t_client>::iterator &it)
{
	fds.erase(it->socket);
	FD_CLR(it->socket, &init_set);
	close(it->socket);
	it = clients.erase(it);
	--it;
	printf("client disconnected\n");
}

void HTTP::init_client(t_client &client)
{
	client.socket = 0;
	client.addr_len = sizeof(client.addr);
	/* req */
	client.req.req_line_parsed = 0;
	client.req.req_header_parsed = 0;
	client.req.req_body_parsed = 0;
	client.req.content_length = -1;
	client.addr_len = sizeof(client.addr);
	client.req_arrived = false;
	client.res_sent = false;
	client.res.status_code = 0;
	client.req.chunk_size_read = -1;
	/* res */
	client.res.status_code = 0;
	client.res.sent_head = false;
	client.res.sent_body = false;
}

void HTTP::init_timeout(struct timeval &timeout, int sec, int usec)
{
	timeout.tv_sec = sec;
	timeout.tv_usec = usec;
}

void HTTP::http_select(int fdmax, fd_set &read_set, fd_set &write_set, struct timeval &timeout)
{
	int ret;

	if ((ret = select(fdmax + 1, &read_set, &write_set, NULL, &timeout)) < 0)
		throw FailToSelect();
	/* debug */
	if (ret == 0)
		printf("waiting client\n");
}

void HTTP::handle_methods(t_client &cli, char **env)
{
	t_req &req = cli.req;

	if (req.method == "GET")
		handle_get(cli, env);
	// else if (req.method == "HEAD")
	// 	handle_head(cli);
	// ...
}

void HTTP::renew_client_timestamp(t_client &cli)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	cli.time_stamp = tv.tv_sec;
}

void HTTP::respond_service_unavailable(t_client &cli)
{
	t_res &res = cli.res;

	res.status_code = 503; // ServiceUnavailable
	res_generator(cli);
	std::string send = res.head + res.body;
	write(cli.socket, send.c_str(), send.size());
}

const char *HTTP::FailToSetServerSocket::what() const throw()
{
	return "HTTP : fail to set server socket\n";
}

const char *HTTP::FailToSetClientSocket::what() const throw()
{
	return "HTTP : fail to set client socket\n";
}

const char *HTTP::FailToSelect::what() const throw()
{
	return "HTTP : fail to select\n";
}

const char *HTTP::FailToAccept::what() const throw()
{
	return "HTTP : fail to accept\n";
}