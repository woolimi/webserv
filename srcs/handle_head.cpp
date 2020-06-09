#include "webserv.hpp"


void handle_head(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file)
{
	t_req &req = cli.req;
	std::string real_path = make_real_path(loc->root, req.path);

	if (is_file)
		make_file_res(cli, loc, env, real_path, file);
	else
	{
		std::vector<std::string>::iterator it;
		std::string file_path;
		// folder with index index.php index.html
		for (it = loc->index.begin(); it != loc->index.end(); ++it)
		{
			file_path.clear();
			file_path = real_path + *it;
			if (file_check(file_path) == OK)
			{
				make_file_res(cli, loc, env, file_path, *it);
				return;
			}
		}
		// folder listing
		if (loc->autoindex == "on")
		{
			if (folder_path == "")
				folder_path = "/";
			make_folder_list_res(cli, loc, folder_path, real_path);
		}
		else
			cli.res.status_code = 404;
	}
}
