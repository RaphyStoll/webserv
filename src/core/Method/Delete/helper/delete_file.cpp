#include "Delete.hpp"
#include "ResponseBuilder.hpp"
#include "RouteMatcher.hpp"
#include "libftpp.hpp"

#include <sstream>
#include <unistd.h>
#include <cstdio>

using namespace webserv::http;

bool webserv::http::Delete::_deleteFile(const std::string& path) {
    if (std::remove(path.c_str()) == 0) {
        return true;
    }
    return false;
}