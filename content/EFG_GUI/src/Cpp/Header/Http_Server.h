#pragma once

#ifndef _HTTP_SERVER_H__
#define _HTTP_SERVER_H__

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define NOMINMAX

#include "JSON.h"
#include <windows.h>

//#define LOG_REQUEST_FROM_CLIENT

class Http_Server {
public:
	Http_Server(const std::string& port);
	~Http_Server();

	struct Reaction_Handler {
		virtual void get_reponse(std::string* response, const std::string& request) = 0; // return false when shut down command is received
	};

	//here the request the next command is received and the response is also sent, after the reactor computed it
	void React_to_client(Reaction_Handler* reactor);

	void Recv_request(std::string* request);
	//call it only after Recv_request
	void Send_response(const std::string& response);
private:
	struct __Headers_t;
	__Headers_t __GET_next(std::string* request_without_key , const ULONG& size_of_request = 2048);

	void __send_RESPONSE(const __Headers_t& Req, const std::string& buffer_to_send);
	void __send_REFUSE(const __Headers_t& Req);

	void __get_from_unknown_headers(std::list<std::string>* header_values, const std::list<std::string>& header_names, const __Headers_t& request);
	void __get_from_unknown_headers(std::string* header_values, const std::string& header_names, const __Headers_t& request);
// data
	HANDLE										hReqQueue;
	std::string										port_to_accept;
	bool												Is_initialized;
	std::string										server_process_ID;
	__Headers_t*								pending_request;
};

#endif // !_HASH_ENCODER_H__