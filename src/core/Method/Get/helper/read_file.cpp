#include "Get.hpp"
#include "ResponseBuilder.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <fstream>

using namespace webserv::http;

std::string webserv::http::Get::_readFile(const std::string &path) {
  std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
  if (!file.is_open())
    return "";

  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}