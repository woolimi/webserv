#include "webserv.hpp"
// 1. check req.route
// req.route is folder ?
// req.route is file ?
// 
// case1 folder
// check index attribute, if it exist, need to show file /index.html
// check autoindex on
// make page 
//
// case2 file
// check server set cgi or not
// if no cgi
// make res.body
// if cgi
// communicate with cgi

static t_location *find_matched_location(t_server &serv, std::string &folder_path, std::string &file)
{
	t_location *ret;
	size_t max_matched_size = 0;
	size_t pos;

	std::map<route, t_location>::iterator it;

	if (folder_path.empty() && file == "/")
		return &serv.location["/"];
	// folder request
	if (file == "/")
	{
		// 100% match
		std::string fdpath = folder_path + "/";
		for (it = serv.location.begin(); it != serv.location.end(); ++it)
		{
			if (it->first == fdpath)
				return &it->second;
		}
		// part match
		for (it = serv.location.begin(); it != serv.location.end(); ++it)
		{
			if ((pos = fdpath.find(it->first)) != std::string::npos && pos == 0 && it->first.size() > max_matched_size)
			{
				ret = &it->second;
				max_matched_size = fdpath.size();
			}
		}
		return ret;
	}
	else // file request
	{
		// 100% match
		std::string fdpath = folder_path + file;
		for (it = serv.location.begin(); it != serv.location.end(); ++it)
		{
			if (it->first == fdpath)
				return &it->second;
		}
		return &serv.location["/"];
	}
}

void read_file(std::string &real_file_path)
{
	// int fd = open();
}

void set_status_code_and_throw(int code, t_client &cli)
{
	cli.res.status_code = code;
	throw cli;
}

void handle_get(t_client &cli)
{
	t_server &serv = cli.server;
	t_req &req = cli.req;
	t_res &res = cli.res;
	t_location *loc;
	struct stat info;

	// location 은 항상 /test/ 폴더로 가정....
	// ex) req.path = "/test/a/index.html"
	// folder_path = "/test/a"
	// file = "/index.html"

	std::string folder_path = req.path.substr(0, req.path.find_last_of('/'));
	std::string file = req.path.substr(req.path.find_last_of('/'));
	// find maximun mached location route
	loc = find_matched_location(serv, folder_path, file);

	std::string real_path = (loc->root + req.path);
	size_t pos = real_path.find("//");
	if (pos != std::string::npos)
		real_path.replace(pos, 2, "/");

	if (file == "/") // folder request
	{
		if (loc->index.empty())
		{
			// autoindex on ? off ?

		} else // 파일 요청과 동등...
		{
			// check index file is exist
			// if it is not,  check autoindex
			// if it is not, 404
		}
	} else // file request
	{
		int fd = open(real_path.c_str(), O_RDONLY);
		if (fd < 0)
			set_status_code_and_throw(404, cli);
		errno = 0;
		fstat(fd, &info);
		if (errno == EACCES || !S_ISREG(info.st_mode))
			set_status_code_and_throw(403, cli);
		char buff[MAX_BUFFER_SIZE + 1];
		int nb_read;
		while ((nb_read = read(fd, buff, MAX_BUFFER_SIZE)) > 0)
		{
			buff[nb_read];
			res.body += std::string(buff);
		}
		if (nb_read < 0)
			set_status_code_and_throw(403, cli);
		close(fd);
		res.headers["Content-Type"] = mimetype(file.substr(file.find_last_of('.')));
			
		set_status_code_and_throw(200, cli);
	}
}