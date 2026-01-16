#include"ConfigParser.hpp"

ServerConfig::ServerConfig() :
	listen(""),
	port(-1),
	root(""),
	index(""),
	max_body_size(0),
	error_pages(),
	routes()
{
}

RouteConfig::RouteConfig() :
	path(""),
	root(""),
	methods(),
	directory_listing(false),
	upload(false),
	upload_path(""),
	cgi(false),
	cgi_extention(""),
	cgi_path("")
{
}