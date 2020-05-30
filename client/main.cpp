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

void send_request_and_receive_respond(const std::string &server_name, const std::string &port_str)
{
	int res;
	t_clients *cl = get_clients();
	cl->client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int reuse = 1;
	setsockopt(cl->client_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	struct sockaddr_in server_addr;
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int port = std::stoi(port_str);
	server_addr.sin_port = htons(port);

	if (connect(cl->client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		print_error((server_name + " is not runing").c_str());
	// send request
	res = write(cl->client_socket, cl->request.c_str(), cl->request.size());
	if (res < 0)
		print_error(("fail to send request to " + server_name).c_str());

	// receive respond
	char buff[BUFF_SIZE + 1];
	res = read(cl->client_socket, buff, BUFF_SIZE);
	if (res < 0)
		print_error(("fail to receive respond from " + server_name).c_str());
	buff[res] = 0;
	cl->respond.clear();
	cl->respond += std::string(buff);

	// close connection with server
	close(cl->client_socket);

	// make server_name.res file
	int fd = open((server_name + ".res").c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777);
	if (fd < 0)
		print_error(("fail to create " + server_name + ".res").c_str());
	res = write(fd, cl->respond.c_str(), cl->respond.size());
	if (res < 0)
		print_error(("fail to write on " + server_name + ".res").c_str());
	close(fd);
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

	// if ((it = cl->conf.find("WEBSERV_PORT")) != cl->conf.end() && !it->second.empty())
	// {
	// 	send_request_and_receive_respond("WEBSERV", it->second);
	// }
	return 0;
}
