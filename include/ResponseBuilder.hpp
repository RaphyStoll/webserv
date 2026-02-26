#ifndef RESPONSEBUILDER_HPP
#define RESPONSEBUILDER_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include <sys/stat.h>
#include <unistd.h>

#include "Client.hpp"
#include "Request.hpp"
#include "libftpp.hpp"

namespace webserv {
namespace http {
class ResponseBuilder {
public:
  static std::string build(const http::Request &req, const ServerConfig &config,
                           webserv::core::Client &client);

  // Générateur d'erreur complet
  static std::string generateError(int code, const ServerConfig &config);

  // Helper recupere le message du code (ex: 200 -> "OK")
  static std::string getStatusMessage(int code);

private:
  // Helper interne pour lire un fichier (utilisé pour les pages d'erreur
  // custom)
  static std::string _readFile(const std::string &path);
};

} // namespace http
} // namespace webserv

#endif