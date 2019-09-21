#include "../__Header/CRF_Shell.h"
#include <string>
#include <sstream>
#include <iostream>
using namespace std;
using namespace Segugio;

#define PORT_SERVER string("8080")



JS_interface::JS_interface(CRF_Shell* to_process, const std::string& port) : Connection(port), life(true), inteface_closed(false), ref_to_shell(to_process) {

	this->_Reactor = new Reactor(this);
	this->pmutex = new mutex();
	this->comm_loop = new thread(&JS_interface::__comm_loop, this);

	system("Interface.html");

}

JS_interface::~JS_interface() {

	this->pmutex->lock();
	this->life = false;
	this->pmutex->unlock();

	this->comm_loop->join();

	delete this->pmutex;
	delete this->comm_loop;
	delete this->_Reactor;

}

void JS_interface::__comm_loop() {

	bool life_copy;
	while (true) {
		this->pmutex->lock();
		life_copy = this->life;
		this->pmutex->unlock();

		if (!life_copy)
			break;

		this->Connection.React_to_client(this->_Reactor);
	}

	while (!this->inteface_closed) {
		this->Connection.React_to_client(this->_Reactor);
	}

}

void get_JSON_graph(string* JSON_to_send, const CRF_Shell::gaph_structure_info& structure);
void JS_interface::Reactor::get_reponse(std::string* response, const std::string& request) {

	bool life_copy;
	this->pInterface->pmutex->lock();
	life_copy = this->pInterface->life;
	this->pInterface->pmutex->unlock();

	if (request.compare("Upd") == 0) {
		/////////////////////////
		//// Update request ////
		/////////////////////////
		if (!life_copy) {
			*response = "End";
			this->pInterface->inteface_closed = true;
			return;
		}

		CRF_Shell::gaph_structure_info actual_structure;
		this->pInterface->ref_to_shell->Get_actual_structure(&actual_structure);

		if (!actual_structure.Variables.empty()) {
			get_JSON_graph(response , actual_structure);
		}
		else
			*response = "null";
	}
	else {
		/////////////////////////////
		//// Command request ////
		/////////////////////////////

		if (!life_copy)
			return;

		cout << request << endl;
		this->pInterface->ref_to_shell->process_received_command(request);
		*response = "Ack";
	}

};




struct __var_info {
	const string*	    name;
	int				        id;
};
const __var_info* find_node(const string& var_name, const list<__var_info>& nodes_info) {

	for (auto it = nodes_info.begin(); it != nodes_info.end(); it++) {
		if (it->name->compare(var_name) == 0) {
			return &(*it);
			break;
		}
	}
	abort();

};
void get_JSON_graph(string* JSON_to_send, const CRF_Shell::gaph_structure_info& structure) {

	int id_counter = 0;

	JSON_array nodes;
	list<__var_info> nodes_info;
	for (auto it = structure.Variables.begin(); it != structure.Variables.end(); it++) {
		JSON_tag temp_node;

		temp_node.Add_field("label" , it->name);
		temp_node.Add_field("shape", "image");
		if(it->observed_flag) 
			temp_node.Add_field("image", "../CRF/doc/picture/svg/Variable_Observed.svg");
		else
			temp_node.Add_field("image", "../CRF/doc/picture/svg/Variable.svg");
		temp_node.Add_field("color" , "#000000");
		temp_node.Add_field("id", to_string(id_counter));
		nodes_info.push_back(__var_info());
		nodes_info.back().name = &it->name;
		nodes_info.back().id = id_counter;
		id_counter++;

		nodes.Append(&temp_node);
	}

	JSON_array edges;
	for (auto it = structure.Potentials.begin(); it != structure.Potentials.end(); it++) {
		JSON_tag temp_node;

		temp_node.Add_field("id" , to_string(id_counter));
		id_counter++;
		temp_node.Add_field("size", "10");
		temp_node.Add_field("shape", "image");
		temp_node.Add_field("image", "../CRF/doc/picture/svg/Potential_Shape.svg");
		temp_node.Add_field("color", "#000000");

		nodes.Append(&temp_node);

		JSON_tag temp_edge;
		temp_edge.Add_field("from", to_string(find_node(it->var_A, nodes_info)->id) );
		temp_edge.Add_field("to", to_string(id_counter - 1));
		edges.Append(&temp_edge);
		if (!it->var_B.empty()) {
			JSON_tag temp_edge2;
			temp_edge2.Add_field("from", to_string(find_node(it->var_B, nodes_info)->id));
			temp_edge2.Add_field("to", to_string(id_counter - 1));
			edges.Append(&temp_edge2);
		}
	}

	JSON_tag Str;
	Str.Add_nested("nodes", &nodes);
	Str.Add_nested("edges", &edges);

	Str.stringify(JSON_to_send);

}






