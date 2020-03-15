#include "../Header/Interface_Handler.h"
#include <iostream>
using namespace std;

#ifdef _DEBUG
#pragma comment (lib, "../x64/Debug/EFG.lib")
#else
#pragma comment (lib, "../x64/Release/EFG.lib")
#endif // DEBUG
using namespace EFG;


string get_download_folder() {

	system("@echo %username% > __temp_name");

	ifstream f("__temp_name");
	string temp;
	getline(f, temp);
	f.close();
	system("DEL __temp_name");

	temp.pop_back();
	temp = "C:\\Users\\" + temp;
	temp += "\\Downloads\\__temp_download__.txt";

	return temp;

}
Interface_GUI::Interface_GUI(const std::string& port) : Connection(port) {

	this->Graph = NULL;
	this->DOWNLOAD_TEMP_FILE = get_download_folder();	   
	system(string("DEL \"" + this->DOWNLOAD_TEMP_FILE + "\"").c_str());
	system("cls");

	//launch index.html
	system("EFG_GUI.html");

	string temp;
	char comm_symbol;
	while (true) {
		this->Connection.Recv_request(&temp);
		cout << temp << endl;
		cout << "Parsed command:\n";

		Command comm(temp);
		comm.Print();

		temp = "null";
		comm_symbol = comm.Get_name();
		if (comm_symbol != '-') {
			if (comm_symbol == 'T') break;

			this->__process_command(comm, &temp);
		}

		this->Connection.Send_response(temp);
	}

}

void Interface_GUI::__process_command(const Command& comm, std::string* response) {

	char comm_symbol = comm.Get_name();

	if (comm_symbol == 'V') this->__V_create_Variable(comm, response);

	else if (comm_symbol == 'X') this->__X_create_Graph(comm, response);

	else if (comm_symbol == 'P') this->__P_create_Potential(comm, response);

	else if (comm_symbol == 'O') this->__O_set_Observations(comm, response);

	else if (comm_symbol == 'I') this->__I_get_Marginals(comm, response);

	else if (comm_symbol == 'M') this->__M_get_MAP(comm, response);

	else if (comm_symbol == 'A') this->__A_absorb_Graph(comm, response);

	else if (comm_symbol == 'R') this->__R_save_Graph(comm, response);

	else if (comm_symbol == 'B') this->__B_script(comm, response);

}

void Interface_GUI::__clean_model() {

	if (this->Graph != NULL) delete this->Graph;
	for (auto it = this->Open_set.begin(); it != this->Open_set.end(); it++)
		delete *it;
	this->Open_set.clear();

}

void Interface_GUI::__recompute_propagation_results() {

	this->MAP_computed.clear();
	this->Marginals_computed.clear();

	if (this->Graph != NULL) {
		list<EFG::Categoric_var*> hidden_set;
		this->Graph->Get_Actual_Hidden_Set(&hidden_set);
		for (auto it = hidden_set.begin(); it != hidden_set.end(); it++) {
			this->Marginals_computed.emplace_back();
			this->Marginals_computed.back().Var = *it;
		}
	}

}

Interface_GUI::~Interface_GUI() {

	this->__clean_model();

}

Graph*		Import_from_config_file(const Command* command, const string& dwnl_fldr) {

	EFG::Graph* new_graph;
	// import xml structure	
	auto folder = command->Get_value('p');
	auto config_file = command->Get_value('f');
	if (config_file == NULL) return new EFG::Graph();

	if (config_file->compare("%download") == 0) {
		Sleep(200);
		new_graph = new EFG::Graph(dwnl_fldr);
		system(string("DEL \"" + dwnl_fldr + "\"").c_str());
	}
	else if (folder == NULL)
		new_graph = new EFG::Graph(*config_file);
	else
		new_graph = new EFG::Graph(*config_file, *folder);

	return new_graph;

}

