#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>

#include "libftpp.hpp"

#define MAX_BODY_SIZE_LIMIT 5000000//(1024 * 1024)
#define MIN_BODY_SIZE_LIMIT 0
#define PORT_MIN 1024
#define PORT_MAX 65535
#define ERROR_CODE_MIN 400 //100-400 pour les code non error
#define ERROR_CODE_MAX 599

namespace ServerInfo
{
	const std::string SERVER_SOFTWARE = "webserv/1.0";
	const std::string GATEWAY_INTERFACE = "CGI/1.1";
}


enum ParseState
{
	GLOBAL,
	IN_SERVER,
	IN_ROUTE
};

class RouteConfig
{
	public :

	std::string path;
	std::string root;
	std::vector<std::string> methods;
	bool directory_listing;
	bool upload;
	std::string upload_path;
	bool cgi;
	std::string cgi_extension;
	std::string cgi_path;
	std::map<int, std::string> redirect;

	RouteConfig();

	public :

	void print(void) const;

};

class ServerConfig
{
	public :

	std::string server_name;
	std::string listen;
	int port;
	std::string root;
	std::string index;
	size_t max_body_size;
	std::map<int, std::string> error_pages;
	std::vector<RouteConfig> routes;
	bool autoindex;

	ServerConfig();

	void print(void) const;
};

typedef std::map<int, std::vector<ServerConfig> > NetworkConfig;

class DataConfig {

	public :

	DataConfig();
	~DataConfig();
	
	std::string config_path;
	NetworkConfig	cParser();

	private :

	size_t _i;
	ParseState _state;
	std::vector<std::string> _brutLine;
	std::vector<std::string> _token;
	ServerConfig _currentServer;
	RouteConfig _currentRoute;
	std::vector<ServerConfig> _servers;


	void 	openConf();
	void	tockenize();
	void	pars_state();

	void	state_global();
	void	state_server();
	void	state_route();

	void	dir_server();
	void	dir_route();

	void	ipp();
	void	valid_port();
	void	valid_max_body_size();
	void	valid_error_code();
	void	valid_path();
	void	valid_file();
	int		valid_ipv4();

};


#endif