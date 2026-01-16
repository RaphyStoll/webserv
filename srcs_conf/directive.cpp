#include"ConfigParser.hpp"

void dir_server(DataConfig *data)
{
	

	std::vector<std::string> token = data->token;
	size_t i = data->i;

//	std::cerr << " in dir server " ;

	if(token[i] == "listen")
	{
		i++;
		if(i >= token.size())
			std::cerr << "erreur acces token" << std::endl; //SDU il faut sortir sinon segfault
		data->currentServer.listen = token[i];
		i++;
	}
	else if(token[i] == "port")
	{
		i++;
		if(i >= token.size())
			std::cerr << "erreur acces token" << std::endl; //SDU il faut sortir sinon segfault
		data->currentServer.port = libftpp::str::StringUtils::stoi(token[i]); //SDU :trop long
		i++;
	}
	else if(token[i] == "root")
	{
		i++;
		if(i >= token.size())
			std::cerr << "erreur acces token" << std::endl; //SDU il faut sortir sinon segfault
		data->currentServer.root = token[i];
		i++;
	}
	else if(token[i] == "index")
	{
		i++;
		if(i >= token.size())
			std::cerr << "erreur acces token" << std::endl; //SDU il faut sortir sinon segfault
		data->currentServer.index = token[i];
		i++;
	}
	else
	{
		std::cerr << "commande server non prise en compte" << std::endl;
		i++;
	}
	
	data->i = i;
}

/*
	std::string path;
	std::string root;
	std::vector<std::string> methods;
	bool directory_listing;
	bool upload;
	std::string upload_path;
	bool cgi;
	std::string cgi_extention;
	std::string cgi_path;
*/

void dir_route(DataConfig *data)
{
	

	std::vector<std::string> token = data->token;
	size_t i = data->i;

//	std::cerr << " in dir route " ;

	if(token[i] == "path")
	{
		i++;
		if(i >= token.size())
			std::cerr << "erreur acces token" << std::endl; //SDU il faut sortir sinon segfault
		data->currentRoute.path = token[i];
		i++;
	}
	else if(token[i] == "root")
	{
		i++;
		if(i >= token.size())
			std::cerr << "erreur acces token" << std::endl; //SDU il faut sortir sinon segfault
		data->currentRoute.root = token[i];
		i++;
	}
	else if(token[i] == "directory_listing")
	{
		i++;
		if(i >= token.size())
			std::cerr << "erreur acces token" << std::endl; //SDU il faut sortir sinon segfault
		data->currentRoute.directory_listing = libftpp::str::StringUtils::stoi(token[i]); //SDU :trop long
		i++;
	}
	else
	{
		std::cerr << "commande route non prise en compte" << std::endl;
		i++;
	}
	
	data->i = i;
}