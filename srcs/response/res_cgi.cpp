#include "webserv.hpp"

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

bool execute_cgi(t_client &cli, t_location &loc, char **env, std::string &real_path, std::string &ext)
{
	int p2c_fd[2];
	int c2p_fd[2];
	int pid;
	int status;

	if (pipe(p2c_fd) < 0 || pipe(c2p_fd) < 0 || (pid = fork()) < 0)
	{
		cli.res.status_code = 404;	
		return;
	}

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
		if (execve(av[0], (char **)av, new_env) < 0)
			std::cout << strerror(errno) << std::endl;
		exit(1);
	}
	else // parent
	{
		close(c2p_fd[1]);
		close(p2c_fd[0]);
		if (!cli.req.body.empty())
		{ // send POST data
			if (write(p2c_fd[1], cli.req.body.c_str(), cli.req.body.size()) <= 0)
			{
				close(p2c_fd[1]);
				kill(pid, SIGKILL);
				cli.res.status_code = 404;
				return;
			}
			cli.req.body.clear();
		}
		cli.res.fd = c2p_fd[0];
		waitpid(pid, &status, 0);
		if (!WIFEXITED(status))
		{
			cli.res.status_code = 404;
			return;
		}
		close(p2c_fd[1]);
	}
}