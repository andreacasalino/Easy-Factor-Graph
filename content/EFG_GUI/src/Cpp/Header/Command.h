#ifndef  __COMMAND_H__
#define __COMMAND_H__

#include <string>
#include <vector>
#include <list>

class Command {
public:
	Command(const std::string& raw);

	const char&									  Get_name() const { return this->Name; };
	const std::vector<std::string>* Get_values(const char& option_name) const; //return NULL when option does not exist
	const std::string*							  Get_value(const char& option_name) const; //return NULL when option does not exist

	void												Print() const; //mainly for debug
private:
	void __find_separators(std::list<std::size_t>* positions, const std::string& word);

	struct __option {
		char											name;
		std::vector<std::string>			values;
	};
	char												Name;
	std::list<__option>						Options;
};


std::string char_pt_2_string(char* buffer, const size_t& Size);

#endif // ! __COMMAND_H__
