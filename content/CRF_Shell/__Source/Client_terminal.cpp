#include "../__Header/Client_terminal.h"
#include <iostream>
using namespace std;


void Client_Terminal::__get_next_command(std::string* command) {

	fgets(&this->next_istruction[0], sizeof(this->next_istruction), stdin);
	*command = string(&next_istruction[0]);
	if(command->back() == '\n')
		command->pop_back();

}

void Client_Terminal::__send_response(const std::string& response) {

	cout << response << endl;

}