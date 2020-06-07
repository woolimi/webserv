#include "webserv.hpp"

std::string int_to_hexstr(int n)
{
	std::string charset = "0123456789abcdef";
	std::string ret;

	while (n >= 16)
	{
		ret.insert(0, 1, charset[n % 16]);
		n /= 16;
	}
	ret.insert(0, 1, charset[n % 16]);
	return ret;
}