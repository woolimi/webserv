#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <errno.h>
# include <cstdio>
# include <cstdlib>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <sys/time.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <string>
# include <string.h>
# include <vector>
# include <map>
# include <set>
# include <netinet/in.h>
# include <iostream>

# define DEFAULT_CONF "/home/wpark/Documents/webserv/webserv.conf"
# define MAX_BUFFER_SIZE 100

typedef std::string route;
typedef struct s_location
{
	std::string root;
	std::string autoindex;
	std::string index;
	std::string allow;
} t_location;

typedef struct s_server
{
	int socket;
	struct sockaddr_in addr;
	socklen_t addr_len;
	int listen;
	std::string server_name;
	std::string root;
	std::string eror_page;
	int client_max_body_size;
	/* access with route ex) server[0].location["/"] */
	std::map<route, t_location> location;
} t_server;

typedef struct s_req
{
	/* server info */
	t_server server;
	/* raw data */
	std::string raw;
	/* request line */
	std::string method;	 // essential
	std::string path;	 // essential
	std::string version; // essential
	/* header */
	std::string host; // essential
	std::string accept;
	/* body */
	std::string body;
} t_req;

typedef struct s_res
{
	std::string raw;
	/* respond line */
	std::string version;
	std::string status_code;
	std::string status_msg;
	/* header */
	std::string accept_charsets;
	std::string accept_language;
	std::string allow;
	std::string authorization;
	std::string content_language;
	std::string content_length;
	std::string content_location;
	std::string content_type;
	std::string date;
	std::string host; // essential
	std::string last_modified;
	std::string location;
	std::string referer;
	std::string server;
	std::string transfer_encoding;
	std::string user_agent;
	std::string www_authenticate;
	/* body */
	std::string body;
} t_res;

typedef struct s_client
{
	int socket;
	bool req_arrived;
	bool res_sent;
	struct sockaddr_in addr;
	socklen_t addr_len;
	t_req req;
	t_res res;
	t_server server;
} t_client;

void req_interpreter(t_req &req);
void res_generator(t_req &req, t_res &res, t_server &server);

#endif
