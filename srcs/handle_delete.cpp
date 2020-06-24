#include "webserv.hpp"

void handle_delete(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file)
{
    (void)env;

	struct stat fileStat;
	if(stat(folder_path.c_str(), &fileStat) < 0 || !is_file)    
	{
		cli.res.status_code = 404;
		return ;
	}
	std::vector<std::string>::iterator it;
	for (it = loc->index.begin(); it != loc->index.end(); ++it)
	{
		if ("/" + *it == file)
			break;
	}
	if (it != loc->index.end())
	{
		cli.res.status_code = 403;
		return;
	}
	if ((fileStat.st_mode & S_IWUSR) == 0)
	{
		cli.res.status_code = 403;
		return ;
	}
	unlink(folder_path.c_str());
	cli.res.status_code = 204;
}