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

void HTTP::run()
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
		manage_clients(read_set, write_set, init_set, fds);
		manage_servers(read_set, init_set, fds);
	}
}

void HTTP::manage_clients(fd_set &read_set, fd_set &write_set, fd_set &init_set, std::set<int> &fds)
{
	std::vector<t_client>::iterator it;
	char buffer[MAX_BUFFER_SIZE + 1];
	ssize_t nb_read = 0;
	struct timeval tv;

	for (it = clients.begin(); it != clients.end(); ++it)
	{
		// receive request
		if (FD_ISSET(it->socket, &read_set))
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
			
			int i = 0;
			if (!it->req.req_line_parsed && !it->req.req_header_parsed && !it->req.req_body_parsed)
			{
				while (buffer[i] && is_newline_char(buffer[i]))//skip leading empty lines before the request
					i++;
				it->req.raw += std::string(&buffer[i]);
			}
			else
				it->req.raw += std::string(buffer);
			gettimeofday(&tv, NULL);
			it->time_stamp = tv.tv_sec; // renew timestamp
			if (it->req.raw.find("\r\n") == std::string::npos)
				continue;
			if (it->req.req_line_parsed != 2)
			{
				it->req.req_line_parsed = 1;
				parse_request_line((char*)it->req.raw.substr(0, it->req.raw.find("\r\n")).c_str(), *it);
				it->req.req_line_parsed = 2;
				it->req.raw = it->req.raw.substr(it->req.raw.find("\r\n") + 2);
				std::cout << it->req.method << " ";
				std::cout << it->req.path << " ";
				std::cout << it->req.version << std::endl;
				// check request line
				// if invalid, throw client with error status code
				// if valid, in the function
				// set it.req_line_parsed = 2
				// subtract request line from it.raw  
				// req_interpreter(,REQ_LINE, it);
			}
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
				
				// check request header
				// if invalid, throw client with error status code
				// if valid, in the function
				// set it.req_header_arrived = true;
				// subtract all header lines from it.raw
				// req_interpreter(,REQ_HEADER, it);
			}
			if (it->req.req_line_parsed == 2 && it->req.req_header_parsed == 2 && it->req.req_body_parsed != 2)
			{
				if (it->req_arrived)
				{
					it->req.req_body_parsed = 2;
				}
				else
					parse_request_body(*it);
				// case1. content-length : make req.body, remove part from it.raw, check req.body size.
					// if body size < content-length it->req_body_arrived = false;
					// if body size == content-length it->req_body_arrived = true
					// if body size > content-length it->req_body_arrived = true, throw client with error code
				// case2. transfer-encoding : chunk : 
				// case3. unsupportable transfer-encoding
				// case4. empty body. it->req_body_arrive = true;
			}
			if (it->req_arrived && !it->res_sent)
			{
				std::cout << "Body: " << it->req.body << std::endl;
				std::cout << "Status: " << it->res.status_code << std::endl;
				// handle_methods(*it);
				// res_generator(*it); // with conent-length
				//reset all request parsing flags
			}
		}

		// make response
		// if response is not set
		// if (it->res.status_code == 0)
		// {
		// }

		// send response (Content-Length)
		if (!it->res.raw.empty() && FD_ISSET(it->socket, &write_set))
		{
			if (send_response(it) < 0)
				disconnect(init_set, fds, it);
			continue;
		} else if (it->res.raw.empty() && it->req.req_line_parsed == true)
		{	// for keeping alive...
			it->req.req_line_parsed = 0;
			it->req.req_header_parsed = 0;
			it->req.req_body_parsed = 0;
			it->req_arrived = false;
			it->res_sent = false;
			it->res.status_code = 0;
		}

		// close client when timeout
		gettimeofday(&tv, NULL);
		// if (tv.tv_sec - it->time_stamp > CLIENT_TIMEOUT_SEC)
		// {
		// 	disconnect(init_set, fds, it);
		// 	continue;
		// }

		// Server reject client connection with 503 respond
		if (it - clients.begin() > MAX_CLIENT)
		{
			respond_service_unavailable(it);
			disconnect(init_set, fds, it);
			continue;
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
			// std::cout << "FD1: "<< FD_ISSET(new_client.socket, &init_set) << std::endl;
			printf("client connected\n");
		}
	}
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
	client.req.req_line_parsed = 0;
	client.req.req_header_parsed = 0;
	client.req.req_body_parsed = 0;
	client.req.content_length = -1;
	client.addr_len = sizeof(client.addr);
	client.req_arrived = false;
	client.res_sent = false;
	// client.req_line_arrived = false;
	// client.req_header_arrived = false;
	// client.req_body_arrived = false;
	client.res.status_code = 0;
	client.req.chunk_size_read = -1;
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

void HTTP::handle_methods(t_client &cli)
{
	t_req &req = cli.req;
	if (req.method == "GET")
		handle_get(cli);
	// else if (req.method == "HEAD")
	// 	handle_head(cli);
	// ...
}

void HTTP::respond_service_unavailable(std::vector<t_client>::iterator &it)
{
	it->res.status_code = 503; // ServiceUnavailable
	res_generator(*it);
	write(it->socket, it->res.raw.c_str(), it->res.raw.size());
}

int HTTP::send_response(std::vector<t_client>::iterator &it)
{
	struct timeval tv;
	ssize_t nb_write;

	if (it->res.raw.size() > MAX_BUFFER_SIZE)
		nb_write = write(it->socket, it->res.raw.c_str(), MAX_BUFFER_SIZE);
	else
		nb_write = write(it->socket, it->res.raw.c_str(), it->res.raw.size());
	if (nb_write == -1)
		return (-1);
	if (nb_write == 0)
		return (0);
	// if sent response without problem, renew client time
	gettimeofday(&tv, NULL);
	it->time_stamp = tv.tv_sec;
	// remove sent data from res.raw
	it->res.raw.erase(0, nb_write);
	return (1);
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