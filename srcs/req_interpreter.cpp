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

static void parse_request_line(char *request_line, t_client &client)
{
	int size = 0;
	std::set<std::string> methods = {"GET", "POST", "PUT", "OPTIONS", "HEAD", "DELETE", "TRACE", "CONNECT"};
	char **request_line_split = ft_split(request_line, " \t\n\r\v");
	while (request_line_split[size] != 0)
		size++;
	if (size != 3)
	{
		free_tab(request_line_split);
		client.res.status_code = 400;
		return;
	}
	client.req.method = request_line_split[0];
	if (methods.find(client.req.method) == methods.end())
	{
		free_tab(request_line_split);
		client.res.status_code = 400;
		return;
	}
	client.req.path = request_line_split[1];
	if ( client.req.path.find("/") != 0)
	{
		free_tab(request_line_split);
		client.res.status_code = 400;
		return;
	}
	client.req.version = request_line_split[2];
	free_tab(request_line_split);

	int x;
	if(((x = client.req.version.find("/")) != client.req.version.rfind("/")) || (client.req.version.find("/") == std::string::npos))
	{
		client.res.status_code = 400;
		return;
	}
	if((client.req.version.find(".") != client.req.version.rfind(".")) || (client.req.version.find(".") == std::string::npos))
	{
		client.res.status_code = 400;
		return;
	}

	if(client.req.version.substr(0, x) != "HTTP")
	{
		client.res.status_code = 400;
		return;
	}

	std::string vno = client.req.version.substr(client.req.version.find("/") + 1);
	if(vno.length() > 6)
	{
		client.res.status_code = 400;
		return;
	}

	if(vno[0] != '1')
	{
		client.res.status_code = 505;
		return;
	}
		
	for (int i = 1; i < vno.length(); i++)
	{
		if(!ft_isdigit((char)vno[i]) && vno[i] != '.')
		{
			client.res.status_code = 400;
			return;
		}
	}
	client.res.status_code = 200;
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

void req_interpreter(t_client &client)
{
	t_req &req = client.req;
	char **header;
	int i = 0;
	while (req.raw[i] && is_newline_char(req.raw[i]))
		i++;
	req.raw = req.raw.substr(i);
	std::string::size_type body_start = req.raw.find("\r\n\r\n");
	if (body_start == std::string::npos)
		body_start = req.raw.find("\n\n");
	if (body_start != std::string::npos)
		header = ft_split(req.raw.substr(0, body_start).c_str(), "\r\n");
	else
		header = ft_split(req.raw.c_str(),"\r\n");

	if(header == NULL || header[0] == NULL)
	{
		free_tab(header);
		client.res.status_code = 123456;
		return;
	}
	parse_request_line(header[0], client);
	if (client.res.status_code != 200)
	{
		free_tab(header);
		throw client;
	}

	for(int i = 1; header[i] != 0; i++)
	{
		std::string str(header[i]);
		if (str.find(":") == std::string::npos)
		{
			free_tab(header);
			client.res.status_code = 400;
			throw client;
		}

		if(str.find(":") == 0)
			continue;

		std::string key = str.substr(0, str.find(":"));
		transform(key.begin(), key.end(), key.begin(), ::tolower);
		std::string value = str.substr(str.find(":") + 1);
		if (key.find_first_of(" \n\r\t\f\v") == 0 || key.find_first_of("\n\r\t\f\v") == (key.length() - 1))
		{
			free_tab(header);
			client.res.status_code = 400;
			throw client;
		}

		value = trim(value);
		if(req.headers.find(key) != req.headers.end())
		{
			req.headers[key] += ",";
			req.headers[key] += value;
		}
		else
			req.headers[key] = value;
	}
	// for(std::map<std::string, std::string>::iterator it = req.headers.begin();
    // it != req.headers.end(); ++it)
	// {
    // 	std::cout << it->first << "-" << it->second << "\n";
	// }
	if (body_start > 0)
		req.body = req.raw.substr(body_start + 4);
	free_tab(header);
	client.res.status_code = 200;
}