#include <cerrno>
#include <cstring>
#include <string>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "EventLoop.hpp"

using namespace webserv;

void webserv::core::EventLoop::_handle_poll_events() {
  for (size_t i = 0; i < _poll_fds.size(); ++i) {

    if (_poll_fds[i].revents == 0)
      continue;

    int fd = _poll_fds[i].fd;
    short revents = _poll_fds[i].revents;

    _logger << "[EventLoop] Event detected on fd " << fd
            << ", revents: " << revents << std::endl;

    std::map<int, webserv::core::Client>::iterator it_client =
        _clients.find(fd);
    bool is_known_client = (it_client != _clients.end());

    if (is_known_client) {
      it_client->second.updateLastActivity();
    }

    if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
      _close_connection(fd, i);
      i--;
      continue;
    }

    if (revents & POLLIN) {
      bool is_listener = false;
      for (size_t j = 0; j < _listen_sockets.size(); ++j) {
        if (fd == _listen_sockets[j]) {
          is_listener = true;
          break;
        }
      }

      if (is_listener)
        _accept_new_connection(fd);
      else {
        _handle_client_data(fd, i);
        if (_clients.find(fd) == _clients.end()) {
          continue;
        }
      }
    }

    if (revents & POLLOUT) {
      std::map<int, webserv::core::Client>::iterator current_it =
          _clients.find(fd);

      if (current_it != _clients.end()) {
        webserv::core::Client &client = current_it->second;
        libftpp::Buffer::Buffer &respBuffer = client.getResponseBuffer();

        if (respBuffer.empty() && client.getFileFd() != -1) {
          char buf[8192];
          ssize_t read_bytes = read(client.getFileFd(), buf, sizeof(buf));
          if (read_bytes > 0) {
            if (client.isChunked()) {
              char hex[32];
              int hex_len = snprintf(hex, sizeof(hex), "%zx\r\n", read_bytes);
              respBuffer.append(hex, hex_len);
              respBuffer.append(buf, read_bytes);
              respBuffer.append("\r\n", 2);
            } else {
              respBuffer.append(buf, read_bytes);
            }
          } else if (read_bytes == 0) {
            if (client.isChunked()) {
              respBuffer.append("0\r\n\r\n", 5);
            }
            close(client.getFileFd());
            client.setFileFd(-1);
            client.setChunked(false);
          } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
              _logger << "[EventLoop] File read error on fd "
                      << client.getFileFd() << ": " << std::strerror(errno)
                      << std::endl;
              _close_connection(fd, i);
              i--;
              continue;
            }
          }
        }

        if (!respBuffer.empty()) {
          ssize_t bytes = ::send(fd, respBuffer.data(), respBuffer.size(), 0);

          if (bytes < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
              _logger << "[EventLoop] Send error: " << std::strerror(errno)
                      << std::endl;
              _close_connection(fd, i);
              i--;
              continue;
            }
          } else {
            respBuffer.consume(bytes);
          }
        }

        if (respBuffer.empty() && client.getFileFd() == -1) {
          _poll_fds[i].events = POLLIN;

          _logger << "[EventLoop] Response sent fully to " << fd << std::endl;
          _close_connection(fd, i);
          i--;
          continue;
        }
      }
    }
  }
}