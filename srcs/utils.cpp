#include "webserv.hpp"

void renew_client_timestamp(t_client &cli)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	cli.time_stamp = tv.tv_sec;
}

int file_check(std::string file_path)
{
	struct stat info;
	errno = 0;
	int ret = stat(file_path.c_str(), &info);
	std::cout<<"FILEPATHHH." << file_path << std::endl;
	if (ret < 0)
	{
		if (errno == ENOENT) // not exist
			return 404;
		if (errno == EACCES)
		{
			return 403;
		}
	}
	if (!S_ISREG(info.st_mode))
		return 404;
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

t_location *find_matched_location(t_server &serv, std::string &path)
{
	t_location *ret = &serv.location["/"];
	size_t max_matched_size = 0;
	size_t pos;

	std::map<route, t_location>::iterator it;

	for (it = serv.location.begin(); it != serv.location.end(); ++it)
	{
		if (it->first == path) //100% match
		{
			it->second.abs_path = it->second.root;
			if (*it->second.abs_path.rbegin() != '/')
				it->second.abs_path += "/";
			return &it->second;
		}
	}

	for (it = serv.location.begin(); it != serv.location.end(); ++it) // part match
	{
		if ((pos = path.find(it->first) != std::string::npos && pos == 0 && it->first.size() > max_matched_size))
		{
			it->second.abs_path = it->second.root; //root/abc
			if (*it->second.abs_path.rbegin() != '/')
				it->second.abs_path += "/"; 
			it->second.abs_path += path.substr(it->first.size());
			
			ret = &it->second;
			max_matched_size = path.size();
		}
	}
	return ret;
}

// t_location *find_matched_location(t_server &serv, std::string &folder_path, std::string &file)
// {
// 	t_location *ret;
// 	size_t max_matched_size = 0;
// 	size_t pos;

// 	std::map<route, t_location>::iterator it;

// 	if (folder_path.empty() && file == "/")
// 		return &serv.location["/"];
// 	// folder request
// 	if (file == "/")
// 	{
// 		// 100% match
// 		std::string fdpath = folder_path + "/";
// 		for (it = serv.location.begin(); it != serv.location.end(); ++it)
// 		{
// 			if (it->first == fdpath)
// 				return &it->second;
// 		}
// 		// part match
// 		for (it = serv.location.begin(); it != serv.location.end(); ++it)
// 		{
// 			if ((pos = fdpath.find(it->first)) != std::string::npos && pos == 0 && it->first.size() > max_matched_size)
// 			{
// 				ret = &it->second;
// 				max_matched_size = fdpath.size();
// 			}
// 		}
// 		return ret;
// 	}
// 	else // file request
// 	{
// 		// 100% match
// 		std::string fdpath = folder_path + file;
// 		for (it = serv.location.begin(); it != serv.location.end(); ++it)
// 		{
// 			if (it->first == fdpath)
// 				return &it->second;
// 		}
// 		return &serv.location["/"];
// 	}
// }