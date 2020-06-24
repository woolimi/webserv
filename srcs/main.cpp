#include "webserv.hpp"
#include "HTTP.hpp"
#include "ConfigParser.hpp"

void signal_handler(int signo)
{
	if (signo == SIGPIPE)
		return;
	else if (signo == SIGINT)
	{
		HTTP http;
		std::list<t_client> &clients = http.get_clients();
		std::list<t_server> &servers = http.get_servers();
		for (auto it = clients.begin(); it != clients.end(); ++it)
		{
			unlink(it->req.body_fname.c_str());
			unlink(it->res.fname.c_str());
			close(it->res.fd);
			close(it->socket);
		}
		for (auto it = servers.begin(); it != servers.end(); ++it)
		{
			close(it->socket);
		}
		std::cout << std::endl;
		exit(0);
	}
}

int main(int ac, char **av, char **env)
{
	try
	{
		signal(SIGPIPE, signal_handler);
		signal(SIGINT, signal_handler);
		// get config file param. If not exist, use default config
		ConfigParser conf(ac, av);
		// set server
		HTTP http(conf.servers());
		// run server
		http.run(env);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what();
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}