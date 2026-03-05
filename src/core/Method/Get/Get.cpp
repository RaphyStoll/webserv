#include <climits>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Get.hpp"
#include "Request.hpp"
#include "ResponseBuilder.hpp"
#include "RouteMatcher.hpp"
#include "libftpp.hpp"

using namespace webserv::http;

std::string webserv::http::Get::execute(const webserv::http::Request &req,
                                        const ServerConfig &config,
                                        const RouteConfig &route,
                                        webserv::core::Client &client) {
  (void)client;
  libftpp::debug::DebugLogger _logger("get");
  int httpCode = 200;
  (void)httpCode; // FIXME : pas use pour l'instant mais comme la fontion n'est
                  // pas fini je le laisse

  std::string effectiveRoute =
      webserv::http::RouteMatcher::getEffectiveRoot(config, route);

  std::string reqPath = req.getPath();
  std::string fullPath;

  if (reqPath.find(route.path) == 0) {
    std::string suffix = reqPath.substr(route.path.length());
    fullPath = libftpp::str::PathUtils::join(effectiveRoute, suffix);
  } else {
    // Cas fallback
    fullPath = libftpp::str::PathUtils::join(effectiveRoute, reqPath);
  }

  _logger << "Get effectiveRoute: " << effectiveRoute << std::endl;
  _logger << "Get fullPath: " << fullPath << std::endl;

  if (fullPath.find(effectiveRoute) != 0)
    return _logger << fullPath << "find" << effectiveRoute << "failed"
                   << std::endl,
           ResponseBuilder::generateError(403, config);

  if (!libftpp::str::PathUtils::exists(fullPath))
    return _logger << fullPath << " don't exist" << std::endl,
           ResponseBuilder::generateError(404, config);

  // index et auto index
  if (libftpp::str::PathUtils::isDirectory(fullPath)) {

    std::string indexName;
    if (config.index.empty()) {
      _logger << indexName << "= empty Fallback to index.htm" << std::endl;
      indexName = "index.html";

    } else {
      _logger << indexName << "= " << config.index << std::endl;
      indexName = config.index;
    }

    std::string indexPath = libftpp::str::PathUtils::join(fullPath, indexName);
    if (libftpp::str::PathUtils::exists(indexPath))
      fullPath = indexPath;
    else {
      if (route.directory_listing) {
        std::string htmlContent = _displayAutoIndex(fullPath, req.getPath());
        if (htmlContent.empty())
          return ResponseBuilder::generateError(500, config);
        return _createSuccessResponse(htmlContent, "index.html");
      }
      return _logger << "no route.directory_listing for auto index"
                     << std::endl,
             ResponseBuilder::generateError(403, config);
    }
  }
  if (route.cgi == true) {
    std::string ext = route.cgi_extension;

    if (fullPath.length() >= ext.length() &&
        fullPath.compare(fullPath.length() - ext.length(), ext.length(), ext) ==
            0) {
      _logger << "=====  CGI (GET)  =====" << std::endl;
      _logger << "cgi = " << req.getPath() << std::endl;

      if (client.getCgi().run(req, config, route)) {
        client.setExecutingCgi(true);
        return "";
      } else {
        return ResponseBuilder::generateError(500, config);
      }
    }
  }
  if (access(fullPath.c_str(), R_OK) != 0) {
    return _logger << "acces(fullPAth) | " << fullPath
                   << "dont have permittion for Read" << std::endl,
           ResponseBuilder::generateError(403, config);
  }

  _logger << "===== static file =====" << std::endl;
  std::string content = _readFile(fullPath);
  struct stat s;
  stat(fullPath.c_str(), &s);
  if (content.empty() && s.st_size > 0) {
    return _logger << content << "is empty or s.st_size > 0" << std::endl,
           ResponseBuilder::generateError(500, config);
  }

  return _createSuccessResponse(content, fullPath);
}

// std::string webserv::http::Get::execute(const http::Request& req, const
// ServerConfig& config, const RouteConfig& route)
// {
// 	(void)route;
// 	int httpCode = 200;
// 	std::string content = "";
// 	std::string fullPath = "";

// 	if (httpCode != 200)
// 		return ResponseBuilder::generateError(httpCode, config);

// 	//   http://37.59.120.163:9003/search?query=webserv   //SDU
// 	for(size_t i = 0; i < config.routes.size(); i++) //SDU CGI, verifier ou
// il faut le positionner
// 	{
// 		if(req.getPath() == config.routes[i].path)
// 		{
// 			fullPath = "cgi.html";

// 			content = execute_cgi(req, config, i);
// 			return _createSuccessResponse(content, fullPath);
// 		}
// 	}
// 	// TODO RAPH: modif getSecurePath pour aller avec cgi
// 	// remplir full path avec vrai path cgi et descendre le block en haut ->
// en bas 	fullPath = _getSecurePath(config.root, req.getPath(),
// httpCode);//SDU : trop restrictif
// 	//std::cerr << "fullPath = " <<  fullPath << " code = " << httpCode <<
// std::endl; 	struct stat s; 	if (stat(fullPath.c_str(), &s) == 0 &&
// (s.st_mode & S_IFDIR)) { 		if (!_checkIndexFile(fullPath, httpCode,
// config)) {
// 			// TODO: RAPH
// 			// Ici, plus tard : quelque cgose comme if
// (config.autoindex) return AutoIndex::gen(...); 			return
// ResponseBuilder::generateError(httpCode, config);
// 		}
// 	}

// 	if (access(fullPath.c_str(), R_OK) != 0)
// 	{
// 		return ResponseBuilder::generateError(403, config);
// 	}
// 	content = _readFile(fullPath);
// 	if (content.empty() && s.st_size > 0)
// 		return ResponseBuilder::generateError(500, config);

// 	return _createSuccessResponse(content, fullPath);
// }

std::string
webserv::http::Get::_displayAutoIndex(const std::string &path,
                                      const std::string &requestPath) {
  DIR *dir = opendir(path.c_str());
  if (!dir)
    return ""; // error 500

  std::ostringstream html;
  html << "<html><head><title>Index of " << requestPath << "</title></head>\n";
  html << "<body><h1>Index of " << requestPath << "</h1>\n";
  html << "<hr><pre>\n";

  if (requestPath != "/")
    html << "<a href=\"../\">..</a>\n";

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    std::string name = entry->d_name;
    if (name == "." || name == "..")
      continue;

    std::string fullPath = path + "/" + name;
    struct stat st;
    stat(fullPath.c_str(), &st);

    if (S_ISDIR(st.st_mode))
      name += "/";

    html << "<a href=\"" << name << "\">" << name << "</a>\n";
    html << "\n";
  }
  closedir(dir);
  html << "</pre><hr>";
  html << "<script>function a() { setInterval(bg, "
          "250);alert(\"hehe..\");function bg() {var col =\"#\" + "
          "((Math.random() * 0xffffff) << 0).toString(16).padStart(6, \"0\"); "
          "document.body.style.backgroundColor = col; }}</script >";
  html << "<button onclick=\"a()\">Free "
          "wallets !!!</button>";
  html << "</body></html>";
  return html.str();
}