void Interface_GUI::__V_create_Variable(const Command& command, std::string* response) {

	auto variables = command.Get_values('v');
	auto dimensions = command.Get_values('s');
	if ((variables == NULL) || (dimensions == NULL)) return;
	if (variables->size() != dimensions->size()) return;

	size_t V_size;

	if (this->Graph == NULL)  this->Graph = new EFG::Graph();

	Categoric_var* clone;
	bool temp;
	for (size_t c = 0; c < variables->size(); c++) {
		clone = NULL;
		if (this->Graph != NULL)
			clone = this->Graph->Find_Variable((*variables)[c]);
		if (clone == NULL) {
			temp = true;
			for (auto it = this->Open_set.begin(); it != this->Open_set.end(); it++) {
				if ((*it)->Get_name().compare((*variables)[c]) == 0) {
					temp = false;
					break;
				}
			}

			V_size = (size_t)atoi((*dimensions)[c].c_str());
			if (temp && (V_size > 0)) this->Open_set.push_back(new Categoric_var(V_size, (*variables)[c]));
		}
	}

	if(response != NULL) this->__get_graph_JSON(response);

}

void Interface_GUI::__X_create_Graph(const Command& command, std::string* response) {

	auto new_graph = Import_from_config_file(&command, this->DOWNLOAD_TEMP_FILE);

	this->__clean_model();
	this->Graph = new_graph;
	this->__recompute_propagation_results();

	if (response != NULL) this->__get_graph_JSON(response);

}

void Interface_GUI::__A_absorb_Graph(const Command& command, std::string* response) {

	EFG::Graph* to_absorb = Import_from_config_file(&command, this->DOWNLOAD_TEMP_FILE);
	if (to_absorb == NULL)
		return;

	if (this->Graph == NULL)
		this->Graph = new EFG::Graph();
	this->Graph->Absorb(*to_absorb);
	delete to_absorb;
	this->__recompute_propagation_results();

	if (response != NULL) this->__get_graph_JSON(response);

}

void Interface_GUI::__P_create_Potential(const Command& command, std::string* response) {

	auto var_names = command.Get_values('v');
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

	Potential_Shape* new_shape = NULL;
	if ((command.Get_value('c') != NULL) && (Variables.size() > 1)) {
		if (command.Get_value('c')->compare("T") == 0)
			new_shape = new Potential_Shape(Variables, true);
		else if (command.Get_value('c')->compare("F") == 0)
			new_shape = new Potential_Shape(Variables, false);
		else return;
	}
	else if (command.Get_value('s') != NULL) {
		string location(*command.Get_value('s'));
		if (location.compare(this->DOWNLOAD_TEMP_FILE) == 0) {
			Sleep(200);
			new_shape = new Potential_Shape(Variables, this->DOWNLOAD_TEMP_FILE); 
			system(string("DEL \"" + this->DOWNLOAD_TEMP_FILE + "\"").c_str());
		}
		else new_shape = new Potential_Shape(Variables, *command.Get_value('s'));
	}
	else return;

	if (this->Graph == NULL) {
		this->Graph = new EFG::Graph();
	}

	if (command.Get_value('w') == NULL) {
		this->Graph->Insert(*new_shape);
		delete new_shape;
	}
	else {
		float w = (float)atof(command.Get_value('w')->c_str());
		auto temp_pot = new Potential_Exp_Shape(*new_shape, w);
		this->Graph->Insert(*temp_pot);
		delete temp_pot;
	}

	for (auto it = to_remove_from_Open_set.begin(); it != to_remove_from_Open_set.end(); it++)
		this->Open_set.remove(*it);

	this->__recompute_propagation_results();

	if (response != NULL) this->__get_graph_JSON(response);

}

void Interface_GUI::__O_set_Observations(const Command& command, std::string* response) {

	if (this->Graph == NULL) return;

	if (command.Get_values('v') == NULL) {
		this->Graph->Set_Evidences({}, {});
	}
	else {
		auto variables = command.Get_values('v');
		auto values = command.Get_values('n');
		if ((variables == NULL) || (values == NULL)) return;
		if (variables->size() != values->size()) return;

		list<Categoric_var*> Observations;
		list<size_t>					Values;
		this->Graph->Get_Actual_Observation_Set_Var(&Observations);
		this->Graph->Get_Actual_Observation_Set_Val(&Values);
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
		this->Graph->Set_Evidences(Observations, Values);
	}

	this->__recompute_propagation_results();
	if (response != NULL) this->__get_graph_JSON(response);

}

