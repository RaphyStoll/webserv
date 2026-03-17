#ifndef SIGNALHANDLER_HPP
# define SIGNALHANDLER_HPP

# include <csignal>

namespace webserv
{
	class SignalHandler {
		public:
			static void setup();
			static bool isRunning();
			static void requestShutdown();

		private:
			static volatile sig_atomic_t _running;
			static void _shutdownHandler(int signum);
	};
}

#endif
