#include "EventLoop.hpp"
#include <signal.h>
#include <sys/types.h>

using namespace webserv;

void webserv::core::EventLoop::_check_timeouts() {
  unsigned long long now = libftpp::time::Clock::now_ms();

  static unsigned long long last_check = 0;
  if (now - last_check < 1000) {
    return;
  }
  last_check = now;

  for (size_t i = 0; i < _poll_fds.size();) {
    int fd = _poll_fds[i].fd;

    std::map<int, webserv::core::Client>::iterator it = _clients.find(fd);

    if (it != _clients.end()) {
      webserv::core::Client &client = it->second;

      if (client.hasTimedOut(now, 60000)) {
        _logger << "[EventLoop] Timeout expired for fd " << fd
                << " - Closing connection." << std::endl;
        _close_connection(fd, i);
        continue;
      }

      if (client.hasCgiTimedOut(now, 60000)) { // Assuming CGI timeout is also 60s
        _logger << "[EventLoop] CGI Timeout expired for fd " << fd
                << " - Killing CGI process." << std::endl;
        if (client.getCgi().getPid() > 0) {
          kill(client.getCgi().getPid(), SIGKILL);
        }
        // TODO: Optionally send a 504 Gateway Timeout response here before closing,
        // or let the CGI pipe close handle it depending on design.
        _close_connection(fd, i);
        continue;
      }
    }

    i++;
  }
}