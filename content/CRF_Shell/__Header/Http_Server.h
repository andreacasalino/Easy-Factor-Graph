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

#include <string>
#include <list>
#include <windows.h>

//#define LOG_REQUEST_FROM_CLIENT



class I_JSON_composite {
public:
	virtual ~I_JSON_composite();
	void stringify(std::string* result) { result->clear(); this->__stringify(result); };
protected:
	I_JSON_composite(const I_JSON_composite&) { abort(); };
	I_JSON_composite() {};

	virtual I_JSON_composite*  __copy() const = 0;
	virtual void __stringify(std::string* result) = 0;
	void __clone_nested(I_JSON_composite* to_clone);

	I_JSON_composite* ___copy(const I_JSON_composite* to_copy) { return to_copy->__copy(); };
	void							 ___stringify(std::string* result, I_JSON_composite* to_str) { to_str->__stringify(result); };

	std::list<I_JSON_composite*>	  Nested_elements;
};

class JSON_tag : public I_JSON_composite {
public:
	JSON_tag() {};

	void Add_field(const std::string& name, const std::string& value);
	void Add_nested(const std::string& name, const I_JSON_composite* to_nest);
private:
	virtual I_JSON_composite*  __copy() const;
	virtual void __stringify(std::string* result);

	struct __field {
		std::string name;
		std::string value;
	};

	std::list<std::string>	 Nested_names;
	std::list<__field>			 Nested_fields;
};

class JSON_array : public I_JSON_composite {
public:
	JSON_array() {};

	void Append(const I_JSON_composite* to_nest);
private:
	virtual I_JSON_composite*  __copy() const;
	virtual void __stringify(std::string* result);
};




class Http_Server {
public:
	Http_Server(const std::string& port);
	~Http_Server();

	struct Reaction_Handler {
		virtual void get_reponse(std::string* response, const std::string& request) = 0; // return false when shut down command is received
	};

	void React_to_client(Reaction_Handler* reactor);
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
};

#endif // !_HASH_ENCODER_H__