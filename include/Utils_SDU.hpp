#ifndef UTILS_SDU_HPP
# define UTILS_SDU_HPP

void print_vect(const std::vector<std::string> &vect)
{
	size_t i = 0;
	while(i < vect.size())
	{
		std::cout << "vect[" << i << "] : " << vect[i] << std::endl;
		i++;
	}
}

#endif