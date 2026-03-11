#include <cstring>
#include <stdexcept>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "../../../include/BootStrap.hpp"

using namespace libftpp::net;
using namespace libftpp::str;
using namespace webserv;

void BootStrap::_setup_sockets() 
	{
		NetworkConfig::const_iterator it;
		for (it = _config.begin(); it != _config.end(); ++it) {
			int port = it->first;

			if (it->second.empty()) continue; 

			_bind_servers_for_port(port, it->second);
		}

		if (_listen_sockets.empty()) {
			_logger << "[BootStrap] no server avalible shuthdown server"  << std::endl;
			throw std::runtime_error("No sockets could be initialized. Check availability of addresses/ports.");
		}
	}
