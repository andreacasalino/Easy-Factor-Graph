#include "../__Header/Shell_Clients.h"
using namespace std;

#ifdef _DEBUG
#pragma comment (lib, "../x64/Debug/CRF.lib")
#else
#pragma comment (lib, "../x64/Release/CRF.lib")
#endif // DEBUG
using namespace Segugio;

//#define SHOW_PARSED_COMMAND
#define JS_INTERFACE_PORT string("8001")






Command::Command(const std::string& raw) : Name('-') {

	list<size_t> positions;
	this->__find_separators(&positions, raw);
	if (positions.empty()) {
		if (raw.size() == 1)
			this->Name = raw[0];
	}
	else {
		if (positions.size() % 2 != 0)
			return;
		if (positions.front() != 1)
			return;

		this->Name = raw[0];

		size_t p1, p2, p3, k;
		auto it_opt = this->Options.begin();
		__option* existing_opt = NULL;
		string value;
		while (!positions.empty()) {
			p1 = positions.front(); positions.pop_front();
			p2 = positions.front(); positions.pop_front();
			if ((p2 - p1) == 2) {
				existing_opt = NULL;
				for (it_opt = this->Options.begin(); it_opt != this->Options.end(); it_opt++) {
					if (it_opt->name == raw[p1 + 1]) {
						existing_opt = &(*it_opt);
						break;
					}
				}

				if (existing_opt == NULL) {
					this->Options.push_back(__option());
					this->Options.back().name = raw[p1 + 1];
					existing_opt = &this->Options.back();
				}

				if (positions.empty()) p3 = raw.size();
				else p3 = positions.front();

				value.clear();
				k = p2;
				k++;
				for (k; k < p3; k++) {
					if ((raw[k] != '\n') && (raw[k] != ' '))
						value.push_back(raw[k]);
				}
				existing_opt->values.push_back(value);
			}
		}
	}

}

const std::vector<std::string>* Command::Get_values(const char& option_name) const {

	for (auto it = this->Options.begin(); it != this->Options.end(); it++) {
		if (it->name == option_name)
			return &it->values;
	}
	return NULL;

}

const std::string*	 Command::Get_value(const char& option_name) const {

	auto vals = this->Get_values(option_name);
	if (vals == NULL) return NULL;
	return &vals->front();

}

void Command::__find_separators(std::list<std::size_t>* positions, const std::string& word) {

	positions->clear();
	for (size_t k = 0; k < word.size(); k++) {
		if (word[k] == '$')
			positions->push_back(k);
	}

}

void Command::Print() const {

	string to_print;

	to_print = "Name:" ;
	to_print += " ";
	to_print.back() = this->Name;
	system(string("ECHO " + to_print).c_str());

	for (auto it = this->Options.begin(); it != this->Options.end(); it++) {
		to_print.clear();
		to_print = "Option:";
		to_print += " ";
		to_print.back() = it->name;
		to_print +=  ",[";
		if (!it->values.empty()) {
			auto itt = it->values.begin();
			to_print += *itt;
			itt++;
			for (itt; itt != it->values.end(); itt++)
				to_print += "," + *itt;
		}
		to_print += "]";
		system(string("ECHO " + to_print).c_str());
	}

}



CRF_Shell::Client::Client(CRF_Shell* shell_to_link) {

	this->Data.Mutex = new mutex();
	this->Data.bRequest = false;
	this->Data.Request = NULL;
	this->Data.bResponse = false;

	shell_to_link->Requests.push_back(&this->Data);

	this->__loop_can_start = false;
	this->th__loop = new thread(&Client::__loop, this);

}

CRF_Shell::Client::~Client() {

	this->th__loop->join();
	delete this->th__loop;
	delete this->Data.Mutex;
	if (this->Data.Request != NULL)
		delete this->Data.Request;

}

