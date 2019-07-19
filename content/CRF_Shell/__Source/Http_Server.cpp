#include "../__Header/Http_Server.h"
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






//DWORD SendHttpResponse(
//	IN HANDLE        hReqQueue,
//	IN PHTTP_REQUEST pRequest,
//	IN USHORT        StatusCode,
//	IN PSTR          pReason,
//	IN PSTR          pEntityString
//)
//{
//	HTTP_RESPONSE   response;
//	HTTP_DATA_CHUNK dataChunk;
//	DWORD           result;
//	DWORD           bytesSent;
//
//	//
//	// Initialize the HTTP response structure.
//	//
//	INITIALIZE_HTTP_RESPONSE(&response, StatusCode, pReason);
//
//	//
//	// Add a known header.
//	//
//	ADD_KNOWN_HEADER(response, HttpHeaderContentType, "text/html");
//	//ADD_KNOWN_HEADER(response, HttpHeaderConnection, "Upgrade");
//	//ADD_KNOWN_HEADER(response, HttpHeaderUpgrade, "websocket");
//
//	if (pEntityString)
//	{
//		// 
//		// Add an entity chunk.
//		//
//		dataChunk.DataChunkType = HttpDataChunkFromMemory;
//		dataChunk.FromMemory.pBuffer = pEntityString;
//		dataChunk.FromMemory.BufferLength =
//			(ULONG)strlen(pEntityString);
//
//		response.EntityChunkCount = 1;
//		response.pEntityChunks = &dataChunk;
//	}
//
//	// 
//	// Because the entity body is sent in one call, it is not
//	// required to specify the Content-Length.
//	//
//
//	result = HttpSendHttpResponse(
//		hReqQueue,           // ReqQueueHandle
//		pRequest->RequestId, // Request ID
//		0,                   // Flags
//		&response,           // HTTP response
//		NULL,                // pReserved1
//		&bytesSent,          // bytes sent  (OPTIONAL)
//		NULL,                // pReserved2  (must be NULL)
//		0,                   // Reserved3   (must be 0)
//		NULL,                // LPOVERLAPPED(OPTIONAL)
//		NULL                 // pReserved4  (must be NULL)
//	);
//
//	if (result != NO_ERROR)
//	{
//		wprintf(L"HttpSendHttpResponse failed with %lu \n", result);
//	}
//
//	return result;
//}
//
//
//
//DWORD SendHttpPostResponse(
//	IN HANDLE        hReqQueue,
//	IN PHTTP_REQUEST pRequest
//)
//{
//	HTTP_RESPONSE   response;
//	DWORD           result;
//	DWORD           bytesSent;
//	PUCHAR          pEntityBuffer;
//	ULONG           EntityBufferLength;
//	ULONG           BytesRead;
//	ULONG           TempFileBytesWritten;
//	HANDLE          hTempFile;
//	TCHAR           szTempName[MAX_PATH + 1];
//	CHAR            szContentLength[MAX_ULONG_STR];
//	HTTP_DATA_CHUNK dataChunk;
//	ULONG           TotalBytesRead = 0;
//
//	BytesRead = 0;
//	hTempFile = INVALID_HANDLE_VALUE;
//
//	//
//	// Allocate space for an entity buffer. Buffer can be increased 
//	// on demand.
//	//
//	EntityBufferLength = 2048;
//	pEntityBuffer = (PUCHAR)ALLOC_MEM(EntityBufferLength);
//
//	if (pEntityBuffer == NULL)
//	{
//		result = ERROR_NOT_ENOUGH_MEMORY;
//		wprintf(L"Insufficient resources \n");
//		goto Done;
//	}
//
//	//
//	// Initialize the HTTP response structure.
//	//
//	INITIALIZE_HTTP_RESPONSE(&response, 200, "OK");
//
//	//
//	// For POST, echo back the entity from the
//	// client
//	//
//	// NOTE: If the HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY flag had been
//	//       passed with HttpReceiveHttpRequest(), the entity would 
//	//       have been a part of HTTP_REQUEST (using the pEntityChunks
//	//       field). Because that flag was not passed, there are no
//	//       o entity bodies in HTTP_REQUEST.
//	//
//
//	if (pRequest->Flags & HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS)
//	{
//		// The entity body is sent over multiple calls. Collect 
//		// these in a file and send back. Create a temporary 
//		// file.
//		//
//
//		if (GetTempFileName(
//			L".",
//			L"New",
//			0,
//			szTempName
//		) == 0)
//		{
//			result = GetLastError();
//			wprintf(L"GetTempFileName failed with %lu \n", result);
//			goto Done;
//		}
//
//		hTempFile = CreateFile(
//			szTempName,
//			GENERIC_READ | GENERIC_WRITE,
//			0,                  // Do not share.
//			NULL,               // No security descriptor.
//			CREATE_ALWAYS,      // Overrwrite existing.
//			FILE_ATTRIBUTE_NORMAL,    // Normal file.
//			NULL
//		);
//
//		if (hTempFile == INVALID_HANDLE_VALUE)
//		{
//			result = GetLastError();
//			wprintf(L"Cannot create temporary file. Error %lu \n",
//				result);
//			goto Done;
//		}
//
//		do
//		{
//			//
//			// Read the entity chunk from the request.
//			//
//			BytesRead = 0;
//			result = HttpReceiveRequestEntityBody(
//				hReqQueue,
//				pRequest->RequestId,
//				0,
//				pEntityBuffer,
//				EntityBufferLength,
//				&BytesRead,
//				NULL
//			);
//
//			switch (result)
//			{
//			case NO_ERROR:
//
//				if (BytesRead != 0)
//				{
//					TotalBytesRead += BytesRead;
//					WriteFile(
//						hTempFile,
//						pEntityBuffer,
//						BytesRead,
//						&TempFileBytesWritten,
//						NULL
//					);
//				}
//				break;
//
//			case ERROR_HANDLE_EOF:
//
//				//
//				// The last request entity body has been read.
//				// Send back a response. 
//				//
//				// To illustrate entity sends via 
//				// HttpSendResponseEntityBody, the response will 
//				// be sent over multiple calls. To do this,
//				// pass the HTTP_SEND_RESPONSE_FLAG_MORE_DATA
//				// flag.
//
//				if (BytesRead != 0)
//				{
//					TotalBytesRead += BytesRead;
//					WriteFile(
//						hTempFile,
//						pEntityBuffer,
//						BytesRead,
//						&TempFileBytesWritten,
//						NULL
//					);
//				}
//
//				//
//				// Because the response is sent over multiple
//				// API calls, add a content-length.
//				//
//				// Alternatively, the response could have been
//				// sent using chunked transfer encoding, by  
//				// passimg "Transfer-Encoding: Chunked".
//				//
//
//				// NOTE: Because the TotalBytesread in a ULONG
//				//       are accumulated, this will not work
//				//       for entity bodies larger than 4 GB. 
//				//       For support of large entity bodies,
//				//       use a ULONGLONG.
//				// 
//
//
//				sprintf_s(szContentLength, MAX_ULONG_STR, "%lu", TotalBytesRead);
//
//				ADD_KNOWN_HEADER(
//					response,
//					HttpHeaderContentLength,
//					szContentLength
//				);
//
//				result =
//					HttpSendHttpResponse(
//						hReqQueue,           // ReqQueueHandle
//						pRequest->RequestId, // Request ID
//						HTTP_SEND_RESPONSE_FLAG_MORE_DATA,
//						&response,       // HTTP response
//						NULL,            // pReserved1
//						&bytesSent,      // bytes sent-optional
//						NULL,            // pReserved2
//						0,               // Reserved3
//						NULL,            // LPOVERLAPPED
//						NULL             // pReserved4
//					);
//
//				if (result != NO_ERROR)
//				{
//					wprintf(
//						L"HttpSendHttpResponse failed with %lu \n",
//						result
//					);
//					goto Done;
//				}
//
//				//
//				// Send entity body from a file handle.
//				//
//				dataChunk.DataChunkType =
//					HttpDataChunkFromFileHandle;
//
//				dataChunk.FromFileHandle.
//					ByteRange.StartingOffset.QuadPart = 0;
//
//				dataChunk.FromFileHandle.
//					ByteRange.Length.QuadPart =
//					HTTP_BYTE_RANGE_TO_EOF;
//
//				dataChunk.FromFileHandle.FileHandle = hTempFile;
//
//				result = HttpSendResponseEntityBody(
//					hReqQueue,
//					pRequest->RequestId,
//					0,           // This is the last send.
//					1,           // Entity Chunk Count.
//					&dataChunk,
//					NULL,
//					NULL,
//					0,
//					NULL,
//					NULL
//				);
//
//				if (result != NO_ERROR)
//				{
//					wprintf(
//						L"HttpSendResponseEntityBody failed %lu\n",
//						result
//					);
//				}
//
//				goto Done;
//
//				break;
//
//
//			default:
//				wprintf(
//					L"HttpReceiveRequestEntityBody failed with %lu \n",
//					result);
//				goto Done;
//			}
//
//		} while (TRUE);
//	}
//	else
//	{
//		// This request does not have an entity body.
//		//
//
//		result = HttpSendHttpResponse(
//			hReqQueue,           // ReqQueueHandle
//			pRequest->RequestId, // Request ID
//			0,
//			&response,           // HTTP response
//			NULL,                // pReserved1
//			&bytesSent,          // bytes sent (optional)
//			NULL,                // pReserved2
//			0,                   // Reserved3
//			NULL,                // LPOVERLAPPED
//			NULL                 // pReserved4
//		);
//		if (result != NO_ERROR)
//		{
//			wprintf(L"HttpSendHttpResponse failed with %lu \n",
//				result);
//		}
//	}
//
//Done:
//
//	if (pEntityBuffer)
//	{
//		FREE_MEM(pEntityBuffer);
//	}
//
//	if (INVALID_HANDLE_VALUE != hTempFile)
//	{
//		CloseHandle(hTempFile);
//		DeleteFile(szTempName);
//	}
//
//	return result;
//}
//
//
//
//DWORD DoReceiveRequests(
//	IN HANDLE hReqQueue
//)
//{
//	ULONG              result;
//	HTTP_REQUEST_ID    requestId;
//	DWORD              bytesRead;
//	PHTTP_REQUEST      pRequest;
//	PCHAR              pRequestBuffer;
//	ULONG              RequestBufferLength;
//
//	//
//	// Allocate a 2 KB buffer. This size should work for most 
//	// requests. The buffer size can be increased if required. Space
//	// is also required for an HTTP_REQUEST structure.
//	//
//	RequestBufferLength = sizeof(HTTP_REQUEST) + 2048;
//	pRequestBuffer = (PCHAR)ALLOC_MEM(RequestBufferLength);
//
//	if (pRequestBuffer == NULL)
//	{
//		return ERROR_NOT_ENOUGH_MEMORY;
//	}
//
//	pRequest = (PHTTP_REQUEST)pRequestBuffer;
//
//	//
//	// Wait for a new request. This is indicated by a NULL 
//	// request ID.
//	//
//
//	HTTP_SET_NULL_ID(&requestId);
//
//	while (true)
//	{
//		RtlZeroMemory(pRequest, RequestBufferLength);
//
//		result = HttpReceiveHttpRequest(
//			hReqQueue,          // Req Queue
//			requestId,          // Req ID
//			0,                  // Flags
//			pRequest,           // HTTP request buffer
//			RequestBufferLength,// req buffer length
//			&bytesRead,         // bytes received
//			NULL                // LPOVERLAPPED
//		);
//
//
//
//		if (NO_ERROR == result)
//		{
//			//
//			// Worked! 
//			// 
//			switch (pRequest->Verb)
//			{
//			case HttpVerbGET:
//				wprintf(L"Got a GET request for %ws \n",
//					pRequest->CookedUrl.pFullUrl);
//
//				result = SendHttpResponse(
//					hReqQueue,
//					pRequest,
//					101,
//					PSTR("OK"),
//					PSTR("Hey! You hit the server \r\n")
//				);
//				break;
//
//			case HttpVerbPOST:
//
//				wprintf(L"Got a POST request for %ws \n",
//					pRequest->CookedUrl.pFullUrl);
//
//				result = SendHttpPostResponse(hReqQueue, pRequest);
//				break;
//
//			default:
//				/*wprintf(L"Got a unknown request for %ws \n",
//					pRequest->CookedUrl.pFullUrl);
//
//				result = SendHttpResponse(
//					hReqQueue,
//					pRequest,
//					503,
//					PSTR("Not Implemented"),
//					NULL
//				);*/
//				abort();
//				break;
//			}
//
//			if (result != NO_ERROR)
//			{
//				break;
//			}
//
//			//
//			// Reset the Request ID to handle the next request.
//			//
//			HTTP_SET_NULL_ID(&requestId);
//		}
//		else if (result == ERROR_MORE_DATA)
//		{
//			//
//			// The input buffer was too small to hold the request
//			// headers. Increase the buffer size and call the 
//			// API again. 
//			//
//			// When calling the API again, handle the request
//			// that failed by passing a RequestID.
//			//
//			// This RequestID is read from the old buffer.
//			//
//			requestId = pRequest->RequestId;
//
//			//
//			// Free the old buffer and allocate a new buffer.
//			//
//			RequestBufferLength = bytesRead;
//			FREE_MEM(pRequestBuffer);
//			pRequestBuffer = (PCHAR)ALLOC_MEM(RequestBufferLength);
//
//			if (pRequestBuffer == NULL)
//			{
//				result = ERROR_NOT_ENOUGH_MEMORY;
//				break;
//			}
//
//			pRequest = (PHTTP_REQUEST)pRequestBuffer;
//
//		}
//		else if (ERROR_CONNECTION_INVALID == result &&
//			!HTTP_IS_NULL_ID(&requestId))
//		{
//			// The TCP connection was corrupted by the peer when
//			// attempting to handle a request with more buffer. 
//			// Continue to the next request.
//
//			HTTP_SET_NULL_ID(&requestId);
//		}
//		else
//		{
//			break;
//		}
//
//	}
//
//	if (pRequestBuffer)
//	{
//		FREE_MEM(pRequestBuffer);
//	}
//
//	return result;
//}





