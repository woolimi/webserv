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

static void set_status_code_and_throw(int code, t_client &cli)
{
	cli.res.status_code = code;
	throw cli;
}

static std::string make_real_path(std::string &root, std::string &path)
{
	std::string real_path = root + path;
	size_t pos = real_path.find("//");
	if (pos != std::string::npos)
		real_path.replace(pos, 2, "/");
	return (real_path);
}

static bool file_check(int fd, struct stat &info, t_client & cli)
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

static char **cgi_env(t_client &cli, char **env, std::string &real_path)
{
	t_req &req = cli.req;
	t_server &serv = cli.server;
	std::map<std::string, std::string> new_env;
	char **ret;

	new_env["AUTH_TYPE"] = "";
	new_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	new_env["PATH_INFO"] = real_path;
	new_env["PATH_TRANSLATED"] = "";
	new_env["QUERY_STRING"] = "";
	new_env["REMOTE_ADDR"] = inet_ntoa(cli.addr.sin_addr); // ??
	new_env["REMOTE_IDENT"] = "";
	new_env["REMOTE_USER"] = "";
	new_env["REQUEST_METHOD"] = req.method;
	new_env["REQUEST_URI"] = req.path;
	new_env["SCRIPT_NAME"] = "";
	new_env["SCRIPT_FILENAME"] = real_path;
	new_env["SERVER_NAME"] = serv.server_name;
	new_env["SERVER_PORT"] = std::to_string(serv.listen);
	new_env["SERVER_PROTOCOL"] = req.version;
	new_env["SERVER_SOFTWARE"] = SERVER_NAME;
	if (req.method == "POST")
		new_env["CONTENT_LENGTH"] = req.body.size();
	int nb = 0;
	while (env[nb] != 0)
		nb++;
	nb += new_env.size();
	ret = new char*[nb + 1];
	int i = 0;
	while (env[i] != 0)
	{
		ret[i] = ft_strdup(env[i]);
		i++;
	}
	std::map<std::string, std::string>::iterator it;
	for (it = new_env.begin(); it != new_env.end(); ++it)
	{
		ret[i++] = ft_strdup((it->first + "=" + it->second).c_str());
	}
	ret[i] = 0;
	return (ret);
}

static void execute_cgi(t_client &cli, t_location &loc, char **env, std::string &real_path, std::string &ext)
{
	// 1 write, 0 read
	int p2c_fd[2]; 
	int c2p_fd[2];
	int pid;
	int status;

	if (pipe(p2c_fd) < 0 || pipe(c2p_fd) < 0)
		set_status_code_and_throw(404, cli);
	pid = fork();
	if (pid < 0)
		set_status_code_and_throw(404, cli);

	if (pid == 0) // child
	{
		dup2(p2c_fd[0], 0);
		close(p2c_fd[1]);
		dup2(c2p_fd[1], 1);
		close(c2p_fd[0]);

		const char *av[2];
		av[0] = loc.cgi["path"].c_str();
		av[1] = 0;
		errno = 0;
		char **new_env = cgi_env(cli, env, real_path);
		if (execve(av[0], (char**)av, new_env) < 0)
			std::cout << strerror(errno) << std::endl;
		exit(1);
	}
	else // parent
	{
		close(c2p_fd[1]);
		close(p2c_fd[0]);
		if (!cli.req.body.empty())
		{	// send POST data
			if (write(p2c_fd[1], cli.req.body.c_str(), cli.req.body.size()) <= 0)
			{
				close(p2c_fd[1]);
				kill(pid, SIGKILL);
				set_status_code_and_throw(404, cli);
			}
			cli.req.body.clear();
		}
		cli.res.fd = c2p_fd[0];
		waitpid(pid, &status, 0);
		if (!WIFEXITED(status))
			set_status_code_and_throw(404, cli);
		close(p2c_fd[1]);
	}
}

