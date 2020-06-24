#include "ConfigParser.hpp"

ConfigParser::ConfigParser(int ac, char **av)
{
	int fd;
	if (ac > 2)
		throw Usage();

	getcwd(this->cur_path, 256);
	if (ac == 1) // default conf
	{
		std::string default_conf;
		default_conf = std::string(cur_path) + "/" + std::string(DEFAULT_CONF_NAME);
		if ((fd = open(default_conf.c_str(), O_RDONLY)) < 0)
			throw DefaultConfNotExist();
	}
	else // custom conf
	{
		if ((fd = open(av[1], O_RDONLY)) < 0)
			throw CustomConfNotExist();
	}
	parsing(fd);
	verify_server_settings();
}

void ConfigParser::verify_server_settings()
{
	// case1. same server_name, same port
	// ->	ignore second server, and show message
	// case2. different server_name, same port
	// ->	ignore second server, and show message
	// case3. same server_name, different port
	// ->	run 2 server
	// case4. different server_name, different port
	// ->	run 2 server

	// find case 1, 2
	std::vector<t_server>::iterator it;
	for (it = srvs.begin(); it != srvs.end(); ++it)
	{
		std::vector<t_server>::iterator i;
		for (i = it + 1; i != srvs.end(); ++i)
		{
			if (it->listen == i->listen)
				break;
		}
		if (i != srvs.end())
		{
			std::cerr << "conflicting server port \""
				<< i->server_name << ":" << i->listen << "\" ignored"<< std::endl;
			srvs.erase(i);
			it = srvs.begin();
		}
	}
}

void ConfigParser::parsing(int fd)
{
	std::string raw;
	std::vector<std::string> tokens;

	get_config_data(fd, raw);
	trim_data(raw);
	tokenizing(raw, tokens);

	std::vector<std::string>::iterator it;
	int block_level = 0;
	t_server sv;
	t_location lc;
	for (it = tokens.begin(); it != tokens.end(); ++it)
	{
		if (block_level == 0)
			block_level_0(sv, it, tokens.end(), block_level);
		else if (block_level == 1)
			block_level_1(sv, lc, it, tokens.end(), block_level);
		else if (block_level == 2)
			block_level_2(sv, lc, it, tokens.end(), block_level);
		else
			throw FormatError("Config Error : Wrong block '" + *it + "'\n");
	}
	if (block_level != 0)
		throw FormatError("Config Error : Block {} termination error.\n");
	if (!srvs.size())
		throw FormatError("Config Error : At least one server blcok is needed\n");
	// if server has no location, add default location.
	std::vector<t_server>::iterator s;
	for (s = srvs.begin(); s != srvs.end() ; ++s)
	{
		if (s->location.empty())
		{
			t_location lc;
			default_location_config(lc, sv);
			sv.location["/"] = lc;
		}
		for (auto it = s->location.begin(); it != s->location.end(); ++it)
		{
			if (it->second.client_max_body_size == -1)
				it->second.client_max_body_size = s->client_max_body_size;
			if (it->second.upload_folder.empty())
				it->second.upload_folder = it->second.root;
		}
	}
}

void ConfigParser::block_level_0(t_server &sv, std::vector<std::string>::iterator &it,
	const std::vector<std::string>::iterator &end, int &block_level)
{
	default_server_config(sv);
	if (*it == "server")
	{
		if (++it != end && *it == "{")
			block_level++;
		else
			throw FormatError("Config Error : Invalid server block\n");
	}
	else
		throw FormatError("Config Error : Invalid server block\n");
}

