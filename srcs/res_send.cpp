#include "webserv.hpp"

extern int count;

bool send_res_head(t_client &cli)
{
	t_res &res = cli.res;

	// std::cerr << "Header: " << cli.res.head << std::endl;
	int ret = send(cli.socket, cli.res.head.c_str(), cli.res.head.size(), MSG_NOSIGNAL);

	/* debug */
	if (count > 1) {
		write(3, "\nclient id : ", 13);
		write(3, cli.id.c_str(), cli.id.size());
		write(3, " status code : ", 15);
		write(3, std::to_string(cli.res.status_code).c_str(), std::to_string(cli.res.status_code).size());
		write(3, "\n", 1);
		// usleep(10);
	}
	/* debug */

	if (ret < 0)
		return false; // disconnect
	if (ret == 0)
		return true;
	res.sent_head = true;
	// std::cout << "send res head" << std::endl;
	return true;
}

bool send_res_body(t_client &cli)
{
	t_res &res = cli.res;

	int ret = send(cli.socket, cli.res.body.c_str(), cli.res.body.size(), MSG_NOSIGNAL);
	if (ret != 0)
	// std::cerr << res.content_length << "ret: " << ret << std::endl;
	
	if (ret < 0)
		return false; // disconnect

	if (res.headers.find("Transfer-Encoding") != res.headers.end())
	{
		// if (res.body.empty())
		// 	res.body += "0\r\n\r\n";
		if (res.body.find("0\r\n\r\n") != std::string::npos)
			cli.res_sent = true;
		// else
		// 	std::cerr << "body: "<< cli.res.body << std::endl;
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
