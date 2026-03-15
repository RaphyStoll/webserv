#include <cerrno>
#include <cstring>
#include <string>
#include <cstdio>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "EventLoop.hpp"

using namespace webserv;

namespace {

bool writeCgiBodyFromTmpFile(int pipeFd, webserv::core::Cgi &cgi,
                             const webserv::http::Request &req,
                             libftpp::debug::DebugLogger &logger) {
  int tmpFd = open(req.getBodyTmpPath().c_str(), O_RDONLY);
  if (tmpFd < 0) {
    logger << "[EventLoop] Failed to open request tmp file: "
           << req.getBodyTmpPath() << std::endl;
    return false;
  }

  char chunk[8192];
  off_t offset = static_cast<off_t>(cgi.getBodyBytesWritten());
  ssize_t readBytes = pread(tmpFd, chunk, sizeof(chunk), offset);
  close(tmpFd);

  if (readBytes > 0) {
    ssize_t written = write(pipeFd, chunk, static_cast<size_t>(readBytes));
    if (written > 0)
      cgi.addBodyBytesWritten(static_cast<size_t>(written));
    else if (written < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
      logger << "[EventLoop] CGI write error: " << std::strerror(errno)
             << std::endl;
      return false;
    }
    return true;
  }

  if (readBytes == 0)
    return true;

  if (errno != EAGAIN && errno != EWOULDBLOCK) {
    logger << "[EventLoop] CGI tmp read error: " << std::strerror(errno)
           << std::endl;
    return false;
  }
  return true;
}

} // namespace

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

    std::map<int, webserv::core::Client *>::iterator it_cgi = _cgiFds.find(fd);
    bool is_cgi_fd = (it_cgi != _cgiFds.end());

    if (is_known_client) {
      it_client->second.updateLastActivity();
    } else if (is_cgi_fd) {
      it_cgi->second->updateLastActivity();
    }

    if (is_cgi_fd) {
      webserv::core::Client *client = it_cgi->second;
      webserv::core::Cgi &cgi = client->getCgi();

      if ((revents & POLLIN) && fd == cgi.getPipeOutReadFd()) {
        char buf[8192];
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n > 0) {
          client->appendResponse(std::string(buf, n));
          for (size_t k = 0; k < _poll_fds.size(); ++k) {
            if (_poll_fds[k].fd == client->getFd()) {
              _poll_fds[k].events |= POLLOUT;
              break;
            }
          }
        } else if (n == 0) {
          cgi.reset();
          client->setExecutingCgi(false);
          _cgiFds.erase(fd);
          _poll_fds.erase(_poll_fds.begin() + i);
          i--;
          for (size_t k = 0; k < _poll_fds.size(); ++k) {
            if (_poll_fds[k].fd == client->getFd()) {
              _poll_fds[k].events |= POLLOUT;
              break;
            }
          }
          continue;
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
          _logger << "[EventLoop] CGI read error: " << std::strerror(errno)
                  << std::endl;
        }
      }

      if ((revents & POLLOUT) && fd == cgi.getPipeInWriteFd()) {
        const webserv::http::Request &req = client->getParser().getRequest();
        const std::string &body = req.getBody();
        size_t written = cgi.getBodyBytesWritten();

        if (req.hasBodyTmpFile()) {
          if (!writeCgiBodyFromTmpFile(fd, cgi, req, _logger)) {
            cgi.closePipeInWriteFd();
            _cgiFds.erase(fd);
            _poll_fds.erase(_poll_fds.begin() + i);
            i--;
            continue;
          }

          int tmpFd = open(req.getBodyTmpPath().c_str(), O_RDONLY);
          if (tmpFd < 0) {
            _logger << "[EventLoop] Failed to reopen request tmp file: "
                    << req.getBodyTmpPath() << std::endl;
            cgi.closePipeInWriteFd();
            _cgiFds.erase(fd);
            _poll_fds.erase(_poll_fds.begin() + i);
            i--;
            continue;
          }
          char eofProbe;
          ssize_t probe = pread(tmpFd, &eofProbe, 1,
                                static_cast<off_t>(cgi.getBodyBytesWritten()));
          close(tmpFd);
          if (probe == 0) {
            cgi.closePipeInWriteFd();
            _cgiFds.erase(fd);
            _poll_fds.erase(_poll_fds.begin() + i);
            i--;
          }
          continue;
        }

        if (written < body.size()) {
          ssize_t n = write(fd, body.c_str() + written, body.size() - written);
          if (n > 0) {
            cgi.addBodyBytesWritten(n);
          } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            _logger << "[EventLoop] CGI write error: " << std::strerror(errno)
                    << std::endl;
          }
        }
        if (cgi.getBodyBytesWritten() >= body.size()) {
          cgi.closePipeInWriteFd();
          _cgiFds.erase(fd);
          _poll_fds.erase(_poll_fds.begin() + i);
          i--;
        }
        continue;
      }

      if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
        cgi.reset();
        client->setExecutingCgi(false);
        _cgiFds.erase(fd);
        _poll_fds.erase(_poll_fds.begin() + i);
        i--;
        for (size_t k = 0; k < _poll_fds.size(); ++k) {
          if (_poll_fds[k].fd == client->getFd()) {
            _poll_fds[k].events |= POLLOUT;
            break;
          }
        }
      }
      continue;
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
          if (!client.isExecutingCgi()) {
            _poll_fds[i].events = POLLIN;

            _logger << "[EventLoop] Response sent fully to " << fd << std::endl;
            _close_connection(fd, i);
            i--;
            continue;
          } else {
            _poll_fds[i].events = POLLIN;
          }
        }
      }
    }
  }
}