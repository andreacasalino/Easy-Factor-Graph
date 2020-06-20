#include "../Header/Http_Server.h"
#include <stdio.h>
#include <fstream>
#include <http.h>
#include <iostream>


using namespace std;

#pragma comment(lib, "httpapi.lib")



struct Http_Server::__Headers_t {
	PCHAR					 pRequestBuffer;
	PHTTP_REQUEST pRequest;
};


//
// Macros.
//
#define INITIALIZE_HTTP_RESPONSE( resp, status, reason )    \
    do                                                      \
    {                                                       \
        RtlZeroMemory( (resp), sizeof(*(resp)) );           \
        (resp)->StatusCode = (status);                      \
        (resp)->pReason = (reason);                         \
        (resp)->ReasonLength = (USHORT) strlen(reason);     \
    } while (FALSE)

#define ADD_KNOWN_HEADER(Response, HeaderId, RawValue)               \
    do                                                               \
    {                                                                \
        (Response).Headers.KnownHeaders[(HeaderId)].pRawValue =      \
                                                          (RawValue);\
        (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength = \
            (USHORT) strlen(RawValue);                               \
    } while(FALSE)

#define ALLOC_MEM(cb) HeapAlloc(GetProcessHeap(), 0, (cb))

#define FREE_MEM(ptr) HeapFree(GetProcessHeap(), 0, (ptr))



void SET_UNKNOWN_HEADERS(USHORT* numb_of_unknowns, PHTTP_UNKNOWN_HEADER* unknowns, const std::list<std::string>& names, const std::list<std::string>& vals) {

	if (names.size() != vals.size())
		abort();

	(*numb_of_unknowns) = (USHORT)names.size();

	(*unknowns) = (HTTP_UNKNOWN_HEADER*)malloc(names.size() * sizeof(HTTP_UNKNOWN_HEADER));
	size_t k = 0;
	auto it2 = vals.begin();
	for (auto it = names.begin(); it != names.end(); it++) {
		(*unknowns)[k].NameLength = (USHORT)it->size();
		(*unknowns)[k].pName = it->c_str();

		(*unknowns)[k].RawValueLength = (USHORT)it2->size();
		(*unknowns)[k].pRawValue = it2->c_str();

		k++;
		it2++;
	}

}



Http_Server::Http_Server(const std::string& port) :
hReqQueue(NULL),  port_to_accept(port), pending_request(NULL) {

	this->server_process_ID = to_string(GetCurrentProcessId());

	ULONG           retCode;
	HTTPAPI_VERSION HttpApiVersion = HTTPAPI_VERSION_1;

	//
	// Initialize HTTP Server APIs
	//
	retCode = HttpInitialize(HttpApiVersion, HTTP_INITIALIZE_SERVER, NULL);

	if (retCode != NO_ERROR)
	{
		wprintf(L"HttpInitialize failed with %lu \n", retCode);
		abort();
	}

	//
	// Create a Request Queue Handle
	//
	retCode = HttpCreateHttpHandle(
		&this->hReqQueue,        // Req Queue
		0						 // Reserved
	);

	if (retCode != NO_ERROR)
	{
		wprintf(L"HttpCreateHttpHandle failed with %lu \n", retCode);
		abort();
	}

	string URL_to_accept_full = "http://localhost:" + this->port_to_accept + "/";

	retCode = HttpAddUrl(
		this->hReqQueue,    // Req Queue
		PCWSTR(std::wstring(URL_to_accept_full.begin(), URL_to_accept_full.end()).c_str()),      // Fully qualified URL
		NULL          // Reserved
	);

	if (retCode != NO_ERROR)
	{
		wprintf(L"HttpAddUrl failed with %lu \n", retCode);
		abort();
	}

	this->Is_initialized = false;

}

Http_Server::~Http_Server() {

	
	 //Call HttpRemoveUrl for all added URLs.
	string URL_to_accept_full = "http://localhost:" + this->port_to_accept + "/";

	HttpRemoveUrl(
		this->hReqQueue,     // Req Queue
		PCWSTR(std::wstring(URL_to_accept_full.begin(), URL_to_accept_full.end()).c_str())        // Fully qualified URL
	);

	//
	// Close the Request Queue handle.
	//
	if (this->hReqQueue)
	{
		CloseHandle(this->hReqQueue);
	}

	// 
	// Call HttpTerminate.
	//
	HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);

	if (this->pending_request != NULL)
		delete this->pending_request;

}

