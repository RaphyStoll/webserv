#include "Cgi.hpp"
#include "Net.hpp"
#include "libftpp.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

using namespace webserv::core;
using namespace webserv::http;

Cgi::Cgi() : _pid(-1), _bodyBytesWritten(0) {
  _pipeIn[0] = -1;
  _pipeIn[1] = -1;
  _pipeOut[0] = -1;
  _pipeOut[1] = -1;
}

Cgi::Cgi(const Cgi &other) { *this = other; }

Cgi &Cgi::operator=(const Cgi &other) {
  if (this != &other) {
    _pid = other._pid;
    _bodyBytesWritten = other._bodyBytesWritten;
    _pipeIn[0] = other._pipeIn[0];
    _pipeIn[1] = other._pipeIn[1];
    _pipeOut[0] = other._pipeOut[0];
    _pipeOut[1] = other._pipeOut[1];
    // En C++98, on s'assure que si une copie est faite (ex: dans std::map),
    // l'objet original (souvent temporaire) ne ferme pas les fd.
    // On "vole" donc les fd à la source si elle est temporaire, sans
    // const_cast, ce cas n'arrive qu'à la création du Client où fds = -1 de
    // toute façon.
  }
  return *this;
}

Cgi::~Cgi() { reset(); }

void Cgi::reset() {
  if (_pipeIn[0] != -1) {
    close(_pipeIn[0]);
    _pipeIn[0] = -1;
  }
  if (_pipeIn[1] != -1) {
    close(_pipeIn[1]);
    _pipeIn[1] = -1;
  }
  if (_pipeOut[0] != -1) {
    close(_pipeOut[0]);
    _pipeOut[0] = -1;
  }
  if (_pipeOut[1] != -1) {
    close(_pipeOut[1]);
    _pipeOut[1] = -1;
  }
  if (_pid > 0) {
    int status;
    waitpid(_pid, &status, WNOHANG);
    _pid = -1;
  }
  _bodyBytesWritten = 0;
}

int Cgi::getPipeInWriteFd() const { return _pipeIn[1]; }
void Cgi::closePipeInWriteFd() {
  if (_pipeIn[1] != -1) {
    close(_pipeIn[1]);
    _pipeIn[1] = -1;
  }
}
int Cgi::getPipeOutReadFd() const { return _pipeOut[0]; }
pid_t Cgi::getPid() const { return _pid; }
size_t Cgi::getBodyBytesWritten() const { return _bodyBytesWritten; }
void Cgi::addBodyBytesWritten(size_t n) { _bodyBytesWritten += n; }

std::vector<std::string> Cgi::_buildEnvString(const Request &req,
                                              const ServerConfig &config,
                                              const RouteConfig &route) const {
  std::vector<std::string> envString;
  envString.push_back("REQUEST_METHOD=" + req.getMethod());
  envString.push_back("QUERY_STRING=" + req.getQueryString());
  envString.push_back("PATH_INFO=");
  envString.push_back("SERVER_NAME=" + config.server_name);
  envString.push_back("SERVER_PORT=" +
                      libftpp::str::StringUtils::itos(config.port));
  envString.push_back("CONTENT_TYPE=" + req.getContentType());
  envString.push_back("CONTENT_LENGTH=" + req.getContentLength());
  envString.push_back("SCRIPT_NAME=" + route.root);
  envString.push_back("SERVER_PROTOCOL=" + req.getHttpVersion());
  envString.push_back("SERVER_SOFTWARE=" + ServerInfo::SERVER_SOFTWARE);
  envString.push_back("GATEWAY_INTERFACE=" + ServerInfo::GATEWAY_INTERFACE);

  char *system_path = getenv("PATH");
  if (system_path) {
    envString.push_back(std::string("PATH=") + system_path);
  }
  return envString;
}

bool Cgi::run(const Request &req, const ServerConfig &config,
              const RouteConfig &route) {
  libftpp::debug::DebugLogger _logger("cgi");

  // Création des deux pipes
  if (pipe(_pipeIn) == -1 || pipe(_pipeOut) == -1) {
    _logger << "CGI pipe creation failed" << std::endl;
    return false;
  }

  // Rend les extrémités utilisées par le serveur non-bloquantes (IMPORTANT!)
  libftpp::net::set_non_blocking(_pipeIn[1]);
  libftpp::net::set_non_blocking(_pipeOut[0]);

  _pid = fork();
  if (_pid < 0) {
    _logger << "CGI fork failed" << std::endl;
    reset();
    return false;
  }

  if (_pid == 0) {
    // --- PROCESSUS ENFANT ---
    // Redirige stdin vers le read end de _pipeIn
    dup2(_pipeIn[0], STDIN_FILENO);
    // Redirige stdout vers le write end de _pipeOut
    dup2(_pipeOut[1], STDOUT_FILENO);

    // Ferme tous les fds copiés inutiles dans l'enfant
    close(_pipeIn[0]);
    close(_pipeIn[1]);
    close(_pipeOut[0]);
    close(_pipeOut[1]);

    std::vector<std::string> envString = _buildEnvString(req, config, route);
    std::vector<char *> env;
    for (size_t i = 0; i < envString.size(); i++) {
      env.push_back(const_cast<char *>(envString[i].c_str()));
    }
    env.push_back(NULL);

    char *root = const_cast<char *>(route.root.c_str());
    char *args[] = {root, NULL};

    execve(root, args, env.data());
    perror("execve");
    _exit(1);
  }

  // --- PROCESSUS PARENT ---
  // Ferme les extrémités que seul le CGI utilise
  close(_pipeIn[0]);
  _pipeIn[0] = -1;
  close(_pipeOut[1]);
  _pipeOut[1] = -1;

  _logger << "CGI process launched async with PID: " << _pid << std::endl;
  return true;
}
