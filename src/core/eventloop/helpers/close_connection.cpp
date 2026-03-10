#include "../../../include/EventLoop.hpp"
#include "../../../lib/LIBFTPP/include/Net.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace libftpp::net;
using namespace webserv;

void webserv::core::EventLoop::_close_connection(int fd, size_t poll_index) {
  if (_clients.find(fd) != _clients.end()) {
    webserv::core::Client &client = _clients.at(fd);
    if (client.isExecutingCgi()) {
      int pipeOut = client.getCgi().getPipeOutReadFd();
      int pipeIn = client.getCgi().getPipeInWriteFd();

      if (pipeOut != -1) {
        _cgiFds.erase(pipeOut);
        for (size_t k = 0; k < _poll_fds.size(); ++k) {
          if (_poll_fds[k].fd == pipeOut) {
            _poll_fds[k] = _poll_fds.back();
            _poll_fds.pop_back();
            if (k == poll_index)
              poll_index = _poll_fds.size() -
                           1;
            else if (k < poll_index)
              poll_index--;

            break;
          }
        }
      }
      if (pipeIn != -1) {
        _cgiFds.erase(pipeIn);
        for (size_t k = 0; k < _poll_fds.size(); ++k) {
          if (_poll_fds[k].fd == pipeIn) {
            _poll_fds[k] = _poll_fds.back();
            _poll_fds.pop_back();
            if (k == poll_index)
              poll_index = _poll_fds.size() - 1;
            else if (k < poll_index)
              poll_index--;
            break;
          }
        }
      }
    }
  }

  close(fd);

  if (poll_index < _poll_fds.size()) {
    _poll_fds[poll_index] = _poll_fds.back();
  }
  _poll_fds.pop_back();
  _clients.erase(fd);

  _logger << "[EventLoop] Connection closed (fd: " << fd << ")" << std::endl;
}