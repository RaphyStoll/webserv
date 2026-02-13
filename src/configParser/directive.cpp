#include"ConfigParser.hpp"

void DataConfig::dir_server()
{
	size_t i = _i;

	if(_token[i] == "server_name")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		_currentServer.server_name = _token[i];
		i++;
	}
	else if(_token[i] == "listen")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		_currentToken = _token[i];
		if (valid_ipv4() == -1)
			throw std::runtime_error("ipv4 adress ko : " + _currentToken);
		_currentServer.listen = _token[i];
		i++;
	}
	else if(_token[i] == "port")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);

		_currentToken = _token[i];
		valid_port();
		_currentServer.port = libftpp::str::StringUtils::stoi(_token[i]);
		i++;
	}
	else if(_token[i] == "root")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		_currentToken = _token[i];
		valid_path();
		_currentServer.root = _token[i];
		i++;
	}
	else if(_token[i] == "autoindex")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		if(_token[i] == "off")
			_currentServer.autoindex = 0;
		else if(_token[i] == "on")
			_currentServer.autoindex = 1;
		else
			std::runtime_error("Unknow directory_listing directive : " + _token[i]);
		i++;
	}
	else if(_token[i] == "index")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		_currentToken = _token[i];
		valid_file();
		_currentServer.index = _token[i];
		i++;
	}
	else if(_token[i] == "max_body_size")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		_currentToken = _token[i];
		valid_max_body_size();
		_currentServer.max_body_size = libftpp::str::StringUtils::stoi(_token[i]);
		i++;
	}
	else if(_token[i] == "error_page")
	{
		i++;
		if(i + 1 >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		size_t n = libftpp::str::StringUtils::stoi(_token[i]);
		_currentToken = _token[i];
		valid_error_code();
		_currentToken = _token[i + 1];
		valid_path();
		_currentServer.error_pages.insert(std::make_pair(n,_token[i + 1]));
		i+=2;
	}
	else
	{
		throw std::runtime_error("Unknow server directive : " +  _token[i] );
		i++;
	}
	_i = i;
}

void DataConfig::dir_route()
{
	size_t i = _i;

	if(_token[i] == "root")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		_currentToken = _token[i];
		valid_path();
		_currentRoute.root = _token[i];
		i++;
	}
	else if(_token[i] == "methods")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		while(_token[i] == "GET" || _token[i] == "POST" || _token[i] == "DELETE")
		{
			_currentRoute.methods.push_back(_token[i]);
			i++;
		}
	}
	else if(_token[i] == "directory_listing")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		if(_token[i] == "off")
			_currentRoute.directory_listing = 0;
		else if(_token[i] == "on")
			_currentRoute.directory_listing = 1;
		else
			std::runtime_error("Unknow directory_listing directive : " + _token[i]);
		i++;
	}
	else if(_token[i] == "upload")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		if(_token[i] == "off")
			_currentRoute.upload = 0;
		else if(_token[i] == "on")
			_currentRoute.upload = 1;
		else
			std::runtime_error("Unknow upload directive : " + _token[i]);
		i++;
	}
	else if(_token[i] == "upload_path")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		_currentToken = _token[i];
		valid_path();
		_currentRoute.upload_path = _token[i];
		i++;
	}
	else if(_token[i] == "cgi")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		if(_token[i] == "off")
			_currentRoute.cgi = 0;
		else if(_token[i] == "on")
			_currentRoute.cgi = 1;
		else
			std::runtime_error("Unknow upload directive : " + _token[i]);
		i++;
	}
	else if(_token[i] == "cgi_extension")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		_currentRoute.cgi_extension = _token[i];
		i++;
	}
	else if(_token[i] == "cgi_path")
	{
		i++;
		if(i >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		_currentToken = _token[i];
//		valid_path(); //SDU chiant pour les test, a remettre.
		_currentRoute.cgi_path = _token[i];
		i++;
	}
	else if(_token[i] == "redirect")
	{
		i++;
		if(i + 1 >= _token.size())
			throw std::out_of_range("Acces token : " + _token[i-1]);
		size_t n = libftpp::str::StringUtils::stoi(_token[i]);
		_currentRoute.redirect.insert(std::make_pair(n,_token[i + 1]));
		i+=2;
		//completer les verif de doublons et compagnie si besoin
	}
	else
		throw std::runtime_error("Unknow route directive : " +  _token[i]);

	_i = i;
}