void __append(std::string* to_modify, const std::string& to_append) {

	for (auto it = to_append.begin(); it != to_append.end(); it++)
		to_modify->push_back(*it);

}

I_JSON_composite::~I_JSON_composite() {

	for (auto it = this->Nested_elements.begin();
		it != this->Nested_elements.end(); it++)
		delete *it;

}

void I_JSON_composite::__clone_nested(I_JSON_composite* to_clone) {

	for (auto it = to_clone->Nested_elements.begin();
		it != to_clone->Nested_elements.end(); it++)
		this->Nested_elements.push_back(this->___copy(*it));

}

void JSON_tag::Add_field(const std::string& name, const std::string& value) {

	this->Nested_fields.push_back(__field());
	this->Nested_fields.back().name = name;
	this->Nested_fields.back().value = value;

}

void JSON_tag::Add_nested(const std::string& name, const I_JSON_composite* to_nest) {

	this->Nested_names.push_back(name);
	this->Nested_elements.push_back(this->___copy(to_nest));

}

I_JSON_composite*  JSON_tag::__copy() const {

	auto __new = new JSON_tag();
	__new->__clone_nested((I_JSON_composite*)this);

	__new->Nested_names = this->Nested_names;
	__new->Nested_fields = this->Nested_fields;

	return __new;

}

