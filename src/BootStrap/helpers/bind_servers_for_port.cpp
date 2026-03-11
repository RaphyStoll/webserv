#include "../../../include/BootStrap.hpp"

using namespace webserv;

void BootStrap::_bind_servers_for_port(int port, const std::vector<ServerConfig>& servers)
{
	std::vector<std::string> binded_hosts;

	for (size_t i = 0; i < servers.size(); ++i) {
		std::string host = servers[i].listen;
		if (host.empty())
			host = "0.0.0.0";

		bool already_binded = false;
		for (size_t j = 0; j < binded_hosts.size(); ++j) {
			if (binded_hosts[j] == host) {
				already_binded = true;
				break;
			}
		}

		if (already_binded)
			continue;

		try {
			int sock_fd = _create_listener_socket(port, host);
			_listen_sockets.push_back(sock_fd);
			binded_hosts.push_back(host);

			std::cout << "Listening on http://" << host << ":" << port << std::endl;
			_logger << "[BootStrap] Listening on http://" << host << ":" << port << " (fd: " << sock_fd << ")" << std::endl;
		} catch (const std::exception& e) {
			std::cerr << "Error: Could not bind port " << port << " on host " << host << ": " << e.what() << std::endl;
			_logger << "[BootStrap] Failed to bind port " << port << " on host " << host << ": " << e.what() << std::endl;
		}
	}
}