#include "webserv.hpp"

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

std::string make_real_path(std::string &root, std::string &path)
{
	std::string real_path = root + path;
	size_t pos = real_path.find("//");
	if (pos != std::string::npos)
		real_path.replace(pos, 2, "/");
	return (real_path);
}

bool file_check(int fd, struct stat &info, t_client & cli)
{
	if (fd < 0)
	{
		cli.res.status_code = 404;
		return false;
	}
	if (errno == EACCES || !S_ISREG(info.st_mode))
	{
		cli.res.status_code = 403;
		return false;
	}
	return true;
}

void handle_get(t_client &cli)
{
	t_server &serv = cli.server;
	t_req &req = cli.req;
	t_res &res = cli.res;
	t_location *loc;
	struct stat info;
	bool is_file = true;
	char buff[MAX_BUFFER_SIZE + 1];
	// ex) req.path = "/test/a/index.html"
	// folder_path = "/test/a"
	// file = "/index.html"
	try
	{
		std::string folder_path = req.path.substr(0, req.path.find_last_of('/'));
		std::string file = req.path.substr(req.path.find_last_of('/'));
		if (file == "/")
			is_file = false;
		loc = find_matched_location(serv, folder_path, file);
		std::string real_path = make_real_path(loc->root, req.path);

		if (is_file)
		{
			int fd = open(real_path.c_str(), O_RDONLY);
			errno = 0;
			fstat(fd, &info);
			if (!file_check(fd, info, cli))
				throw cli;
			size_t file_size = info.st_size;
			std::string ext = file.substr(file.find_last_of('.'));
			if (!loc->cgi.empty() && loc->cgi.find(ext) != loc->cgi.end())
			{
				// transfer-encoding
			}
			else
			{
				if (file_size < MAX_BUFFER_SIZE)
				{
					int ret = read(fd, buff, MAX_BUFFER_SIZE);
					if (ret < 0)
					{
						close(fd);
						set_status_code_and_throw(404, cli);
					}
					buff[ret] = 0;
					res.status_code = 200;
					res.body += std::string(buff);
					res.headers["Content-Length"] = res.body.size();
					res.headers["Content-Type"] = mimetype(ext);
				}
			}
		} else
		{
			// folder request
		}
	}
	catch (t_client &client)
	{
		res_generator(client); // with conent-length
	}
}