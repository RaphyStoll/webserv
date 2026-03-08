#include <climits>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "ConfigParser.hpp"
#include "Delete.hpp"
#include "Get.hpp"
#include "Post.hpp"
#include "ResponseBuilder.hpp"
#include "RouteMatcher.hpp"
#include "libftpp.hpp"

using namespace webserv::http;

std::string webserv::http::Post::execute(const webserv::http::Request &req,
                                         const ServerConfig &config,
                                         const RouteConfig &route,
                                         webserv::core::Client &client) {
  (void)client;
  libftpp::debug::DebugLogger _logger("post");
  int httpCode = 200;

  std::string effectiveRoute =
      webserv::http::RouteMatcher::getEffectiveRoot(config, route);

  std::string reqPath = req.getPath();
  std::string fullPath;

  if (reqPath.find(route.path) == 0) {
    std::string suffix = reqPath.substr(route.path.length());
    fullPath = libftpp::str::PathUtils::join(effectiveRoute, suffix);
  } else {
    fullPath = libftpp::str::PathUtils::join(effectiveRoute, reqPath);
  }

  _logger << "Post effectiveRoute: " << effectiveRoute << std::endl;
  _logger << "Post reqPath: " << reqPath << std::endl;
  _logger << "Post routePath: " << route.path << std::endl;
  _logger << "Post fullPath (fixed): " << fullPath << std::endl;

  if (libftpp::str::PathUtils::isDirectory(fullPath)) {
    if (route.cgi == false) {
      return _logger << "POST on directory without CGI forbidden" << std::endl,
             ResponseBuilder::generateError(403, config);
    }
  }

  if (route.cgi == true) {
    std::string ext = route.cgi_extension;

    if (fullPath.length() >= ext.length() &&
        fullPath.compare(fullPath.length() - ext.length(), ext.length(), ext) ==
            0) {
      _logger << "=====  CGI (POST)  =====" << std::endl;
      _logger << "cgi = " << req.getPath() << std::endl;

      if (client.getCgi().run(req, config, route)) {
        client.setExecutingCgi(true);
        return "";
      } else {
        return ResponseBuilder::generateError(500, config);
      }
    }
  }

  if (route.upload) {
    std::string uploadPath =
        _getUploadPath(req.getPath(), route, config, httpCode);
    if (uploadPath.empty()) {
      return _logger << "Upload path invalid or unauthorized. Error: "
                     << httpCode << std::endl,
             ResponseBuilder::generateError(httpCode, config);
    }

    _logger << "Saving POST data to: " << uploadPath << std::endl;

    bool writeSuccess = _writeFile(uploadPath, req);

    if (req.hasBodyTmpFile())
    {
      std::string tmpPath = req.getBodyTmpPath();
      if (!tmpPath.empty())
        unlink(tmpPath.c_str());
    }

    if (!writeSuccess) {
      return _logger << "System Error: Could not write file to " << uploadPath
                     << std::endl,
             ResponseBuilder::generateError(500, config);
    }

    _logger << "File successfully saved." << std::endl;

    std::ostringstream response;
    response << "HTTP/1.1 201 Created\r\n";
    response << "Location: " << req.getPath() << "\r\n";
    response << "Content-Type: text/plain\r\n";
    response << "Content-Length: 17\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << "Upload successful";

    return response.str();
  }

  return _logger << "POST Method not supported or upload not enabled for "
                 << req.getPath() << std::endl,
         ResponseBuilder::generateError(403, config);
}