void	CRF_Shell::Client::__loop() {

	bool btemp;
	while (true) {
		this->Data.Mutex->lock();
		btemp = this->__loop_can_start;
		this->Data.Mutex->unlock();
		if (btemp)
			break;
	}

	string next_comm;
	while (true) {
		this->__get_next_command(&next_comm);
		this->Data.Request = new Command(next_comm);
		if (this->Data.Request->Get_name() == 'B') {
			string file(*this->Data.Request->Get_value('f'));
			delete this->Data.Request;

			ifstream f(file);
			if (f.is_open()) {
				while (!f.eof()) {
					getline(f, next_comm);
					this->Data.Request = new Command(next_comm);
					this->__write_command_and_wait_response();
					this->__send_response(this->Data.Response);
				}
			}
			else { 
				this->__send_response("null");
				f.close();
			}
		}
		else {
			this->__write_command_and_wait_response();
			this->__send_response(this->Data.Response);
		}
	}

}

void	CRF_Shell::Client::__write_command_and_wait_response() {

#ifdef SHOW_PARSED_COMMAND
	this->Data.Request->Print();
#endif

	this->Data.Mutex->lock();
	this->Data.bRequest = true;
	this->Data.bResponse = false;
	this->Data.Mutex->unlock();

	bool b_copy;
	while (true) {
		this->Data.Mutex->lock();
		b_copy = this->Data.bResponse;
		this->Data.Mutex->unlock();

		if (b_copy) {
			delete this->Data.Request;
			break;
		}
	}

}

void	CRF_Shell::Client::__allow_loop_to_start() {

	this->Data.Mutex->lock();
	this->__loop_can_start = true;
	this->Data.Mutex->unlock();

}



void CRF_Shell::Activate_loop() {

	auto it_req = this->Requests.begin();
	Command* comm_to_process;
	while (true) {
		for (it_req = this->Requests.begin(); it_req != this->Requests.end(); it_req++) {
			comm_to_process = NULL;
			(*it_req)->Mutex->lock();
			if ((*it_req)->bRequest) 
				comm_to_process = (*it_req)->Request;
			(*it_req)->Mutex->unlock();

			if (comm_to_process != NULL) {
				this->__get_response(comm_to_process, &(*it_req)->Response);
				(*it_req)->Mutex->lock();
				(*it_req)->bResponse = true;
				(*it_req)->bRequest = false;
				(*it_req)->Mutex->unlock();
			}
		}
	}

}

