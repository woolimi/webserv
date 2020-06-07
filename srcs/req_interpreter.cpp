#include "webserv.hpp"

int is_newline_char(char c)
{
	if (c == '\r' || c == '\n')
		return (1);
	return (0);
}

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

void set_http_status(t_client &client, int status)
{
	if (client.res.status_code == 0)
		client.res.status_code = status;
}

void parse_request_line(char *request_line, t_client &client)
{
	int size = 0;
	std::set<std::string> methods = {"GET", "POST", "PUT", "OPTIONS", "HEAD", "DELETE", "TRACE", "CONNECT"};
	char **request_line_split = ft_split(request_line, " \t");
	while (request_line_split[size] != 0)
		size++;
	if (size != 3)
	{
		free_tab(request_line_split);
		set_http_status(client, 400);
		return;
	}
	client.req.method = request_line_split[0];
	if (methods.find(client.req.method) == methods.end())
	{
		free_tab(request_line_split);
		set_http_status(client, 405);
		return;
	}
	client.req.path = request_line_split[1];
	if (client.req.path.find("/") != 0)
	{
		free_tab(request_line_split);
		set_http_status(client, 400);
		return;
	}
	client.req.version = request_line_split[2];
	free_tab(request_line_split);

	int x;
	if(((x = client.req.version.find("/")) != client.req.version.rfind("/")) || (client.req.version.find("/") == std::string::npos))
	{
		set_http_status(client, 400);
		return;
	}
	if((client.req.version.find(".") != client.req.version.rfind(".")) || (client.req.version.find(".") == std::string::npos))
	{
		set_http_status(client, 400);
		return;
	}

	if(client.req.version.substr(0, x) != "HTTP")
	{
		set_http_status(client, 404);
		return;
	}

	std::string vno = client.req.version.substr(client.req.version.find("/") + 1);
	if (vno[vno.length() - 2] == '\r' && vno[vno.length() - 1] == '\n')
		vno = vno.substr(0, vno.length() - 2);
	if(vno.length() > 6)
	{
		set_http_status(client, 400);
		return;
	}

	if(vno[0] != '1' || vno[1] != '.')
	{
		set_http_status(client, 505);
		return;
	}		
	for (int i = 1; i < vno.length(); i++)
	{
		if(vno[i] && !ft_isdigit((char)vno[i]) && vno[i] != '.')
		{
			set_http_status(client, 400);
			return;
		}
	}
}

static std::string ltrim(const std::string& s)
{
	size_t start = s.find_first_not_of(" \n\r\t\f\v");
	return (start == std::string::npos) ? "" : s.substr(start);
}

