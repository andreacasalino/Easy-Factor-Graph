#pragma once
#ifndef __CLIENT_SOCKET__H__
#define __CLIENT_SOCKET__H__

#include "Shell_Clients.h"
#include "Stream_Socket.h"

class Client_Socket : public CRF_Shell::Client {
public:
	Client_Socket(CRF_Shell* shell_to_link, const std::string& port) : Client(shell_to_link), Connection(port) { this->Connection.InitConnection(); this->__allow_loop_to_start(); };
private:
	virtual void			__get_next_command(std::string* command);
	virtual void			__process_response();

// data
	Stream_to_Client		Connection;
};


#endif