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

	if (res.headers.find("Transfer-Encoding") != res.headers.end()
		&& res.body.find("0\r\n\r\n") != std::string::npos)
	{
		res.body.clear();
		cli.res_sent = true;
		close(res.fd);
		return true;
	}
	else
	{
		res.content_length -= ret;
		if (res.content_length == 0)
		{
			cli.res_sent = true;
			if (res.fd != -1)
				close(res.fd);
		}
	}
	
	if (ret < cli.res.body.size())
		cli.res.body.erase(0, ret);
	else
		cli.res.body.clear();

	renew_client_timestamp(cli);
	return true;
}
