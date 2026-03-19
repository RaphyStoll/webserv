#ifndef DELETE_METHOD_HPP
#define DELETE_METHOD_HPP

#include "Client.hpp"
#include "ConfigParser.hpp"
#include "Request.hpp"
#include "libftpp.hpp"

#include <string>

namespace webserv {
namespace http {
class Delete {
public:
  // main function de del execute la methode http
  static std::string execute(const webserv::http::Request &req,
                             const ServerConfig &server,
                             const RouteConfig &route,
                             webserv::core::Client &client);

private:
  // calcul le path a del et check les droit
  static std::string _getDeletePath(const std::string &reqPath,
                                    const ServerConfig &server,
                                    const RouteConfig &route, int &httpCode,
                                    libftpp::debug::DebugLogger _logger);

  // del le ficher
  static bool _deleteFile(const std::string &path,
                          libftpp::debug::DebugLogger _logger);
};

} // namespace http
} // namespace webserv

#endif