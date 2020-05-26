#include "ConfigParser.hpp"

ConfigParser::ConfigParser(int ac, char **av)
{
	int fd;

	if (ac > 2)
		throw Usage();

	if (ac == 1) // default conf
	{
		if ((fd = open(DEFAULT_CONF, O_RDONLY)) < 0)
			throw DefaultConfNotExist();
	}
	else // conf
	{
		if ((fd = open(av[1], O_RDONLY)) < 0)
			throw CustomConfNotExist();
	}
	parsing(fd);
}

void ConfigParser::parsing(int fd)
{
	/* need to make parsing code here */
	/* But for now, just give dummy data for test */
	/* real data has to be came from config */

	// make server info
	t_server sv;
	sv.listen = 8080;
	sv.client_max_body_size = 1; // 1MB = 1,000,000 bytes
	sv.server_name = "localhost";
	sv.root = "/home/wpark/Documents/webserv/www/";
	// make location info
	t_location loc;
	loc.allow = "GET POST";
	loc.autoindex = "on";
	loc.index = "index.html";
	loc.root = "/home/wpark/Documents/webserv/www/";
	sv.location["/"] = loc;

	srvs.push_back(sv);

	close(fd);
}

const char *ConfigParser::Usage::what() const throw()
{
	return "Usage : ./webserv [conf_path(optionnal)]\n";
}

const char *ConfigParser::DefaultConfNotExist::what() const throw()
{
	return "ConfigParser : Default configuration file not exist\n";
}

const char *ConfigParser::CustomConfNotExist::what() const throw()
{
	return "ConfigParser : Custom configuration file not exist\n";
}

ConfigParser::~ConfigParser()
{
}

std::vector<t_server> &ConfigParser::servers()
{
	return srvs;
}