Graph*		Import_from_config_file(const Command* command) {

	// import xml structure	
	auto folder = command->Get_value('p');
	auto config_file = command->Get_value('f');
	if (config_file == NULL) {
		return NULL;
	}

	Segugio::Graph* new_graph;
	if (folder == NULL)
		new_graph = new Segugio::Graph(*config_file);
	else
		new_graph = new Segugio::Graph(*config_file, *folder);
	return new_graph;

}
void CRF_Shell::__get_response(const Command* command, std::string* JSON_result) {

	char symbol = command->Get_name();
	*JSON_result = "null";
	if (symbol != '-') {

		if(symbol == 'T') {
			// terminate
			abort(); //TODO
		}

		else if (symbol == 'U') {
			this->__get_graph_JSON(JSON_result);
		}

		else if (symbol == 'V') {
			// create new variable
			auto variables = command->Get_values('v');
			auto dimensions = command->Get_values('s');
			if ((variables == NULL) || (dimensions == NULL)) return;
			if (variables->size() != dimensions->size()) return;

			if (this->Graph == NULL)  this->Graph = new Segugio::Graph();
			this->Graph_has_changed = true;

			Categoric_var* clone;
			bool temp;
			for (size_t c = 0; c < variables->size(); c++) {
				clone = NULL;
				if(this->Graph != NULL)
					clone = this->Graph->Find_Variable((*variables)[c]);
				if (clone == NULL) {
					temp = true;
					for (auto it = this->Open_set.begin(); it != this->Open_set.end(); it++) {
						if ((*it)->Get_name().compare((*variables)[c]) == 0) {
							temp = false;
							return;
						}
					}

					if (temp) this->Open_set.push_back( new Categoric_var((size_t)atoi((*dimensions)[c].c_str()), (*variables)[c]));
				}
			}
		}

		else if (symbol == 'X') {
			// import xml structure	
			Segugio::Graph* new_graph = Import_from_config_file(command);
			if (new_graph == NULL) 
				return;

			if (this->Graph != NULL) delete this->Graph;
			this->Graph = new Segugio::Graph();
			new_graph->Set_Observation_Set_var({});
			new_graph->Set_Observation_Set_val({});

			for (auto it = this->Open_set.begin(); it != this->Open_set.end(); it++)
				delete *it;
			this->Open_set.clear();
			this->Graph = new_graph;
			this->Graph_has_changed = true;
		}


		else if (symbol == 'P') {
			// create new potential
			auto var_names = command->Get_values('v');
			list<Categoric_var*> Variables;
			list<Categoric_var*> to_remove_from_Open_set;
			auto it_open = this->Open_set.begin();
			for (auto it = var_names->begin(); it != var_names->end(); it++) {
				Variables.push_back(this->Graph->Find_Variable(*it));
				if (Variables.back() == NULL) {
					for (it_open = this->Open_set.begin(); it_open != this->Open_set.end(); it_open++) {
						if ((*it_open)->Get_name().compare(*it) == 0) {
							Variables.back() = *it_open;
							to_remove_from_Open_set.push_back(*it_open);
							break;
						}
					}
				}
				if (Variables.back() == NULL)
					return;
			}

			if (Variables.empty()) return;
			else if (Variables.size() > 2) return;

			Potential_Shape* new_shape;
			if ((command->Get_value('c') != NULL) && (Variables.size() > 1)) {
				if (command->Get_value('c')->compare("T") == 0)
					new_shape = new Potential_Shape(Variables, true);
				else if(command->Get_value('c')->compare("F") == 0)
					new_shape = new Potential_Shape(Variables, false);
				else return;
			}
			else if(command->Get_value('s') != NULL) {
				new_shape = new Potential_Shape(Variables, *command->Get_value('s'));
			}
			else return;

			if (this->Graph == NULL) {
				this->Graph = new Segugio::Graph();
				this->Graph->Set_Observation_Set_var({});
				this->Graph->Set_Observation_Set_val({});
			}

			this->Graph_has_changed = true;
			if (command->Get_value('w') == NULL) 
				this->Graph->Insert(new_shape);
			else {
				float w = (float)atof(command->Get_value('w')->c_str());
				auto temp_pot = new Potential_Exp_Shape(new_shape, w);
				this->Graph->Insert(temp_pot);
			}

			for (auto it = to_remove_from_Open_set.begin(); it != to_remove_from_Open_set.end(); it++)
				this->Open_set.remove(*it);
		}


		else if (symbol == 'O') {
			// set observation set
			if (this->Graph == NULL) return;

			if (command->Get_values('v') == NULL) {
				this->Graph->Set_Observation_Set_var({});
				this->Graph->Set_Observation_Set_val({});
				this->Graph_has_changed = true;
			}
			else {
				auto variables = command->Get_values('v');
				auto values = command->Get_values('n');
				if ((variables == NULL) || (values == NULL)) return;
				if (variables->size() != values->size()) return;

				list<Categoric_var*> Observations;
				list<size_t>					Values;
				this->Graph->Get_Actual_Observation_Set(&Observations);
				this->Graph->Get_Observation_Set_val(&Values);
				auto it_ob = Observations.begin();
				Categoric_var* clone = NULL;
				auto it_vv = values->begin();
				for (auto it_v = variables->begin(); it_v != variables->end(); it_v++) {
					clone = NULL;
					for (it_ob = Observations.begin(); it_ob != Observations.end(); it_ob++) {
						if ((*it_ob)->Get_name().compare(*it_v) == 0) {
							clone = *it_ob;
							break;
						}
					}

					if (clone == NULL) {
						Observations.push_back(this->Graph->Find_Variable(*it_v));
						Values.push_back((size_t)atoi(it_vv->c_str()));
					}
					it_vv++;
				}
				this->Graph->Set_Observation_Set_var(Observations);
				this->Graph->Set_Observation_Set_val(Values);
				this->Graph_has_changed = true;
			}
		}

		else if (symbol == 'I') {
			// get marginals
			if (this->Graph == NULL) return;

			auto variables = command->Get_values('v');
			if (variables == NULL) return;
			list<float> prob_den;
			JSON_array densities;
			for (auto it = variables->begin(); it != variables->end(); it++) {
				this->Graph->Get_marginal_distribution(&prob_den, this->Graph->Find_Variable(*it));
				JSON_numerical_array temp(prob_den);
				densities.Append(temp);
			}
			densities.stringify(JSON_result);
		}

		else if (symbol == 'M') {
			// get MAP
			if (this->Graph == NULL) return;

			list<size_t> MAP;
			this->Graph->MAP_on_Hidden_set(&MAP);
			list<float> MAP_flt;
			for (auto it = MAP.begin(); it != MAP.end(); it++)
				MAP_flt.push_back((float)(*it));
			JSON_numerical_array temp(MAP_flt);
			temp.stringify(JSON_result);
		}

		else if (symbol == 'S') {
			// get hidden or observed set
			if (this->Graph == NULL) return;

			list<Categoric_var*> Set;
			auto option = command->Get_value('k');
			if (option->compare("O") == 0)
				this->Graph->Get_Actual_Observation_Set(&Set);
			else if (option->compare("H") == 0)
				this->Graph->Get_Actual_Hidden_Set(&Set);
			else return;

			*JSON_result = "[";
			if (!Set.empty()) {
				*JSON_result += "\"";
				*JSON_result += Set.front()->Get_name();
				*JSON_result += "\"";
				Set.pop_front();
				for (auto it = Set.begin(); it != Set.end(); it++) {
					*JSON_result += ",\"";
					*JSON_result += Set.front()->Get_name();
					*JSON_result += "\"";
				}
			}
			*JSON_result += "]";
		}

		else if (symbol == 'J') {
			// launch GUI
			if (this->JS_Interface == NULL) {
				this->JS_Interface = new JS_Client(this);
				system("Interface.html");
			}
		}

		else if (symbol == 'A') {
			// absorb the structure in a config file
			Segugio::Graph* to_absorb = Import_from_config_file(command);
			if (to_absorb == NULL)
				return;

			if (this->Graph == NULL)
				this->Graph = new Segugio::Graph();
			this->Graph->Absorb(to_absorb);
			delete to_absorb;
			this->Graph_has_changed = true;
		}

	}
}