void Interface_GUI::__I_get_Marginals(const Command& command, std::string* response) {

	if (this->Graph == NULL) return;

	auto variables = command.Get_values('v');
	if (variables == NULL) return;
	Categoric_var* temp_var;
	auto it_m = this->Marginals_computed.begin();
	for (auto it = variables->begin(); it != variables->end(); it++) {
		temp_var = this->Graph->Find_Variable(*it);
		for (it_m = this->Marginals_computed.begin(); it_m != this->Marginals_computed.end(); it_m++) {
			if (it_m->Var == temp_var) {
				this->Graph->Get_marginal_distribution(&it_m->Marginals, temp_var);
				break;
			}
		}
	}

	if (response != NULL) this->__get_graph_JSON(response);

}

void Interface_GUI::__M_get_MAP(const Command& command, std::string* response) {

	if (this->Graph == NULL) return;
	if (!this->MAP_computed.empty()) return;

	this->Graph->MAP_on_Hidden_set(&this->MAP_computed);
	if (response != NULL) this->__get_graph_JSON(response);

}

void Interface_GUI::__R_save_Graph(const Command& command, std::string* response) {

	if (this->Graph == NULL) return;
	auto f_option = command.Get_value('f');
	if (f_option == NULL) return;

	this->Graph->Reprint(*f_option);

}

void get_all_lines(ifstream* f, list<string>* lines) {

	if (!f->is_open()) return;

	while(!f->eof()) {
		lines->push_back(string());
		getline(*f, lines->back());
	}

}
void Interface_GUI::__B_script(const Command& command, std::string* response) {

	auto files = command.Get_values('f');
	if (files->empty()) return;

	list<string> comm_list;
	if (files->front().compare("%download") == 0) {
		Sleep(200);
	   ifstream f_B(this->DOWNLOAD_TEMP_FILE);
	   get_all_lines(&f_B, &comm_list);
	   f_B.close();
	   system(string("DEL \"" + this->DOWNLOAD_TEMP_FILE + "\"").c_str());
	}
	else {
		ifstream f_B(files->front());
		get_all_lines(&f_B, &comm_list);
		f_B.close();
	}

	for (auto it = comm_list.begin(); it != comm_list.end(); it++) 
		this->__process_command(*it, NULL);

	if (response != NULL) this->__get_graph_JSON(response);

};

