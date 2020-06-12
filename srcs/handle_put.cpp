#include "webserv.hpp"

void handle_put(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file)
{
	t_req &req = cli.req;
	// std::string real_path = make_real_path(loc->root, req.path);
	// std::cout << "Real: " << real_path << std::endl;
	std::cout << "abs: " << loc->abs_path << std::endl;
	std::cout << "is_file: " << is_file << std::endl;
	std::cout << "is_sizeof body: " << req.body.size() << std::endl;

	int fd = open((loc->abs_path).c_str(), O_CREAT  | O_WRONLY, 0777);
	std::cout << req.body.c_str() << std::endl;
	write(fd, req.body.c_str(), req.body.size());
	close(fd);
	cli.res.status_code = 201;
	cli.res_sent = true;


}
