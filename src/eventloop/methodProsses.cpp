#include "../../include/EventLoop.hpp"
using namespace webserv;

std::string EventLoop::runGetMethod(const http::Request &req)
{
    // 1. Logique : Chercher la ressource demandée par req.getPath()
    // Pour l'instant, on simule une réponse 200 OK
    (void)req;
    std::string body = "<html><body><h1>Hello from Webserv!</h1></body></html>";
    
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

bool EventLoop::runDeletMethod(const http::Request &req)
{
	(void)req;
	return printf("runDeletMethod ok\n");
}

bool EventLoop::runPostMethod(const http::Request &req)
{
	(void)req;
	return printf("runPostMethod ok\n");
}