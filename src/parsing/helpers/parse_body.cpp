#include "RequestParser.hpp"
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <unistd.h>
#include <fcntl.h>
#include <climits>
#include <cerrno>

using namespace webserv::http;

unsigned int RequestParser::_tmpFileCounter = 0;

RequestParser::State RequestParser::_determineBodyParsing()
{
	if (_hasTransferEncoding && _seenContentLength) // les 2 ne peuvent pas se retrouver en meme temps
	{
		_errorCode = 400;
		return ERROR;
		// ou alors enlever le content length mais plus risqué
		// _request.removeHeader("Content-Length");
	}

	if (_hasTransferEncoding)
	{
		_state = PARSING_CHUNK_SIZE;
		return _state;
	}

	std::string contentLengthStr = _request.getHeader("Content-Length");

	// body du content-length
	if (!contentLengthStr.empty())
	{
		char *end;
		_contentLength = std::strtoul(contentLengthStr.c_str(), &end, 10);
		if (*end != '\0')
		{
			_errorCode = 400; // Content-Length error
			return ERROR;
		}
		if (_contentLength > _maxBodySize)
		{
			_errorCode = 413; // Trop gros
			return ERROR;
		}
		if (_contentLength == 0)
			return COMPLETE;
		_state = PARSING_BODY_LENGTH;
		_bodyBytesRemaining = _contentLength;
		return _state;
	}

	// pas de body attendu pour get delete etc
	// si pas de context length pour post alors error
	if (_request.getMethod() == "POST")
	{
		_errorCode = 411;
		return ERROR;
	}
	return COMPLETE;
}

RequestParser::State RequestParser::_parseBodyByLength()
{
	size_t available = _buffer.size();
	size_t toRead = std::min(available, _bodyBytesRemaining);

	if (toRead > 0)
	{
		size_t totalSize = _request.getBodySize() + toRead;
		if (totalSize > BODY_TMP_THRESHHOLD && !_usingTmpFile)
		{
			if (!_initTmpFile())
			{
				_errorCode = 500;
				return ERROR;
			}
		}

		if (_usingTmpFile)
		{
			ssize_t written = write(_bodyTmpFd, _buffer.c_str(), toRead);
			if (written < 0 || static_cast<size_t>(written) != toRead)
			{
				_errorCode = 500;
				_cleanupTmpFile();
				return ERROR;
			}
		}
		else
			_request.appendBody(_buffer.substr(0, toRead));

		_buffer.erase(0, toRead);
		_bodyBytesRemaining -= toRead;
	}

	if (_bodyBytesRemaining == 0)
	{
		if (_usingTmpFile)
		{
			close(_bodyTmpFd);
			_bodyTmpFd = -1;
		}
		_state = COMPLETE;
	}
	return _state;
}