size_t Get_node_id(const std::list<Categoric_var*>& hidden_set, const std::list<Categoric_var*>& observed_set, const Categoric_var* var_to_find, const size_t& open_set_size) {

	size_t counter = 0;
	auto it = hidden_set.begin();
	for (it; it != hidden_set.end(); it++) {
		if (*it == var_to_find)
			return counter;
		counter++;
	}
	for (it = observed_set.begin(); it != observed_set.end(); it++) {
		if (*it == var_to_find)
			return counter + open_set_size;
		counter++;
	}
	return string::npos;

}
template<typename T>
void add_potentials(JSON_array& edges, JSON_array& nodes, const vector<T*>& potentials, list<Categoric_var*>& hidden_set, list<Categoric_var*>& observed_set, size_t& k, const string& edge_image, const size_t& open_set_size) {

	for (auto it = potentials.begin(); it != potentials.end(); it++) {
		JSON_tag temp;
		temp.Add_field("label", "");
		temp.Add_field("shape", "image");
		temp.Add_field("image", edge_image);

		temp.Add_field("color", "#000000");
		temp.Add_field("id", to_string(k));
		nodes.Append(temp);

		JSON_tag temp2;
		temp2.Add_field("from", to_string(Get_node_id(hidden_set, observed_set, (*it)->Get_involved_var()->front(), open_set_size)));
		temp2.Add_field("to", to_string(k));
		edges.Append(temp2);
		if ((*it)->Get_involved_var()->size() > 1) {
			temp2.Add_field("from", to_string(k));
			temp2.Add_field("to", to_string(Get_node_id(hidden_set, observed_set, (*it)->Get_involved_var()->back(), open_set_size)));
			edges.Append(temp2);
		}

		k++;
	}

}
void Interface_GUI::__get_graph_JSON(std::string* graph_JSON) {

	vector<Potential_Shape*> shapes;
	vector<list<Potential_Exp_Shape*>> exp_tunable_clusters;
	vector<Potential_Exp_Shape*> exp_constant;
	this->Graph->Get_structure(&shapes, &exp_tunable_clusters, &exp_constant);
	vector<Potential_Exp_Shape*> exp_tunable;
	if (!exp_tunable.empty()) {
		auto it = exp_tunable_clusters.begin();
		auto it2 = exp_tunable_clusters.begin()->begin();
		size_t V = 0;
		for (it = exp_tunable_clusters.begin(); it != exp_tunable_clusters.end(); it++)
			V += it->size();
		exp_tunable.reserve(V);
		for (it = exp_tunable_clusters.begin(); it != exp_tunable_clusters.end(); it++) {
			for (it2 = it->begin(); it2 != it->end(); it2++)
				exp_tunable.push_back(*it2);
		}
	}



	list<Categoric_var*> hidden_set, observed_set;
	this->Graph->Get_Actual_Hidden_Set(&hidden_set);
	this->Graph->Get_Actual_Observation_Set_Var(&observed_set);
	list<size_t> observed_vals;
	this->Graph->Get_Actual_Observation_Set_Val(&observed_vals);
	if (hidden_set.empty() && observed_set.empty())
		this->Graph->Get_All_variables_in_model(&hidden_set);

	JSON_array nodes;
	JSON_array names_sizes;
	size_t k = 0;
	if (this->MAP_computed.empty()) {
		for (auto it = hidden_set.begin(); it != hidden_set.end(); it++) {
			JSON_tag temp;
			temp.Add_field("label", (*it)->Get_name());
			temp.Add_field("shape", "image");
			temp.Add_field("image", "./image/Variable.svg");
			temp.Add_field("color", "#000000");
			temp.Add_field("id", to_string(k));
			nodes.Append(temp);
			k++;
			JSON_tag temp2;
			temp2.Add_field("N", (*it)->Get_name());
			temp2.Add_field("S", to_string((*it)->size()));
			names_sizes.Append(temp2);
		}
	}
	else {
		auto it_MAP = this->MAP_computed.begin();
		for (auto it = hidden_set.begin(); it != hidden_set.end(); it++) {
			JSON_tag temp;
			temp.Add_field("label", (*it)->Get_name() + " = " + to_string(*it_MAP) + "(MAP)");
			temp.Add_field("shape", "image");
			temp.Add_field("image", "./image/Variable.svg");
			temp.Add_field("color", "#000000");
			temp.Add_field("id", to_string(k));
			nodes.Append(temp);
			k++;
			it_MAP++;
			JSON_tag temp2;
			temp2.Add_field("N", (*it)->Get_name());
			temp2.Add_field("S", to_string((*it)->size()));
			names_sizes.Append(temp2);
		}
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
		JSON_tag temp2;
		temp2.Add_field("N", (*it)->Get_name());
		temp2.Add_field("S", to_string((*it)->size()));
		names_sizes.Append(temp2);
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

	JSON_array edges;
	add_potentials(edges, nodes, shapes, hidden_set, observed_set, k, "./image/Potential_Shape.svg", this->Open_set.size());
	add_potentials(edges, nodes, exp_tunable, hidden_set, observed_set, k, "./image/Potential_Exp_Shape_tunable.svg", this->Open_set.size());
	add_potentials(edges, nodes, exp_constant, hidden_set, observed_set, k, "./image/Potential_Exp_Shape_fixed.svg", this->Open_set.size());

	JSON_tag graph;
	graph.Add_nested("nodes", nodes);
	graph.Add_nested("edges", edges);

	JSON_array marginals;
	for (auto it = this->Marginals_computed.begin(); it != this->Marginals_computed.end(); it++) {
		JSON_numerical_array temp(it->Marginals);
		marginals.Append(temp);
	}

	graph.Add_nested("marginals" , marginals);
	graph.Add_nested("names", names_sizes);

	graph.stringify(graph_JSON);

	//ofstream f("JSON_log");
	//f << *graph_JSON;
	//f.close();

}
