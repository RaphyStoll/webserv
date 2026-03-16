#ifndef CLIENT_HPP
#define CLIENT_HPP

# include "libftpp.hpp"
#include "Cgi.hpp"
#include "RequestParser.hpp"

#include <string>

#define NEW_CONNECTION_TIMEOUT 1000 * 60 // 1000 = 1s
#define CGI_TIMEOUT 1000 * 30

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

  // Gestion CGI
  webserv::core::Cgi &getCgi();
  bool isExecutingCgi() const;
  void setExecutingCgi(bool isExecuting);
  void markCgiStartTime();

  // Gestion du Timeout
  void updateLastActivity();
  bool hasTimedOut(unsigned long long now_ms,
                   unsigned long long timeout_limit) const;
  bool hasCgiTimedOut(unsigned long long now_ms,
                      unsigned long long cgi_timeout_limit) const;

  // Gestion des données
  void appendResponse(const std::string &data);
  bool hasResponseToSend() const;
  void clearResponseBuffer();

private:
  mutable libftpp::debug::DebugLogger _logger;
  int _fd;
  int _fileFd;
  bool _isChunked;
  bool _isExecutingCgi;
  webserv::core::Cgi _cgi;
  webserv::http::RequestParser _parser;
  libftpp::time::Timeout _last_activity;
  libftpp::time::Timeout _cgi_start_time;
  libftpp::Buffer::Buffer _response_buffer;
};

} // namespace core
} // namespace webserv

#endif
