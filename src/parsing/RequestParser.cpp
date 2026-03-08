#include "RequestParser.hpp"
#include <cctype>
#include <cstdlib>
#include <string>

using namespace webserv::http;

RequestParser::RequestParser()
	: _state(PARSING_REQUEST), _buffer(""), _errorCode(0), _contentLength(0),
	  _maxBodySize(1048576), _bodyBytesRemaining(0), _currentChunkSize(0),
	  _chunkBytesRemaining(0), _seenContentLength(false), _contentLengthHeaderValue(""),
	  _hasTransferEncoding(false), _headerCount(0), _bodyTmpFd(-1), _usingTmpFile(false) {}

RequestParser::~RequestParser()
{
	_cleanupTmpFile();
}

RequestParser::State RequestParser::parse(const char *data, size_t size, const NetworkConfig &conf) // SDU suppr namespace
{
	if (!_configResolved)
	{
		_config = &conf;
		_resolveConfigLimits();
	}

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
				_resolveConfigLimits();
				if (!_validateHeaders())
				{
					_state = ERROR;
					return _state;
				}
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
	_cleanupTmpFile();
	_bodyTmpFd = -1;
}

void RequestParser::_resolveConfigLimits()
{
	if (_configResolved || _config == NULL || _config->empty())
		return;

	_configResolved = true;

	std::string host = _request.getHeader("Host");
	std::string hostName;
	int hostPort = 0;

	if (!host.empty())
	{
		size_t colonPos = host.find(':');
		hostName = (colonPos != std::string::npos) ? host.substr(0, colonPos) : host;
		if (colonPos != std::string::npos)
		{
			std::string portStr = host.substr(colonPos + 1);
			hostPort = std::atoi(portStr.c_str());
		}
	}

	if (hostPort != 0)
	{
		NetworkConfig::const_iterator byPort = _config->find(hostPort);
		if (byPort != _config->end() && !byPort->second.empty())
		{
			const std::vector<ServerConfig> &servers = byPort->second;
			for (size_t i = 0; i < servers.size(); ++i)
			{
				if (!hostName.empty() && servers[i].server_name == hostName)
				{
					_maxBodySize = servers[i].max_body_size;
					return;
				}
			}
			_maxBodySize = servers[0].max_body_size;
			return;
		}
	}

	for (NetworkConfig::const_iterator it = _config->begin(); it != _config->end(); ++it)
	{
		const std::vector<ServerConfig> &servers = it->second;
		if (servers.empty())
			continue;
		for (size_t i = 0; i < servers.size(); ++i)
		{
			if (!hostName.empty() && servers[i].server_name == hostName)
			{
				_maxBodySize = servers[i].max_body_size;
				return;
			}
		}
	}

	for (NetworkConfig::const_iterator it = _config->begin(); it != _config->end(); ++it)
	{
		if (!it->second.empty())
		{
			_maxBodySize = it->second[0].max_body_size;
			return;
		}
	}
}
