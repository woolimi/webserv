#include "webserv.hpp"

// debug
extern int count;

static void httphHeader_to_cgiHeader(std::string &attr)
{
	attr.insert(0, "HTTP_");
	for (size_t i = 0; i != attr.size(); i++)
	{
		if (attr[i] == '-')
			attr[i] = '_';
		else
			attr[i] = ft_toupper(attr[i]);
	}
}

static char **cgi_env(t_client &cli, char **env, std::string &real_path)
{
	t_req &req = cli.req;
	t_server &serv = cli.server;
	std::map<std::string, std::string> new_env;
	char **ret;

	new_env["AUTH_TYPE"] = "";
	new_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	new_env["PATH_INFO"] = cli.req.path;
	new_env["PATH_TRANSLATED"] = "";
	new_env["QUERY_STRING"] = cli.req.query_string;
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
		new_env["CONTENT_LENGTH"] = std::to_string(req.body.size());
	for (auto it = req.headers.begin(); it != req.headers.end(); ++it)
	{
		std::string attr = it->first;
		httphHeader_to_cgiHeader(attr);
		new_env[attr] = it->second;
	}

	int nb = 0;
	while (env[nb] != 0)
		nb++;
	nb += new_env.size();
	ret = new char *[nb + 1];
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

bool execute_cgi(t_client &cli, t_location &loc, char **env, std::string &realpath, std::string &ext)
{
	(void)ext;

	int pid;
	int status;
	std::string ranfname = "./obj/" + random_fname();
	int resfd = open(ranfname.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0777);
	if (resfd < 0)
	{
		cli.res.status_code = 404;
		return false;
	}

	if ((pid = fork()) < 0)
	{
		cli.res.status_code = 404;
		return false;
	}

	if (pid == 0) // child
	{
		dup2(resfd, 1);
		close(resfd);
		if (cli.req.method == "POST") {
			cli.req.body_fd = open(cli.req.body_fname.c_str(), O_RDONLY);
			dup2(cli.req.body_fd, 0);
			close(cli.req.body_fd);
		}
		else if (cli.req.method == "GET")
			close(0);
		const char *av[3] = {loc.cgi["path"].c_str(), realpath.c_str(), 0};
		char **new_env = cgi_env(cli, env, realpath);
		if (execve(av[0], (char **)av, new_env) < 0)
			std::cerr << strerror(errno) << std::endl;
		exit(1);
	}
	// parent
	cli.res.fname = ranfname;
	cli.res.fd = resfd;
	waitpid(pid, &status, 0);
	// debug
	if (!WIFEXITED(status))
	{
		cli.res.status_code = 404;
		return false;
	}
	lseek(resfd, 0, SEEK_SET);
	return true;
}