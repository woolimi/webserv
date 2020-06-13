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

int ft_pow(int x, unsigned int y) 
{ 
    if (y == 0) 
        return 1; 
    else if (y % 2 == 0) 
        return ft_pow(x, y / 2) * ft_pow(x, y / 2); 
    else
        return x * ft_pow(x, y / 2) * ft_pow(x, y / 2); 
} 

unsigned int hex2dec(std::string hex)
{
    unsigned int result = 0;
    for (int i=0; i<hex.length(); i++) {
		if (ft_isalpha(hex[i]))
			hex[i] = toupper(hex[i]);
        if (hex[i]>=48 && hex[i]<=57)
        {
            result += (hex[i]-48)*ft_pow(16,hex.length()-i-1);
        } else if (hex[i]>=65 && hex[i]<=70) {
            result += (hex[i]-55)*ft_pow(16,hex.length( )-i-1);
        } else if (hex[i]>=97 && hex[i]<=102) {
            result += (hex[i]-87)*ft_pow(16,hex.length()-i-1);
        }
    }
    return result;
}

void parse_query_string(t_client &client)
{
	std::string::size_type pos = 0;
	std::string::size_type pos2;
	
	pos = client.req.path.find("?");
	pos2 = client.req.path.find("/n");
	
	client.req.query_string = client.req.path.substr(pos + 1, pos2);
	client.req.path.replace(pos, pos2 - pos, "\n");
}

void uri_decode(t_client &client)
{
	std::string::size_type pos;
	std::string hex;
	if (client.req.path.find("?") != std::string::npos)
		parse_query_string(client);
	while(1)
	{
		pos = client.req.path.find("%");
		if (pos == std::string::npos)
			break;
		hex = client.req.path.substr(pos + 1, 2);
		char ascii_value = hex2dec(hex);
		client.req.path.replace(pos, pos + 3, std::string(1, ascii_value));
	}
}

void parse_request_line(char *request_line, t_client &client)
{

	int size = 0;
	std::set<std::string> methods = {"GET", "POST", "PUT", "OPTIONS", "HEAD", "DELETE", "TRACE", "CONNECT"};
	char **request_line_split = ft_split(request_line, " \t");
	std::cout <<"Request Line1: "<< request_line << std::endl;
	while (request_line_split[size] != 0)
		size++;
	if (size != 3)
	{
		std::cout << size << std::endl;
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
	uri_decode(client);
	
	// std::cout << "path: " << client.req.path << std::endl;
	// std::cout << "qs: " << client.req.query_string << std::endl;
	
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
			std::cout << it->first << "==" << it->second << "\n";
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
	num = hex2dec(chunk_size);
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
			req.chunk_size_read = read_chunk_size((char*)req.raw.substr(0, req.raw.find("\r\n")).c_str(), client);
			if (req.chunk_size_read < 0)
			{
				std::cout << req.raw;
				exit(0);
				set_http_status(client, 400);
			}
			req.raw = req.raw.substr(req.raw.find("\r\n") + 2);
			if (req.chunk_size_read == 0 && client.req.raw[0] == '\r' && client.req.raw[1] == '\n')
			{
				req.req_body_parsed = 2;
				client.req_arrived = true;
				return;
			}
			return;
		}

		if (req.chunk_size_read >= req.raw.length() && req.raw.find("\r\n") == std::string::npos)
			return;
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
		// else
		// 	req.raw = "";
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
