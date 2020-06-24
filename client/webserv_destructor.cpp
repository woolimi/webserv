#include <iostream>
#include <signal.h>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <map>
#include <set>
#include <dirent.h>
#include <vector>
#include <sys/stat.h>
#include <arpa/inet.h>

#define BUFF_SIZE 4096
#define NO_CONTENT "204"
#define CONTENT_CREATED "201"

typedef std::string attr;
typedef std::string value;

typedef struct s_clients
{
	std::string conf_file_data;
	std::map<attr, value> conf;
	std::set<std::string> fnames;
	std::string nginx_port;
	std::string webserv_port;
	std::string request;
	std::string request_original;
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

int	ft_strlen(char *str)
{
	int i;

	i = 0;
	while (str[i])
		i++;
	return (i);
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
	get_value("REQUEST_FILEPATH");
}

void set_request_paths(t_clients *cl)
{
	struct stat info;
	DIR *dp = NULL;
	char buff[256];
	struct dirent *entry = NULL;

	std::string orig_path = getcwd(buff, 256);
	if (chdir((std::string(orig_path) + cl->conf["REQUEST_FILEPATH"]).c_str()) < 0 || !(dp = opendir("./")))
	{
		perror("Error in Request Path");
		exit(1);
	}

	while((entry = readdir(dp)) != NULL)
	{
		stat(entry->d_name, &info);
		if (ft_strlen(entry->d_name) == 1 && entry->d_name[0] == '.')
			continue;
		if (!S_ISDIR(info.st_mode))
			cl->fnames.insert(std::string(entry->d_name));
	}
}

void req_response(t_clients *cl, std::string port, std::string server)
{
	char buff[4096];
	/*
		take filename
		read file
		send request
		receive response
		extract status code
		write res
		next
	*/
	std::set<std::string>::iterator it = cl->fnames.begin();
	int sock = 0;
	struct	sockaddr_in	serv_addr;
	int req_num = 1;

	while (it != cl->fnames.end())
	{
		int fd = open(it->c_str(), O_RDONLY);
		if (fd < 0)
		{
			std::cerr << "Cannot open " + *it << std::endl;
			continue;
		}
		int ret = read(fd, buff, BUFF_SIZE);
		if (ret < 0)
		{
			std::cerr << "Cannot read " + *it << std::endl;
			continue;
		}
		buff[ret] = '\0';
		// std::cout << "Request: " << buff << std::endl;
	
		//creating connection socket

		if ((sock = socket(AF_INET, SOCK_STREAM, 0))< 0)
		{
			std::cerr << "Cannot create socket " + *it << std::endl;
			exit(1);
		}

		memset(&serv_addr, '0', sizeof(serv_addr));
		
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(atoi(port.c_str()));


		//Convert IPv4 and IPv6 addresses from text to binary
		if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
		{
			perror("\n Error: Invalid Address/Address not supported\n");
			exit(1);
		}

		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			perror("\n Error: Connection Failed\n");
			exit(1);
		}
		send(sock, buff, ret, 0);
		// std::cout << *it << " request sent\n";

		ret = read(sock, buff, BUFF_SIZE);
		buff[ret] = '\0';
		// std::cout << "Response: " << buff << std::endl;
		std::string response(buff);
		size_t pos = response.find(" ");
		std::string status_code = response.substr(pos + 1, 3);
		// if (status_code == NO_CONTENT)
		// 	status_code = CONTENT_CREATED;

		// std::cout << "status: " << status_code << std::endl;
		std::string res_content;
		res_content = "#" + std::to_string(req_num) + " " + status_code + "\n";
		int res_fd = open(("../" + server + ".res").c_str(), O_CREAT | O_WRONLY |O_APPEND, 0777);
		if (res_fd < 0)
		{
			perror("Can't open response file\n");
			exit(1);
		}
		write(res_fd, res_content.c_str(), res_content.size());
		close(sock);
		close(fd);
		it++;
		req_num++;
	}
}

int main()
{
	t_clients *cl = get_clients();
	parse_conf();

	/*
		get filenames
		request_response for NGINX
		request_response for WEBSERV
		generate result using diff.
	*/

 	set_request_paths(cl);
	req_response(cl, cl->conf["NGINX_PORT"], "NGINX");
	req_response(cl, cl->conf["WEBSERV_PORT"], "WEBSERV");
	// int i;
	// i = 0;
	// std::set<std::string>::iterator it = cl->fnames.begin();
	// while (it != cl->fnames.end())
	// {
	// 	std::cout <<  "\n" + *it++;
	// }
}