void Http_Server::__get_from_unknown_headers(std::list<std::string>* header_values, const std::list<std::string>& header_names, const __Headers_t& request) {

	list<string> names;
	USHORT k;
	for (k = 0; k < request.pRequest->Headers.UnknownHeaderCount; k++) {
		names.push_back(string());
		names.back() = request.pRequest->Headers.pUnknownHeaders[k].pName;
	}

	header_values->clear();
	list<string>::iterator it2;
	for (auto it = header_names.begin(); it != header_names.end(); it++) {
		header_values->push_back(string());
		k = 0;
		for (it2 = names.begin(); it2 != names.end(); it2++) {
			if (it2->compare(*it) == 0) {
				header_values->back() = request.pRequest->Headers.pUnknownHeaders[k].pRawValue;
				break;
			}
			k++;
		}
	}

}

void Http_Server::__get_from_unknown_headers(std::string* header_values, const std::string& header_names, const __Headers_t& request) {

	list<string> temp = { header_names };
	list<string> vals;
	this->__get_from_unknown_headers(&vals, temp, request);
	*header_values = vals.front();

}

void Http_Server::React_to_client(Reaction_Handler* reactor) {

	string request;
	auto request_handler = this->__GET_next(&request);

#ifdef LOG_REQUEST_FROM_CLIENT
	cout << request << endl;
#endif // LOG_REQUEST_FROM_CLIENT

	string response;
	reactor->get_reponse(&response, request);
	this->__send_RESPONSE(request_handler, response);

}

void Http_Server::Recv_request(string* request) {

	if (this->pending_request != NULL) abort();

	this->pending_request = new __Headers_t(this->__GET_next(request));

#ifdef LOG_REQUEST_FROM_CLIENT
	cout << request << endl;
#endif // LOG_REQUEST_FROM_CLIENT

}

void Http_Server::Send_response(const string& response) {

	if (this->pending_request == NULL) abort();

	this->__send_RESPONSE(*this->pending_request, response);

	delete this->pending_request;
	this->pending_request = NULL;

}

Http_Server::__Headers_t Http_Server::__GET_next(std::string* request_without_key, const ULONG& size_of_request) {

	while (true)
	{
		__Headers_t result_h;

		//
		// Allocate a 2 KB buffer. This size should work for most 
		// requests. The buffer size can be increased if required. Space
		// is also required for an HTTP_REQUEST structure.
		//
		ULONG RequestBufferLength = sizeof(HTTP_REQUEST) + size_of_request;
		result_h.pRequestBuffer = (PCHAR)ALLOC_MEM(RequestBufferLength);

		//if (pRequestBuffer == NULL)
		//	abort();

		result_h.pRequest = (PHTTP_REQUEST)result_h.pRequestBuffer;


		HTTP_REQUEST_ID    requestId;
		HTTP_SET_NULL_ID(&requestId);
		RtlZeroMemory(result_h.pRequest, RequestBufferLength);
		DWORD              bytesRead;

		// Wait for a new request. This is indicated by a NULL 
		ULONG result = HttpReceiveHttpRequest(
			hReqQueue, requestId, 0, result_h.pRequest, RequestBufferLength, &bytesRead, NULL);
		//Req Queue  ,  Req ID	   , Flags,    HTTP request buffer,			req buffer length, bytes received,   LPOVERLAPPED

		if (NO_ERROR != result)
		{
			wprintf(L"HttpReceiveHttpRequest failed with %lu \n", result);
			abort();
		}

#ifdef DEBUG_COUT
		string ttemp[2];
		for (USHORT k = 0; k < 41; k++) {
			if (result_h.pRequest->Headers.KnownHeaders[k].RawValueLength > 0) {
				ttemp[0] = result_h.pRequest->Headers.KnownHeaders[k].pRawValue;
				cout << ttemp[0] << " : " << ttemp[1] << endl;
			}
		}
		for (USHORT k = 0; k < result_h.pRequest->Headers.UnknownHeaderCount; k++) {
			ttemp[0] = result_h.pRequest->Headers.pUnknownHeaders[k].pName;
			ttemp[1] = result_h.pRequest->Headers.pUnknownHeaders[k].pRawValue;
			cout << ttemp[0] << " : " << ttemp[1] << endl;
		}
#endif

		if (!this->Is_initialized) {
			this->__send_RESPONSE(result_h, this->server_process_ID);
			this->Is_initialized = true;
		}
		else {
			//__get_from_unknown_headers(std::string* header_values, const std::string& header_names, const __Headers_t& request)
			*request_without_key = result_h.pRequest->Headers.KnownHeaders[20].pRawValue;
			size_t pos_temp = 0;
			string key;
			for (size_t k = 0; k < request_without_key->size(); k++) {
				if ((*request_without_key)[k] == '_') {
					pos_temp = k;
					break;
				}
				key.push_back((*request_without_key)[k]);
			}
			if (pos_temp == 0)
				abort();
			*request_without_key = string(*request_without_key, pos_temp + 1);

			//system(string("ECHO " + key + "   " + *request_without_key).c_str());
			if (key.compare(this->server_process_ID) == 0)
				return result_h;
			else
				this->__send_REFUSE(result_h);
		}
	}

}

