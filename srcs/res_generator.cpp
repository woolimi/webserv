#include "webserv.hpp"
#include "HttpStatus.hpp"

void res_generator(t_client &cli)
{
	t_req &req = cli.req;
	t_res &res = cli.res;
	t_server &serv = cli.server;

	std::cout << "STATUS: " << res.status_code << std::endl;
	if (!HttpStatus::isSuccessful(res.status_code))
	{
		// make dynamic error page
		// if default error page is set, use default one.
	} else
	{
		std::cout << "Status code: " << res.status_code << std::endl;
		res.raw += "HTTP/1.1 " + std::to_string(res.status_code) + " " + HttpStatus::reasonPhrase(res.status_code);
		res.raw += "\r\n";
		res.raw += "Server: webserv/1.0";
		res.raw += "\r\n";
		res.raw += "Date: ";
		res.raw += "\r\n";
		res.raw += "Content-Length: ";
		res.raw += "\r\n";
		res.raw += "\r\n";
		res.raw += res.body; // body data is already set in handle_[method_name] function.
	}
}