static std::string rtrim(const std::string& s)
{
	size_t end = s.find_last_not_of(" \n\r\t\f\v");
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

static std::string trim(const std::string& s)
{
	return rtrim(ltrim(s));
}

// If error is occured
// 1. set status code
// 2. throw t_client
// Don't need to setup status code message and body. 
// because it will be done in res_generator()

// If error is not occured
// 1. set method from request ex) req.method = "GET"
// 2. set path from request ex) req.path = "/index.html"
// 3. if body exist in req.raw, put body data into req.body

void parse_request_header(t_client &client, std::string header_sub)
{
	t_req &req = client.req;

	if (!header_sub.empty() && header_sub[0] == '\r' && header_sub[1] == '\n')
	{
		for(std::map<std::string, std::string>::iterator it = req.headers.begin(); it != req.headers.end(); ++it)
		{
			std::cout << it->first << "-" << it->second << "\n";
		}
		client.req.req_header_parsed = 2;
		if (req.version != "1.0" && req.headers.find("host") == req.headers.end())
			set_http_status(client, 400);
		if (req.headers.find("content-length") == req.headers.end() && req.headers.find("transfer-encoding") == req.headers.end()) 
				client.req_arrived = true;
		return;
	}

	if (header_sub.find(":") == std::string::npos || header_sub.find(":") == 0)
		return;
	std::string key = header_sub.substr(0, header_sub.find(":"));
	transform(key.begin(), key.end(), key.begin(), ::tolower);
	std::string value = header_sub.substr(header_sub.find(":") + 1);
	if (key.find_first_of(" \n\r\t\f\v") == 0 || key.find_first_of("\n\r\t\f\v") == (key.length() - 1))
	{
		set_http_status(client, 400);
		return;
	}

	value = trim(value);
	if(req.headers.find(key) != req.headers.end())
	{
		req.headers[key] += ",";
		req.headers[key] += value;
	}
	else
		req.headers[key] = value;


	// for(std::map<std::string, std::string>::iterator it = req.headers.begin();
	// it != req.headers.end(); ++it)
	// {
	// 	std::cout << it->first << "-" << it->second << "\n";
	// }
}

int convert(char* num)
{
	int len = ft_strlen(num);
	int base = 1;
	int temp = 0;
	for (int i = len - 1; i >= 0; i--)
	{
		if (num[i] >= '0' && num[i] <= '9')
		{
			temp += (num[i] - 48)*base;
			base = base * 16;
		}
		else if (num[i] >= 'A' && num[i] <= 'F')
		{
			temp += (num[i] - 55) * base;
			base = base*16;
		}
	}
	return temp;
}

int read_chunk_size(char *chunk_size, t_client &client)
{
	int len = ft_strlen(chunk_size);
	int num = 0;
	if (ft_strlen(chunk_size) == 3 && chunk_size[0] == '0' && chunk_size[0] == '\r' && chunk_size[0] == '\n')
		return 0;
	for(int i = 0; i < len - 2; i++)
	{
		if(!(chunk_size[i] >= '0' && chunk_size[i] <= '9') && !(chunk_size[i] >= 'a' && chunk_size[i] <= 'f'))
			return -1;
	}
	num = convert(chunk_size);
	return num;
}

int string_is_digit(std::string str)
{
	int i = 0;
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (0);
		i++;
	}
	return (1);
}

void parse_request_body(t_client &client)
{
	t_req& req = client.req;

	if (req.headers.find("transfer-encoding") != req.headers.end() && req.headers.find("content-length") != req.headers.end())
	{
		set_http_status(client, 400);
		client.req_arrived = true;
		client.req.req_body_parsed = 2;
		return;
	}
	if (req.headers.find("transfer-encoding") != req.headers.end())
	{
		if (client.req.chunk_size_read < 0 && req.raw.empty())
			return;
		if(req.chunk_size_read < 0)
		{
			client.req.chunk_size_read = read_chunk_size((char*)req.raw.substr(0, req.raw.find("\r\n")).c_str(), client);
			if (req.chunk_size_read < 0)
				set_http_status(client, 400);
			req.raw = req.raw.substr(req.raw.find("\r\n") + 2);
			return;
		}
		if (req.chunk_size_read >= req.raw.length() && req.raw.find("\r\n") == std::string::npos)
			continue;
		if (req.chunk_size_read != (req.raw.substr(0, req.raw.find("\r\n")).length()))
		{
			set_http_status(client, 400);
			return;
		}
		if (req.chunk_size_read == 0 && client.req.raw[0] == '\r' && client.req.raw[1] == '\n')
		{
			req.req_body_parsed = 2;
			client.req_arrived = true;
			return;
		}
		if (req.chunk_size_read > (req.raw.substr(0, req.raw.find("\r\n")).length()))
			req.body += req.raw.substr(0, req.raw.find("\r\n"));
		else
			req.body += req.raw.substr(0, req.chunk_size_read);
		req.raw = req.raw.substr(req.raw.find("\r\n") + 2);
		req.chunk_size_read = -1;
		if (req.raw.find("\r\n\r\n") != std::string::npos)
			req.raw = req.raw.substr(req.raw.find("\r\n\r\n") + 4);
		else
			req.raw = "";
	}
	else if (req.headers.find("content-length") != req.headers.end())
		{
			if (req.content_length == -1)
			{
				if (!string_is_digit(req.headers["content-length"]))
				{
					set_http_status(client, 400);
					req.req_body_parsed = 2;
					if (req.raw.find("\r\n\r\n") != std::string::npos)
						req.raw = req.raw.substr(req.raw.find("\r\n\r\n") + 4);
					else
						req.raw = ""; 
					return;
				}
				req.content_length = ft_atoi((char *)req.headers["content-length"].c_str());
			}
			if (req.content_length <= req.raw.size())
			{
				req.body += req.raw.substr(0, req.content_length);
				req.raw = req.raw.substr(req.content_length);
				req.content_length = 0;
				req.req_body_parsed = 2;
				client.req_arrived = true;
				return;
			}
			else if (req.content_length > req.raw.size())
			{
				req.body += req.raw;
				req.content_length -= req.raw.size();
				req.raw = "";
				return;
			}
		}
		else
		{
			set_http_status(client, 411);
			return;
		}
}

