#include <fstream>
#include <sstream>
#include <climits>
#include <cstdlib>
#include <vector>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "Post.hpp"
#include "ResponseBuilder.hpp"
#include "libftpp.hpp"
#include "Request.hpp"
#include "RouteMatcher.hpp"

using namespace webserv::http;

std::string webserv::http::Post::execute(const webserv::http::Request& req, const ServerConfig& config, const RouteConfig& route)
{
	libftpp::debug::DebugLogger _logger("post");
	int httpCode = 200;
	(void)httpCode; // FIXME : pas use pour l'instant mais comme la fontion n'est pas fini je le laisse (pareil sur get)

	std::string effectiveRoute = webserv::http::RouteMatcher::getEffectiveRoot(config, route);
	
	std::string reqPath = req.getPath();
	std::string fullPath;
	
	if (reqPath.find(route.path) == 0) {
		std::string suffix = reqPath.substr(route.path.length());
		fullPath = libftpp::str::PathUtils::join(effectiveRoute, suffix);
	} else {
		// Cas fallback si jamais ça match pas (peu probable si RouteMatcher a bien fait son taff)
		fullPath = libftpp::str::PathUtils::join(effectiveRoute, reqPath);
	}

	_logger << "Post effectiveRoute: " << effectiveRoute << std::endl;
	_logger << "Post reqPath: " << reqPath << std::endl;
	_logger << "Post routePath: " << route.path << std::endl;
	_logger << "Post fullPath (fixed): " << fullPath << std::endl;


	if (libftpp::str::PathUtils::isDirectory(fullPath)) {
		if (route.cgi == false) {
			// TODO: Pour l'instant, on rejette le POST sur un dossier sans CGI.
			return _logger << "POST on directory without CGI forbidden" << std::endl,
				ResponseBuilder::generateError(403, config);
		}
	}

	if (route.cgi == true) {
		std::string ext = route.cgi_extension;
		
		if (fullPath.length() >= ext.length() && 
			fullPath.compare(fullPath.length() - ext.length(), ext.length(), ext) == 0) 
		{
			_logger << "===== CGI (POST) =====" << std::endl;
			_logger << "cgi = " << req.getPath() << std::endl;

			// TODO SDU: cgi ici Call
			// std::string output = execute_cgi(req, config, 0); 
			// return output;
			
			return "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nCGI POST OUTPUT (TODO)"; 
		}
	}

	if (req.getBody().size() > config.max_body_size) {
		return ResponseBuilder::generateError(413, config);
	}
	_logger << "Writing to file: " << fullPath << std::endl;
	
	std::ofstream file(fullPath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!file.is_open())
		return _logger << "Could not open file for writing: " << fullPath << std::endl,
			ResponseBuilder::generateError(500, config);

	file.write(req.getBody().c_str(), req.getBody().size());
	file.close();

	int responseCode = 201;
	std::ostringstream response;
	response << "HTTP/1.1 " << responseCode << " " << ResponseBuilder::getStatusMessage(responseCode) << "\r\n";
	if (responseCode == 201) {
		response << "Location: " << req.getPath() << "\r\n";
	}
	response << "Content-Length: 0\r\n";
	response << "Connection: close\r\n\r\n";
	
    return response.str();
}