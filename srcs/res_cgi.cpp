#include "webserv.hpp"

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
	{
		new_env["HTTP_X_SECRET_HEADER_FOR_TEST"] = "1"; // MANUAL ADDITION OF HEADER 
		new_env["CONTENT_LENGTH"] = std::string(ft_itoa(req.body.size()));
		// if count === 3
		// DEBUG("CAUGHT!!!!!");
		// std::cerr << req.body.size() << std::endl;
		// for(std::map<std::string, std::string>::iterator it = new_env.begin();
		// it != new_env.end(); ++it)
		// {
		// 	std::cerr << it->first << "-" << it->second << std::endl;
		// }
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
	int pid;
	int status;
	std::string ranfname = "cgires_" + random_fname();
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
	if (!WIFEXITED(status))
	{
		cli.res.status_code = 404;
		return false;
	}
	lseek(resfd, 0, SEEK_SET);
	return true;
}