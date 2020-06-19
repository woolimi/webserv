if [ $# -eq 1 ] && [ $1 = 'rm' ]; then
	sudo fuser -k 8000/tcp
else
	sudo ln -sf /home/wpark/Documents/webserv/nginx/nginx.conf /etc/nginx/sites-enabled/default
	sudo nginx
fi