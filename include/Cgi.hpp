#ifndef CGI_HPP
#define CGI_HPP

# include "libftpp.hpp"

#include "ConfigParser.hpp"
#include "Request.hpp"
#include <string>
#include <sys/types.h>
#include <vector>

namespace webserv {
namespace core {

class Cgi {
public:
  Cgi();
  ~Cgi();
  Cgi(const Cgi &other);
  Cgi &operator=(const Cgi &other);

  void reset();

  // Lance l'exécution. fork et execve. Retourne true si succès.
  bool run(const http::Request &req, const ServerConfig &config,
           const RouteConfig &route);

  int getPipeInWriteFd() const;
  void closePipeInWriteFd();
  int getPipeOutReadFd() const;
  pid_t getPid() const;

  size_t getBodyBytesWritten() const;
  void addBodyBytesWritten(size_t n);

private:
  pid_t _pid;
  size_t _bodyBytesWritten;
  int _pipeIn[2];  // [0] CGI lit, [1] Serveur écrit
  int _pipeOut[2]; // [0] Serveur lit, [1] CGI écrit

  std::vector<std::string> _buildEnvString(const http::Request &req,
                                           const ServerConfig &config,
                                           const RouteConfig &route) const;
};

} // namespace core
} // namespace webserv

#endif
