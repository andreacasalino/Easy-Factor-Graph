#pragma once
#ifndef __STREAM_SOCKET_H__
#define __STREAM_SOCKET_H__

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

class I_Stream_Socket {
public:
	~I_Stream_Socket();
// methods
	virtual void InitConnection() = 0;

	void Recv(char* recvbuf, const int& N_byte);
	void Send(const char* sendbuf, const int& N_byte);

	virtual void Recv(int* data); //can change deriving (Big-Little endian)
	virtual void Send(const int& data); //can change deriving (Big-Little endian)
protected:
	I_Stream_Socket(const std::string& server_address, const std::string& port) :
		mAddress_server(server_address), mPort(port), mConnection(INVALID_SOCKET) {};
// methods
	void throw_recv_error();
	void throw_send_error();
// data
	SOCKET			mConnection; 
	std::string		mAddress_server;
	std::string		mPort;
};



class Stream_to_Server : public I_Stream_Socket {
public:
	Stream_to_Server(const std::string& server_address, const std::string& port):
		I_Stream_Socket(server_address, port) {}; //use "" for local host
	Stream_to_Server(const std::string& port) : Stream_to_Server("", port) {}; //localhost is assumed

	void InitConnection();
};



class Stream_to_Client : public I_Stream_Socket {
public:
	Stream_to_Client(const std::string& server_address, const std::string& port) : 
		I_Stream_Socket(server_address, port) {};
	Stream_to_Client(const std::string& port) : I_Stream_Socket("", port) {};

	void InitConnection();
};

#endif // !__SOCKET_MY_H__