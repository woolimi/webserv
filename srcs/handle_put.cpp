#include "webserv.hpp"

void handle_put(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file)
{
	int fd;
	t_req &req = cli.req;
	// std::string real_path = make_real_path(loc->root, req.path);
	// std::cout << "Real: " << real_path << std::endl;
	std::cout << "abs: " << loc->abs_path << std::endl;
	std::cout << "upload folder: " << loc->upload_folder <<std::endl;
	// std::cout << "is_file: " << is_file << std::endl;
	// std::cout << "is_sizeof body: " << req.body.size() << std::endl;
	// std::cout << loc->abs_path <<std::endl;

	if (loc->upload_folder.empty())
		fd = open((loc->abs_path).c_str(), O_CREAT  | O_WRONLY | O_TRUNC, 0777);
	else
		fd = open((loc->upload_folder + loc->abs_path.substr(loc->abs_path.find_last_of('/'))).c_str(), O_CREAT  | O_WRONLY | O_TRUNC, 0777);

	if(fd < 0)
	{
		std::cout << "404 from here\n";
		cli.res.status_code = 404;
		cli.res_sent = true;
		return ;
	}
	write(fd, req.body.c_str(), req.body.size());
	close(fd);
	cli.res.status_code = 201;
	cli.res_sent = true;
}