void Http_Server::__send_RESPONSE(const __Headers_t& Req, const std::string& buffer_to_send) {

	DWORD           result2;
	HTTP_RESPONSE   response;
	DWORD           bytesSent;

//
// Initialize the HTTP response structure.
//
	INITIALIZE_HTTP_RESPONSE(&response, 200, PSTR("OK"));

	ADD_KNOWN_HEADER(response, HttpHeaderContentType, "text/html");

	std::list<std::string> names = {"Access-Control-Allow-Origin" };
	std::list<std::string> values = { "*" };
	SET_UNKNOWN_HEADERS(&response.Headers.UnknownHeaderCount, &response.Headers.pUnknownHeaders, names, values);

	PSTR body_as_PSTR =(PSTR) buffer_to_send.c_str();

	HTTP_DATA_CHUNK dataChunk;
	dataChunk.DataChunkType = HttpDataChunkFromMemory;
	dataChunk.FromMemory.pBuffer = body_as_PSTR;
	dataChunk.FromMemory.BufferLength = (ULONG)strlen(body_as_PSTR);
	response.EntityChunkCount = 1;
	response.pEntityChunks = &dataChunk;

	result2 = HttpSendHttpResponse(
		this->hReqQueue,           // ReqQueueHandle
		Req.pRequest->RequestId, // Request ID
		0,                   // Flags
		&response,           // HTTP response
		NULL,                // pReserved1
		&bytesSent,          // bytes sent  (OPTIONAL)
		NULL,                // pReserved2  (must be NULL)
		0,                   // Reserved3   (must be 0)
		NULL,                // LPOVERLAPPED(OPTIONAL)
		NULL                 // pReserved4  (must be NULL)
	);

	if (result2 != NO_ERROR)
	{
		wprintf(L"HttpSendHttpResponse failed with %lu \n", result2);
		abort();
	}

	if (Req.pRequestBuffer)
		FREE_MEM(Req.pRequestBuffer);

}

void Http_Server::__send_REFUSE(const __Headers_t& Req) {

	DWORD           result2;
	HTTP_RESPONSE   response;
	DWORD           bytesSent;

	//
	// Initialize the HTTP response structure.
	//
	INITIALIZE_HTTP_RESPONSE(&response, 400, PSTR("REFUSED"));

	ADD_KNOWN_HEADER(response, HttpHeaderContentType, "text/html");

	result2 = HttpSendHttpResponse(
		this->hReqQueue,           // ReqQueueHandle
		Req.pRequest->RequestId, // Request ID
		0,                   // Flags
		&response,           // HTTP response
		NULL,                // pReserved1
		&bytesSent,          // bytes sent  (OPTIONAL)
		NULL,                // pReserved2  (must be NULL)
		0,                   // Reserved3   (must be 0)
		NULL,                // LPOVERLAPPED(OPTIONAL)
		NULL                 // pReserved4  (must be NULL)
	);

	if (result2 != NO_ERROR)
	{
		wprintf(L"HttpSendHttpResponse failed with %lu \n", result2);
		abort();
	}

	if (Req.pRequestBuffer)
		FREE_MEM(Req.pRequestBuffer);

}