void make_file_res(t_client &cli, t_location *loc, char **env, std::string &real_path, std::string &file)
{
	t_res &res = cli.res;
	struct stat info;
	int fd = open(real_path.c_str(), O_RDONLY);
	errno = 0;
	fstat(fd, &info);
	if (!file_check(fd, info, cli))
		throw cli;
	size_t file_size = info.st_size;
	std::string ext = file.substr(file.find_last_of('.'));

	// make res.head
	if (!loc->cgi.empty() && loc->cgi["extension"] == ext)
	{
		char buff[MAX_BUFFER_SIZE + 1];
		execute_cgi(cli, *loc, env, real_path, ext);
		int ret = read(res.fd, buff, MAX_BUFFER_SIZE);
		buff[MAX_BUFFER_SIZE] = 0;
		std::string raw = buff;
		std::string tmp;
		size_t pos;
		while ((pos = raw.find("\r\n")) != std::string::npos)
		{
			tmp = raw.substr(0, pos);
			raw.erase(0, pos + 2);
			if (tmp == "")
				break;
			pos = tmp.find(": ");
			res.headers[tmp.substr(0, pos)] = tmp.substr(pos + 2);
		}
		res.headers["Transfer-Encoding"] = "chunked";
		res.body += int_to_hexstr(raw.size()) + "\r\n";
		res.body += raw + "\r\n";
	}
	else
	{
		res.headers["Transfer-Encoding"] = "chunked";
		res.headers["Content-Type"] = mimetype(ext);
		res.fd = fd;
	}
	res.status_code = 200;

	throw cli;
}

void make_folder_list_res(t_client &cli, t_location *loc, std::string &uri_path, std::string &real_path)
{
	char buff[256];
	t_res &res = cli.res;
	DIR *dp = NULL;
	struct dirent *entry = NULL;
	std::set<std::string> fnames;
	struct stat info;
	std::string orig_path = getcwd(buff, 256);

	if (chdir(real_path.c_str()))
		set_status_code_and_throw(404, cli);
	if (!(dp = opendir("./")))
		set_status_code_and_throw(404, cli);
	res.body += "<html>\n";
	res.body += "\t<head>\n";
	res.body += "\t\t<title>Index of " + uri_path + "</title>\n";
	res.body += "\t</head>\n";
	res.body += "\t<body bgcolor=\"white\">\n";
	res.body += "\t\t<h1>Index of " + uri_path + "</h1>\n";
	res.body += "\t<hr>\n\t<pre>\n";
	
	while ((entry = readdir(dp)) != NULL)
	{
		stat(entry->d_name, &info);
		if (ft_strlen(entry->d_name) == 1 && entry->d_name[0] == '.')
			continue;
		if (S_ISDIR(info.st_mode))
			fnames.insert(std::string(entry->d_name) + "/");
		else
			fnames.insert(std::string(entry->d_name));
	}

	std::set<std::string>::iterator it;
	for (it = fnames.begin(); it != fnames.end(); ++it)
	{
		stat(it->c_str(), &info);
		time_t t = info.st_mtime;
		struct tm *lctime = localtime(&t);
		strftime(buff, sizeof(buff), "%d-%h-%Y %H:%M", lctime);
		res.body += "<a href=\"" + *it + "\">" + *it + "</a>";
		int nb_tab = (8 - it->size() / 7);
		while (nb_tab-- > 0)
			res.body += "\t";
		res.body += buff;
		res.body += "\t\t";
		res.body += std::to_string(info.st_size);
		res.body += "\n";
	}
	closedir(dp);
	res.status_code = 200;
	res.body += "\t</pre>\n\t<hr>\n\t</body>\n</html>";
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = std::to_string(res.body.size());
	throw cli;
}

// ex) req.path = "/test/a/index.html"
// folder_path = "/test/a"
// file = "/index.html"
void handle_get(t_client &cli, char **env)
{
	t_server &serv = cli.server;
	t_req &req = cli.req;
	t_location *loc;
	bool is_file = true;
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
			make_file_res(cli, loc, env, real_path, file);
		} else
		{
			std::vector<std::string>::iterator it;
			struct stat info;
			std::string file_path;
			// folder with index index.php index.html
			for (it = loc->index.begin(); it != loc->index.end(); ++it)
			{
				file_path.clear();
				file_path = real_path + *it;
				int ret = stat(file_path.c_str(), &info);
				if (ret == 0)
					make_file_res(cli, loc, env, file_path, *it);
			}
			// folder listing
			if (loc->autoindex == "on")
			{
				if (folder_path == "")
					folder_path = "/";
				make_folder_list_res(cli, loc, folder_path, real_path);
			}
			else
				set_status_code_and_throw(404, cli);
		}
	}
	catch (t_client &client)
	{
		res_generator(client);
	}
}
