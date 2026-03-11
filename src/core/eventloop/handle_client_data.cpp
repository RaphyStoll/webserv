#include "Client.hpp"
#include "EventLoop.hpp"
#include "Request.hpp"
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"
#include "libftpp.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace libftpp::net;
using namespace webserv;

void webserv::core::EventLoop::_handle_client_data(int client_fd,
                                                   size_t poll_index) {
  _logger << "Handling client data on fd " << client_fd << std::endl;
  char buffer[4096];
  ssize_t bytes = ::read(client_fd, buffer, sizeof(buffer));

  if (bytes < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    }
    _logger << "[EventLoop] read error on fd " << client_fd << ": "
            << std::strerror(errno) << std::endl;
    _close_connection(client_fd, poll_index);
  } else if (bytes == 0) {
    _logger << "[EventLoop] Client disconnected (fd: " << client_fd << ")"
            << std::endl;
    _close_connection(client_fd, poll_index);
  } else {
    if (_clients.find(client_fd) == _clients.end())
      return;
    webserv::core::Client &client = _clients.at(client_fd);

    ::http::RequestParser &parser = client.getParser();
    ::http::RequestParser::State state =
        parser.parse(buffer, bytes, webserv::core::EventLoop::getConfig());

    if (state == http::RequestParser::COMPLETE) {
      _logger << "[EventLoop] Request complete on fd " << client_fd
              << std::endl;

      ::http::Request &req = parser.getRequest();
      _logger << "Method: " << req.getMethod() << " Path: " << req.getPath()
              << std::endl;

      const ServerConfig &srvConfig = _getServerConfig(client_fd, req);

      std::string responseData =
          webserv::http::ResponseBuilder::build(req, srvConfig, client);

      if (client.isExecutingCgi()) {
        _logger << "[EventLoop] CGI started on fd " << client_fd
                << ", registering pipes" << std::endl;
        int pipeOut = client.getCgi().getPipeOutReadFd();
        if (pipeOut != -1) {
          struct pollfd pfd;
          pfd.fd = pipeOut;
          pfd.events = POLLIN;
          pfd.revents = 0;
          _poll_fds.push_back(pfd);
          _cgiFds[pipeOut] = &client;
        }

        int pipeIn = client.getCgi().getPipeInWriteFd();
        if (pipeIn != -1) {
          struct pollfd pfd2;
          pfd2.fd = pipeIn;
          pfd2.events = POLLOUT;
          pfd2.revents = 0;
          _poll_fds.push_back(pfd2);
          _cgiFds[pipeIn] = &client;
        }

        client.appendResponse("HTTP/1.1 200 OK\r\nConnection: close\r\n");
        _poll_fds[poll_index].events = POLLIN | POLLOUT;
      } else if (!responseData.empty()) {
        client.appendResponse(responseData);
        _poll_fds[poll_index].events = POLLIN | POLLOUT;
      }

      if (!client.isExecutingCgi() && !req.keepAlive()) {
        client.reset();
      }

    } else if (state == http::RequestParser::ERROR) {
		int errorCode = parser.getErrorCode();
    	_logger << "[EventLoop] Parsing error on fd " << client_fd << " with code " << errorCode << std::endl;
    	const ServerConfig &srvConfig = _getServerConfig(client_fd, parser.getRequest());
    	std::string errorResponse = webserv::http::ResponseBuilder::generateError(errorCode, srvConfig);
		client.appendResponse(errorResponse);
    	_poll_fds[poll_index].events = POLLIN | POLLOUT;
    }
  }
}