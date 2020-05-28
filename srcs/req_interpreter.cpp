#include "webserv.hpp"

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


void req_interpreter(t_req &req)
{
	(void)req;
	// need to get data from raw
	// int x = req.raw.find("\r\n", 18);
	// if (x >= 0)
	// {
	// 	std::cout <<x;	
	// 	std::cout << " :Body exists\n";
	// }
	// std::cout << "---------------" << std::endl;
	// std::cout << req.raw << std::endl;
	// std::cout << "---------------" << std::endl;
}