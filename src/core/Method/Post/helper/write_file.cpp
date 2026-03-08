#include "Post.hpp"

#include <fstream>
#include <string>

bool webserv::http::Post::_writeFile(const std::string& path, const Request& req)
{
	std::ofstream file(path.c_str(), std::ios::binary | std::ios::trunc);
	if (!file.is_open()) {
		return false;
	}

	if (req.hasBodyTmpFile()) {
		std::ifstream src(req.getBodyTmpPath().c_str(), std::ios::binary);
		if (!src.is_open()) {
			file.close();
			return false;
		}

		char buffer[8192];
		while (src.good()) {
			src.read(buffer, sizeof(buffer));
			std::streamsize readBytes = src.gcount();
			if (readBytes > 0)
				file.write(buffer, readBytes);
			if (file.fail()) {
				src.close();
				file.close();
				return false;
			}
		}

		if (!src.eof()) {
			src.close();
			file.close();
			return false;
		}
		src.close();
	} else {
		file << req.getBody();
		if (file.fail()) {
			file.close();
			return false;
		}
	}

	file.close();
	return true;
}
