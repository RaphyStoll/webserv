#include "Delete.hpp"
#include "RouteMatcher.hpp"
#include "libftpp.hpp"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

using namespace webserv::http;

std::string
webserv::http::Delete::_getDeletePath(const std::string &reqPath,
                                      const ServerConfig &server,
                                      const RouteConfig &route, int &httpCode,
                                      libftpp::debug::DebugLogger _logger) {

  std::string effectiveRoot = RouteMatcher::getEffectiveRoot(server, route);

  std::string fullPath;
  if (reqPath.find(route.path) == 0) {
    std::string suffix = reqPath.substr(route.path.length());
    fullPath = libftpp::str::PathUtils::join(effectiveRoot, suffix);
  } else {
    fullPath = libftpp::str::PathUtils::join(effectiveRoot, reqPath);
  }

  _logger << "DEBUG: effectiveRoot=" << effectiveRoot << " reqPath=" << reqPath
          << " routePath=" << route.path << " fullPath=" << fullPath
          << std::endl;

  char resolvedRoot[PATH_MAX];
  char resolvedPath[PATH_MAX];

  if (realpath(effectiveRoot.c_str(), resolvedRoot) == NULL) {
    httpCode = 500;
    return "";
  }

  if (realpath(fullPath.c_str(), resolvedPath) == NULL) {
    httpCode = 404;
    return "";
  }

  std::string absRoot(resolvedRoot);
  std::string absPath(resolvedPath);
  if (absPath.find(absRoot) != 0) {
    httpCode = 403;
    _logger << "DEBUG: Directory Traversal detected (" << absPath << " vs "
            << absRoot << ")" << std::endl;
    return "";
  }

  if (libftpp::str::PathUtils::isDirectory(absPath)) {
    httpCode = 403;
    return "";
  }
  if (access(absPath.c_str(), W_OK) != 0) {
    httpCode = 403;
    return "";
  }

  return absPath;
}