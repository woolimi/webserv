#include "webserv.hpp"

/*
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"CONNECT",
	"OPTIONS",
	"TRACE"
*/

void handle_methods(t_client &cli)
{
	t_req &req = cli.req;
	if (req.method == "GET")
	{

	}
	else if (req.method == "HEAD")
	{

	}
	else if (req.method == "POST")
	{

	}
	else if (req.method == "PUT")
	{

	}
	else if (req.method == "DELETE")
	{

	}
	else if (req.method == "CONNECT")
	{

	}
	else if (req.method == "OPTIONS")
	{

	}
	else if (req.method == "TRACE")
	{

	}
}