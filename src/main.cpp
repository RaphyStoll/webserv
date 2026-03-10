#include "../include/BootStrap.hpp"
#include "../include/ConfigParser.hpp"
#include "../include/EventLoop.hpp"
#include "../include/SignalHandler.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace webserv;
// TODO: nettoyer l'arbo www de -> site1 cree site 2
// TODO: verifier si tout est ok -> config / est obligatoire ?
// TODO: time out cgi
int main(int argc, char** argv) {
	
	libftpp::debug::DebugLogger::cleanAll();

	SignalHandler::setup();

	DataConfig data;
	libftpp::debug::DebugLogger _logger("Main");
	_logger << "Server started" << std::endl;
	data.config_path = "config/config.conf";

	if (argc > 1) {
		data.config_path = argv[1];
	}
	try {

		_logger << "[Main] Loading configuration from " << data.config_path << "..." << std::endl;

		NetworkConfig net_config = data.cParser();

		//TODO: boucle d'iterateur qui lance 1 boot strap par serveur -> test 2 serv mm port 2 ip diff
		_logger << "[Main] Config loaded. Initializing BootStrap..." << std::endl;
		BootStrap bootstrap(net_config);

		bootstrap.start();

		webserv::core::EventLoop loop(bootstrap.getListenSockets(), net_config);
		loop.run();

		_logger << "[Main] Server shutdown complete" << std::endl;

	} catch (const std::exception& e) {
		std::cerr << "Fatal Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
