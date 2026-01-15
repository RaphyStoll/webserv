# include "ConfigParser.hpp"

static bool supprComment(std::string &line)
{
	size_t commentPos = line.find('#');

	if(commentPos != std::string::npos)
		line = line.substr(0, commentPos);

	line = libftpp::str::StringUtils::trim(line);

	return(0);
	
}

void print_vect(const std::vector<std::string> &vect)
{
	size_t i = 0;
	while(i < vect.size())
	{
		std::cout << "vect[" << i << "] : " << vect[i] << std::endl;
		i++;
	}
}

void tockenize(DataConfig *data)
{
	std::string line;
	std::string word;
	char c;

	size_t i;
	size_t j;
	
	i = 0;
	while(i < data->brut_line.size())
	{
		word.clear();
		line = data->brut_line[i];
		j = 0;
		while(j < line.size())
		{
			c = line[j];
			if(c == ' ')
			{
				if(! word.empty())
					data->token.push_back(word);
				word.clear();
			}
			else if (c == '{' || c == '}')
			{
				if(! word.empty())
					data->token.push_back(word);
				word.clear();
				word += c;
				if(! word.empty())
					data->token.push_back(word);
				word.clear();
			}
			else
			{
				word += c;
			}
			j++;
		}
		if(! word.empty())
			data->token.push_back(word);
		i++;
	}
}

void pars(DataConfig *data)
{

	data->i = 0;
	data->state = GLOBAL;
	std::vector<std::string> token = data-> token;
	std::cout << "token.size = " << token.size() << std::endl;

	while(data->i < token.size())
	{
		if(data->state == GLOBAL)
		{
			state_global(data);
		}
		else if(data->state == IN_SERVER)
		{
			state_server(data);
		}
		if(data->state == IN_ROUTE)
		{
			state_route(data);
		}
	}

	if (data->state != GLOBAL)
		std::cerr << "block {} unclosed" << std::endl;

	std::cout << "fin du parsing" << std::endl;
}

static bool openFileAndParseConfig(const std::string & config_path, DataConfig * data)
{
	std::cout << "config_path = " << config_path << std::endl;

//	std::ifstream file(config_path);
//	if(!file)
//	{
//		std::cerr << "fichier introuvable" << std::endl;
//		return (-1);
//	}

	std::ifstream file(config_path.c_str(), std::ios::in);
	if (!file.is_open())
	{
		std::cerr << "fichier introuvable" << std::endl;
		return (-1);
	}

	std::string line;
	while( std::getline(file, line))
	{
		supprComment(line);
		if(!line.empty())
			data->brut_line.push_back(line);
	}

	if(data->brut_line.empty()) //tester + tard??
	{
		std::cerr << "fichier de config vide" << std::endl;
		return (-1);
	}

//	print_vect(data->brut_line);
	std::cout << std::endl;
	tockenize(data);
	print_vect(data->token);
	std::cout << std::endl;
	pars(data);


	return(1);
}

int main () 
{
	std::cout << "lancement de webserv" << std::endl;
	DataConfig data;

	std::string config_folder = "config";
	std::string config_file = "config.conf";
	std::string config_path = config_folder + "/" + config_file;

	if (!openFileAndParseConfig(config_path, &data))
	{
		std::cerr << "error openFileAndParseConfig" << std::endl;
		return (-1);
	}

	return(0);
}