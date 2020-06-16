#include "webserv.hpp"

bool send_res_head(t_client &cli)
{
	t_res &res = cli.res;

	// std::cout << cli.res.head << std::endl;

	int ret = send(cli.socket, cli.res.head.c_str(), cli.res.head.size(), MSG_NOSIGNAL);
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

	std::cout << "sending res body" << std::endl;
	int ret = send(cli.socket, cli.res.body.c_str(), cli.res.body.size(), MSG_NOSIGNAL);
	if (ret < 0)
		return false; // disconnect

	if (res.headers.find("Transfer-Encoding") != res.headers.end())
	{
		if (res.body.find("0\r\n\r\n") != std::string::npos)
		{
			cli.res_sent = true;
			if (!res.fname.empty())
				unlink(res.fname.c_str());
		}
	}
	else
	{
		res.content_length -= ret;
		if (res.content_length == 0)
			cli.res_sent = true;
	}

	if (ret < cli.res.body.size())
		cli.res.body.erase(0, ret);
	else
		cli.res.body.clear();
	renew_client_timestamp(cli);
	return true;
}
