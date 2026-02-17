#include "RequestParser.hpp"
#include <cctype>
#include <string>

using namespace webserv::http;

RequestParser::RequestParser()
	: _state(PARSING_REQUEST), _buffer(""), _errorCode(0), _contentLength(0),
	  _maxBodySize(1048576), _bodyBytesRemaining(0), _currentChunkSize(0),
	  _chunkBytesRemaining(0), _seenContentLength(false), _contentLengthHeaderValue(""),
	  _hasTransferEncoding(false), _headerCount(0) {}

RequestParser::~RequestParser() {}

RequestParser::State RequestParser::parse(const char *data, size_t size, const NetworkConfig &conf) //SDU suppr namespace
{
	(void)conf;
	if (_state == COMPLETE || _state == ERROR)
		return _state;

	_buffer.append(data, size);

	// fin de ligne
	if (_buffer.size() > DEFAULT_MAX_REQUEST_SIZE)
	{
		_errorCode = 413;
		_state = ERROR;
		return _state;
	}

	while (true)
	{
		State prevState = _state;
		switch (_state)
		{
		case PARSING_REQUEST:
			_state = _parseRequestLineState();
			break;

		case PARSING_HEADERS:
			_state = _parseHeadersState();
			if (_state == PARSING_BODY_LENGTH || _state == PARSING_CHUNK_SIZE ||
				_state == COMPLETE)
			{
				if (!_validateHeaders())
				{
					_state = ERROR;
					return _state;
				}
				_resolveConfigLimits();
			}
			break;

		case PARSING_BODY_LENGTH:
			_state = _parseBodyByLength();
			break;

		case PARSING_CHUNK_SIZE:
		case PARSING_CHUNK_DATA:
		case PARSING_TRAILER:
			_state = _parseChunkedBody();
			break;
		default:
			return _state;
		}
		if (_state == prevState)
			return _state;
		if (_state == COMPLETE || _state == ERROR)
			return _state;
	}
}

Request &RequestParser::getRequest() { return _request; }

void RequestParser::reset()
{
	_state = PARSING_REQUEST;
	_buffer.clear();
	_errorCode = 0;
	_contentLength = 0;
	_maxBodySize = DEFAULT_MAX_BODY_SIZE;
	_bodyBytesRemaining = 0;
	_currentChunkSize = 0;
	_chunkBytesRemaining = 0;
	_seenContentLength = false;
	_contentLengthHeaderValue.clear();
	_hasTransferEncoding = false;
	_headerCount = 0;
	_config = NULL;
	_configResolved = false;
	_request = Request();
}

void RequestParser::_resolveConfigLimits(){
	if (_configResolved || _config == NULL)
		return;

	_configResolved = true;

	std::string host = _request.getHeader("Host");
	if (host.empty())
		return;

	size_t colonPos = host.find(':');
	std::string hostName = (colonPos != std::string::npos) ? host.substr(0, colonPos) : host;
	int hostPort = 0;
	if (colonPos != std::string::npos)
	{
		std::string portStr = host.substr(colonPos + 1);
		hostPort = std::atoi(portStr.c_str());
	}

	for (NetworkConfig::const_iterator portIt = _config->begin(); portIt != _config->end(); ++portIt)
	{
		if (hostPort != 0 && portIt->first != hostPort)
			continue;
		const std::vector<ServerConfig>& servers = portIt->second;
		for (size_t i = 0; i < servers.size(); ++i)
		{
			if (servers[i].server_name == hostName || hostPort != 0)
			{
				_maxBodySize = servers[i].max_body_size;
				return;
			}
		}

		if (!servers.empty() && (hostPort == 0 || portIt->first == hostPort))
		{
			_maxBodySize = servers[0].max_body_size;
			return;
		}
	}
}