void find_with_that_symbol(list<const string*>* trovate , const char& sym,const  list<CRF_Shell::parsed_option>& opts) {

	trovate->clear();
	for (auto it = opts.begin(); it != opts.end(); it++) {
		if (it->symbol == sym)
			trovate->push_back(&it->value);
	}

}

void Init_Commands(list<CRF_Shell::Command*>* comms, CRF_Shell* shell) {

	comms->clear();

// T
	struct Terminator : public CRF_Shell::Command {
		Terminator(CRF_Shell* shell) : CRF_Shell::Command('T', shell) {};
		void process_received_command(const std::list<CRF_Shell::parsed_option>& options) {

			this->get_data()->life = false;

		}
	};
	comms->push_back(new Terminator(shell));

// V
	struct Variable_Creator : public CRF_Shell::Command {
		Variable_Creator(CRF_Shell* shell) : CRF_Shell::Command('V', shell) {};
		void process_received_command(const std::list<CRF_Shell::parsed_option>& options) {

			this->get_data()->update_needed = true;

			list<const string*> names;
			find_with_that_symbol(&names, 'v', options);

			list<const string*> sizes;
			find_with_that_symbol(&sizes, 's', options);

			if (names.size() != sizes.size()) {
				system("ECHO invalid command, ignored");
				return;
			}

			if (!names.empty()) {
				if (this->get_data()->Graph_actual == NULL)
					this->get_data()->Graph_actual = new Graph();
			}

			auto it_size = sizes.begin();
			Categoric_var* clone;
			for (auto it_names = names.begin(); it_names != names.end(); it_names++) {
				clone = this->get_data()->Graph_actual->Find_Variable(**it_names);

				if (clone == NULL) {
					for (auto it = this->get_data()->open_variables.begin(); it != this->get_data()->open_variables.end(); it++) {
						if ((*it)->Get_name().compare(**it_names) == 0) {
							system("ECHO invalid command, ignored");
							return;
						}
					}

					this->get_data()->open_variables.push_back(new Categoric_var((size_t)atoi((*it_size)->c_str()), **it_names));
				}
				else {
					system("ECHO invalid command, ignored");
					return;
				}

				it_size++;
			}

		}
	};
	comms->push_back(new Variable_Creator(shell));

// X
	struct XML_Creator : public CRF_Shell::Command {
		XML_Creator(CRF_Shell* shell) : CRF_Shell::Command('X', shell) {};
		void process_received_command(const std::list<CRF_Shell::parsed_option>& options) {

			this->get_data()->update_needed = true;

			for (auto it = this->get_data()->open_variables.begin();
				it != this->get_data()->open_variables.end(); it++)
				delete *it;
			this->get_data()->open_variables.clear();
			if (this->get_data()->Graph_actual != NULL) delete this->get_data()->Graph_actual;
			this->get_data()->Graph_actual = NULL;


			const string* config_file;
			const string* folder = NULL;

			list<const string*> temp;
			find_with_that_symbol(&temp, 'p', options);
			if (temp.empty()) {
				system("ECHO invalid command, ignored");
				return;
			}

			config_file = temp.front();

			find_with_that_symbol(&temp, 'f', options);
			if (!temp.empty()) {
				folder = temp.front();
			}

			if (folder == NULL)
				this->get_data()->Graph_actual = new Graph(*config_file);
			else
				this->get_data()->Graph_actual = new Graph(*config_file, *folder);
			this->get_data()->Graph_actual->Set_Observation_Set_var({});
			this->get_data()->Graph_actual->Set_Observation_Set_val({});
		}
	};
	comms->push_back(new XML_Creator(shell));

// B
	struct Binary_Potential_handler : public CRF_Shell::Command {
		Binary_Potential_handler(CRF_Shell* shell) : CRF_Shell::Command('B', shell) {};
		void process_received_command(const std::list<CRF_Shell::parsed_option>& options) {

			this->get_data()->update_needed = true;

			list<const string*> var_names;
			find_with_that_symbol(&var_names, 'v', options);
			if (var_names.size() != 2) {
				system("ECHO invalid command, ignored");
				return;
			}

			struct Peer {
				Peer(Binary_Potential_handler* hndlr, const string* name) {

					this->Variable = hndlr->get_data()->Graph_actual->Find_Variable(*name);
					if (this->Variable == NULL) {
						auto op = &hndlr->get_data()->open_variables;
						for (auto it = op->begin(); it != op->end(); it++) {
							if ((*it)->Get_name().compare(*name) == 0) {
								this->Variable = *it;
								this->already_in_graph = false;
								break;
							}
						}
					}
					else
						this->already_in_graph = true;

				}

				Categoric_var*	 Variable;
				bool			 already_in_graph;
			};
			Peer V1(this, var_names.front()), V2(this, var_names.back());

			if (V1.Variable == NULL) {
				system("ECHO invalid command, ignored");
				return;
			}
			if (V2.Variable == NULL) {
				system("ECHO invalid command, ignored");
				return;
			}

			Potential_Shape* new_shape;

			list<const string*> temp;
			find_with_that_symbol(&temp, 's', options);
			if (!temp.empty())
				new_shape = new Potential_Shape({ V1.Variable,V2.Variable }, temp.front());
			else {
				find_with_that_symbol(&temp, 'c', options);
				if (!temp.empty()) {
					if (temp.front()->compare("T") == 0)
						new_shape = new Potential_Shape({ V1.Variable,V2.Variable }, true);
					else if (temp.front()->compare("F") == 0)
						new_shape = new Potential_Shape({ V1.Variable,V2.Variable }, false);
					else {
						system("ECHO invalid command, ignored");
						return;
					}
				}
				else {
					system("ECHO invalid command, ignored");
					return;
				}
			}

			size_t Size_old = this->get_data()->Graph_actual->Get_structure_size();
			find_with_that_symbol(&temp, 'w', options);
			if (!temp.empty()) {
				auto exp_pot = new Potential_Exp_Shape(new_shape, (float)atof(temp.front()->c_str()));
				this->get_data()->Graph_actual->Insert(exp_pot);
				delete exp_pot;
			}
			else {
				this->get_data()->Graph_actual->Insert(new_shape);
				delete new_shape;
			}
			if (Size_old == 0) {
				this->get_data()->Graph_actual->Set_Observation_Set_var({});
				this->get_data()->Graph_actual->Set_Observation_Set_val({});
			}

			if (!V1.already_in_graph) {
				auto tempp = V1.Variable;
				this->get_data()->open_variables.remove(tempp);
				delete tempp;
			}
			if (!V2.already_in_graph) {
				auto tempp = V2.Variable;
				this->get_data()->open_variables.remove(tempp);
				delete tempp;
			}

		}
	};
	comms->push_back(new Binary_Potential_handler(shell));

// U
	struct Unary_Potential_handler : public CRF_Shell::Command {
		Unary_Potential_handler(CRF_Shell* shell) : CRF_Shell::Command('U', shell) {};
		void process_received_command(const std::list<CRF_Shell::parsed_option>& options) {

			this->get_data()->update_needed = true; 

			list<const string*> var_names;
			find_with_that_symbol(&var_names, 'v', options);
			if (var_names.size() != 1) {
				system("ECHO invalid command, ignored");
				return;
			}

			Categoric_var* V = this->get_data()->Graph_actual->Find_Variable(*var_names.front());
			if (V == NULL) {
				system("ECHO invalid command, ignored");
				return;
			}

			list<const string*> temp;

			Potential_Shape* new_shape;
			find_with_that_symbol(&temp, 's', options);
			if (temp.size() != 1) {
				system("ECHO invalid command, ignored");
				return;
			}

			new_shape = new Potential_Shape({ V }, temp.front());

			find_with_that_symbol(&temp, 'w', options);
			if (temp.empty()) {
				this->get_data()->Graph_actual->Insert(new_shape);
				delete new_shape;
			}
			else {
				auto exp_pot = new Potential_Exp_Shape(new_shape, (float)atof(temp.front()->c_str()));
				this->get_data()->Graph_actual->Insert(exp_pot);
				delete exp_pot;
			}

		};
	};
	comms->push_back(new Unary_Potential_handler(shell));

// O
	struct Observation_handler : public CRF_Shell::Command {
		Observation_handler(CRF_Shell* shell) : CRF_Shell::Command('O', shell) {};
		void process_received_command(const std::list<CRF_Shell::parsed_option>& options) {

			this->get_data()->update_needed = true;

			list<const string*> variables;
			find_with_that_symbol(&variables, 'v', options);

			list<const string*> vals;
			find_with_that_symbol(&vals, 'n', options);

			if (vals.size() != variables.size()) {
				system("ECHO invalid command, ignored");
				return;
			}

			if (variables.empty()) {
				this->get_data()->Graph_actual->Set_Observation_Set_var({});
				this->get_data()->Graph_actual->Set_Observation_Set_val({});
				return;
			}

			list<Categoric_var*> Ob_vars;
			this->get_data()->Graph_actual->Get_Actual_Observation_Set(&Ob_vars);
			list<size_t>		 Ob_vals;
			this->get_data()->Graph_actual->Get_Observation_Set_val(&Ob_vals);
			auto it_val = vals.begin();
			Categoric_var* temp_ref;
			for (auto it_var = variables.begin(); it_var != variables.end(); it_var++) {
				temp_ref = this->get_data()->Graph_actual->Find_Variable(**it_var);
				if (temp_ref == NULL) {
					system("ECHO invalid command, ignored");
					return;
				}

				Ob_vars.push_back(temp_ref);
				Ob_vals.push_back((size_t)atoi((*it_val)->c_str()));

				it_val++;
			}
			this->get_data()->Graph_actual->Set_Observation_Set_var(Ob_vars);
			this->get_data()->Graph_actual->Set_Observation_Set_val(Ob_vals);
		};
	};
	comms->push_back(new Observation_handler(shell));

// I
	struct Inference_handler : public CRF_Shell::Command {
		Inference_handler(CRF_Shell* shell) : CRF_Shell::Command('I', shell) {};
		void process_received_command(const std::list<CRF_Shell::parsed_option>& options) {

			list<const string*> variable;
			find_with_that_symbol(&variable, 'v', options);
			if (variable.size() != 1) {
				system("ECHO invalid command, ignored");
				return;
			}

			if (this->get_data()->Graph_actual->Find_Variable(*variable.front()) == NULL) {
				system("ECHO invalid command, ignored");
				return;
			}

			list<float> marginals;
			this->get_data()->Graph_actual->Get_marginal_distribution(&marginals, this->get_data()->Graph_actual->Find_Variable(*variable.front()));
			
			this->__send_result(marginals);
		}
	};
	comms->push_back(new Inference_handler(shell));

// M
	struct MAP_handler : public CRF_Shell::Command {
		MAP_handler(CRF_Shell* shell) : CRF_Shell::Command('M', shell) {};
		void process_received_command(const std::list<CRF_Shell::parsed_option>& options) {

			list<size_t> MAP;
			this->get_data()->Graph_actual->MAP_on_Hidden_set(&MAP);

			list<float> MAP_as_float;
			for (auto it = MAP.begin(); it != MAP.end(); it++)
				MAP_as_float.push_back((float)(*it));
			this->__send_result(MAP_as_float);

		}
	};
	comms->push_back(new MAP_handler(shell));

// S
	struct Set_handler : public CRF_Shell::Command {
		Set_handler(CRF_Shell* shell) : CRF_Shell::Command('S', shell) {};
		void process_received_command(const std::list<CRF_Shell::parsed_option>& options) {

			list<const string*> kind;
			find_with_that_symbol(&kind, 'k', options);
			if (kind.size() != 1) {
				system("ECHO invalid command, ignored");
				return;
			}

			list<Categoric_var*> vars;
			if (kind.front()->front() == 'O')
				this->get_data()->Graph_actual->Get_Actual_Observation_Set(&vars);
			else if(kind.front()->front() == 'H')
				this->get_data()->Graph_actual->Get_Actual_Hidden_Set(&vars);
			else {
				system("ECHO invalid command, ignored");
				return;
			}

			list<string> names;
			for (auto it = vars.begin(); it != vars.end(); it++)
				names.push_back((*it)->Get_name());

			this->__send_result(names);
		}
	};
	comms->push_back(new Set_handler(shell));

// C
	struct Sequence_commands : public CRF_Shell::Command {
		Sequence_commands(CRF_Shell* shell) : CRF_Shell::Command('C', shell) {};
		void process_received_command(const std::list<CRF_Shell::parsed_option>& options) {

			list<const string*> temp;
			find_with_that_symbol(&temp, 'f', options);
			if (temp.size() != 1) {
				system("ECHO invalid command, ignored");
				return;
			}

			ifstream f_bat(*temp.front());
			if (!f_bat.is_open()) {
				f_bat.close();
				system("ECHO invalid command, ignored");
				return;
			}

			string comm;
			while (!f_bat.eof()) {
				getline(f_bat, comm);

				if (!this->get_data()->life) {
					f_bat.close();
					break;
				}

				this->ref_to_shell->process_received_command(comm , false);
			}

			f_bat.close();
		}
	};
	comms->push_back(new Sequence_commands(shell));

//J 
	struct Interface_activator : public CRF_Shell::Command {
		Interface_activator(CRF_Shell* shell) : CRF_Shell::Command('J', shell) {};
		void process_received_command(const std::list<CRF_Shell::parsed_option>& options) {

			if (this->get_data()->Interface == NULL) {
				this->get_data()->update_needed = true;
				this->get_data()->Interface = new JS_interface(this->ref_to_shell, PORT_SERVER);
			}

		}
	};
	comms->push_back(new Interface_activator(shell));

}

