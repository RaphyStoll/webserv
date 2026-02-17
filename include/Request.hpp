#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <cstddef>
#include <map>
#include <string>

#include "libftpp.hpp"

namespace webserv
{
	namespace http
	{
		class Request
		{
		private:
			mutable libftpp::debug::DebugLogger _logger_request;
			std::string _method;
			std::string _path;
			std::string _queryString;
			std::string _httpVersion;
			std::map<std::string, std::string> _headers;
			std::string _body;
			std::string _contentLength;
			std::string _contentType;

		public:
			Request();
			~Request();

			std::string getMethod() const;
			std::string getPath() const;
			std::string getQueryString() const;
			std::string getHttpVersion() const;
			std::string getHeader(const std::string &name) const;
			std::string getBody() const;
			std::string getContentLength() const;
			std::string getContentType() const;

			void setMethod(const std::string &method);
			void setPath(const std::string &path);
			void setQueryString(const std::string &query);
			void setHttpVersion(const std::string &version);
			void setHeader(const std::string &name, const std::string &value);
			void setBody(const std::string &body);
			void setContentLength(const std::string &content_l);
			void setContentType(const std::string &content_s);

			void appendBody(const std::string &data);
			size_t getBodySize() const;

			bool keepAlive() const
			{
				std::string connection = getHeader("Connection");
				if (_httpVersion == "HTTP/1.1")
					return (connection != "close");
				else
					return (connection == "keep-alive");
			}

			void print(void) const; //SDU
		};
	} // namespace http
}
#endif
