#include "webserv.hpp"

static std::string random_fname(void)
{
	std::string ret;
	std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int nb_char = 15;
	int i = 0;
	while (nb_char-- > 0)
	{
		ret += charset[rand() % charset.size()];
	}
	return ret;
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

bool execute_cgi(t_client &cli, t_location &loc, char **env, std::string &realpath, std::string &ext)
{
	int c2p_fd[2];
	int pid;
	int status;
	std::string ranfname = random_fname();
	int resfd = open(ranfname.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0777);
	std::cout << "HERE21\n";

	if (resfd < 0 || pipe(c2p_fd) < 0)
	{
		cli.res.status_code = 404;	
		return false;
	}
	// c2p_fd[1]
	std::cout << "fd[0]: "<< c2p_fd[0] << std::endl;
	std::cout << "fd[1]: "<< c2p_fd[1] << std::endl;
	std::cout << "HERE22\n";
	size_t write_val = 0;// =  write(c2p_fd[1], cli.req.body.c_str(), cli.req.body.size());
	size_t body_size = cli.req.body.size();
	    // int fd = open("/home/user42/Desktop/mashar/projects/webserv/www/YoupiBanane/youpi.bla", O_WRONLY | O_TRUNC, 0777);

	while (body_size > 0)
	{
		write_val =  write(c2p_fd[1], cli.req.body.c_str(), MAX_BUFFER_SIZE);
		// write_val =  write(fd, cli.req.body.c_str(), MAX_BUFFER_SIZE);
		cli.req.body.erase(0, write_val);
		body_size -= write_val;
		std::cout << write_val+MAX_BUFFER_SIZE << "\t" << (long)body_size <<std::endl;
		// if (write_val + MAX_BUFFER_SIZE > (long)body_size)
			// break;
	}
	std::cout << "Write_val: " << write_val << std::endl;
	if (!cli.req.body.empty() &&  write_val < 0)
	{
		std::cout << strerror(errno)<<std::endl;
		cli.res.status_code = 404;
		close(resfd);
		return false;
	}
	std::cout << "HERE23\n";

	close(c2p_fd[1]);

	if ((pid = fork()) < 0)
	{
		cli.res.status_code = 404;
		close(resfd);
		return false;
	}

	if (pid == 0) // child
	{
		dup2(resfd, 1);
		close(resfd);
		dup2(c2p_fd[0], 0);
		close(c2p_fd[0]);
		const char *av[3] = { loc.cgi["path"].c_str(), realpath.c_str(), 0 };
		char **new_env = cgi_env(cli, env, realpath);
		if (execve(av[0], (char **)av, new_env) < 0)
			std::cerr << strerror(errno) << std::endl;
		exit(1);
	}
	// parent
	close(c2p_fd[0]);
	waitpid(pid, &status, 0);
	if (!WIFEXITED(status))
	{
		cli.res.status_code = 404;
		return false;
	}
	lseek(resfd, 0, SEEK_SET);
	cli.res.fname = ranfname;
	cli.res.fd = resfd;
	return true;
}