#include "webserv.hpp"

std::string make_real_path(std::string &root, std::string &path)
{
	std::string real_path = root + path;
	size_t pos = real_path.find("//");
	if (pos != std::string::npos)
		real_path.replace(pos, 2, "/");
	return (real_path);
}

void handle_get(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file)
{
	if (is_file) {
		make_file_res(cli, loc, env, folder_path, file);
	}
	else
	{
		if (folder_path == "")
			folder_path = "/";
		if (*folder_path.rbegin() != '/')
			folder_path += "/";

		std::string file_path;
		// folder with index youpi.bad_extension
		if (!loc->index.empty())
		{
			for (auto it = loc->index.begin(); it != loc->index.end(); ++it)
			{
				file_path.clear();
				file_path = folder_path + *it;
				if (file_check(file_path) == OK)
				{
					make_file_res(cli, loc, env, file_path, *it);
					return;
				}
			}
			cli.res.status_code = 404;
			return;
		}

		// folder listing
		if (loc->autoindex == "on")
			make_folder_list_res(cli, loc, folder_path);
		else
			cli.res.status_code = 404;
	}
}