void ConfigParser::block_level_1(t_server &sv, t_location &lc, std::vector<std::string>::iterator &it,
	const std::vector<std::string>::iterator &end, int &block_level)
{
	if (*it == "}")
	{
		block_level--;
		srvs.push_back(sv);
	}
	else if (*it == "location" && ++it != end)
	{
		default_location_config(lc, sv);
		// check if route start from '/'
		tmp_route = *it;
		if (tmp_route[0] != '/')
			throw FormatError("Config Error : Invalid route '" + tmp_route + "'\n");
		// check if location route is already exist
		std::map<route, t_location>::iterator found = sv.location.find(tmp_route);
		if (found != sv.location.end())
			throw FormatError("Config Error : Duplicate location route '" + tmp_route + "'\n");
		sv.location[tmp_route] = lc;
		// check if location open block with {
		if (++it != end && *it == "{")
		{
			block_level++;
			return;
		}
		else
			throw FormatError("Config Error : Invalid location block\n");
	}
	else if (!is_server_attr(*it))
		throw FormatError("Config Error : Invalid attribute '" + *it + "'\n");
	else
	{
		if (*it == "listen" && ++it != end && *it != ";")
		{
			if (!ft_isstrdigit(it->c_str()))
				throw FormatError("Config Error : Invalid value of 'listen " + *it + "'\n");
			sv.listen = ft_atoi(it->c_str());
		}
		else if (it == end)
			throw FormatError("Config Error : Invalid 'listen'\n");

		if (*it == "root" && ++it != end && *it != ";")
		{
			struct stat info;
			errno = 0;
			if (stat(it->c_str(), &info) != 0)
			{
				if (errno == EACCES)
					throw FormatError("Config Error : No right to access root directory '" + *it + "'\n");
				throw FormatError("Config Error : root directory '" + *it + "' does not exist.\n");
			}
			if (!(S_ISDIR(info.st_mode)))
				throw FormatError("Config Error : root '" + *it + "' is not directory\n");
			sv.root = *it;
		}
		else if (it == end)
			throw FormatError("Config Error : Invalid 'root'\n");

		if (*it == "error_page" && ++it != end && *it != ";")
		{
			struct stat info;
			errno = 0;
			if (stat(it->c_str(), &info) != 0)
			{
				if (errno == EACCES)
					throw FormatError("Config Error : No right to access error_page file '" + *it + "'\n");
				throw FormatError("Config Error : error_page '" + *it + "' does not exist\n");
			}
			if (!(S_ISREG(info.st_mode)))
				throw FormatError("Config Error : error_page '" + *it + "' is not file\n");
			sv.error_page = *it;
		}
		else if (it == end)
			throw FormatError("Config Error : Invalid 'error_page'\n");

		if (*it == "client_max_body_size" && ++it != end && *it != ";")
		{
			sv.client_max_body_size = str_to_size(*it);
			if (sv.client_max_body_size == 0)
				throw FormatError("Config Error : Invalid client_max_body_size '" + *it + "'\n");
		}
		else if (it == end)
			throw FormatError("Config Error : Invalid 'client_max_body_size'\n");

		if (*it == "server_name" && ++it != end && *it != ";")
			sv.server_name = *it;
		else if (it == end)
			throw FormatError("Config Error : Invalid 'client_max_body_size'\n");

		/* check ";" termination */
		++it;
		if (it != end && *it == ";")
			return ;
		else
			throw FormatError("Config Error : Missing ';' after " + *--it + "\n");
	}
}


