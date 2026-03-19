#include <string>
#include <cstring>
#include <climits>

#include <stdlib.h>

#include "Post.hpp"
#include "RouteMatcher.hpp"

using namespace webserv::http;

	// 1. trouver la racine du serveur pour avoir path abs
	// 2. build chemin cible
	// 3. check si ../.. si oui les enlever si non ok
	// 4. check si dossier = interdit, il faut que ce soit un fichier
	// 5. check si dossier parent as les droit W_OK
	
	// ex: si /uploads/images/img.png
	// 1. build ./uploads/images/img.png ou autre si sous dossier dans la config
	// 2. si post ../../uploads/images/img.png -> uploads/images/img.png ou erreur?
	// 3. si post uploads/images = dossier = error: bad request
	// 4. check si uploads et images ont les droit W_OK

	// si erreur return ""
	// si reussi return full path


std::string webserv::http::Post::_getUploadPath(const std::string& reqPath, const RouteConfig& route, const ServerConfig& server, int& httpCode) {
    
    std::string targetDir;

    if (route.upload && !route.upload_path.empty()) {
        targetDir = route.upload_path;
    } 
    else {
        targetDir = RouteMatcher::getEffectiveRoot(server, route);
    }

    char resolvedRoot[PATH_MAX];
    if (realpath(targetDir.c_str(), resolvedRoot) == NULL) {
        httpCode = 500;
        return "";
    }
    std::string absRoot = resolvedRoot;

    size_t lastSlash = reqPath.find_last_of('/');
    std::string filename = (lastSlash != std::string::npos) ? reqPath.substr(lastSlash + 1) : reqPath;

    if (filename.empty()) {
        httpCode = 400;
        return "";
    }

    std::string fullPath = absRoot + "/" + filename;

    return fullPath;
}
