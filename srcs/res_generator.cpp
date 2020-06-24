#include "webserv.hpp"

static void make_res_line(t_res &res)
{
	res.head += "HTTP/1.1 " + std::to_string(res.status_code) + " " + HttpStatus::reasonPhrase(res.status_code);
	res.head += "\r\n";
}

static void add_common_res_header(t_res &res)
{
	timeval tv;

	gettimeofday(&tv, NULL);

	res.headers["Server"] = SERVER_NAME;
	res.headers["Date"] = gmt_time_string(tv.tv_sec);
}

static void make_res_header(t_res &res)
{
	std::map<std::string, std::string>::iterator it = res.headers.begin();
	for (; it != res.headers.end(); ++it)
	{
		res.head += it->first;
		res.head += ": ";
		res.head += it->second;
		res.head += "\r\n";
	}
}

static void make_default_error_body(t_res &res)
{
	res.body.clear();
	res.body += "<html>\n";
	res.body += "\t<head>\n";
	res.body += "\t\t<title>" + std::to_string(res.status_code) + " " + HttpStatus::reasonPhrase(res.status_code) + "</title>\n";
	res.body += "\t</head>\n";
	res.body += "\t<body bgcolor = \"white\">\n";
	res.body += "\t\t<center>\n";
	res.body += "\t\t\t<h1>" + std::to_string(res.status_code) + " " + HttpStatus::reasonPhrase(res.status_code) + "</h1>\n";
	res.body += "\t\t</center>\n";
	res.body += "\t\t<hr>\n";
	res.body += "\t\t<center> webserv/1.1</center>\n";
	res.body += "\t</body>\n";
	res.body += "</html>\n";
}

static void make_default_error_page(t_res &res)
{
	// prepare
	add_common_res_header(res);
	make_default_error_body(res);
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = std::to_string(res.body.size());
	if (res.status_code == 408)
		res.headers["Connection"] = "close";
	res.content_length = res.body.size();
	// make res.raw
	make_res_line(res);
	make_res_header(res);
	res.head += "\r\n";
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
	res.body = buff;
	add_common_res_header(res);
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = std::to_string(res.body.size());
	if (res.status_code == 408)
		res.headers["Connection"] = "close";
	res.content_length = res.body.size();
	// make res.head / body
	make_res_line(res);
	make_res_header(res);
	res.head += "\r\n";
}

void res_generator(t_client &cli)
{
	t_res &res = cli.res;

	if (!res.is_cgi && !HttpStatus::isSuccessful(res.status_code))
	{
		if (cli.server.error_page.empty())
			make_default_error_page(res);
		else
			make_custom_error_page(cli, res);
	}
	else // success
	{
		add_common_res_header(res);
		make_res_line(res);
		if(cli.req. method == "PUT" || cli.req. method == "POST")
		{
			res.headers["Location"] = cli.req.path;
		}
		make_res_header(res);
		res.head += "\r\n";
	}
}
