int main(int argc, char** argv)
{
	int server_fd;

	//1.  DataConfig raw_config = parseConfig("file.conf");
	// 2. On la convertit en config "réseau" (Map par port) pour pouvoir passer a bootStrap
	// quelque chose du genre: std::map<int, std::vector<ServerConfig> > net_config;

	bootStrap(&server_fd);
}