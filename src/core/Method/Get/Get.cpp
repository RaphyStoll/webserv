#include "Get.hpp"
#include "ResponseBuilder.hpp"

#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <climits>
#include <cstdlib>
#include <sys/wait.h>

using namespace webserv::http;

std::string execute_cgi(const ::http::Request& req, const ServerConfig& config)
{
	std::cout << std::endl << "on est dans CGI" << std::endl << std::endl;
	req.print();
//	config.print();
	(void)config;

	int pipefd[2];
	pipe(pipefd);

	pid_t pid = fork();

	if(pid == 0 ) // fils
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		char *env[]= {
			(char*)"REQUEST_METHOD=GET",
			(char*)"QUERY_STRING=query=webserv",
			(char*)"PATH_INFO=/search",
			(char*)"SERVER_NAME=VPSSDU_l_163",
			(char*)"SERVER_PORT=9003",
			NULL
		};

		char *args[] = {
			(char*)"./cgi/search.py",
			NULL
		};

		execve("./cgi/search.py",args,env);
		perror("execve");
		_exit(1);
	}

	//parent
	close(pipefd[1]);

	std::string output;
	char buffer[1024];
	ssize_t n;

	while((n = read(pipefd[0], buffer, sizeof(buffer))) > 0)
		output.append(buffer,n);

	close(pipefd[0]);

	waitpid(pid, NULL, 0);

	return(output);
}


std::string webserv::http::Get::execute(const ::http::Request& req, const ServerConfig& config)
{
	int httpCode = 200;

	std::string fullPath = _getSecurePath(config.root, req.getPath(), httpCode);
/*	if (httpCode != 200)
		return ResponseBuilder::generateError(httpCode, config);

	struct stat s;
	if (stat(fullPath.c_str(), &s) == 0 && (s.st_mode & S_IFDIR)) {
		if (!_checkIndexFile(fullPath, httpCode)) {
			// TODO: RAPH
			// Ici, plus tard : quelque cgose comme if (config.autoindex) return AutoIndex::gen(...);
			return ResponseBuilder::generateError(httpCode, config);
		}
	}

	if (access(fullPath.c_str(), R_OK) != 0)
		return ResponseBuilder::generateError(403, config);

	if (content.empty() && s.st_size > 0)
		return ResponseBuilder::generateError(500, config);

	std::string content = _readFile(fullPath);
*/
//	std::cout << "full_path = " << fullPath << std::endl; //SDU

	std::string content = "";

	if(req.getPath() == "/search")
		content = execute_cgi(req, config);

	return _createSuccessResponse(content, fullPath);
}
