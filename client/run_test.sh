rm WEBSERV.res
rm NGINX.res

clang++ -Wall -Werror -Wextra webserv_destructor.cpp -o webserv_destructor
./webserv_destructor

diff NGINX.res WEBSERV.res
