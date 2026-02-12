#include "methods/Delete.hpp"
#include "ResponseBuilder.hpp"
#include "RouteMatcher.hpp"
#include "libftpp.hpp"

#include <sstream>
#include <unistd.h>
#include <cstdio>

using namespace webserv::http;

std::string webserv::http::Delete::_getDeletePath(const std::string& reqPath, const ServerConfig& server, const RouteConfig& route, int& httpCode) {

	std::string effectiveRoot = RouteMatcher::getEffectiveRoot(server, route);
	std::string fullPath = libftpp::str::PathUtils::join(effectiveRoot, reqPath);

	if (fullPath.find(effectiveRoot) != 0) {
		httpCode = 403
		return "";
	}
	if (!libftpp::str::PathUtils::exists(fullPath)) {
		httpCode = 404;
		return "";
	}
	if (libftpp::str::PathUtils::isDirectory(fullPath)) {
		httpCode = 403; // Forbidden
		return "";
	}
	if (access(fullPath.c_str(), W_OK) != 0) {
		httpCode = 403;
		return "";
	}

	return fullPath;
}