void JSON_tag::__stringify(std::string* result) {

	size_t N_elements = this->Nested_elements.size() + this->Nested_fields.size(), k = 1;

	result->push_back('{');
	for (auto it = this->Nested_fields.begin();
		it != this->Nested_fields.end(); it++) {
		__append(result, "\"" + it->name + "\":\"" + it->value + "\"");

		if (k != N_elements)
			result->push_back(',');
		k++;
	}
	auto it_name = this->Nested_names.begin();
	for (auto it = this->Nested_elements.begin();
		it != this->Nested_elements.end(); it++) {
		__append(result, "\"" + *it_name + "\":");
		this->___stringify(result, *it);
		it_name++;

		if (k != N_elements)
			result->push_back(',');
		k++;
	}
	result->push_back('}');

}

void JSON_array::Append(const I_JSON_composite* to_nest) {

	this->Nested_elements.push_back(this->___copy(to_nest));

}

I_JSON_composite*  JSON_array::__copy() const {

	auto __new = new JSON_array();
	__new->__clone_nested((I_JSON_composite*)this);
	return __new;

}

void JSON_array::__stringify(std::string* result) {

	size_t N_elements = this->Nested_elements.size(), k = 1;

	result->push_back('[');
	for (auto it = this->Nested_elements.begin();
		it != this->Nested_elements.end(); it++) {
		this->___stringify(result, *it);

		if (k != N_elements)
			result->push_back(',');
		k++;
	}
	result->push_back(']');

}

void JSON_numerical_array::__stringify(std::string* result) {

	result->push_back('[');
	size_t L = this->Values.size();
	if (L > 0) {
		if (L == 1)
			(*result) += to_string(this->Values.front());
		else {
			auto it = this->Values.begin();
			for (size_t k = 0; k < (L - 1); k++) {
				(*result) += to_string(*it);
				(*result) += ',';
				it++;
			}
			(*result) += to_string(this->Values.back());
		}
	}

	result->push_back(']');

}






Http_Server::Http_Server(const std::string& port) :
hReqQueue(NULL),  port_to_accept(port) {

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