RequestParser::State RequestParser::_parseChunkedBody()
{
	while (true)
	{
		if (_state == PARSING_CHUNK_SIZE)
		{
			size_t lineEnd = _buffer.find("\r\n");
			if (lineEnd == std::string::npos)
				return _state; // pas assez de donnees

			std::string sizeLine = _buffer.substr(0, lineEnd);
			_buffer.erase(0, lineEnd + 2);

			size_t semicolon = sizeLine.find(';');
			if (semicolon != std::string::npos)
				sizeLine = sizeLine.substr(0, semicolon);

			libftpp::str::StringUtils::trim(sizeLine);

			if (sizeLine.empty())
			{
				_errorCode = 400;
				_cleanupTmpFile();
				return ERROR;
			}

			for (size_t i = 0; i < sizeLine.size(); ++i)
			{
				char c = sizeLine[i];
				if (!std::isxdigit(static_cast<unsigned char>(c)))
				{
					_errorCode = 400;
					_cleanupTmpFile();
					return ERROR;
				}
			}

			if (sizeLine.size() > 8)
			{
				_errorCode = 413;
				_cleanupTmpFile();
				return ERROR;
			}

			char *end;
			errno = 0;
			_currentChunkSize = std::strtoul(sizeLine.c_str(), &end, 16);

			if (errno == ERANGE || _currentChunkSize == ULONG_MAX)
			{
				_errorCode = 413;
				_cleanupTmpFile();
				return ERROR;
			}

			if (_currentChunkSize > _maxBodySize || _request.getBodySize() + _currentChunkSize > _maxBodySize)
			{
				_errorCode = 413;
				_cleanupTmpFile();
				return ERROR;
			}

			if (_currentChunkSize == 0)
			{
				_state =
					PARSING_TRAILER; // dernier chunk du coup on lit les CRLF trailer
				continue;
			}

			_state = PARSING_CHUNK_DATA;
			_chunkBytesRemaining = _currentChunkSize;
		}

		if (_state == PARSING_CHUNK_DATA)
		{
			size_t available = _buffer.size();
			size_t toRead = std::min(available, _chunkBytesRemaining);

			if (toRead > 0)
			{
				size_t totalSize = _request.getBodySize() + toRead;
				if (totalSize > BODY_TMP_THRESHHOLD && !_usingTmpFile)
				{
					if (!_initTmpFile())
					{
						_errorCode = 500;
						return ERROR;
					}
				}

				if (_usingTmpFile)
				{
					ssize_t written = write(_bodyTmpFd, _buffer.c_str(), toRead);
					if (written < 0 || static_cast<size_t>(written) != toRead)
					{
						_errorCode = 500;
						_cleanupTmpFile();
						return ERROR;
					}
				}
				else
					_request.appendBody(_buffer.substr(0, toRead));
				_buffer.erase(0, toRead);
				_chunkBytesRemaining -= toRead;
			}

			if (_chunkBytesRemaining == 0)
			{
				if (_buffer.size() < 2)
					return _state;
				if (_buffer.substr(0, 2) != "\r\n")
				{
					_errorCode = 400;
					_cleanupTmpFile();
					return ERROR;
				}
				_buffer.erase(0, 2);
				_state = PARSING_CHUNK_SIZE;
			}
			else
				return _state;
		}

		if (_state == PARSING_TRAILER)
		{
			size_t lineEnd = _buffer.find("\r\n");
			if (lineEnd == std::string::npos)
				return _state;

			if (lineEnd == 0)
			{
				_buffer.erase(0, 2);
				if (_usingTmpFile)
				{
					close(_bodyTmpFd);
					_bodyTmpFd = -1;
					_request.setBodyTmpPath(_bodyTmpPath);
					_bodyTmpPath.clear();
					_usingTmpFile = false;
				}
				return COMPLETE;
			}
			_buffer.erase(0, lineEnd + 2);
		}
	}
}

std::string RequestParser::_generateTmpPath()
{
	std::ostringstream os;
	os << "/tmp/webserv_body_" << getpid() << "_" << _tmpFileCounter++ << std::time(NULL);
	return os.str();
}

bool RequestParser::_initTmpFile()
{
	_bodyTmpPath = _generateTmpPath();
	_bodyTmpFd = open(_bodyTmpPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (_bodyTmpFd < 0)
	{
		_bodyTmpPath = _generateTmpPath();
		_bodyTmpFd = open(_bodyTmpPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
		if (_bodyTmpFd < 0)
			return false;
	}
	_usingTmpFile = true;

	if (_request.getBodySize() > 0)
	{
		const std::string &existingBody = _request.getBody();
		ssize_t written = write(_bodyTmpFd, existingBody.c_str(), existingBody.size());
		if (written < 0 || static_cast<size_t>(written) != existingBody.size())
		{
			_cleanupTmpFile();
			return false;
		}
		_request.setBody("");
	}
	return true;
}

void RequestParser::_cleanupTmpFile()
{
	if (_usingTmpFile)
	{
		if (_bodyTmpFd >= 0)
		{
			close(_bodyTmpFd);
			_bodyTmpFd = -1;
		}
		if (!_bodyTmpPath.empty())
		{
			unlink(_bodyTmpPath.c_str());
			_bodyTmpPath.clear();
		}
		_usingTmpFile = false;
	}
}
