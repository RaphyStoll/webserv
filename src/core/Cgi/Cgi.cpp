#include "Cgi.hpp"
#include "Net.hpp"
#include "RouteMatcher.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <signal.h>
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
    killProcess();
  }
  _bodyBytesWritten = 0;
}

void Cgi::killProcess() {
  if (_pid > 0) {
    kill(_pid, SIGKILL);
    int status;
    waitpid(_pid, &status, WNOHANG);
    _pid = -1;
  }
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

  if (pipe(_pipeIn) == -1 || pipe(_pipeOut) == -1) {
    _logger << "CGI pipe creation failed" << std::endl;
    return false;
  }

  libftpp::net::set_non_blocking(_pipeIn[1]);
  libftpp::net::set_non_blocking(_pipeOut[0]);

  _pid = fork();
  if (_pid < 0) {
    _logger << "CGI fork failed" << std::endl;
    reset();
    return false;
  }

  if (_pid == 0) {
    // CHILD
    dup2(_pipeIn[0], STDIN_FILENO);
    dup2(_pipeOut[1], STDOUT_FILENO);

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

    std::string executable = route.cgi_path.empty() ? fullPath : route.cgi_path;

    std::vector<char *> args_vec;
    if (!route.cgi_path.empty()) {
      args_vec.push_back(const_cast<char *>(route.cgi_path.c_str()));
    }
    args_vec.push_back(const_cast<char *>(fullPath.c_str()));
    args_vec.push_back(NULL);

    _logger << "-----------------------------------" << std::endl;
    for (size_t i = 0; i < args_vec.size(); i++) {
      if (args_vec[i] != NULL) {
        _logger << "args_vec[" << i << "]: " << args_vec[i] << std::endl;
      }
    }
    _logger << "executable = " << executable << std::endl;

    for (size_t i = 0; i < env.size(); i++) {
      if (env[i] != NULL) {
        _logger << "env[" << i << "]: " << env[i] << std::endl;
      }
    }
    execve(executable.c_str(), args_vec.data(), env.data());
    _logger << "execve fail" << std::endl;
    perror("execve");
    _exit(1);
  }

  // PARENT
  close(_pipeIn[0]);
  _pipeIn[0] = -1;
  close(_pipeOut[1]);
  _pipeOut[1] = -1;

  _logger << "CGI process launched async with PID: " << _pid << std::endl;
  return true;
}
