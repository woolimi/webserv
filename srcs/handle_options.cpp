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

	(void)env;
	(void)is_file;
	(void)file;
	(void)folder_path;

    std::map<route, t_location>::iterator it;

    cli.res.headers["Allow: "] = vector_to_string_with_delimitter(loc->allow, ", ");
    cli.res.status_code = 200;
}