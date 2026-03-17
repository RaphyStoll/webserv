#include "ResponseBuilder.hpp"
#include "Delete.hpp"
#include "Get.hpp"
#include "Post.hpp"
#include "RouteMatcher.hpp"

#include <iostream>
#include <sstream>
#include <string>

using namespace webserv::http;

std::string
webserv::http::ResponseBuilder::build(const webserv::http::Request &req,
                                      const ServerConfig &config,
                                      webserv::core::Client &client) {
  libftpp::debug::DebugLogger _logger("ResponseBuilder");
  _logger << "Building response for " << req.getMethod() << " " << req.getPath()
          << std::endl;

  const RouteConfig &route = RouteMatcher::findRoute(req.getPath(), config);

  if (!route.redirect.empty()) {
    std::map<int, std::string>::const_iterator it = route.redirect.begin();
    int redirectCode = it->first;
    std::string redirectUrl = it->second;

    _logger << "Redirecting " << req.getPath() << " to " << redirectUrl
            << " with code " << redirectCode << std::endl;

    std::ostringstream response;
    response << "HTTP/1.1 " << redirectCode << " "
             << getStatusMessage(redirectCode) << "\r\n";
    response << "Location: " << redirectUrl << "\r\n";
    response << "Connection: close\r\n\r\n";
    return response.str();
  }

  if (!RouteMatcher::isMethodAllowed(req.getMethod(), route)) {
    _logger << "Method " << req.getMethod() << " not allowed for route "
            << req.getPath() << std::endl;
    return generateError(405, config);
  }

  std::string method = req.getMethod();

  if (method == "GET") {
    return Get::execute(req, config, route, client);
  } else if (method == "POST") {
    return Post::execute(req, config, route, client);
  } else if (method == "DELETE") {
    return Delete::execute(req, config, route, client);
  }

  // Méthode inconnue ou non gérée
  return generateError(501, config);
}

std::string
webserv::http::ResponseBuilder::generateError(int code,
                                              const ServerConfig &config) {
  libftpp::debug::DebugLogger _logger("ResponseBuilder");
  _logger << "Generating error response for code: " << code << std::endl;

  std::string msg = getStatusMessage(code);
  std::string body;

  bool customPageFound = false;

  std::map<int, std::string>::const_iterator it = config.error_pages.find(code);
  if (it != config.error_pages.end()) {
    std::string filePath = it->second;

    struct stat s;
    if (::stat(filePath.c_str(), &s) == 0 && !(s.st_mode & S_IFDIR) &&
        ::access(filePath.c_str(), R_OK) == 0) {
      body = _readFile(filePath);
      if (!body.empty()) {
        customPageFound = true;
      }
    }
  }

  // Fallback
  if (!customPageFound) {
    std::ostringstream oss;
    oss << "<!DOCTYPE html><html><head><title>" << code << " " << msg
        << "</title></head>";
    oss << "<body style=\"font-family: sans-serif; text-align: center; "
           "padding: 50px;\">";
    oss << "<h1>Errorr " << code << "</h1>";
    oss << "<h2>" << msg << "</h2>";
    oss << "<hr><p>webserv/1.0</p></body></html>";
    body = oss.str();
  }

  std::ostringstream response;
  response << "HTTP/1.1 " << code << " " << msg << "\r\n";
  response << "Content-Type: text/html\r\n";
  response << "Content-Length: " << body.length() << "\r\n";
  response << "Connection: close\r\n";
  response << "\r\n";
  response << body;

  return response.str();
}

// =============================================================================
// 3. HELPERS
// =============================================================================
