#include "webserv.hpp"
#include "HTTP.hpp"
#include "ConfigParser.hpp"

int main(int ac, char **av, char **env)
{
	try
	{
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