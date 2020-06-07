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
# include <algorithm>
# include <vector>
# include <map>
# include <set>
# include <netinet/in.h>
# include <iostream>
# include "libft.h"

# define DEFAULT_CONF_NAME "webserv.conf"
# define MAX_BUFFER_SIZE 4096
# define MAX_CLIENT 100
# define CLIENT_TIMEOUT_SEC 1

typedef std::string route;
typedef std::string extension;
typedef std::string path;

typedef struct s_location
{
	std::string root;
	std::string autoindex;
	std::vector<std::string> index;
	std::vector<std::string> allow;
	std::map<extension, path> cgi;
} t_location;

typedef struct s_server
{
	/* run server */
	int socket;
	struct sockaddr_in addr;
	socklen_t addr_len;
	/* config */
	int listen;
	std::string server_name;
	std::string root;
	std::string error_page;
	int client_max_body_size;
	/* access with route ex) server[0].location["/"] */
	std::map<route, t_location> location;
} t_server;

typedef struct s_req
{
	int new_line;
	int req_line_parsed; // 0 - Not Started, 1 - Started, 2 - Complete
	int req_header_parsed;// 0 - Not Started, 1 - Started, 2 - Complete
	int req_body_parsed;// 0 - Not Started, 1 - Started, 2 - Complete
	int content_length;
	int chunk_size_read;
	std::string version;
	/* raw data */
	std::string raw; // 100 + 100
	/* request line */
	std::string req_line; // ex) GET /index.html HTTP/1.1
	std::string method;
	std::string path;
	/* header */
	std::map<std::string, std::string> headers;
	/* body */
	std::string body;
} t_req;

typedef struct s_res
{
	std::string raw;
	int status_code;
	std::string res_line; // ex) HTTP/1.1 400 Bad Request
	std::map<std::string, std::string> headers;
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
	int time_stamp;
} t_client;

void free_tab(char **args, int length);
void parse_request_line(char *request_line, t_client &client);
void parse_request_header(t_client &client, std::string header_sub);
void parse_request_body(t_client &client);

void req_interpreter(t_client &cli);
void res_generator(t_client &cli);
std::string mimetype(const std::string &extension);
void handle_get(t_client &cli);
int is_newline_char(char c);

// void handle_head(t_client &cli);
// void handle_post(t_client &cli);
// void handle_put(t_client &cli);
// void handle_delete(t_client &cli);
// void handle_connect(t_client &cli);
// void handle_options(t_client &cli);
// void handle_trace(t_client &cli);

#endif