void parse_line(const string& riga, list<string>* slices) {

	istringstream iss(riga);
	slices->clear();
	while (true) {
		if (iss.eof()) {
			break;
		}
		slices->push_back(string());
		iss >> slices->back();
	}

};

void remove_empty_slices(list<string>* slices) {

	auto it = slices->begin();
	while (it != slices->end()) {
		if (it->empty()) it = slices->erase(it);
		else it++;
	}

}

CRF_Shell::Command* CRF_Shell::__parse_command(std::list<parsed_option>* result, const std::string& command_raw) {

	result->clear();
	list<string> slices;
	parse_line(command_raw, &slices);
	remove_empty_slices(&slices);

	if (slices.empty()) return NULL;

	if (slices.front().size() != 1) return NULL;

	char command_name = slices.front()[0];
	slices.pop_front();

	while (!slices.empty()) {
		result->push_back(parsed_option());

		if (slices.front()[0] != '-')
			return NULL;

		if (slices.front()[2] != ':')
			return NULL;

		if (slices.front()[3] != '\"')
			return NULL;

		if (slices.front().back() != '\"')
			return NULL;

		result->back().symbol = slices.front()[1];
		result->back().value = string(slices.front(), 4);
		result->back().value.pop_back();
		slices.pop_front();
	}

	for (auto it = this->__Data.Recognized_commands.begin(); it != this->__Data.Recognized_commands.end(); it++) {
		if ((*it)->get_symbol() == command_name)
			return *it;
	}
	return NULL;

}

