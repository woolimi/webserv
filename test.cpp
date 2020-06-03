#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
int main(int argc, char const *argv[])
{
	int fd = open("./.gitignore", O_RDONLY);
	char buff[10001];
	int res = read(fd, buff, 10000);
	buff[res] = 0;
	std::cout << buff << std::endl;
	return 0;
}
