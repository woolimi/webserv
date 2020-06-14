#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

#include "webserv.hpp"

class ConfigParser
{
private:
	ConfigParser();
	std::vector<t_server> srvs;
	char cur_path[256];
	std::string tmp_route;
	void get_config_data(int fd, std::string &raw);
	void trim_data(std::string &raw);
	void parsing(int fd);
	void tokenizing(std::string &raw, std::vector<std::string> &tokens);
	void default_server_config(t_server &sv);
	void default_location_config(t_location &lc);
	bool is_server_attr(std::string const &attr);
	bool is_location_attr(std::string const &attr);
	bool is_http_method(std::string &token);
	void block_level_0(t_server &sv, std::vector<std::string>::iterator &it,
		const std::vector<std::string>::iterator &end, int &block_level);
	void block_level_1(t_server &sv, t_location &lc, std::vector<std::string>::iterator &it,
		const std::vector<std::string>::iterator &end, int &block_level);
	void block_level_2(t_server &sv, t_location &lc, std::vector<std::string>::iterator &it,
		const std::vector<std::string>::iterator &end, int &block_level);
	size_t str_to_size(std::string const &str);
	void verify_server_settings();
	std::string server_attr[5] = {
		"listen",
		"server_name",
		"root",
		"error_page",
		"client_max_body_size"
	};
	std::string location_attr[6] = {
		"root",
		"autoindex",
		"index",
		"allow",
		"cgi",
		"upload_folder",
	};
	std::string http_methods[8] = {
		"GET",
		"HEAD",
		"POST",
		"PUT",
		"DELETE",
		"CONNECT",
		"OPTIONS",
		"TRACE"
	};

public:
	ConfigParser(int ac, char **av);
	// ConfigParser(ConfigParser const &other);
	// ConfigParser &operator=(ConfigParser const &other);
	~ConfigParser();

	std::vector<t_server> &servers();

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
	class FailToReadConfigFile : public std::exception
	{
		virtual const char *what() const throw();
	};
	class FormatError : public std::exception
	{
	private:
		std::string message;
	public:
		explicit FormatError(const std::string &message);
		virtual const char *what() const throw();
	};
};

#endif