// void req_interpreter(t_client &client)
// {
// 	t_req &req = client.req;
// 	if (req.raw.length() == 2 && req.raw[0] == '\r' && req.raw[1] == '\n' && !req.req_line_parsed)
// 	{//empty line before the request line
// 		client.res.status_code = 123456;
// 		return;
// 	}
	
// 	if (!req.req_line_parsed)
// 	{
// 		parse_request_line((char*)req.raw.c_str(), client);
// 		if (client.res.status_code != 200 && client.res.status_code != 0)
// 			throw client;
// 		return;
// 	}

// 	if (!req.req_header_parsed)
// 	{
// 		if (client.req.raw.length() == 2 && client.req.raw[0] == '\r' && client.req.raw[1] == '\n')
// 		{
// 			req.req_header_parsed = 1;
// 			if (req.headers.find("content-length") == req.headers.end() && req.headers.find("transfer-encoding") == req.headers.end()) 
// 				client.req_arrived = true;
// 			return;
// 		}
// 		parse_request_header(client);
// 		if (client.res.status_code == 400)
// 			throw client;
// 		return;
// 	}

// 	if (!req.req_body_parsed)
// 	{
// 		if (req.headers.find("transfer-encoding") != req.headers.end())
// 		{
// 			if(req.chunk_size_read < 0)
// 			{
// 				client.req.chunk_size_read = read_chunk_size((char*)req.raw.c_str(), client);
// 				if (req.chunk_size_read < 0)
// 				{
// 					client.res.status_code = 400;
// 					throw client;
// 				}
// 				return;
// 			}
// 			if (req.chunk_size_read != (req.raw.length() - 2))
// 			{
// 				client.res.status_code = 400;
// 				throw client;
// 			}
// 			if (req.chunk_size_read == 0 && client.req.raw.length() == 2 && client.req.raw[0] == '\r' && client.req.raw[1] == '\n')
// 			{
// 				req.req_body_parsed = 1;
// 				client.req_arrived = true;
// 				return;
// 			}
// 			req.body += req.raw;
// 			req.chunk_size_read = -1;
// 		}
// 		else if (req.headers.find("content-length") != req.headers.end())
// 		{
// 			if (req.content_length == -1)
// 			{
// 				req.content_length = ft_atoi((char *)req.headers["content-length"].c_str());
// 				if (req.content_length < 0)
// 				{
// 					client.res.status_code = 400;
// 					throw client;
// 				}
// 			}
// 			req.content_length -= (req.raw.size() - 2);
// 			if (req.content_length < 0)
// 			{
// 				client.res.status_code = 400;
// 				throw client;
// 			}
// 			req.body += req.raw;
// 			if (req.content_length == 0 && (req.req_body_parsed = 1))
// 			{
// 				client.req_arrived = true;
// 			}
// 		}
// 		else
// 		{
// 			client.res.status_code = 411;
// 			throw client;
// 		}
// 	}

// 	// // for(std::map<std::string, std::string>::iterator it = req.headers.begin();
// 	// // it != req.headers.end(); ++it)
// 	// // {
// 	// // 	std::cout << it->first << "-" << it->second << "\n";
// 	// // }
// 	// if (body_start > 0)
// 	// 	req.body = req.raw.substr(body_start + 4);
// 	// free_tab(header);
// 	// client.res.status_code = 200;
// }