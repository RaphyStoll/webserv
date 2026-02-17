#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include "ConfigParser.hpp"
#include "Request.hpp"
#include "libftpp.hpp"

#include <cstddef>
#include <string>

// default if not in config // todo check w/ sylvain
static const size_t DEFAULT_MAX_REQUEST_SIZE = 8192;
static const size_t DEFAULT_MAX_URI_LENGTH = 2048;
static const size_t DEFAULT_MAX_HEADER_COUNT = 100;
static const size_t DEFAULT_MAX_HEADER_LINE_LENGTH = 4096;
static const size_t DEFAULT_MAX_BODY_SIZE = 1048576;

namespace webserv
{
	namespace http
	{
		class RequestParser
		{
		public:
			enum State
			{
				PARSING_REQUEST,
				PARSING_HEADERS,
				PARSING_BODY_LENGTH,
				PARSING_CHUNK_SIZE,
				PARSING_CHUNK_DATA,
				PARSING_TRAILER,
				COMPLETE,
				ERROR
			};
			RequestParser();
			~RequestParser();

			State parse(const char *data, size_t size,
						const NetworkConfig &config);//SDU suppr namespace
			Request &getRequest();

			int getErrorCode() const { return _errorCode; }

			void reset();

		private:
			Request _request;
			State _state;
			std::string _buffer;
			int _errorCode;

			unsigned long _contentLength;
			unsigned long _maxBodySize;
			unsigned long _bodyBytesRemaining;
			unsigned long _currentChunkSize;
			unsigned long _chunkBytesRemaining;

			bool _seenContentLength;
			std::string _contentLengthHeaderValue;
			bool _hasTransferEncoding;
			size_t _headerCount;

			const NetworkConfig* _config;
			bool _configResolved;

			bool _parseRequestLine(const std::string &line);
			bool _parseHeader(const std::string &line);

			State _determineBodyParsing();
			State _parseBodyByLength();
			State _parseChunkedBody();
			bool _validateHeaders();
			State _parseRequestLineState();
			State _parseHeadersState();
			bool _isValidMethod(const std::string &method) const;
			bool _isValidHeaderName(const std::string &name) const;
			bool _isValidRequestTarget(const std::string &uri);
			bool _hasDotDotSegment(const std::string &path) const;
			bool _parseContentLengthHeader(const std::string &value);
			bool _parseTransferEncodingHeader(const std::string &value);
			void _resolveConfigLimits();
		};
	} // namespace http
} // namespace webserv
#endif
