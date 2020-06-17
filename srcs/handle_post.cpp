#include "webserv.hpp"

void handle_post(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file)
{
	int fd;
	t_req &req = cli.req;
    // std::string real_path = loc->upload_folder + loc->abs_path.substr(loc->abs_path.find_last_of('/'));
	// std::cout << "Real Path is : " << real_path <<std::endl;
	// make_file_res(cli, loc, env, real_path, file);

	fd = open((loc->upload_folder + loc->abs_path.substr(loc->abs_path.find_last_of('/'))).c_str(), O_CREAT  | O_WRONLY | O_APPEND, 0777);
	if(fd < 0)
	{
		cli.res.status_code = 404;
		cli.res_sent = true;
		return ;
	}
	write(fd, req.body.c_str(), req.body.size());
	close(fd);
	cli.res.status_code = 200;
	cli.res_sent = true;
}