size_t Get_node_id(const std::list<Categoric_var*>& hidden_set, const std::list<Categoric_var*>& observed_set, const Categoric_var* var_to_find) {

	size_t counter = 0;
	auto it = hidden_set.begin();
	for (it; it != hidden_set.end(); it++) {
		if (*it == var_to_find)
			return counter;
		counter++;
	}
	for (it = observed_set.begin(); it != observed_set.end(); it++) {
		if (*it == var_to_find)
			return counter;
		counter++;
	}
	return string::npos;

}
template<typename T>
void add_potentials(JSON_array& edges, JSON_array& nodes, const list<const T*>& potentials, list<Categoric_var*>& hidden_set, list<Categoric_var*>& observed_set, size_t& k, const string& edge_image) {

	for (auto it = potentials.begin(); it != potentials.end(); it++) {
		JSON_tag temp;
		temp.Add_field("label", "");
		temp.Add_field("shape", "image");
		temp.Add_field("image", edge_image);

		temp.Add_field("color", "#000000");
		temp.Add_field("id", to_string(k));
		nodes.Append(temp);

		JSON_tag temp2;
		temp2.Add_field("from", to_string(Get_node_id(hidden_set, observed_set, (*it)->Get_involved_var_safe()->front())));
		temp2.Add_field("to", to_string(k));
		edges.Append(temp2);
		if ((*it)->Get_involved_var_safe()->size() > 1) {
			temp2.Add_field("from", to_string(k));
			temp2.Add_field("to", to_string(Get_node_id(hidden_set, observed_set, (*it)->Get_involved_var_safe()->back())));
			edges.Append(temp2);
		}

		k++;
	}

}
void CRF_Shell::__get_graph_JSON(std::string* graph_JSON) {

	if (this->Graph_has_changed) {
		list<const Potential_Shape*> shapes;
		list<list<const Potential_Exp_Shape*>> exp_tunable_clusters;
		list<const Potential_Exp_Shape*> exp_constant;
		this->Graph->Get_structure(&shapes, &exp_tunable_clusters, &exp_constant);
		list<const Potential_Exp_Shape*> exp_tunable;
		if (!exp_tunable.empty()) {
			auto it2 = exp_tunable_clusters.begin()->begin();
			for (auto it = exp_tunable_clusters.begin(); it != exp_tunable_clusters.end(); it++) {
				for (it2 = it->begin(); it2 != it->end(); it2++)
					exp_tunable.push_back(*it2);
			}
		}



		list<Categoric_var*> hidden_set, observed_set;
		this->Graph->Get_Actual_Hidden_Set(&hidden_set);
		this->Graph->Get_Actual_Observation_Set(&observed_set);
		list<size_t> observed_vals;
		this->Graph->Get_Observation_Set_val(&observed_vals);
		if (hidden_set.empty() && observed_set.empty())
			this->Graph->Get_All_variables_in_model(&hidden_set);

		JSON_array nodes;
		size_t k = 0;
		for (auto it = hidden_set.begin(); it != hidden_set.end(); it++) {
			JSON_tag temp;
			temp.Add_field("label", (*it)->Get_name());
			temp.Add_field("shape", "image");
			temp.Add_field("image", "./image/Variable.svg");
			temp.Add_field("color", "#000000");
			temp.Add_field("id", to_string(k));
			nodes.Append(temp);
			k++;
		}
		auto it_val = observed_vals.begin();
		for (auto it = observed_set.begin(); it != observed_set.end(); it++) {
			JSON_tag temp;
			temp.Add_field("label", (*it)->Get_name() + " = " + to_string(*it_val));
			temp.Add_field("shape", "image");
			temp.Add_field("image", "./image/Variable_Observed.svg");
			temp.Add_field("color", "#000000");
			temp.Add_field("id", to_string(k));
			nodes.Append(temp);
			k++;
			it_val++;
		}
		for (auto it = this->Open_set.begin(); it != this->Open_set.end(); it++) {
			JSON_tag temp;
			temp.Add_field("label", (*it)->Get_name());
			temp.Add_field("shape", "image");
			temp.Add_field("image", "./image/Variable.svg");
			temp.Add_field("color", "#000000");
			temp.Add_field("id", to_string(k));
			nodes.Append(temp);
			k++;
		}

		JSON_array edges;
		add_potentials(edges, nodes, shapes, hidden_set, observed_set, k, "./image/Potential_Shape.svg");
		add_potentials(edges, nodes, exp_tunable, hidden_set, observed_set, k, "./image/Potential_Exp_Shape_tunable.svg");
		add_potentials(edges, nodes, exp_constant, hidden_set, observed_set, k, "./image/Potential_Exp_Shape_fixed.svg");

		JSON_tag graph;
		graph.Add_nested("nodes", nodes);
		graph.Add_nested("edges", edges);
		graph.stringify(graph_JSON);
		this->Graph_has_changed = false;
	}
	else *graph_JSON = "null";

}

CRF_Shell::~CRF_Shell() {

	if (this->Graph != NULL) delete this->Graph;
	for (auto it = this->Open_set.begin(); it != this->Open_set.end(); it++)
		delete *it;
	if(this->JS_Interface != NULL) delete this->JS_Interface;

}



JS_Client::JS_Client(CRF_Shell* shell_to_link) : Client(shell_to_link) , Listener(JS_INTERFACE_PORT) {

	ofstream f("__temp.bat");
	f << "Interface.hmtl";
	f.close();
	system("__temp.bat");
	this->__allow_loop_to_start();
	system("DEL __temp.bat");

};

void	JS_Client::__get_next_command(std::string* command) {

	this->Listener.Recv_request(command);

};

void	JS_Client::__send_response(const std::string& response) {

	this->Listener.Send_response(response);

}