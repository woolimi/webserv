#include "HTTP.hpp"

std::vector<t_server> HTTP::servers;
std::vector<t_client> HTTP::clients;

HTTP::HTTP()
{
}

// debug
int count = 0;


HTTP::HTTP(std::vector<t_server> &srvs)
{
	servers = srvs;

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
		init_timeout(timeout, SERVER_TIMEOUT_SEC, SERVER_TIMEOUT_USEC); // sec, usec
		http_select(fdmax, read_set, write_set, timeout);
		manage_clients(read_set, write_set, init_set, fds, env);
		manage_servers(read_set, init_set, fds);
	}
}


void HTTP::manage_clients(fd_set &read_set, fd_set &write_set, fd_set &init_set, std::set<int> &fds, char **env)
{
	std::vector<t_client>::iterator it;
	char buffer[MAX_BUFFER_SIZE + 1];
	ssize_t nb_read = 0;
	static int log_fd;

	for (it = clients.begin(); it != clients.end(); ++it)
	{
		// close client when timeout
		if (!it->req_arrived && check_client_timeout(*it))
		{
			printf("client timeout\n");
			disconnect(init_set, fds, it);
			continue;
		}

		// receive request
		if (!it->req_arrived)
		{			
			if (!it->req.raw.empty() || FD_ISSET(it->socket, &read_set))
			{	// something to read
				nb_read = read(it->socket, buffer, MAX_BUFFER_SIZE);
				if (nb_read == 0) {
					// printf("connection closed by client\n");
					disconnect(init_set, fds, it);
					continue;
				}
				// connection is closed from client side
				if (nb_read < 0)
				{
					if (it->req.raw.empty())
					{
						disconnect(init_set, fds, it);
						continue;
					}
					nb_read = 0;
				}
				//debug
				buffer[nb_read] = '\0';
				// DEBUG("read: ");
				// DEBUG(buffer);
				// static int x;
				std::string tmp(buffer, buffer + nb_read);
				if (count > 1 && tmp.size() != 0)
				{
					// std::cout << tmp.size() << "";
					// std::cout << "nb_read" << nb_read << std::endl;
					size_t n = 0;
					while ((n = tmp.find("\r\n", n)) != std::string::npos)
					{
						tmp.replace(n, 2, "RN");
						n += 2;
					}
					n = 0;
					while ((n = tmp.find("\n", n)) != std::string::npos)
					{
						tmp.replace(n, 1, "N");
						n += 1;
					}

					write(3, std::string("\n===sock : " + it->id + "=====\n").c_str(), 17 + it->id.size());
					if (nb_read > 100)
						write(3, tmp.c_str(), 100);
					else
						write(3, tmp.c_str(), nb_read);
				}
				// if (count == 5)
				// {
				// 	DEBUG(buffer);
				// }
				renew_client_timestamp(*it);
				skip_leading_empty_line(*it, buffer, nb_read);

			}

			if (it->req.raw.find("\r\n") == std::string::npos)
				continue;

			// request line parsing
			if (it->req.req_line_parsed != 2)
			{
				it->req.req_line_parsed = 1;
				parse_request_line(it->req.raw.substr(0, it->req.raw.find("\r\n")), *it);
				it->req.req_line_parsed = 2;
				if (!it->req.raw.empty())
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
					// std::cout << "basket\n";
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
				res_generator(*it);
		}

		// send res head
		if (it->req_arrived && !it->res.sent_head && FD_ISSET(it->socket, &write_set))
		{
			if (!send_res_head(*it)) {
				disconnect(init_set, fds, it);
				continue;
			}

			if (it->res.status_code == 413)
				count++;

			// debug
			// if (it->req.method == "POST")
			// 	count++;

			if (it->req.method == "PUT" ||
				(it->req.method == "POST" && it->res.status_code == 201)) {
				disconnect(init_set, fds, it);
				continue;
			}
			if (it->req.method == "HEAD" || it->res.content_length == 0)
			{
				reset_req_and_res(*it);
				renew_client_timestamp(*it);
				continue;
			}
		}

		// make res body from fd
		if (it->req_arrived && !it->res_sent
			&& it->res.fd != -1 && it->res.body.empty())
		{
			make_res_body_from_fd(*it);
			continue;
		}


		if (it->req_arrived && !it->res_sent && FD_ISSET(it->socket, &write_set))
		{
			if (!send_res_body(*it))
			{
				disconnect(init_set, fds, it);
				continue;
			}
			// if (it->res_sent)
			// 	printf("sent all response body\n");
			renew_client_timestamp(*it);
		}

		// treat more request or disconnect client
		if (it->res_sent)
		{
			reset_req_and_res(*it);
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
			if (clients.size() > MAX_CLIENT)
				continue;
			new_client.socket = accept(it->socket, (sockaddr *)&new_client.addr, &new_client.addr_len);

			if (count > 1)
			{
				/* debug */
				new_client.id = random_fname();
				write(3, "\nconnected client id : ", 22);
				write(3, new_client.id.c_str(), new_client.id.size());
				write(3, "\n", 1);
				/* debug */
			}

			if (new_client.socket < 0)
				throw FailToAccept();
			if (fcntl(new_client.socket, F_SETFL, O_NONBLOCK) < 0)
				throw FailToSetClientSocket();

			renew_client_timestamp(new_client);
			new_client.server = *it;
			clients.push_back(new_client);
			fds.insert(new_client.socket); //fdmax
			FD_SET(new_client.socket, &init_set);
			std::cout << "client " + new_client.id + " connected\n";
		}
	}
}

void HTTP::disconnect(fd_set &init_set, std::set<int> &fds, std::vector<t_client>::iterator &it)
{
	if (count > 1) {
		/* debug */
		write(3, "\ndisconnect client id : ", 24);
		write(3, it->id.c_str(), it->id.size());
		write(3, "\n", 1);
		/* debug */
	}
	std::cout << "client " + it->id + " disconnected\n";

	fds.erase(it->socket);
	FD_CLR(it->socket, &init_set);
	close(it->socket);
	if (!it->res.fname.empty())
		unlink(it->res.fname.c_str());
	if (!it->req.body_fname.empty())
	{
		if (it->req.is_cgi)
			unlink(it->req.body_fname.c_str());
	}
	if (it->res.fd != -1)
		close(it->res.fd);

	it = clients.erase(it);
	--it;
}

void HTTP::init_client(t_client &client)
{
	client.socket = 0;
	client.addr_len = sizeof(client.addr);
	/* req */
	client.req.req_line_parsed = 0;
	client.req.body = "";
	client.req.req_header_parsed = 0;
	client.req.req_body_parsed = 0;
	client.req.content_length = -1;
	client.addr_len = sizeof(client.addr);
	client.req_arrived = false;
	client.res_sent = false;
	client.res.status_code = 0;
	client.req.chunk_size_read = -1;
	client.req.is_cgi = false;
	/* res */
	client.res.status_code = 0;
	client.res.sent_head = false;
	client.res.content_length = -1;
	client.res.fd = -1;
	client.res.is_cgi = false;
}

void HTTP::init_timeout(struct timeval &timeout, int sec, int usec)
{
	timeout.tv_sec = sec;
	timeout.tv_usec = usec;
}

void HTTP::reset_req_and_res(t_client &cli)
{
	cli.req.req_line_parsed = 0;
	cli.req.req_header_parsed = 0;
	cli.req.req_body_parsed = 0;
	cli.req.body.clear();
	cli.req_arrived = false;
	cli.req.content_length = -1;
	cli.res_sent = false;
	cli.res.status_code = 0;
	cli.res.sent_head = false;
	cli.res.head.clear();
	cli.res.headers.clear();
	cli.res.body.clear();
	cli.req.headers.clear();
	cli.req.method.clear();
	cli.req.path.clear();
	cli.req.chunk_size_read = -1;
	close(cli.res.fd);
	cli.res.fd = -1;
	if (!cli.res.fname.empty())
	{
		unlink(cli.res.fname.c_str());
		cli.res.fname.clear();
	}
	if (!cli.req.body_fname.empty())
	{
		if (cli.req.is_cgi)
			unlink(cli.req.body_fname.c_str());
		cli.req.body_fname.clear();
	}
	cli.req.is_cgi = false;
	cli.res.is_cgi = false;
	// cli.req.raw.clear();
	// printf("reset\n");
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
	// if (ret == 0)
	// 	printf("waiting client\n");
}

void HTTP::handle_methods(t_client &cli, char **env)
{
	t_server &serv = cli.server;
	t_req &req = cli.req;
	t_location *loc = cli.req.loc;
	bool is_file = true;
	std::string folder_path;
	std::string file;

	// loc = find_matched_location(serv, req.path);
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

	auto it1 = std::find(loc->allow.begin(), loc->allow.end(), req.method);
	if (it1 == loc->allow.end())
	{
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
	else if (req.method == "POST")
		handle_post(cli, env, loc, is_file, folder_path, file);
	else if (req.method == "OPTIONS")
		handle_options(cli, env, loc, is_file, folder_path, file);
	else if (req.method == "TRACE")
		handle_trace(cli, env, loc, is_file, folder_path, file);
	else if (req.method == "DELETE")
		handle_delete(cli, env, loc, is_file, folder_path, file);

	// 	handle_head(cli);
	// ...
}

bool HTTP::check_client_timeout(t_client &cli)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if (tv.tv_sec - cli.time_stamp > CLIENT_TIMEOUT_SEC)
	{
		cli.res.status_code = 408; // client request timeout
		res_generator(cli);
		std::string str = cli.res.head + cli.res.body;
		send(cli.socket, str.c_str(), str.size(), MSG_NOSIGNAL);
		return true;
	}
	return false;
}

void HTTP::res_service_unavailable(t_client &cli)
{
	cli.res.status_code = 503; // Service Unavailable
	res_generator(cli);
	send(cli.socket, cli.res.head.c_str(), cli.res.head.size(), MSG_NOSIGNAL);
	cli.req_arrived = true;
	cli.res_sent = true;
	printf("max client exceed disconnect");
}

void HTTP::skip_leading_empty_line(t_client &cli, char *buffer, size_t nb_read)
{
	int i = 0;
	if (cli.req.req_line_parsed != 2 && !cli.req.req_header_parsed && !cli.req.req_body_parsed)
	{ 
		if (!buffer[i] && !cli.req.raw.empty())
		{
			while (is_newline_char(cli.req.raw[i]))
				i++;
			// cli.req.raw = &cli.req.raw[i];
			cli.req.raw.erase(0, i); // erase(0, i)
			return;
		}
		while (buffer[i] && is_newline_char(buffer[i])) //skip leading empty lines before the request
			i++;
		// cli.req.raw += std::string(&buffer[i]); // insert
		cli.req.raw.insert(cli.req.raw.end(), buffer + i, buffer + nb_read); // insert
		i = 0;
		while (is_newline_char(cli.req.raw[i]))
			i++;
		cli.req.raw.erase(0, i); // erase(0, i)
		// cli.req.raw = &cli.req.raw[i]; // erase(0, i)
		// std::cout << "RAW: [" << cli.req.raw << "]\nBUUUFFER: [" << buffer << "]\n";
	}
	else
		cli.req.raw.insert(cli.req.raw.end(), buffer + i, buffer + nb_read); // insert
		// cli.req.raw += std::string(buffer);
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
	std::string str = "HTTP : fail to accept" + std::string(strerror(errno)) + "\n";
	return str.c_str();
}

std::vector<t_client> &HTTP::get_clients()
{
	return this->clients;
}

std::vector<t_server> &HTTP::get_servers()
{
	return this->servers;
}