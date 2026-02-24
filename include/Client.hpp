#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "ConfigParser.hpp"
#include "RequestParser.hpp"
#include "libftpp.hpp"
#include <string>

namespace webserv {
namespace core {

class Client {
public:
  // Constructeurs / Destructeur
  Client(int fd);
  Client();
  ~Client();
  void reset();

  // Getters
  int getFd() const;
  webserv::http::RequestParser &getParser();
  libftpp::Buffer::Buffer &getResponseBuffer();
  int getFileFd() const;
  void setFileFd(int fd);
  bool isChunked() const;
  void setChunked(bool chunked);

  // Gestion du Timeout
  void updateLastActivity();
  bool hasTimedOut(unsigned long long now_ms,
                   unsigned long long timeout_limit) const;

  // Gestion des données
  void appendResponse(const std::string &data);
  bool hasResponseToSend() const;
  void clearResponseBuffer();

private:
  mutable libftpp::debug::DebugLogger _logger;
  int _fd;
  int _fileFd;
  bool _isChunked;
  webserv::http::RequestParser _parser;
  libftpp::time::Timeout _last_activity;
  libftpp::Buffer::Buffer _response_buffer;
};

} // namespace core
} // namespace webserv

#endif
