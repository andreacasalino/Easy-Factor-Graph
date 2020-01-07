#pragma once
#ifndef __SHELL_CLIENTS__H__
#define __SHELL_CLIENTS__H__

#include "Http_Server.h"
#include "Command.h"
#include "../../../../EFG/Header/Graphical_model.h"



class Interface_GUI {
public:
	Interface_GUI(const std::string& port = "8001");
	~Interface_GUI();
	
private:
	void __get_graph_JSON(std::string* graph_JSON);									//null when nothing has to be notified

	void __clean_model();
	void __recompute_propagation_results();
	void __process_command(const Command& command, std::string* response);

	void __X_create_Graph(const Command& command, std::string* response);
	void __A_absorb_Graph(const Command& command, std::string* response);
	void __V_create_Variable(const Command& command, std::string* response);
	void __P_create_Potential(const Command& command, std::string* response);
	void __O_set_Observations(const Command& command, std::string* response);
	void __I_get_Marginals(const Command& command, std::string* response);
	void __M_get_MAP(const Command& command, std::string* response);
	void __R_save_Graph(const Command& command, std::string* response);
	void __B_script(const Command& command, std::string* response);

	struct Infer_result {
		std::list<float>						   Marginals;
		EFG::Categoric_var*     Var;
	};

// data
	std::string																	DOWNLOAD_TEMP_FILE;
	Http_Server																Connection;
	EFG::Graph*														Graph;
	std::list<EFG::Categoric_var*>					Open_set;	//variables created but not already inserted in the graph

	std::list<Infer_result>												Marginals_computed;
	std::list<size_t>														MAP_computed;
};

#endif 