#include "webserv.hpp"

void res_generator(t_req &req, t_res &res, t_server &server)
{
	// generate res depends on req and server
	(void)req;
	// res.version = "HTTP/1.1";
	// res.status_code = "200";
	// res.status_msg = "OK";	
	// res.content_type = "text/html";
	// res.server = "webserv";
	// int fd = open("/home/user42/Desktop/mashar/projects/webserv/www/default.html", O_RDONLY);
	// char buff[1001];
	// int ret = read(fd, buff, 1001);
	// close(fd);
	// buff[ret] = '\0';
	// res.body += std::string(buff);

	// res.raw = "";
	// res.raw += res.version + " " + res.status_code + " " + res.status_msg + "\n";
	// res.raw += "Content-Type : " + res.content_type + "\n";
	// res.raw += "Server: " + res.server + "\n";
	// res.raw += "\n";
	// res.raw += res.body;
}