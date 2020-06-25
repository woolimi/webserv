#include "webserv.hpp"

int islog()
{
	static int islog = 0;
	return (islog);
}

void log(std::string msg)
{
	if (islog())
		std::cerr << msg << std::endl;
}

void debug(std::string msg)
{
	std::cerr << "\033[33m" << msg << "\033[0m" << std::endl;
}

std::string random_fname(void)
{
	std::string ret;
	std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int nb_char = 15;
	while (nb_char-- > 0)
	{
		ret += charset[rand() % charset.size()];
	}
	return ret;
}

void renew_client_timestamp(t_client &cli)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	cli.time_stamp = tv.tv_sec;
}

int isDirectory(const char *path)
{
	struct stat statbuf;

	if (stat(path, &statbuf) != 0)
		return 0;
	return S_ISDIR(statbuf.st_mode);
}

int file_check(std::string file_path)
{
	struct stat info;
	errno = 0;
	int ret = stat(file_path.c_str(), &info);
	if (ret < 0)
	{
		if (errno == ENOENT) // not exist
			return 404;
		if (errno == EACCES)
			return 403;
	}
	if (!S_ISREG(info.st_mode)) {
		return 404;
	}
	return OK;
}

std::string int_to_hexstr(int n)
{
	std::string charset = "0123456789abcdef";
	std::string ret;

	while (n >= 16)
	{
		ret.insert(0, 1, charset[n % 16]);
		n /= 16;
	}
	ret.insert(0, 1, charset[n % 16]);
	return ret;
}

t_location *find_matched_location(t_server &serv, std::string &path, t_client &cli)
{
	(void)cli;
	t_location *ret = &serv.location["/"];
	size_t max_matched_size = 0;
	size_t pos;

	std::map<route, t_location>::iterator it;

	for (it = serv.location.begin(); it != serv.location.end(); ++it)
	{
		if ((it->first == path) || (it->first == path + "/")) //100% match
		{
			it->second.abs_path = it->second.root;
			if (*it->second.abs_path.rbegin() != '/')
				it->second.abs_path += "/";
			return &it->second;
		}
	}

	for (it = serv.location.begin(); it != serv.location.end(); ++it) // part match
	{
		if (((pos = path.find(it->first)) != std::string::npos && pos == 0 && it->first.size() > max_matched_size))
		{
			it->second.abs_path = it->second.root; //root/abc
			if (*it->second.abs_path.rbegin() != '/')
				it->second.abs_path += "/"; 
			// std::cout << "abs here: " << it->second.abs_path <<std::endl ;
			// std::cout << "UF here : " << it->second.upload_folder <<std::endl;
			it->second.abs_path += path.substr(it->first.size());
			ret = &it->second;
			max_matched_size = it->first.size();
		}
	}
	return ret;
}

std::string gmt_time_string(time_t &sec)
{
	struct tm *timeinfo;
	char buffer[80];

	timeinfo = gmtime(&sec);
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
	return buffer;
}