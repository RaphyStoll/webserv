#ifndef GET_METHOD_HPP
#define GET_METHOD_HPP

#include "Client.hpp"
#include "ConfigParser.hpp"
#include "Request.hpp"
# include "libftpp.hpp"

#include <string>

namespace webserv {
namespace http {

class Get {
public:
  static std::string execute(const webserv::http::Request &req,
                             const ServerConfig &config,
                             const RouteConfig &route,
                             webserv::core::Client &client);

private:
  // s'assure que le chemin soit viable (pas de ../.. etc)
  static std::string _getSecurePath(const std::string &root,
                                    const std::string &reqPath, int &httpCode);

  // Gère le cas des dossiers (cherche index.html)
  // Retourn false si erreur/interdit.
  static bool _checkIndexFile(std::string &fullPath, int &httpCode,
                              const ServerConfig &config);

  // Construit la string finale HTTP 200 OK
  static std::string _createSuccessResponse(const std::string &content,
                                            const std::string &path,
                                            bool keepAlive = false);

			// helper
			static std::string _readFile(const std::string& path);
			static std::string _getContentType(const std::string& path);

			// auto index
			static std::string _displayAutoIndex(const std::string &dir, const std::string &requestPath);
		};

} // namespace http
} // namespace webserv

#endif
