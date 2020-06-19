#include "webserv.hpp"


void handle_trace(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file)
{
    std::map<std::string, std::string> &headers = cli.req.headers;

    cli.res.body += cli.req.method;
    cli.res.body += " ";
    cli.res.body += cli.req.path;
    cli.res.body += " ";
    cli.res.body += cli.req.version;
    cli.res.body += "\r\n";
   for(std::map<std::string, std::string>::iterator it = cli.req.headers.begin(); it != cli.req.headers.end(); ++it)
		{
			// std::cout << it->first << "==" << it->second << "\n";
            cli.res.body += it->first;
            cli.res.body += ": ";
            cli.res.body += it->second;
            cli.res.body += "\r\n";
		}
        cli.res.body += "\r\n";
	cli.res.headers["Content-Type"] = "message/http";
	cli.res.headers["Content-Length"] = std::to_string(cli.res.body.size());
    cli.res.status_code = 200;

}