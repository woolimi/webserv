#ifndef HTTP_HPP
# define HTTP_HPP

# include "webserv.hpp"

class HTTP
{
private:
	HTTP();

	std::vector<t_client> clients;
	std::vector<t_server> &servers;
	void init_timeout(struct timeval &timeout, int sec, int usec);
	void http_select(int fdmax, fd_set &read_set, fd_set &write_set, struct timeval &timeout);
	void manage_servers(fd_set &read_set, fd_set &init_set, std::set<int> &fds);
	void manage_clients(fd_set &read_set, fd_set &write_set, fd_set &init_set, std::set<int> &fds);
	void init_client(t_client &client);
	void disconnect(fd_set& init_set, std::set<int> &fds, std::vector<t_client>::iterator &it);
	void handle_methods(t_client &cli);

public:
	HTTP(std::vector<t_server> &srvs);
	// HTTP(HTTP const &other);
	// HTTP &operator=(HTTP const &other);
	~HTTP();

	void run();

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
