#include "webserv.hpp"
#include "HttpStatus.hpp"

static void make_res_line(t_res &res)
{
	res.raw += "HTTP/1.1 " + std::to_string(res.status_code) + " " + HttpStatus::reasonPhrase(res.status_code);
}

static void add_common_res_header(t_res &res)
{
	timeval tv;
	struct tm *timeinfo;
	char buffer[80];

	gettimeofday(&tv, NULL);
	timeinfo = gmtime(&tv.tv_sec);
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);

	res.headers["Server"] = SERVER_NAME;
	res.headers["Date"] = buffer;
}

static void make_res_header(t_res &res)
{
	std::map<std::string, std::string>::iterator it = res.headers.begin();
	for (; it != res.headers.end(); ++it)
	{
		res.raw += it->first;
		res.raw += ": ";
		res.raw += it->second;
		res.raw += "\r\n";
	}
}

static void add_dynamic_error_page(t_res &res)
{
	res.body += "<html><head><title>" + std::to_string(res.status_code) + HttpStatus::reasonPhrase(res.status_code) + "</title></head><body bgcolor = \"white\"><center><h1>" + std::to_string(res.status_code) + HttpStatus::reasonPhrase(res.status_code) + "</h1></center><hr><center> webserv/1.1</center></body></html>";
}

static void make_res_body(t_res &res)
{
	res.raw += res.body;
}

static void make_default_error_page(t_res &res)
{
	// prepare
	add_dynamic_error_page(res);
	add_common_res_header(res);
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = res.body.size();
	// make res.raw
	make_res_line(res);
	make_res_header(res);
	res.raw += "\r\n";
	make_res_body(res);
}

static void make_custom_error_page(t_client &cli, t_res &res)
{
	int fd = open(cli.server.error_page.c_str(), O_RDONLY);
	if (fd < 0)
	{
		make_default_error_page(res);
		return;
	}
	struct stat info;
	fstat(fd, &info);
	char buff[info.st_size + 1];
	int ret = read(fd, buff, info.st_size);
	if (ret < 0)
	{
		close(fd);
		make_default_error_page(res);
		return;
	}
	buff[ret] = 0;
	close(fd);
	// prepare
	add_common_res_header(res);
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = res.body.size();
	res.body = buff;
	// make res.raw
	make_res_line(res);
	make_res_header(res);
	res.raw += "\r\n";
	make_res_body(res);
}

void res_generator(t_client &cli)
{
	t_req &req = cli.req;
	t_res &res = cli.res;
	t_server &serv = cli.server;

	if (!HttpStatus::isSuccessful(res.status_code))
	{
		if (cli.server.error_page.empty())
			make_default_error_page(res);
		else
			make_custom_error_page(cli, res);
	}
	else // success
	{
	}
}