void ConfigParser::block_level_2(t_server &sv, t_location &lc, std::vector<std::string>::iterator &it,
	const std::vector<std::string>::iterator &end, int &block_level)
{
	(void)lc;

	if (*it == "}")
	{
		block_level--;
		return ;
	}
	if (!is_location_attr(*it))
		throw FormatError("Config Error : Invalid attribute '" + *it + "'\n");
	else
	{
		if (*it == "client_max_body_size" && ++it != end && *it != ";")
		{
			sv.location[tmp_route].client_max_body_size = str_to_size(*it);
			if (sv.location[tmp_route].client_max_body_size == 0)
				throw FormatError("Config Error : Invalid client_max_body_size '" + *it + "'\n");
		}
		else if (it == end)
			throw FormatError("Config Error : Invalid 'client_max_body_size' in location\n");

		if (*it == "upload_folder" && ++it != end && *it != ";")
		{
			struct stat info;
			errno = 0;
			if (stat(it->c_str(), &info) != 0)
			{
				if (errno == EACCES)
					throw FormatError("Config Error : No right to access update_folder directory '" + *it + "'\n");
				throw FormatError("Config Error : upload_folder directory '" + *it + "' does not exist.\n");
			}
			if (!(S_ISDIR(info.st_mode)))
				throw FormatError("Config Error : upload_folder '" + *it + "' is not directory\n");
			sv.location[tmp_route].upload_folder = *it;
		}
		else if (it == end)
			throw FormatError("Config Error : Invalid 'upload_folder' in location\n");

		if (*it == "root" && ++it != end && *it != ";")
		{
			struct stat info;
			errno = 0;
			if (stat(it->c_str(), &info) != 0)
			{
				if (errno == EACCES)
					throw FormatError("Config Error : No right to access root directory '" + *it + "'\n");
				throw FormatError("Config Error : root directory '" + *it + "' does not exist.\n");
			}
			if (!(S_ISDIR(info.st_mode)))
				throw FormatError("Config Error : root '" + *it + "' is not directory\n");
			sv.location[tmp_route].root = *it;
		}
		else if (it == end)
			throw FormatError("Config Error : Invalid 'root' in location\n");

		if (*it == "autoindex" && ++it != end && *it != ";")
		{
			if (*it != "on" && *it != "off")
				throw FormatError("Config Error : Invalid autoindex '" + *it + "'\n");
			sv.location[tmp_route].autoindex = *it;
		}
		else if (it == end)
			throw FormatError("Config Error : Invalid 'autoindex' in location\n");

		if (*it == "index" && ++it != end && *it != ";")
		{
			for (; it != end && *it != ";"; ++it)
				sv.location[tmp_route].index.push_back(*it);
			--it;
		}
		else if (it == end)
			throw FormatError("Config Error : Invalid 'index' in location\n");

		if (*it == "allow" && ++it != end && *it != ";")
		{
			sv.location[tmp_route].allow.clear();
			for (; it != end && *it != ";"; ++it)
			{
				if (!is_http_method(*it))
					throw FormatError("Config Error : Invalid 'allow' methods " + *it + " in location\n");
				sv.location[tmp_route].allow.push_back(*it);
			}
			--it;
		}
		else if (it == end)
			throw FormatError("Config Error : Invalid 'allow' in location\n");

		if (*it == "cgi" && ++it != end && *it != ";")
		{
			if (it->find_first_of('.') != 0)
				throw FormatError("Config Error : Invalid cgi extention name '" + *it + "' in location\n");
			else
				sv.location[tmp_route].cgi["extension"] = *it;
			if (++it != end && *it != ";")
				sv.location[tmp_route].cgi["path"] = *it;
			else
				throw FormatError("Config Error : cgi path not exist.\n");
		}
		else if (it == end)
			throw FormatError("Config Error : Invalid 'cgi' in location\n");

		/* check ";" termination */
		++it;
		if (it != end && *it == ";")
			return;
		else
			throw FormatError("Config Error : Missing ';' after " + *--it + "\n");
	}
}

bool ConfigParser::is_http_method(std::string &token)
{
	// GET HEAD POST PUT DELETE CONNECT OPTIONS TRACE
	size_t len = sizeof(http_methods) / sizeof(std::string);
	for (size_t i = 0; i < len; i++)
	{
		if (token == http_methods[i])
			return true;
	}
	return false;
}