CRF_Shell::CRF_Shell(const bool& activate_interface) {

	this->__Data.life = true;
	this->__Data.update_needed = false;
	this->__Data.pmutex = new mutex();
	this->__Data.Graph_actual = NULL;

	Init_Commands(&this->__Data.Recognized_commands, this);

	this->__Data.Interface = NULL;

	//check for command names
	list<Command*> open = this->__Data.Recognized_commands;
	list<Command*>::iterator it;
	while (!open.empty()) {
		it = open.begin();
		it++;
		for (it; it != open.end(); it++) {
			if ((*it)->get_symbol() == open.front()->get_symbol()) {
				system("ECHO found invalid command lists");
				abort();
			}
		}
		open.pop_front();
	}

	if(activate_interface)
		this->process_received_command("J");

};

void CRF_Shell::do_staff() {

	bool life_copy;
	string command;
	while (true) {
		this->__Data.pmutex->lock();
		life_copy = this->__Data.life;
		this->__Data.pmutex->unlock();

		if (!life_copy)
			break;

		this->__get_next_command_from_master(&command);
		this->process_received_command(command);
	};

}

CRF_Shell::~CRF_Shell() {

	if (this->__Data.Interface != NULL)
		delete this->__Data.Interface;

	this->process_received_command("X");

	delete this->__Data.pmutex;

	for (auto it = this->__Data.Recognized_commands.begin(); it != this->__Data.Recognized_commands.end(); it++)
		delete *it;

}

