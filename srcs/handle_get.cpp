#include "webserv.hpp"
// 1. check req.route
// req.route is folder ?
// req.route is file ?
// 
// case1 folder
// check index attribute, if it exist, need to show file /index.html
// check autoindex on
// make page 
//
// case2 file
// check server set cgi or not
// if no cgi
// make res.body
// if cgi
// communicate with cgi

bool is_file(const std::string &route)
{

}

// localhost/test/index.html
void handle_get(t_client &cli)
{
	t_req &req = cli.req;
	t_server &serv = cli.server;

	if (*req.path.rbegin() == '/') // folder /index.php
	{

	} else // file
	{
		// cgi
		//
	}
}