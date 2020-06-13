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
# include <sys/wait.h>
# include <fcntl.h>
# include <string>
# include <string.h>
# include <algorithm>
# include <vector>
# include <map>
# include <set>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <iostream>
# include <dirent.h>
# include "libft.h"
# include "HttpStatus.hpp"

# define DEFAULT_CONF_NAME "webserv.conf"
# define MAX_BUFFER_SIZE 100
# define MAX_CLIENT 100
# define CLIENT_TIMEOUT_SEC 15
# define SERVER_NAME "webserv/1.0"
# define OK 0

typedef std::string route;

typedef struct s_location
{
	std::string root;
	std::string autoindex;
	std::vector<std::string> index;
	std::vector<std::string> allow;
	std::map<std::string, std::string> cgi;
	std::string abs_path;
	// cgi["extension"] = ".php";
	// cgi["paths"] = "/usr/bin/php-cgi";
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
	std::string query_string;
	/* header */
	std::map<std::string, std::string> headers;
	/* body */
	std::string body;
} t_req;

typedef struct s_res
{
	bool sent_head;
	// bool sent_body;
	int fd;
	int status_code;
	std::string res_line;
	std::map<std::string, std::string> headers;
	std::string head; // res_line + headers
	std::string body; // message body
	off_t content_length;
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
int is_newline_char(char c);
void handle_get(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file);
void handle_put(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file);
std::string int_to_hexstr(int n);
t_location *find_matched_location(t_server &serv, std::string &folder_path);
bool execute_cgi(t_client &cli, t_location &loc, char **env, std::string &real_path, std::string &ext);
void make_folder_list_res(t_client &cli, t_location *loc, std::string &uri_path, std::string &real_path);
void make_file_res(t_client &cli, t_location *loc, char **env, std::string &real_path, std::string &file);
int file_check(std::string file_path);
void renew_client_timestamp(t_client &cli);
bool send_res_body(t_client &cli);
bool send_res_head(t_client &cli);
void make_res_body_from_fd(t_client &cli);
std::string make_real_path(std::string &root, std::string &path);


// void handle_head(t_client &cli);
// void handle_post(t_client &cli);
// void handle_put(t_client &cli);
// void handle_delete(t_client &cli);
// void handle_connect(t_client &cli);
// void handle_options(t_client &cli);
// void handle_trace(t_client &cli);

#endif
