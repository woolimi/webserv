#ifndef HTTP_HPP
# define HTTP_HPP

# include "webserv.hpp"

class HTTP
{
private:
	static std::list<t_client> clients;
	static std::list<t_server> servers;
	void init_timeout(struct timeval &timeout, int sec, int usec);
	void http_select(int fdmax, fd_set &read_set, fd_set &write_set, struct timeval &timeout);
	void manage_servers(fd_set &read_set, fd_set &init_set, std::set<int> &fds);
	void manage_clients(fd_set &read_set, fd_set &write_set, fd_set &init_set, std::set<int> &fds, char **env);
	void init_client(t_client &client);
	void disconnect(fd_set &init_set, std::set<int> &fds, std::list<t_client>::iterator &it);
	bool handle_methods(t_client &cli, char **env);
	void skip_leading_empty_line(t_client &cli, char *buffer, size_t nb_read);
	void reset_req_and_res(t_client &cli);
	bool check_client_timeout(t_client &cli);
	void res_too_many_requests(t_client &cli);

public:
	HTTP();
	HTTP(std::list<t_server> &srvs);
	// HTTP(HTTP const &other);
	// HTTP &operator=(HTTP const &other);
	~HTTP();

	void run(char **env);
	std::list<t_client> &get_clients();
	std::list<t_server> &get_servers();
	/* exceptions */
	class FailToSetServerSocket : public std::exception
	{
		virtual const char *what() const throw();
	};
	class FailToSetClientSocket : public std::exception
	{
		virtual const char *what() const throw();
	};
	class FailToSelect : public std::exception
	{
		virtual const char *what() const throw();
	};
	class FailToAccept : public std::exception
	{
		virtual const char *what() const throw();
	};
};

#endif
