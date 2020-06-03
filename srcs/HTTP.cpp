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
	ssize_t nb_read;

	for (it = clients.begin(); it != clients.end(); ++it)
	{
		// receive request
		if (!it->req_arrived && FD_ISSET(it->socket, &read_set))
		{			
			nb_read = read(it->socket, buffer, MAX_BUFFER_SIZE);
			if (nb_read < 0) // When client close connection
			{ // remove clinet
				disconnect(init_set, fds, it);
				continue;
			}
			buffer[nb_read] = '\0';
			it->req.raw = std::string(buffer);
			if (nb_read < MAX_BUFFER_SIZE)
			{
				printf("client sent request\n");
				try
				{
					req_interpreter(*it);
					if (it->res.status_code == 123456 && (it->res.status_code = 0))
						it->req_arrived = false;
					if (it->req_arrived == true)
					{
						handle_methods(*it);
					}
				}
				catch(t_client& client)
				{
					res_generator(client);
				}
			}
			continue;
		}
		// send response
		if (it->req_arrived && !it->res_sent && FD_ISSET(it->socket, &write_set))
		{
			// chunk
			std::cout <<"hek";
			write(it->socket, it->res.raw.c_str(), it->res.raw.size());
			it->res_sent = true;
			if (it->req.req_body_parsed)
				std::cout << "Body: " << it->req.body << std::endl;
			printf("server responded\n");
			continue;
		}
		// close client after sent all request
		if (it->req_arrived && it->res_sent && FD_ISSET(it->socket, &write_set))
		{
			disconnect(init_set, fds, it);
			continue;
		}
		// if no request from client after connection ?
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
	client.res.status_code = 0;
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