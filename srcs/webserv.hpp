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
# define MAX_BUFFER_SIZE 4096
# define MAX_CLIENT 500
# define CLIENT_TIMEOUT_SEC 30
# define SERVER_TIMEOUT_SEC 3
# define SERVER_TIMEOUT_USEC 0
# define SERVER_NAME "webserv/1.0"
# define OK 0
/* debug */
#define DEBUG(x) std::cerr << "\033[33m" << (x) << "\033[0m" << std::endl

typedef std::string route;

typedef struct s_location
{
	std::string root;
	std::string autoindex;
	std::vector<std::string> index;
	std::vector<std::string> allow;
	std::map<std::string, std::string> cgi;
	std::string abs_path;
	std::string upload_folder;
	size_t client_max_body_size;
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
	size_t client_max_body_size;
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
	std::string raw;
	/* request line */
	std::string req_line; // ex) GET /index.html HTTP/1.1
	std::string method;
	std::string path;
	std::string query_string;
	/* header */
	std::map<std::string, std::string> headers;
	/* body */
	std::string body;
	std::string body_fname;
	t_location *loc;
	int body_fd;
	bool is_cgi;
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
	std::string fname; 
	off_t content_length;
	bool is_cgi;
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
	/* debug */
	std::string id;
} t_client;

void free_tab(char **args, int length);
void parse_request_line(std::string request_line, t_client &client);
void parse_request_header(t_client &client, std::string header_sub);
void parse_request_body(t_client &client);

void req_interpreter(t_client &cli);
void res_generator(t_client &cli);
std::string mimetype(const std::string &extension);
int is_newline_char(char c);
void handle_get(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file);
void handle_put(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file);
void handle_post(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file);
void handle_options(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file);
void handle_trace(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file);
void handle_delete(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file);
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
std::string gmt_time_string(time_t &sec);
int isDirectory(const char *path);
std::string random_fname(void);
void set_http_status(t_client &client, int status);

// void handle_head(t_client &cli);
// void handle_post(t_client &cli);
// void handle_put(t_client &cli);
// void handle_delete(t_client &cli);
// void handle_connect(t_client &cli);
// void handle_options(t_client &cli);
// void handle_trace(t_client &cli);

#endif
