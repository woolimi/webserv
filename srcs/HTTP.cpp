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

void HTTP::manage_clients(fd_set &read_set, fd_set &write_set, fd_set &init_set, std::set<int> &fds, char **env)
{
	std::vector<t_client>::iterator it;
	char buffer[MAX_BUFFER_SIZE + 1];
	ssize_t nb_read;
	struct timeval tv;

	for (it = clients.begin(); it != clients.end(); ++it)
	{
		// receive request
		if (FD_ISSET(it->socket, &read_set) && it->res.status_code == 0)
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
			it->req.raw += std::string(buffer);
			renew_client_timestamp(*it);
			try
			{
				if (!it->req_line_arrived)
				{
					// check request line
					// if invalid, throw client with error status code
					// if valid, in the function
					// set it.req_line_arrived = true
					// subtract request line from it.raw  

					// req_interpreter(,REQ_LINE, it);
				}
				if (it->req_line_arrived && !it->req_header_arrived)
				{
					// check request header
					// if invalid, throw client with error status code
					// if valid, in the function
					// set it.req_header_arrived = true;
					// subtract all header lines from it.raw

					// req_interpreter(,REQ_HEADER, it);
				}
				if (it->req_line_arrived && it->req_header_arrived && !it->req_body_arrived)
				{
					// case1. content-length : make req.body, remove part from it.raw, check req.body size.
						// if body size < content-length it->req_body_arrived = false;
						// if body size == content-length it->req_body_arrived = true
						// if body size > content-length it->req_body_arrived = true, throw client with error code
					// case2. transfer-encoding : chunk : 
					// case3. unsupportable transfer-encoding
					// case4. empty body. it->req_body_arrive = true;
				}
			}
			catch (t_client &client)
			{
				res_generator(client); // with conent-length
			}
			continue;
		}

		// response
		if (it->res.head.empty() && !it->res.sent_head)
		{	// make head and keep fd
			handle_methods(*it, env);
		}

		// send res head
		if (!it->res.sent_head && FD_ISSET(it->socket, &write_set))
		{
			if (!send_res_head(*it))
				disconnect(init_set, fds, it);
			continue;
		}
		
		// make res body
		if (!it->res.sent_body && it->res.body.empty())
		{	// read()
			make_res_body_from_fd(*it);
			continue;
		}
		else if (!it->res.sent_body && FD_ISSET(it->socket, &write_set))
		{	// write()
			if (!send_res_body(*it))
				disconnect(init_set, fds, it);
			continue;
		}
		
		if (it->res.sent_body)
		{
			it->req.req_line_parsed = false;
			it->req_header_arrived = false;
			it->req_body_arrived = false;
			it->res.status_code = 0;
			it->res.sent_head = false;
			it->res.sent_body = false;
		}

		// close client when timeout
		gettimeofday(&tv, NULL);
		if (tv.tv_sec - it->time_stamp > CLIENT_TIMEOUT_SEC)
		{
			disconnect(init_set, fds, it);
			continue;
		}

		// Server reject client connection with 503 respond
		if (it - clients.begin() > MAX_CLIENT)
		{
			respond_service_unavailable(*it);
			disconnect(init_set, fds, it);
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
			new_client.server = *it;
			clients.push_back(new_client);
			fds.insert(new_client.socket); //fdmax
			FD_SET(new_client.socket, &init_set);
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
	res.head.clear();
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
	client.req_line_arrived = false;
	client.req_header_arrived = false;
	client.req_body_arrived = false;
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