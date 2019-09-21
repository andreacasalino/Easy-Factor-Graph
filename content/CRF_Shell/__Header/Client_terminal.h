#pragma once
#ifndef __CLIENT_TERMINAL__H__
#define __CLIENT_TERMINAL__H__

#include "Shell_Clients.h"

class Client_Terminal : public CRF_Shell::Client {
public:
	Client_Terminal(CRF_Shell* shell_to_link) : Client(shell_to_link) {};
private:
	virtual void			__get_next_command(std::string* command);
	virtual void			__send_response(const std::string& response);
// cache
	char				next_istruction[1000];
};


#endif