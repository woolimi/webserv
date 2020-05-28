#include "webserv.hpp"

static void	free_tab(char **args)
{
	int i;

	i = 0;
	while (args && args[i] != 0)
	{
		free(args[i]);
		args[i] = 0;
		i++;
	}
	if (args)
	{
		free(args);
		args = 0;
	}
}

// typedef struct s_req
// {
// 	std::string raw;
// 	/* request line */
// 	std::string method;	 // essential
// 	std::string path;	 // essential
// 	std::string version; // essential
// 	/* header */
// 	std::string host; // essential
// 	std::string accept;
// 	/* body */
// 	std::string body;
// } t_req;

const char *BadRequestException::what() const throw()
{
	return "400 Bad Request";// Bad respoonse set a flag to check in response interpretor
}

void parse_request_line(char *request_line, t_req &req)
{
	int size = 0;
	std::set<std::string> methods = {"GET", "POST", "PUT", "OPTIONS", "HEAD", "DELETE", "TRACE", "CONNECT"};
	char **request_line_split = ft_split(request_line, " \t\n\r\v");
	while (request_line_split[size] != 0)
		size++;
	if (size != 3)
	{
		free_tab(request_line_split);
		throw BadRequestException();
	}
	req.method = request_line_split[0];
	if (methods.find(req.method) == methods.end())
	{
		free_tab(request_line_split);
		throw BadRequestException();
	}
	req.path = request_line_split[1];
	if ( req.path.find("/") != 0)
	{
		free_tab(request_line_split);
		throw BadRequestException();
	}
	req.version = request_line_split[2];
	if (req.version != "HTTP/1.0" && req.version != "HTTP/1.1")
		throw BadRequestException();
	free_tab(request_line_split);
}



void req_interpreter(t_req &req)
{
	// need to get data from raw
	(void)req;
	char **header;
	int body_start = req.raw.find("\r\n\r\n");
	if (body_start < 0)
		body_start = req.raw.find("\n\n");
	if (body_start > 0)
		header = ft_split(req.raw.substr(0, body_start).c_str(), "\r\n");
	else
		header = ft_split(req.raw.c_str(),"\r\n");
	try
	{
		parse_request_line(header[0], req);
		for(int i = 1; header[i] != 0; i++)
		{
			std::string str(header[i]);
			req.headers[str.substr(0, str.find(":"))] = str.substr(str.find(":") + 1); //trim; 
			//handle a bad request without a colon in a header field;
		}
		for(std::map<std::string, std::string >::iterator it = req.headers.begin();
    	it != req.headers.end(); ++it)
		{
    		std::cout << it->first << "-" << it->second << "\n";
		}
	}
	catch(std::exception e)
	{
		// std::cout << e.what() <<std::endl;
		e.what();
	}


	if (body_start > 0)
		req.body = req.raw.substr(body_start + 4);
	free_tab(header);
	// for (int i = 0; header[i] != 0; i++ )
	// 	std::cout << header[i] << std::endl;
}