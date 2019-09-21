#include "../__Header/Stream_Socket.h"
#include <iostream>

#pragma comment (lib, "Ws2_32.lib")

I_Stream_Socket::~I_Stream_Socket() {
	// cleanup
	closesocket(this->mConnection);
	WSACleanup();
}

void I_Stream_Socket::Recv(char* recvbuf, const int& N_byte) {

	if (N_byte <= 0) {
		std::cout << "N_byte cannot be less than 0 \n";
		abort(); }

	int iResult = recv(this->mConnection, recvbuf, N_byte, 0);
	if (iResult < 0) this->throw_recv_error();

}

void I_Stream_Socket::Recv(int* data) {

	int retval, tint;
	char Bytes[4];

	// Receiving current node
	retval = recv(this->mConnection, Bytes, 4, 0);
	if (retval > 0) {
		tint = 0;
		tint = 0x000000FF & Bytes[3];
		tint = ((tint << 8) & 0x0000FFFF) | (0x000000FF & Bytes[2]);
		tint = ((tint << 8) & 0x00FFFFFF) | (0x000000FF & Bytes[1]);
		tint = ((tint << 8) & 0xFFFFFFFF) | (0x000000FF & Bytes[0]);

		*data = tint;
	}
	else this->throw_recv_error();

}

void I_Stream_Socket::throw_recv_error() {

	std::cout << "recv error " << WSAGetLastError() << "\n";
	this->~I_Stream_Socket();
	abort();

}

void I_Stream_Socket::Send(const char* sendbuf, const int& N_byte) {

	if (N_byte <= 0) {
		std::cout << "N_byte cannot be less than 0 \n";
		abort();
	}

	int iSendResult = send(this->mConnection, sendbuf, N_byte, 0);
	if (iSendResult == SOCKET_ERROR) this->throw_send_error();

}

void I_Stream_Socket::Send(const int& data) {
	char Bytes[4];
	int retval, myint = data;

	// Sending delta
	for (int i = 0; i < 4; i++) Bytes[i] = (myint >> 8 * i) & 0xFF;
	retval = send(this->mConnection, Bytes, 4, 0);
	if (retval == SOCKET_ERROR)  this->throw_send_error();

}

void I_Stream_Socket::throw_send_error() {

	std::cout << "send error " << WSAGetLastError() << "\n";
	this->~I_Stream_Socket();
	abort();

}





void Stream_to_Server::InitConnection() {

	while (true) { //blocking operation: retry the connection to the server till succeed

		WSADATA wsaData; //Create a WSADATA object called wsaData.
		//Call WSAStartup and return its value as an integer and check for errors.
		this->mConnection = INVALID_SOCKET;
		//CREATING A SOCKET FOR THE CLIENT
		/*Declare an addrinfo object that contains a sockaddr structure and initialize these values.
		For this application, the Internet address family is unspecified so that either an IPv6 or IPv4 address can be
		returned. The application requests the socket type to be a stream socket for the TCP protocol.*/
		struct addrinfo *result = NULL, *ptr = NULL, hints;
		//Send and recv functions used by the client once a connection is established.
		/*The send and recv functions both return an integer value of the number of bytes
		sent or received, respectively, or an error. Each function also takes the same parameters:
		the active socket, a char buffer, the number of bytes to send or receive, and any flags to use.*/
		int iResult;
		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); //WSAStartup function initiates use of the Winsock DLL by a process.
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			abort();
		}
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC; //AF_UNSPEC either an IPv6 or IPv4 address can be returned.
		hints.ai_socktype = SOCK_STREAM; //The application requests the socket type to be a stream socket for the TCP protocol
		hints.ai_protocol = IPPROTO_TCP; //TCP protocol
		if (this->mAddress_server.compare("") == 0) //loaclhost
			iResult = getaddrinfo("127.0.0.1", this->mPort.c_str(), &hints, &result);
		else
			iResult = getaddrinfo(this->mAddress_server.c_str(), this->mPort.c_str(), &hints, &result);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			abort();
		}
		/*Call the socket function and return its value to the ConnectSocket variable. For this application, use the
		first IP address returned by the call to getaddrinfo that matched the address family, socket type, and protocol
		specified in the hints parameter. In this example, a TCP stream socket was specified with a socket type of
		SOCK_STREAM and a protocol of IPPROTO_TCP. The address family was left unspecified (AF_UNSPEC), so the returned
		IP address could be either an IPv6 or IPv4 address for the server.*/

		/*If the client application wants to connect using only IPv6 or IPv4, then the address family needs to be set
		to AF_INET6 for IPv6 or AF_INET for IPv4 in the hints parameter.*/ //SEE LINE 44

		// Attempt to connect to an addres until one succeds
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			// Create a SOCKET for connecting to server
			this->mConnection = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);

			//Check for errors to ensure that the socket is a valid socket.
			/*If the socket call fails, it returns INVALID_SOCKET. The if statement in the previous code is used to catch
			any errors that may have occurred while creating the socket. WSAGetLastError returns an error number associated
			with the last error that occurred.*/
			if (this->mConnection == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				//freeaddrinfo(result);
				WSACleanup(); //WSACleanup is used to terminate the use of the WS2_32 DLL.
				abort();
			}

			// Connect to server.
			iResult = connect(this->mConnection, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(this->mConnection);
				this->mConnection = INVALID_SOCKET;
				continue;
			}

			break;
		}

		// Should really try the next address returned by getaddrinfo
		// if the connect call failed
		// But for this simple example we just free the resources
		// returned by getaddrinfo and print an error message

		freeaddrinfo(result);

		if (this->mConnection == INVALID_SOCKET) {
			//printf("Unable to connect to server!\n");
			WSACleanup();
		}
		else break;

	}

}





void Stream_to_Client::InitConnection() {

	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	this->mConnection = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		abort();
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; //TCP protocol
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	if (this->mAddress_server.compare("") == 0) //loaclhost
		iResult = getaddrinfo(NULL, this->mPort.c_str(), &hints, &result);
	else
		iResult = getaddrinfo(this->mAddress_server.c_str(), this->mPort.c_str(), &hints, &result);

	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		abort();
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		abort();
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		abort();
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		abort();
	}

	// Accept a client socket
	this->mConnection = accept(ListenSocket, NULL, NULL);
	if (this->mConnection == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		abort();
	}

	// No longer need server socket
	closesocket(ListenSocket);

}
