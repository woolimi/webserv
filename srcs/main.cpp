/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <wpark@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/05/19 14:02:48 by wpark             #+#    #+#             */
/*   Updated: 2020/05/28 04:18:17 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <cerrno>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <set>
#include <utility>
#include <iostream>


#include "ConfigParser.hpp"
#include "HTTP.hpp"
#include "webserv.hpp"

int main(int ac, char **av, char **env)
{
	(void)env;
	try
	{
		// get config file param. If not exist, use default config
		ConfigParser conf(ac, av);

		// set server
		// HTTP http(conf.servers());
		// run server
		// http.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what();
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}