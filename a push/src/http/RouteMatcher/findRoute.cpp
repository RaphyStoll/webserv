#include "RouteMatcher.hpp"

const RouteConfig &
webserv::http::RouteMatcher::findRoute(const std::string &reqPath,
                                       const ServerConfig &server) {
  libftpp::debug::DebugLogger _logger("RouteMatcher");
  _logger << "Finding route for path: " << reqPath << std::endl;
  const std::vector<RouteConfig> &routes = server.routes;

  const RouteConfig *bestMatch = NULL;
  size_t bestMatchLength = 0;

  for (size_t i = 0; i < routes.size(); ++i) {
    const std::string &routePath = routes[i].path;
    size_t len = routePath.length();

    if (reqPath.find(routePath) == 0) {

      bool exactMatch = (reqPath.length() == len);
      bool subFolderMatch = (reqPath.length() > len && reqPath[len] == '/');
      bool rootMatch = (len == 1 && routePath[0] == '/');

      if (exactMatch || subFolderMatch || rootMatch) {

        if (bestMatch == NULL || len > bestMatchLength) {
          bestMatch = &routes[i];
          bestMatchLength = len;
        }
      }
    }
  }

  if (bestMatch) {
    _logger << "Best route match found: " << bestMatch->path << std::endl;
    return *bestMatch;
  } else {
    _logger << "No matching route found, fallback to default." << std::endl;
    static const RouteConfig defaultFallbackRoute;
    return defaultFallbackRoute;
  }
}