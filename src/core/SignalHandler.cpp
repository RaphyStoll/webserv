#include "SignalHandler.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

namespace webserv
{
	volatile sig_atomic_t SignalHandler::_running = 1;

	void SignalHandler::setup()
	{
		// SIGPIPE pour ne pas kill le server si un client se déconnecte au moment de l'écriture du socket
		signal(SIGPIPE, SIG_IGN);

		struct sigaction sa;
		std::memset(&sa, 0, sizeof(sa));
		sa.sa_handler = _shutdownHandler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;

		// SIGINT (Ctrl+C)
		if (sigaction(SIGINT, &sa, NULL) == -1) {
			std::cerr << "[SignalHandler] Failed to set SIGINT handler" << std::endl;
		}

		// SIGTERM (kill)
		if (sigaction(SIGTERM, &sa, NULL) == -1) {
			std::cerr << "[SignalHandler] Failed to set SIGTERM handler" << std::endl;
		}
	}

	bool SignalHandler::isRunning()
	{
		return _running != 0;
	}

	void SignalHandler::requestShutdown()
	{
		_running = 0;
	}

	void SignalHandler::_shutdownHandler(int signum)
	{
		(void)signum;
		// Write pour async
		const char msg[] = "\n[SignalHandler] Shutdown signal received, cleaning up...\n";
		write(STDOUT_FILENO, msg, sizeof(msg) - 1);
		_running = 0;
	}
}
