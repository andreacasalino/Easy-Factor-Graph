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

void Client_Socket::__send_response(const std::string& response) {

	this->Connection.Send((int)response.size());
	this->Connection.Send(response.c_str(), (int)response.size());

}