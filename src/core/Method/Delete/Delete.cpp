#include "Delete.hpp"
#include "ResponseBuilder.hpp"
#include "libftpp.hpp"

#include <sstream>
#include <unistd.h>

using namespace webserv::http;

std::string webserv::http::Delete::execute(const webserv::http::Request &req,
                                           const ServerConfig &server,
                                           const RouteConfig &route,
                                           webserv::core::Client &client) {
  (void)client;
  libftpp::debug::DebugLogger _logger("delete");
  int httpCode = 204;

  std::string fullPath =
      _getDeletePath(req.getPath(), server, route, httpCode, _logger);

  _logger << "Delete fullPath: " << fullPath << std::endl;

  if (httpCode != 204) {
    return _logger << "Delete failed. Error code: " << httpCode << std::endl,
           ResponseBuilder::generateError(httpCode, server);
  }
  if (!_deleteFile(fullPath, _logger)) {
    return _logger << "System Error: Could not delete file: " << fullPath
                   << std::endl,
           ResponseBuilder::generateError(500, server);
  }

  _logger << "File deleted successfully: " << fullPath << std::endl;

  std::ostringstream response;

  response << "HTTP/1.1 204 " << ResponseBuilder::getStatusMessage(204)
           << "\r\n";
  if (req.keepAlive())
    response << "Connection: keep-alive\r\n";
  else
    response << "Connection: close\r\n";
  response << "\r\n";

  return response.str();
}