void CRF_Shell::Get_actual_structure(gaph_structure_info* actual_structure) {

	actual_structure->Potentials.clear();
	actual_structure->Variables.clear();

	this->__Data.pmutex->lock();

	if (!this->__Data.update_needed) {
		this->__Data.pmutex->unlock();
		return;
	}

	for (auto it = this->__Data.open_variables.begin(); it != this->__Data.open_variables.end(); it++) {
		actual_structure->Variables.push_back(gaph_structure_info::node_info());
		actual_structure->Variables.back().name = (*it)->Get_name();
		actual_structure->Variables.back().observed_flag = false;
	}

	if (this->__Data.Graph_actual == NULL) {
		this->__Data.pmutex->unlock();
		return;
	}

	list<const Potential*> pot_list;
	this->__Data.Graph_actual->Get_structure(&pot_list);
	if (!pot_list.empty()) {
		list<Categoric_var*> observations;
		this->__Data.Graph_actual->Get_Actual_Observation_Set(&observations);
		list<Categoric_var*> hidden;
		this->__Data.Graph_actual->Get_Actual_Hidden_Set(&hidden);

		for (auto it = observations.begin(); it != observations.end(); it++) {
			actual_structure->Variables.push_back(gaph_structure_info::node_info());
			actual_structure->Variables.back().name = (*it)->Get_name();
			actual_structure->Variables.back().observed_flag = true;
		}
		for (auto it = hidden.begin(); it != hidden.end(); it++) {
			actual_structure->Variables.push_back(gaph_structure_info::node_info());
			actual_structure->Variables.back().name = (*it)->Get_name();
			actual_structure->Variables.back().observed_flag = false;
		}

		const list<Categoric_var*>* var_list;
		for (auto it = pot_list.begin(); it != pot_list.end(); it++) {
			actual_structure->Potentials.push_back(gaph_structure_info::pot_info());
			var_list = (*it)->Get_involved_var_safe();

			actual_structure->Potentials.back().var_A = var_list->front()->Get_name();
			if (var_list->size() > 1)
				actual_structure->Potentials.back().var_B = var_list->back()->Get_name();
		}
	}


	this->__Data.update_needed = false;

	this->__Data.pmutex->unlock();

}

