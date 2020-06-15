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
		if (listen(it->socket, 2500) < 0)
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

		init_timeout(timeout, SERVER_TIMEOUT_SEC, SERVER_TIMEOUT_USEC); // sec, usec
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
		if (!buffer[i] && !cli.req.raw.empty())
		{
			while (is_newline_char(cli.req.raw[i]))
				i++;
			cli.req.raw = &cli.req.raw[i];
			return;
		}
		while (buffer[i] && is_newline_char(buffer[i])) //skip leading empty lines before the request
			i++;
		cli.req.raw += std::string(&buffer[i]);
		i = 0;
		while (is_newline_char(cli.req.raw[i]))
				i++;
		cli.req.raw = &cli.req.raw[i];
		// std::cout << "RAW: [" << cli.req.raw << "]\nBUUUFFER: [" << buffer << "]\n";
		
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
		// gettimeofday(&tv, NULL);
		// if (tv.tv_sec - it->time_stamp > CLIENT_TIMEOUT_SEC)
		// {
		// 	printf("client timeout disconnect\n");
		// 	disconnect(init_set, fds, it);
		// 	continue;
		// }

		// Server reject client connection with 503 respond
		// if (it - clients.begin() > MAX_CLIENT)
		// {
		// 	respond_service_unavailable(*it);
		// 	disconnect(init_set, fds, it);
		// 	printf("max client exceed disconnect");
		// 	continue;
		// }
		// receive request
		if (!it->req_arrived)
		{
			if (!it->req.raw.empty() || FD_ISSET(it->socket, &read_set))
			{	// something to read
				nb_read = read(it->socket, buffer, MAX_BUFFER_SIZE);
				if (nb_read == 0 && it->req.raw.empty())
					continue;
				// Client close connection unexpectly
				if (nb_read < 0)
				{
					if (it->req.raw.empty())
					{
						disconnect(init_set, fds, it);
						continue;
					}
					nb_read = 0;
				}
				buffer[nb_read] = '\0';
				renew_client_timestamp(*it);
				skip_leading_empty_line(*it, buffer);
			}

			if (it->req.raw.find("\r\n") == std::string::npos)
			{
				/* need to find why raw is empty in second step */
				continue;
			}

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
				int x = 0;
				if (it->req.raw.empty() || it->req.raw.find("\r\n") == std::string::npos)
					continue;
				it->req.req_header_parsed = 1;
				while (it->req.req_header_parsed != 2 && !it->req.raw.empty())
				{
					if (it->req.raw.find("\r\n") == std::string::npos)
					{
						x = 1;
						break;
					}
					parse_request_header(*it, it->req.raw.substr(0, it->req.raw.find("\r\n") + 2));
					it->req.raw = it->req.raw.substr(it->req.raw.find("\r\n") + 2);
				}
				if (x)
					continue;
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
			if (it->res.status_code == 0)
			{
				handle_methods(*it, env);
				res_generator(*it);
			}
			else
			{
				// error while parsing request
				res_generator(*it);
			}
		}

		// send res head
		if (it->req_arrived && !it->res.sent_head && FD_ISSET(it->socket, &write_set))
		{
			if (!send_res_head(*it))
				disconnect(init_set, fds, it);
			printf("sent response head\n");
			if (it->req.method == "HEAD" || it->req.method == "PUT")
				it->res_sent = true;
			continue;
		}
		
		// make res body if res.body not exist
		if (it->req_arrived && !it->res_sent && it->res.body.empty())
		{	// read()
			// printf("make res body from fd\n");
			make_res_body_from_fd(*it);
			continue;
		}

		if (it->req_arrived && !it->res_sent && FD_ISSET(it->socket, &write_set))
		{	// write()
			if (!send_res_body(*it))
			{
				disconnect(init_set, fds, it);
				continue;
			}
			if (it->res_sent)
			{
				printf("sent all response body\n");
				continue;
			}
		}
		
		if (it->res_sent)
		{
			// std::cout << it->req.raw.empty() << " " << FD_ISSET(it->socket, &read_set) << std::endl;
			if (it->req.raw.empty() && !FD_ISSET(it->socket, &read_set))
			{
				disconnect(init_set, fds, it);
				printf("disconneted after treat all request\n");
			}
			else
			{
				it->req.req_line_parsed = 0;
				it->req.req_header_parsed = 0;
				it->req.req_body_parsed = 0;
				it->req.body.clear();
				it->req_arrived = false;
				it->req.content_length = -1;
				it->res_sent = false;
				it->res.status_code = 0;
				it->res.sent_head = false;
				it->res.head.clear();
				it->res.headers.clear();
				it->res.body.clear();
				it->req.headers.clear();
				it->req.method.clear();
				it->req.path.clear();
				it->req.chunk_size_read = -1;
				printf("reset\n");
			}
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
	client.res.content_length = 0;
	client.res.fd = -1;
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
	{
		std::cerr << strerror(errno) << std::endl;
		throw FailToSelect();
	}
	/* debug */
	if (ret == 0)
		printf("waiting client\n");
}

int isDirectory(const char *path)
{
   struct stat statbuf;

   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

void HTTP::handle_methods(t_client &cli, char **env)
{
	t_server &serv = cli.server;
	t_req &req = cli.req;
	t_location *loc;
	bool is_file = true;
	std::string folder_path;
	std::string file;

	loc = find_matched_location(serv, req.path);
	is_file = isDirectory(loc->abs_path.c_str()) ? false : true;
	if (is_file)
	{
		folder_path = loc->abs_path;
		file = req.path.substr(req.path.find_last_of('/'));
	}
	else
	{
		folder_path = loc->abs_path;
		file = "/";
	}
	
	// loc = find_matched_location(serv,folder_path, file);
	
	/*
	
	get loc:
			check exact match
				else
			check part match
				else
			`

	case: folder /directory/test/abc
			folder_path = /test/abc/
			file = "/"
			
	case: file /directory/test/abc.txt
			folder_path = /test/
			file = "/abc.txt"

	1. req-path, root path
	2. abs=path = root path + req path
	3. is_file = check_is_file() returns 0 for dir 1 for file

	loc = find_matched_location(serv, req_path, &is_file)


	*/

	std::vector<std::string>::iterator it1;
	for (it1 = loc->allow.begin(); it1 !=loc->allow.end(); ++it1)
	{	
		if (*it1 == req.method)
			break;
	}
	if (it1 == loc->allow.end())
	{
		if (loc->allow.empty())
		{
			cli.res.status_code = 405;
			return;
		}
		std::string allow = loc->allow[0];
		it1 = loc->allow.begin() + 1;
		for (; it1 != loc->allow.end(); ++it1)
			allow = allow + ", " + *it1;
		cli.res.headers["Allow"] = allow;
		cli.res.status_code = 405;
		return ;
	}

	if (req.method == "GET" || req.method == "HEAD")
		handle_get(cli, env, loc, is_file, folder_path, file);
	else if (req.method == "PUT")
		handle_put(cli, env, loc, is_file, folder_path, file);

	// 	handle_head(cli);
	// ...
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