if [ $# -eq 1 ] && [ $1 = 'rm' ]; then
	sudo fuser -k 8000/tcp
	sudo fuser -k 80/tcp
else
	sudo ln -sf /home/user42/Desktop/mashar/projects/webserv/nginx/nginx.conf /etc/nginx/sites-enabled/default
	sudo nginx
fi
