#pragma once
#ifndef _COMMANDS_SHELL_H__
#define _COMMANDS_SHELL_H__

#include <string>
#include <list>
//#include "Stream_Socket.h"
#include "Http_Server.h"
#include "../../CRF/Header/Graphical_model.h"
#include <thread>
#include <mutex>

class CRF_Shell;
class JS_interface {
public:
	JS_interface(CRF_Shell* to_process);
	~JS_interface();

private:
	void __comm_loop();

// data
	bool							life;
	bool							inteface_closed;
	std::mutex*				pmutex;
	std::thread*				comm_loop;

	Http_Server		Connection;
	struct Reactor : public Http_Server::Reaction_Handler {
		Reactor(JS_interface* intrf) : pInterface(intrf) {};
		void get_reponse(std::string* response, const std::string& request);
	private:
		JS_interface*   pInterface;
	};
	Reactor*			_Reactor;
	CRF_Shell*	   ref_to_shell;
};



class CRF_Shell {
public:

	struct parsed_option {
		char	symbol;
		std::string  value;
	};

	struct Data;
	struct Command {
		virtual void process_received_command(const std::list<parsed_option>& options) = 0;

		const char&					get_symbol() { return this->symbol; };
	protected:
		Command(const char& sym, CRF_Shell* to_process) : symbol(sym), ref_to_shell(to_process) {};

		Data* get_data() { return &this->ref_to_shell->__Data; };
		void __send_result(std::list<float>& values) { this->ref_to_shell->__send_result(values); };
		void __send_result(std::list<std::string>& values) { this->ref_to_shell->__send_result(values); };

		CRF_Shell*					ref_to_shell;
	private:
		char								symbol;
	};

	CRF_Shell(const bool& activate_interface = false);
	virtual ~CRF_Shell();


	void do_staff();

	struct gaph_structure_info {
		struct node_info {
			std::string name;
			bool	observed_flag;
		};
		struct pot_info {
			std::string var_A;
			std::string var_B; //is "" for unary potential
		};

		std::list<node_info> Variables;
		std::list<pot_info>     Potentials;
	};
	void Get_actual_structure(gaph_structure_info* actual_structure); //return Variables and Potentials empty when update is not necessary

	void process_received_command(const std::string& command, const bool& use_critical_region = true);

	struct Data {
		bool																life;
		bool																update_needed;
		std::mutex*													pmutex;
		std::list<Segugio::Categoric_var*>		open_variables; //variables created but not already inserted in the graph
		Segugio::Graph*											Graph_actual;
		std::list<Command*>									Recognized_commands;
		JS_interface*													Interface;
	};
protected:
	Command* __parse_command(std::list<parsed_option>* result, const std::string& command_raw);

	virtual void __get_next_command_from_master(std::string* command_received) = 0;
	virtual void __send_result(std::list<float>& values) = 0;
	virtual void __send_result(std::list<std::string>& values) = 0;
// data

	Data __Data;
};



//class Shell_Socket : public CRF_Shell {
//public:
//	Shell_Socket(const bool& Activate_Interface = false);
//private:
//	void __next_command(std::string* command_received);
//// data
//	Stream_to_Server   connection;
//};



class Shell_Debugger : public CRF_Shell {
public:
	Shell_Debugger() : CRF_Shell() {};
private:
	void __get_next_command_from_master(std::string* command_received);
	void __send_result(std::list<float>& values);
	void __send_result(std::list<std::string>& values);
};

#endif