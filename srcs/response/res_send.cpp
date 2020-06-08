#include "webserv.hpp"

bool send_res_head(t_client &cli)
{
	t_res &res = cli.res;
	int ret = write(cli.socket, cli.res.head.c_str(), cli.res.head.size());
	if (ret < 0)
		return false; // disconnect
	if (ret == 0)
		return true;
	res.sent_head = true;
	return true;
}

bool send_res_body(t_client &cli)
{
	t_res &res = cli.res;

	int ret = write(cli.socket, cli.res.body.c_str(), cli.res.body.size());
	if (ret < 0)
		return false; // disconnect
	if (ret == 0)
		return true;
	if (res.headers.find("Content-Length") != res.headers.end())
		res.sent_body = true;
	else if (res.headers.find("Transfer-Encoding") != res.headers.end() && res.headers["Transfer-Encoding"] == "chunked" && cli.res.body == "0\r\n\r\n")
	{
		close(res.fd);
		res.sent_body = true;
	}
	res.body.clear();
	renew_client_timestamp(cli);
	return true;
}
