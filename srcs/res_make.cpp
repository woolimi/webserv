#include "webserv.hpp"

void make_res_body_for_trace(t_client &cli)
{
	
}


void make_res_body_from_fd(t_client &cli)
{
	t_res &res = cli.res;
	char buff[MAX_BUFFER_SIZE + 1];
	int nb_read = read(res.fd, buff, MAX_BUFFER_SIZE);
	if (nb_read == 0)
	{
		// printf("making res body finished\n");
		if (res.headers.find("Transfer-Encoding") != res.headers.end())
			res.body += "0\r\n\r\n";
		else
		{
			cli.res_sent = true;
			res.body = "";
		}
	}
	else if (nb_read < 0)
	{
		// if (nb_read == 0)
		// 	std::cout << "her2e\n";
		if (res.headers.find("Transfer-Encoding") != res.headers.end())
			res.body += "0\r\n\r\n";
	}
	else
	{
		buff[nb_read] = 0;
		if (res.headers.find("Transfer-Encoding") != res.headers.end())
		{
			res.body += int_to_hexstr(nb_read) + "\r\n";
			res.body.insert(res.body.end(), buff, buff + nb_read);
			res.body += "\r\n";
		}
		else
			res.body.insert(res.body.end(), buff, buff + nb_read);
	}
}

void make_file_res(t_client &cli, t_location *loc, char **env, std::string &file_path, std::string &file)
{
	t_res &res = cli.res;
	struct stat info;

	if ((cli.res.status_code = file_check(file_path)) != OK)
		return ;
	// file info
	res.fd = open(file_path.c_str(), O_RDONLY);
	fstat(res.fd, &info);
	size_t file_size = info.st_size;
	std::string ext = "";
	if (file.find(".") != std::string::npos)
		ext = file.substr(file.find_last_of('.'));
	// make res.head
	if (!loc->cgi.empty() && loc->cgi["extension"] == ext)
	{
		if (!execute_cgi(cli, *loc, env, file_path, ext))
			return;

		// calculate content length
		char buff[MAX_BUFFER_SIZE + 1];
		int ret = read(res.fd, buff, MAX_BUFFER_SIZE);
		buff[ret] = 0;
		std::string raw = buff;

		// inherit header + more header
		size_t pos;
		std::string tmp;
		while ((pos = raw.find("\r\n")) != std::string::npos)
		{
			tmp = raw.substr(0, pos);
			raw.erase(0, pos + 2);
			if (tmp == "")
				break;
			pos = tmp.find(": ");
			res.headers[tmp.substr(0, pos)] = tmp.substr(pos + 2);
		}
		res.headers["Transfer-Encoding"] = "chunked";
		res.body += int_to_hexstr(raw.size()) + "\r\n";
		res.body += raw + "\r\n";
	}
	else
	{
		struct stat st;
		res.content_length = lseek(res.fd, 0, SEEK_END);
		lseek(res.fd, 0, SEEK_SET);
		fstat(res.fd, &st);
		res.headers["Content-Type"] = mimetype(ext);
		res.headers["Content-Length"] = std::to_string(res.content_length);
		res.headers["Last-Modified"] = gmt_time_string(st.st_mtim.tv_sec);
	}
	res.status_code = 200;
}

void make_folder_list_res(t_client &cli, t_location *loc, std::string &uri_path, std::string &real_path)
{
	char buff[256];
	t_res &res = cli.res;
	DIR *dp = NULL;
	struct dirent *entry = NULL;
	std::set<std::string> fnames;
	struct stat info;
	std::string orig_path = getcwd(buff, 256);

	// int fd;
	// int flag = 0;
	// std::vector<std::string>::iterator it1;
	// for (it1 = loc->index.begin(); it1 != loc->index.end(); ++it1)
	// {
	// 	std::cout << "filenm: " << uri_path + *it1 << std::endl;
	// 	if ((fd = open((uri_path + *it1).c_str(), O_RDONLY)) >= 0)
	// 	{
	// 		flag = 1;
	// 		close(fd);
	// 		break;
	// 	}
	// }
	// if (flag == 0)
	// {
	// 	res.status_code = 404;
	// 	return;
	// }
	if (chdir(real_path.c_str()) < 0 || !(dp = opendir("./")))
	{
		res.status_code = 404;
		return;
	}
	res.body += "<html>\n";
	res.body += "\t<head>\n";
	res.body += "\t\t<title>Index of " + uri_path + "</title>\n";
	res.body += "\t</head>\n";
	res.body += "\t<body bgcolor=\"white\">\n";
	res.body += "\t\t<h1>Index of " + uri_path + "</h1>\n";
	res.body += "\t<hr>\n\t<pre>\n";

	while ((entry = readdir(dp)) != NULL)
	{
		stat(entry->d_name, &info);
		if (ft_strlen(entry->d_name) == 1 && entry->d_name[0] == '.')
			continue;
		if (S_ISDIR(info.st_mode))
			fnames.insert(std::string(entry->d_name) + "/");
		else
			fnames.insert(std::string(entry->d_name));
	}
	
	std::vector<std::string>::iterator it1;
	for (it1 = loc->index.begin(); it1 != loc->index.end(); ++it1)
	{
		if(fnames.find(*it1) != fnames.end())
			break;
	}
	if (it1 == loc->index.end())
	{
		res.status_code = 404;
		return;
	}

	std::set<std::string>::iterator it;
	for (it = fnames.begin(); it != fnames.end(); ++it)
	{
		stat(it->c_str(), &info);
		time_t t = info.st_mtime;
		struct tm *lctime = localtime(&t);
		strftime(buff, sizeof(buff), "%d-%h-%Y %H:%M", lctime);
		res.body += "<a href=\"" + *it + "\">" + *it + "</a>";
		int nb_tab = (8 - it->size() / 7);
		while (nb_tab-- > 0)
			res.body += "\t";
		res.body += buff;
		res.body += "\t\t";
		res.body += std::to_string(info.st_size);
		res.body += "\n";
	}
	closedir(dp);
	res.status_code = 200;
	res.body += "\t</pre>\n\t<hr>\n\t</body>\n</html>\n";
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = std::to_string(res.body.size());
	res.content_length = res.body.size();
}