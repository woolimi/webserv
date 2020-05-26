#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

#include "webserv.hpp"

class ConfigParser
{
private:
	ConfigParser();
	std::vector<t_server> srvs;
public:
	ConfigParser(int ac, char **av);
	// ConfigParser(ConfigParser const &other);
	// ConfigParser &operator=(ConfigParser const &other);
	~ConfigParser();

	std::vector<t_server> &servers();
	void parsing(int fd);
	/* exceptions */
	class Usage : public std::exception
	{
		virtual const char *what() const throw();
	};
	class DefaultConfNotExist : public std::exception
	{
		virtual const char *what() const throw();
	};
	class CustomConfNotExist : public std::exception
	{
		virtual const char *what() const throw();
	};
};

#endif