void CRF_Shell::process_received_command(const string& command, const bool& use_critical_region) {

	if(use_critical_region)
		this->__Data.pmutex->lock();

	std::list<parsed_option> result;
	Command* comm_to_process = this->__parse_command(&result, command);
	if (comm_to_process == NULL) {
		system("ECHO invalid command, ignored");
		if (use_critical_region)
			this->__Data.pmutex->unlock();
		return;
	}

	comm_to_process->process_received_command(result);

	if (use_critical_region)
		this->__Data.pmutex->unlock();

}




//Shell_Socket::Shell_Socket(const bool& Activate_Interface) : CRF_Shell(Activate_Interface), connection("", "80") {
//
//	this->connection.InitConnection();
//
//}
//
//void Shell_Socket::__next_command(std::string* command_received) {
//
//	int command_size;
//	this->connection.Recv(&command_size);
//
//	char* comm_raw = new char[command_size];
//	this->connection.Recv(comm_raw, command_size);
//	*command_received = string(comm_raw, (size_t)command_size);
//	delete comm_raw;
//
//}





char word[1024];
bool validity;
void Shell_Debugger::__get_next_command_from_master(std::string* command_received) {

	//cout << "$: ";
	fgets(&word[0], sizeof(word), stdin);
	*command_received = string(&word[0]);

}

void Shell_Debugger::__send_result(std::list<float>& values) {

	cout << "____________________________\n";
	for (auto it = values.begin(); it != values.end(); it++)
		cout << " " << *it;
	cout << endl;
	cout << "____________________________\n";

}

void Shell_Debugger::__send_result(std::list<string>& values) {

	cout << "____________________________\n";
	for (auto it = values.begin(); it != values.end(); it++)
		cout << *it << endl;
	cout << "____________________________\n";

}