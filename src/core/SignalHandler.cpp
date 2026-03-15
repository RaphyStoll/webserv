#include "SignalHandler.hpp"
#include <cstring>
#include <iostream>
#include <unistd.h>

namespace webserv {
volatile sig_atomic_t SignalHandler::_running = 1;

void SignalHandler::setup() {
  signal(SIGPIPE, SIG_IGN);

  struct sigaction sa;
  std::memset(&sa, 0, sizeof(sa));
  sa.sa_handler = _shutdownHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    std::cerr << "[SignalHandler] Failed to set SIGINT handler" << std::endl;
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    std::cerr << "[SignalHandler] Failed to set SIGTERM handler" << std::endl;
  }
}

bool SignalHandler::isRunning() { return _running != 0; }

void SignalHandler::requestShutdown() { _running = 0; }

void SignalHandler::_shutdownHandler(int signum) {
  (void)signum;
  const char msg[] =
      "\n[SignalHandler] Shutdown signal received, cleaning up...\n";
  ssize_t n = write(STDOUT_FILENO, msg, sizeof(msg) - 1);
  if (n)
    _running = 0;
}
} // namespace webserv