size_t ConfigParser::str_to_size(std::string const &str)
{
	for (std::string::const_iterator it = str.begin(); it != str.end() - 1; ++it)
	{
		if (!ft_isdigit(*it))
			return (0);
	}

	char last_char = *str.rbegin();
	int tmp = atoi(str.c_str());
	if (tmp <= 0)
		return (0);

	if (last_char == 'm' || last_char == 'M')
		return (tmp * 1000 * 1000);
	if (last_char == 'k' || last_char == 'K')
		return (tmp * 1000);
	if (ft_isdigit(last_char))
		return (tmp);
	else
		return (0);
}

void ConfigParser::default_server_config(t_server &sv)
{
	sv.listen = 80;
	sv.server_name = "";
	sv.root = std::string(cur_path) + "/www/";
	sv.error_page = std::string(cur_path) + "/error.html";
	sv.client_max_body_size = 1000 * 1000; // 1M = 1,000,000 bytes
	sv.location.clear();
}

void ConfigParser::default_location_config(t_location &lc, t_server &sv)
{
	(void)sv;
	lc.root = std::string(cur_path) + "/www/";
	lc.upload_folder = "";
	lc.client_max_body_size = -1;
	lc.autoindex = "off";
	size_t len = sizeof(http_methods) / sizeof(std::string);
	lc.allow.clear();
	for (size_t i = 0; i < len; i++)
		lc.allow.push_back(http_methods[i]);
}

void ConfigParser::trim_data(std::string &raw)
{
	std::string::size_type pos = 0;
	std::string::size_type pos2;
	while ((pos = raw.find("#", pos)) != std::string::npos)
	{
		pos2 = raw.find("\n", pos);
		raw.replace(pos, pos2 - pos, "\n");
		pos += 1;
	}
	pos = 0;
	while ((pos = raw.find(";", pos)) != std::string::npos)
	{
		raw.replace(pos, 1, " ; ");
		pos += 3;
	}
	pos = 0;
	while ((pos = raw.find("{", pos)) != std::string::npos)
	{
		raw.replace(pos, 1, " { ");
		pos += 3;
	}
	pos = 0;
	while ((pos = raw.find("}", pos)) != std::string::npos)
	{
		raw.replace(pos, 1, " } ");
		pos += 3;
	}
}

void ConfigParser::tokenizing(std::string &raw, std::vector<std::string> &tokens)
{
	const char *delimit = " \t\n\r\v";
	char **char_tokens = ft_split(raw.c_str(), delimit);

	if (!char_tokens)
		throw FailToReadConfigFile();
	for (size_t i = 0; char_tokens[i] != 0; i++)
	{
		tokens.push_back(std::string(char_tokens[i]));
	}
	for (size_t i = 0; char_tokens[i] != 0; i++)
		free(char_tokens[i]);
	free(char_tokens);
}

void ConfigParser::get_config_data(int fd, std::string &raw)
{
	char buff[MAX_BUFFER_SIZE + 1];
	int ret;

	while ((ret = read(fd, buff, MAX_BUFFER_SIZE)) > 0)
	{
		buff[ret] = '\0';
		raw += std::string(buff);
	}
	close(fd);
	if (ret < 0)
		throw FailToReadConfigFile();
}

bool ConfigParser::is_server_attr(const std::string &attr)
{
	size_t size = sizeof(server_attr) / sizeof(std::string);

	for (size_t i = 0; i < size; i++)
	{
		if (attr == server_attr[i])
			return true;
	}
	return false;
}

bool ConfigParser::is_location_attr(const std::string &attr)
{
	size_t size = sizeof(location_attr) / sizeof(std::string);

	for (size_t i = 0; i < size; i++)
	{
		if (attr == location_attr[i])
			return true;
	}
	return false;
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

const char *ConfigParser::FailToReadConfigFile::what() const throw()
{
	return "ConfigParser : Fail to read config file\n";
}

ConfigParser::FormatError::FormatError(const std::string &message)
: message(message)
{
}

const char *ConfigParser::FormatError::what() const throw()
{
	return message.c_str();
}

ConfigParser::~ConfigParser()
{
}

std::vector<t_server> &ConfigParser::servers()
{
	return srvs;
}
