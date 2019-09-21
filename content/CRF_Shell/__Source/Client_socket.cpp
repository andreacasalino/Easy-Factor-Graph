#include "../__Header/Client_socket.h"
#include <iostream>
using namespace std;


void Client_Socket::__get_next_command(std::string* command) {

	int dim_command;
	this->Connection.Recv(&dim_command);
	char* temp = (char*)malloc(sizeof(char)*dim_command);
	this->Connection.Recv(temp, dim_command);
	*command = string(temp);
	free(temp);

}

void Client_Socket::__process_response() {

	cout << *this->__get_Response() << endl;

	auto resp = this->__get_Response();
	this->Connection.Send((int)resp->size());
	this->Connection.Send(resp->c_str(), (int)resp->size());

}