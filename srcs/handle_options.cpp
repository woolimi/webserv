#include "webserv.hpp"

static std::string vector_to_string_with_delimitter(std::vector<std::string> v, std::string delimit_string)
{
    std::vector<std::string>::iterator it;
    std::string res = "";

    for (it = v.begin(); it != (v.end() - 1); ++it)
    {
        res += *it;
        res += delimit_string;
    }
    res += *it;
    return res;
}

void handle_options(t_client &cli, char **env, t_location *loc, bool is_file, std::string folder_path, std::string file)
{
    /*
        1. find the loc
            a. exact match
            b. what if the resource doesnt exist? for now, setting it to root

        2. set the Allow header
    */
   std::string &path = cli.req.path;
   t_server &serv = cli.server;
   std::map<route, t_location>::iterator it;
   int set = 0;

   for (it = serv.location.begin(); it != serv.location.end(); ++it)
   {
       if ((it->first == path) || (it->first == path + "/")) //100% match
		{
	        cli.res.headers["Allow: "] = vector_to_string_with_delimitter(it->second.allow, ", ");
            set++;
            break;
		}
   }
    if (set == 0)
        cli.res.headers["Allow: "]  = vector_to_string_with_delimitter(serv.location["/"].allow, ", ");
    cli.res.status_code = 200;
}