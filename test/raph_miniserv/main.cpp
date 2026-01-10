#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include <fstream>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../../lib/LIBFTPP/include/libftpp.hpp"

struct Config {
	std::string	host;
	int			port;
	std::string	root;
	std::string	index;
	std::map<std::string, std::string> routes; //path -> files
};

static bool eventLoop(Config *config);
static bool bootStrap(Config *config, int *server_fd);
static bool openFileAndParseConfig(const std::string &s, Config *config);
static void just_print(std::string s);
static void just_print_error(std::string s);

int main () 
{
	Config config;
	std::string config_folder = "config";
	std::string config_file = "config.conf";
	std::string config_path = config_folder + "/" + config_file;
	int server_fd = -1; // default
	if (!openFileAndParseConfig(config_path, &config))
		return (just_print_error("open_file_and_parse_config failed"), 1);
	if (!bootStrap(&config, &server_fd)) {
		if (server_fd != -1)
			close(server_fd);
		return (just_print_error("bootStrap failed"), 1);
	}
	if (!eventLoop(&config, &server_fd))
		return (just_print_error("event_loop failed"), 1);
	return 0;
}

static void just_print_error(std::string s)
{
	std::cerr << s << std::endl;
}

static void just_print(std::string s)
{
	std::cerr << s << std::endl;
}

static bool openFileAndParseConfig(const std::string &s, Config *config)
{
	std::fstream file(s.c_str());
	if (!file.is_open()) {
		just_print_error("Cannot open " + s);
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty())
			return(just_print_error("getline failed"), false);
		std::vector<std::string> split = libftpp::str::StringUtils::split(line, ' ', 0);
		if (split.size() < 2)
			return(just_print_error("split failed"), false);
		std::string key = libftpp::str::StringUtils::trim(split[0]);
		std::string value = libftpp::str::StringUtils::trim(split[1]);

		if (libftpp::str::StringUtils::iequals(key, "listen"))
			config->host = value;
		else if (libftpp::str::StringUtils::iequals(key, "port"))
			config->port = libftpp::str::StringUtils::stoi(value);
		else if (libftpp::str::StringUtils::iequals(key, "root"))
			config->root = value;
		else if (libftpp::str::StringUtils::iequals(key, "index"))
			config->index = value;
		else if (libftpp::str::StringUtils::iequals(key, "route")) {
			std::string path = libftpp::str::StringUtils::trim(split[1]);
			std::string file = libftpp::str::StringUtils::trim(split[2]);
			config->routes[path] = file;
			//just_print(path + " -> " + config.routes[path]);
		}
		else {
			std::string build_msg = "key not found : " + value;
			just_print_error(build_msg);
			return false;
		}
	}
	return true;
}

static bool bootStrap(Config *config, int *server_fd)
{
	*server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (*server_fd < 0) {
		perror("socket");
		return false;
	}
	int yes = 1;
	if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("setsockopt");
		return false;
	}
	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(config->port);
	if (inet_pton(AF_INET, config->host.c_str(), &addr.sin_addr) < 0) {
		perror("inet_pton");
		return false;
	}

	if (bind(*server_fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return false;
	}

	if (listen(*server_fd, SOMAXCONN) < 0) {
		perror("listen");
		return false;
	}
	just_print("Listening on http://" + config->host + ":" + libftpp::str::StringUtils::itos(config->port));
	return true;
}

static bool eventLoop(Config *config, )
{
	while(true)
	{

	}
	return true;
}

