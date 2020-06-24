#include "webserv.hpp"

bool handle_post(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file)
{
	int fd;
	t_req &req = cli.req;
	t_res &res = cli.res;
	bool &is_cgi = cli.req.is_cgi;
	std::string fname = loc->abs_path.substr(loc->abs_path.find_last_of("/"));
	// check cgi or not
	std::string ext = "";
	if (fname.find(".") != std::string::npos)
		ext = file.substr(file.find_last_of('.'));

	if (!loc->cgi.empty() && loc->cgi["extension"] == ext)
		is_cgi = true;

	if (req.body_fname.empty())
	{
		if (is_cgi)
		{
			req.body_fname = "./obj/" + random_fname();
			req.body_fd = open(req.body_fname.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777);
		}
		else
		{
			req.body_fname = loc->abs_path + req.path;
			req.body_fd = open(req.body_fname.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0777);
		}

		if (req.body_fd < 0)
		{
			res.status_code = 404;
			cli.res_sent = true;
			return true;
		}

		// req.body -> req.body_fname
		int ret = write(req.body_fd, req.body.c_str(), req.body.size());
		close(req.body_fd);
		if (ret < req.body.size())
		{
			set_http_status(cli, 503); // service unavailable
			return true;
		}
		return false;
	}

	// POST with cgi
	if (is_cgi)
	{
		if (!execute_cgi(cli, *loc, env, loc->abs_path, ext))
			return true;
		res.is_cgi = true;

		char buff[MAX_BUFFER_SIZE + 1];
		int ret = read(res.fd, buff, MAX_BUFFER_SIZE);
		if (ret < 0) {
			set_http_status(cli, 503);
			cli.res_sent = true;
			return true;
		}
		buff[ret] = 0;
		std::string raw;
		raw.insert(raw.begin(), buff, buff + ret);

		// inherit header + more header
		size_t pos;
		std::string tmp;
		while ((pos = raw.find("\r\n")) != std::string::npos)
		{
			tmp = raw.substr(0, pos);
			raw.erase(0, pos + 2);
			if (tmp == "")
				break;
			pos = tmp.find(": ");
			std::string attr = tmp.substr(0, pos);
			if (attr == "Status")
				res.status_code = ft_atoi(tmp.substr(pos + 2).c_str());
			else
				res.headers[attr] = tmp.substr(pos + 2);
		}
		res.headers["Transfer-Encoding"] = "chunked";
		res.headers["Content-Location"] = req.path;
		res.body += int_to_hexstr(raw.size()) + "\r\n";
		res.body += raw + "\r\n";
	}
	else // POST without cgi
	{
		std::string filename = req.path.substr(req.path.find_last_of("/") + 1);
		if (filename.empty())
		{
			set_http_status(cli, 200);
			cli.res_sent = true;
			return true;
		}
		set_http_status(cli, 201);
		res.headers["Content-Location"] = req.path;
		cli.res_sent = true;
	}
	return true;
}
