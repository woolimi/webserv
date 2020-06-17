#include "webserv.hpp"

void handle_post(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file)
{
	int fd;
	t_req &req = cli.req;
	t_res &res = cli.res;

    // std::string real_path = loc->upload_folder + loc->abs_path.substr(loc->abs_path.find_last_of('/'));
	// std::cout << "Real Path is : " << real_path <<std::endl;
	// make_file_res(cli, loc, env, real_path, file);
	
	// save body message into file
	req.body_fpath = random_fname();
	req.body_fd = open(req.body_fpath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777);
	if (req.body_fd < 0)
	{
		res.status_code = 404;
		cli.res_sent = true;
		return;
	}
	int ret = write(req.body_fd, req.body.c_str(), req.body.size());
	DEBUG("charactor write");
	DEBUG(ret);
	close(req.body_fd);
	// check file
	std::string fname = loc->abs_path.substr(loc->abs_path.find_last_of("/"));
	std::string ext = "";
	if (fname.find(".") != std::string::npos)
		ext = file.substr(file.find_last_of('.'));


	if (!loc->cgi.empty() && loc->cgi["extension"] == ext)
	{
		if (!execute_cgi(cli, *loc, env, loc->abs_path, ext))
			return;
		res.is_cgi = true;
		DEBUG(res.fname);

		char buff[MAX_BUFFER_SIZE + 1];
		int ret = read(res.fd, buff, MAX_BUFFER_SIZE);
		buff[ret] = 0;
		std::string raw = buff;

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
				continue;
			else
				res.headers[attr] = tmp.substr(pos + 2);
		}
		res.headers["Transfer-Encoding"] = "chunked";
		res.body += int_to_hexstr(raw.size()) + "\r\n";
		res.body += raw + "\r\n";
	}
	cli.res.status_code = 200;
}
