#include <iostream>
#include <signal.h>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstring>
#include <map>
#include <set>
#include <vector>

#define BUFF_SIZE 4096

typedef std::string attr;
typedef std::string value;
typedef struct s_clients
{
	std::string conf_file_data;
	std::map<attr, value> conf;
	std::string nginx_port;
	std::string webserv_port;
	std::string request;
	std::string respond;
	int client_socket;
	s_clients()
		: client_socket(-1) {};
} t_clients;

t_clients *get_clients(void)
{
	static t_clients cl;
	return (&cl);
}

void print_error(const char *message)
{
	perror(message);
	exit(1);
}

void remove_comment()
{
	t_clients *cl = get_clients();
	std::string::size_type pos = 0;
	std::string::size_type pos2;
	while ((pos = cl->conf_file_data.find("#", pos)) != std::string::npos)
	{
		pos2 = cl->conf_file_data.find("\n", pos);
		cl->conf_file_data.replace(pos, pos2 - pos, "\n");
		pos += 1;
	}
}

void get_value(const char *attr_name)
{
	t_clients *cl = get_clients();
	std::string::size_type pos = 0;
	std::string::size_type pos2 = 0;
	if ((pos = cl->conf_file_data.find(attr_name, pos)) != std::string::npos)
	{
		pos = cl->conf_file_data.find("=", pos);
		if (pos != std::string::npos)
			pos++;
		pos2 = cl->conf_file_data.find("\n", pos);
		cl->conf[attr_name] = cl->conf_file_data.substr(pos, pos2 - pos);
	}
}

void parse_conf()
{
	t_clients *cl = get_clients();
	int conf_fd = open("./setup.conf", O_RDONLY);
	if (conf_fd < 0)
		print_error("setup.conf file does not exist");
	char buff[BUFF_SIZE + 1];
	int res;
	while ((res = read(conf_fd, buff, BUFF_SIZE)) > 0)
	{
		buff[res] = 0;
		cl->conf_file_data += std::string(buff);
	}
	if (res < 0)
		print_error("fail to read setup.conf");
	close(conf_fd);
	remove_comment();
	get_value("NGINX_PORT");
	get_value("WEBSERV_PORT");
	get_value("REQUEST_FILE");
}

void ready_to_request()
{
	t_clients *cl = get_clients();

	int fd = open(cl->conf["REQUEST_FILE"].c_str(), O_RDONLY);
	if (fd < 0)
		print_error(("request file '" + cl->conf["REQUEST_FILE"]  + "' does not exist").c_str());
	char buff[BUFF_SIZE + 1];
	int res;
	while ((res = read(fd, buff, BUFF_SIZE)) > 0)
	{
		buff[res] = 0;
		cl->request += std::string(buff);
	}
	if (res < 0)
		print_error(("fail to read request file '" + cl->conf["REQUEST_FILE"] + "'").c_str());
	close(fd);
}

void create_res_file(const std::string &server_name, const std::string &res)
{
	int fd = open((server_name + ".res").c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777);
	if (fd < 0)
		print_error(("fail to create " + server_name + ".res").c_str());
	write(fd, res.c_str(), res.size());
	close(fd);
}

void send_request_and_receive_respond(const std::string &server_name, const std::string &port_str)
{
	fd_set read_set, init_set;

	t_clients *cl = get_clients();
	cl->client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// socket reuse setting
	int reuse = 1;
	setsockopt(cl->client_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	// set server address
	struct sockaddr_in server_addr;
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int port = std::stoi(port_str);
	server_addr.sin_port = htons(port);
	// connect
	if (connect(cl->client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		print_error((server_name + " is not runing").c_str());

	// send request
	int send_request = write(cl->client_socket, cl->request.c_str(), cl->request.size());
	if (send_request < 0)
		print_error(("fail to send request to " + server_name).c_str());

	FD_ZERO(&read_set);
	FD_ZERO(&init_set);
	FD_SET(cl->client_socket, &init_set);
	std::string res;
	struct timeval timeout;

	if (fcntl(cl->client_socket, F_SETFL, O_NONBLOCK) < 0)
		perror("fail to make non block socket");
	while (1)
	{
		read_set = init_set;
		int ret;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		if ((ret = select(cl->client_socket + 1, &read_set, NULL, NULL, &timeout)) < 0)
		{
			perror("select error");
			exit(1);
		}
		if (!FD_ISSET(cl->client_socket, &read_set))
		{	// nothing to read on socket, wait 1sec...
			std::cout << "client read all response" << std::endl;
			close(cl->client_socket);
			create_res_file(server_name, res);
			return;
		}
		if (FD_ISSET(cl->client_socket, &read_set))
		{
			char buff[BUFF_SIZE + 1];
			ret = read(cl->client_socket, buff, BUFF_SIZE);
			if (ret < 0)
			{
				perror("read response");
				return;
			}
			buff[ret] = 0;
			res += buff;
		}
	}
}

int main(int ac, char **av)
{
	t_clients *cl = get_clients();
	parse_conf();
	ready_to_request();

	std::map<attr, value>::iterator it;
	if ((it = cl->conf.find("NGINX_PORT")) != cl->conf.end() && !it->second.empty())
	{
		send_request_and_receive_respond("NGINX", it->second);
	}

	if ((it = cl->conf.find("WEBSERV_PORT")) != cl->conf.end() && !it->second.empty())
	{
		send_request_and_receive_respond("WEBSERV", it->second);
	}
	return 0;
}