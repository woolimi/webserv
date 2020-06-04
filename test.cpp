#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

int main(int argc, char const *argv[])
{
	struct timeval tv;
	int timestamp = gettimeofday(&tv, NULL);
	std::cout << tv.tv_sec << std::endl;
	return 0;
}
