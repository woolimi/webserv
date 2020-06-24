#include "webserv.hpp"

void handle_put(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file)
{
	(void)env;
	(void)is_file;
	(void)file;
	(void)folder_path;

	int fd;
	t_req &req = cli.req;
	fd = open((loc->upload_folder + loc->abs_path.substr(loc->abs_path.find_last_of('/'))).c_str(), O_CREAT  | O_WRONLY | O_TRUNC, 0777);
	if(fd < 0)
	{
		std::cerr << "can't do POST" << std::endl;
		return;
	}
	write(fd, req.body.c_str(), req.body.size()); //protect and/or chunk
	close(fd);
	cli.res.status_code = 201;
}
