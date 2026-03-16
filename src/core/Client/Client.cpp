#include "Client.hpp"
#include "RequestParser.hpp"

#include <unistd.h>



using namespace webserv::core;
using namespace webserv;

webserv::core::Client::Client(int fd)
    : _logger("client"), _fd(fd), _fileFd(-1), _isChunked(false),
      _isExecutingCgi(false), _last_activity(0), _cgi_start_time(0) {
  _last_activity.touch(libftpp::time::Clock::now_ms());
}

webserv::core::Client::Client()
    : _logger("client"), _fd(-1), _fileFd(-1), _isChunked(false),
      _isExecutingCgi(false), _last_activity(0), _cgi_start_time(0) {}

webserv::core::Client::~Client() {
  if (_fileFd != -1) {
    close(_fileFd);
  }
}

int webserv::core::Client::getFd() const {
  _logger << "getFd called: " << _fd << std::endl;
  return _fd;
}

http::RequestParser &webserv::core::Client::getParser() { return _parser; }

libftpp::Buffer::Buffer &webserv::core::Client::getResponseBuffer() {
  return _response_buffer;
}

int webserv::core::Client::getFileFd() const { return _fileFd; }

void webserv::core::Client::setFileFd(int fd) { _fileFd = fd; }

bool webserv::core::Client::isChunked() const { return _isChunked; }

void webserv::core::Client::setChunked(bool chunked) { _isChunked = chunked; }

webserv::core::Cgi &webserv::core::Client::getCgi() { return _cgi; }
bool webserv::core::Client::isExecutingCgi() const { return _isExecutingCgi; }
void webserv::core::Client::setExecutingCgi(bool isExecuting) {
  if (!_isExecutingCgi && isExecuting) {
    markCgiStartTime();
  }
  _isExecutingCgi = isExecuting;
}

void webserv::core::Client::markCgiStartTime() {
  _cgi_start_time.touch(libftpp::time::Clock::now_ms());
}

void webserv::core::Client::updateLastActivity() {
  _logger << "Updating last activity for fd: " << _fd << std::endl;
  _last_activity.touch(libftpp::time::Clock::now_ms());
}

bool webserv::core::Client::hasTimedOut(
    unsigned long long now_ms, unsigned long long timeout_limit) const {
  return _last_activity.elapsed_since(now_ms) >= timeout_limit;
}

bool webserv::core::Client::hasCgiTimedOut(
    unsigned long long now_ms, unsigned long long cgi_timeout_limit) const {
  return _isExecutingCgi && _cgi_start_time.elapsed_since(now_ms) >= cgi_timeout_limit;
}

void webserv::core::Client::appendResponse(const std::string &data) {
  _logger << "Appending " << data.size()
          << " bytes to response buffer for fd: " << _fd << std::endl;
  _response_buffer.append(data);
}

bool webserv::core::Client::hasResponseToSend() const {
  _logger << "Checking if response to send for fd: " << _fd << ": "
          << (!_response_buffer.empty()) << std::endl;
  return !_response_buffer.empty();
}

void webserv::core::Client::clearResponseBuffer() {
  _logger << "Clearing response buffer for fd: " << _fd << std::endl;
  _response_buffer.clear();
}

void webserv::core::Client::reset() {
  _logger << "Resetting client on fd: " << _fd << std::endl;
  _response_buffer.clear();
  _isChunked = false;
  _isExecutingCgi = false;
  _cgi.reset();
  if (_fileFd != -1) {
    close(_fileFd);
    _fileFd = -1;
  }
  _parser.reset();
  updateLastActivity();
}