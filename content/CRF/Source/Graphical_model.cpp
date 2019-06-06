#include "../Header/Graphical_model.h"
using namespace std;

//#define ADD_REGULARIZATION

namespace Segugio {

	Graph::Graph(const std::string& config_xml_file, const std::string& prefix_config_xml_file) {

		XML_reader reader(prefix_config_xml_file + config_xml_file);
		this->Import_from_XML(&reader, prefix_config_xml_file); 

	};






	I_Learning_handler::I_Learning_handler(Potential_Exp_Shape* pot_to_handle) : I_Potential_Decorator(pot_to_handle) {

		this->Destroy_wrapped = false; //this exponential shape will be wrapped also by a Potential to be stored in the graphical model
		this->pWeight = this->Potential_Exp_Shape::Getter_weight_and_shape::Get_weight(pot_to_handle);

		list<size_t*> val_to_search;
		Get_entire_domain(&val_to_search, *this->I_Potential::Getter_4_Decorator::Get_involved_var(pot_to_handle));
		Find_Comb_in_distribution(&this->Extended_shape_domain, val_to_search, *this->Get_involved_var(), this->Potential_Exp_Shape::Getter_weight_and_shape::Get_shape(this->pwrapped));
		for (auto it = val_to_search.begin(); it != val_to_search.end(); it++)
			free(*it);

	}

	I_Learning_handler::I_Learning_handler(I_Learning_handler* other) :
		I_Learning_handler(other->pwrapped) {  };

	void I_Learning_handler::Get_grad_alfa_part(float* alfa, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var) {

		list<I_Distribution_value*> val_in_train_set;
		Find_Comb_in_distribution(&val_in_train_set, comb_in_train_set, comb_var, this->Potential_Exp_Shape::Getter_weight_and_shape::Get_shape(this->pwrapped));

		*alfa = 0.f;
		float temp;
		for (auto it_val = val_in_train_set.begin(); it_val != val_in_train_set.end(); it_val++) {
			if (*it_val != NULL) {
				(*it_val)->Get_val(&temp);
				*alfa += temp;
			}
		}
		*alfa *= 1.f / (float)comb_in_train_set.size();

	}

	void Dot_with_Prob(float* result, const list<float>& marginal_prob, const list<I_Potential::I_Distribution_value*>& shape) {

		*result = 0.f;
		float temp;
		auto itP = marginal_prob.begin();
		for (auto itD = shape.begin(); itD != shape.end(); itD++) {
			if (*itD != NULL) {
				(*itD)->Get_val(&temp);
				*result += temp * *itP;
			}

			itP++;
		}

	}



	class Unary_handler : public I_Learning_handler {
	public:
		Unary_handler(Node* N, Potential_Exp_Shape* pot_to_handle) : I_Learning_handler(pot_to_handle), pNode(N) {};
	private:
		void    Get_grad_beta_part(float* beta);
	// data
		Node*				pNode;
	};

	void Unary_handler::Get_grad_beta_part(float* beta) {

		list<float> marginals;
		list<Potential*> message_union;
		this->pNode->Gather_all_Unaries(&message_union);
		Potential UP(message_union);
		UP.Get_marginals(&marginals);

		Dot_with_Prob(beta , marginals, this->Extended_shape_domain);

	}



	class Binary_handler : public I_Learning_handler {
	public:
		Binary_handler(Node* N1, Node* N2, Potential_Exp_Shape* pot_to_handle);
		~Binary_handler() { delete this->Binary_for_group_marginal_computation; };
	private:
		void    Get_grad_beta_part(float* beta);
	// data
		Node*				pNode1;
		Node*				pNode2;
	// cache
		Potential*						Binary_for_group_marginal_computation;
	};

	Binary_handler::Binary_handler(Node* N1, Node* N2, Potential_Exp_Shape* pot_to_handle) : I_Learning_handler(pot_to_handle), pNode1(N1), pNode2(N2) {

		if (N1->Get_var() != pot_to_handle->Get_involved_var_safe()->front()) {
			Node* C = N2;
			N2 = N1;
			N1 = C;
		}

		auto temp = new Potential_Shape(*this->Get_involved_var());
		temp->Set_ones();
		this->Binary_for_group_marginal_computation = new Potential(temp);

	};

	void Binary_handler::Get_grad_beta_part(float* beta) {

		list<Potential*> union_temp;

		struct info_val {
			Potential*					 Mex_tot;
			list<I_Distribution_value*>  ordered_distr;
			size_t						 var_involved;
		};
		list<info_val> Messages_from_Net;

		list<info_val> infoes;
		this->pNode1->Compute_neighbourhood_messages(&union_temp, this->pNode2);
		if (!union_temp.empty()) {
			Messages_from_Net.push_back(info_val());
			Messages_from_Net.back().Mex_tot = new Potential(union_temp);
			Messages_from_Net.back().var_involved = 0;
			list<size_t*> dom_temp;
			Get_entire_domain(&dom_temp, { this->Binary_for_group_marginal_computation->Get_involved_var_safe()->front() });
			Find_Comb_in_distribution(&Messages_from_Net.back().ordered_distr, dom_temp, 
				{ this->Binary_for_group_marginal_computation->Get_involved_var_safe()->front() }, Messages_from_Net.back().Mex_tot);
			for (auto it = dom_temp.begin(); it != dom_temp.end(); it++)
				free(*it);
		}
		this->pNode2->Compute_neighbourhood_messages(&union_temp, this->pNode1);
		if (!union_temp.empty()) {
			Messages_from_Net.push_back(info_val());
			Messages_from_Net.back().Mex_tot = new Potential(union_temp);
			Messages_from_Net.back().var_involved = 1;
			list<size_t*> dom_temp;
			Get_entire_domain(&dom_temp, { this->Binary_for_group_marginal_computation->Get_involved_var_safe()->back() });
			Find_Comb_in_distribution(&Messages_from_Net.back().ordered_distr, dom_temp,
				{ this->Binary_for_group_marginal_computation->Get_involved_var_safe()->back() }, Messages_from_Net.back().Mex_tot);
			for (auto it = dom_temp.begin(); it != dom_temp.end(); it++)
				free(*it);
		}

		auto pBin_Distr = this->Get_distr();
		float temp, result;
		list<I_Distribution_value*>::iterator it_pos;
		list<info_val>::iterator it_info;
		auto itD2 = this->I_Potential::Getter_4_Decorator::Get_distr(this->Binary_for_group_marginal_computation)->begin();
		for (auto itD = pBin_Distr->begin(); itD != pBin_Distr->end(); itD++) {
			(*itD)->Get_val(&result);

			for (it_info = infoes.begin(); it_info != infoes.end(); it_info++) {
				it_pos = it_info->ordered_distr.begin();
				advance(it_pos, (*itD)->Get_indeces()[it_info->var_involved]);

				(*it_pos)->Get_val(&temp);
				result *= temp;
			}

			(*itD2)->Set_val(result);
			itD2++;
		}

		list<float> Marginals;
		this->Binary_for_group_marginal_computation->Get_marginals(&Marginals);

		Dot_with_Prob(beta, Marginals, this->Extended_shape_domain);

		for (it_info = infoes.begin(); it_info != infoes.end(); it_info++)
			delete it_info->Mex_tot;

	}






	Graph_Learnable::~Graph_Learnable() {

		for (auto it = this->Model_handlers.begin(); it != this->Model_handlers.end(); it++)
			delete *it;
		if (this->pLast_train_set != NULL)
			delete this->pLast_train_set;

	}

	void Graph_Learnable::Insert(Potential_Exp_Shape* pot) {
		
		pot = this->Insert_with_size_check<Potential_Exp_Shape>(pot);

		auto vars = pot->Get_involved_var_safe();

		if (vars->size() == 1) {
			//new unary
			this->Model_handlers.push_back(new Unary_handler(this->Find_Node(vars->front()->Get_name()), pot));
		}
		else {
			//new binary
			Node* N1 = this->Find_Node(vars->front()->Get_name());
			Node* N2 = this->Find_Node(vars->back()->Get_name());

			this->Model_handlers.push_back(new Binary_handler(N1, N2, pot));
		}

	}

	void Graph_Learnable::Insert(Potential_Shape* pot) {

		system("ECHO you can insert a pure shape function only to general graph and not (Conditional) Random Field ");
		abort();

	}

	void Graph_Learnable::Weights_Manager::Get_w(std::list<float>* w, Graph_Learnable* model) {

		w->clear();
		for (auto it = model->Model_handlers.begin(); it != model->Model_handlers.end(); it++) {
			w->push_back(float());
			(*it)->Get_weight(&w->back());
		}

	}

	void Graph_Learnable::Weights_Manager::Get_w_grad(std::list<float>* grad_w, Graph_Learnable* model, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order) {

		grad_w->clear();
		model->Graph_Learnable::Get_w_grad(grad_w, comb_train_set, comb_var_order); //here tha alfa part is appended
		model->Get_w_grad(grad_w, comb_train_set, comb_var_order); //here the beta part is added

#ifdef ADD_REGULARIZATION
		//add regularization term
		float temp;
		auto it_grad = grad_w->begin();
		for (auto it = model->Model_handlers.begin(); it != model->Model_handlers.end(); it++) {
			(*it)->Get_weight(&temp);
			*it_grad -= 2.f * temp;
			it_grad++;
		}
#endif

	}

	void Graph_Learnable::Weights_Manager::Set_w(const std::list<float>& w, Graph_Learnable* model) {

		auto itw = w.begin();
		for (auto it = model->Model_handlers.begin(); it != model->Model_handlers.end(); it++) {
			(*it)->Set_weight(*itw);
			itw++;
		}

	}

	void Graph_Learnable::Get_w_grad(std::list<float>* grad_w, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order) {

		bool recompute_alpha = false;

		if (this->pLast_train_set == NULL) {
			recompute_alpha = true;
			this->pLast_train_set = new proxy_gradient_info(comb_train_set);
		}
		else {
			if (this->pLast_train_set->Last_set != comb_train_set) {
				recompute_alpha = true;
				delete this->pLast_train_set;
				this->pLast_train_set = new proxy_gradient_info(comb_train_set);
			}
		}

		if (recompute_alpha) {
			//recompute alfa part
			this->Alfa_part_gradient.clear();

			for (auto it = this->Model_handlers.begin(); it != this->Model_handlers.end(); it++) {
				this->Alfa_part_gradient.push_back(float());
				(*it)->Get_grad_alfa_part(&this->Alfa_part_gradient.back(), comb_train_set, comb_var_order);
			}
		}

		for (auto it = this->Alfa_part_gradient.begin(); it != this->Alfa_part_gradient.end(); it++)
			grad_w->push_back(*it);

	}

	void Graph_Learnable::Get_structure(std::list<const Potential_Exp_Shape*>* result) {

		result->clear();
		for (auto it = this->Model_handlers.begin(); it != this->Model_handlers.end(); it++)
			result->push_back((*it)->get_wrapped_exp_pot());

	}




	size_t* list_2_malloc(const list<size_t>& l) {

		size_t* res = (size_t*)malloc(l.size() * sizeof(size_t));
		size_t k = 0;
		for (auto it = l.begin(); it != l.end(); it++) {
			res[k] = *it;
			k++;
		}
		return res;

	};

	Random_Field::Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file) {

		XML_reader reader(prefix_config_xml_file + config_xml_file);
		this->Import_from_XML(&reader, prefix_config_xml_file);

	};

	void Random_Field::Get_w_grad(std::list<float>* grad_w, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order) {

		this->Set_Observation_Set_var(list<Categoric_var*>());
		this->Set_Observation_Set_val(list<size_t>());
		this->Belief_Propagation(true);

		float temp;
		auto it_grad = grad_w->begin();
		for (auto it = this->Model_handlers.begin(); it != this->Model_handlers.end(); it++) {
			(*it)->Get_grad_beta_part(&temp);
			*it_grad -= temp;
			it_grad++;
		}

	}

	void Random_Field::Get_Likelihood_estimation(float* result, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order) {

		float log_Z;
		this->Get_Log_Z(&log_Z);

		float E;
		*result = 0.f;
		for (auto it_set = comb_train_set.begin(); it_set != comb_train_set.end(); it_set++) {
			this->Eval_Log_Energy_function(&E, *it_set, comb_var_order);
			*result += E;
		}
		*result = (1.f / (float)comb_train_set.max_size()) * (*result);
		*result -=  log_Z;

	}



	class Binary_handler_with_Observation : public I_Learning_handler {
	public:
		Binary_handler_with_Observation(Node* Hidden_var, size_t* observed_val, I_Learning_handler* handle_to_substitute);
	private:
		void    Get_grad_beta_part(float* beta);
	// data
		Node*				pNode_hidden;
		size_t*				ref_to_val_observed;
	};

	Binary_handler_with_Observation::Binary_handler_with_Observation(Node* Hidden_var, size_t* observed_val, I_Learning_handler* handle_to_substitute) :
		I_Learning_handler(handle_to_substitute), pNode_hidden(Hidden_var), ref_to_val_observed(observed_val) {

		delete handle_to_substitute;

	};

	void Binary_handler_with_Observation::Get_grad_beta_part(float* beta) {

		list<float> marginals;
		list<Potential*> message_union;
		this->pNode_hidden->Gather_all_Unaries(&message_union);
		Potential UP(message_union);
		UP.Get_marginals(&marginals);

		size_t pos_hidden = 0, pos_obsv = 1;
		if (this->Get_involved_var()->back() == this->pNode_hidden->Get_var()) {
			pos_hidden = 1;
			pos_obsv = 0;
		}

		list<size_t*> comb_to_search;
		for (size_t k = 0; k < this->pNode_hidden->Get_var()->size(); k++) {
			comb_to_search.push_back((size_t*)malloc(sizeof(size_t) * 2));
			comb_to_search.back()[pos_hidden] = k;
			comb_to_search.back()[pos_obsv] = *this->ref_to_val_observed;
		}
		list<I_Distribution_value*> distr_conditioned_to_obsv;
		Find_Comb_in_distribution(&distr_conditioned_to_obsv, comb_to_search, *this->Get_involved_var(), this->Potential_Exp_Shape::Getter_weight_and_shape::Get_shape(this->pwrapped));

		Dot_with_Prob(beta, marginals, distr_conditioned_to_obsv);

		for (auto it = comb_to_search.begin(); it != comb_to_search.end(); it++)
			free(*it);

	}




	Conditional_Random_Field::Conditional_Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file) {

		XML_reader reader(prefix_config_xml_file + config_xml_file);
		this->Import_from_XML(&reader, prefix_config_xml_file);

		//read the observed set
		list<string> observed_names;
		XML_reader::Tag_readable root = reader.Get_root();
		list<XML_reader::Tag_readable> vars;
		root.Get_Nested("Variable", &vars);
		string flag;
		for (auto it = vars.begin(); it != vars.end(); it++) {
			if (it->Exist_Field("flag")) {
				flag = it->Get_value("flag");
				if (flag.compare("O") == 0)
					observed_names.push_back(it->Get_value("name"));
			}
		}

		if (observed_names.empty()) {
			system("ECHO Found CRF with no observed variables");
			abort();
		}

		list<Categoric_var*> observed_vars;
		for (auto it = observed_names.begin(); it != observed_names.end(); it++)
			observed_vars.push_back(this->Find_Variable(*it));

		this->Node_factory::Set_Observation_Set_var(observed_vars);

		list<size_t*> temp;
		list<Categoric_var*>::const_iterator it_temp;
		auto it_hnd = this->Model_handlers.begin();
		while(it_hnd != this->Model_handlers.end()){
			temp.clear();
			auto vars = (*it_hnd)->Get_involved_var_safe();
			for (it_temp = vars->begin(); it_temp != vars->end(); it_temp++)
				temp.push_back(this->Get_observed_val_in_case_is_in_observed_set(*it_temp));

			if (temp.size() == 1) {
				if (temp.front() != NULL) {
					//remove this unary handler since is attached to an observation
					delete *it_hnd;
					it_hnd = this->Model_handlers.erase(it_hnd);
					system("ECHO warning: detected redundant potentials attached to observed variables");
				}
				else it_hnd++;
			}
			else {
				if ((temp.front() != NULL) && (temp.back() != NULL)) {
					//remove this binary handler since is attached to an observation
					delete *it_hnd;
					it_hnd = this->Model_handlers.erase(it_hnd);
					system("ECHO warning: detected redundant potentials attached to observed variables");
				}
				else if (temp.front() != NULL) {
					*it_hnd = new Binary_handler_with_Observation(this->Find_Node(vars->back()->Get_name()), temp.front(), *it_hnd);
					it_hnd++;
				}
				else if (temp.back() != NULL) {
					*it_hnd = new Binary_handler_with_Observation(this->Find_Node(vars->front()->Get_name()), temp.back(), *it_hnd);
					it_hnd++;
				}
				else it_hnd++;
			}
		}

	};

	Conditional_Random_Field::Conditional_Random_Field(const std::list<Potential_Exp_Shape*>& potentials, const std::list<Categoric_var*>& observed_var) {

		list<Potential_Exp_Shape*>		binary_edge;
		list<Potential_Exp_Shape*>		unary_edge;
		for (auto it = potentials.begin(); it != potentials.end(); it++) {
			if ((*it)->Get_involved_var_safe()->size() == 1)
				unary_edge.push_back(*it);
			else if ((*it)->Get_involved_var_safe()->size() == 2)
				binary_edge.push_back(*it);
			else {
				system("ECHO invalid component to insert in a graph");
				abort();
			}
		}

		this->Node_factory::Insert(binary_edge);
		for (auto it = unary_edge.begin(); it != unary_edge.end(); it++)
			this->Insert(*it);

		list<Categoric_var*> vars_hidden;
		for (auto it = observed_var.begin(); it != observed_var.end(); it++)
			vars_hidden.push_back(this->Find_Variable(*it));
		this->Node_factory::Set_Observation_Set_var(vars_hidden);

	}

	void extract_observations(std::list<size_t>* result, size_t* entire_vec, const std::list<size_t>& var_pos) {

		result->clear();
		for (auto it = var_pos.begin(); it != var_pos.end(); it++)
			result->push_back(entire_vec[*it]);

	}

	void find_observed_order(list<size_t>* result, const std::list<Categoric_var*>& order_in_model, const std::list<Categoric_var*>& order_in_train_set) {

		result->clear();

		list<Categoric_var*>::const_iterator it_in_train_set;
		size_t k;
		for (auto it = order_in_model.begin(); it != order_in_model.end(); it++) {
			k = 0;
			for (it_in_train_set = order_in_train_set.begin(); it_in_train_set != order_in_train_set.end(); it_in_train_set++) {
				if (*it_in_train_set == *it) {
					result->push_back(k);
					break;
				}
				k++;
			}
		}

	}

	void Conditional_Random_Field::Get_w_grad(std::list<float>* grad_w, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order) {

		list<size_t> pos_of_observed_var;
		list<Categoric_var*> observed_var_temp;
		this->Get_Actual_Observation_Set(&observed_var_temp);
		find_observed_order(&pos_of_observed_var, observed_var_temp, comb_var_order);

		float temp;
		list<float> beta_total;
		auto it = this->Model_handlers.begin();
		for (it; it != this->Model_handlers.end(); it++)
			beta_total.push_back(0.f);
		list<size_t> obsv;

		auto it_beta = beta_total.begin();
		for (auto itL = comb_train_set.begin(); itL != comb_train_set.end(); itL++) {
			extract_observations(&obsv, *itL, pos_of_observed_var);
			this->Set_Observation_Set_val(obsv);
			this->Belief_Propagation(true);

			it = this->Model_handlers.begin();
			for (it_beta = beta_total.begin(); it_beta != beta_total.end(); it_beta++) {
				(*it)->Get_grad_beta_part(&temp);
				*it_beta += temp;
				it++;
			}
		}

		auto it_grad = grad_w->begin();
		temp = 1.f / (float)comb_train_set.size();
		for (it_beta = beta_total.begin(); it_beta != beta_total.end(); it_beta++) {
			*it_beta *= temp;
			*it_grad -= *it_beta;

			it_grad++;
		}

	}

	void Conditional_Random_Field::Get_Likelihood_estimation(float* result,const std::list<size_t*>& comb_train_set,const std::list<Categoric_var*>& comb_var_order) {

		list<size_t> pos_observ;
		list<Categoric_var*> observed_var_temp;
		this->Get_Actual_Observation_Set(&observed_var_temp);
		find_observed_order(&pos_observ, observed_var_temp, comb_var_order);

		float log_Z;
		float E;
		*result = 0.f;
		list<size_t> obsv_temp;
		for (auto it_set = comb_train_set.begin(); it_set != comb_train_set.end(); it_set++) {
			//compute logZ according to these observations
			extract_observations(&obsv_temp, *it_set, pos_observ);
			this->Set_Observation_Set_val(obsv_temp);
			this->Get_Log_Z(&log_Z);
			*result -= log_Z;

			this->Eval_Log_Energy_function(&E, *it_set, comb_var_order);
			*result += E;
		}
		*result = (1.f / (float)comb_train_set.max_size()) * (*result);

	}

	void Conditional_Random_Field::Get_Likelihood_Observations_estimation(float* result, size_t* comb_observations, const std::list<Categoric_var*>& comb_var_order) {

		list<size_t> pos_observ;
		list<Categoric_var*> observed_var_temp;
		this->Get_Actual_Observation_Set(&observed_var_temp);
		find_observed_order(&pos_observ, observed_var_temp, comb_var_order);

		list<size_t> MAP_val;
		extract_observations(&MAP_val, comb_observations, pos_observ);
		this->Set_Observation_Set_val(MAP_val);

		float E;
		list<Categoric_var*> Hidden_vars;
		this->Get_Actual_Hidden_Set(&Hidden_vars);
		list<Categoric_var*> MAP_var = observed_var_temp;
		for (auto it = Hidden_vars.begin(); it != Hidden_vars.end(); it++)
			MAP_var.push_back(*it);
		list<size_t> Hidden_MAP;
		this->MAP_on_Hidden_set(&Hidden_MAP);
		for (auto it = Hidden_MAP.begin(); it != Hidden_MAP.end(); it++)
			MAP_val.push_back(*it);
		this->Eval_Log_Energy_function(&E, MAP_val, MAP_var);

		float Z_tot;
		this->Node_factory::Get_Log_Z(&Z_tot);
		this->Set_Observation_Set_var(observed_var_temp);

		*result = E;
		*result -= Z_tot;

	}

	/*
	template<typename T>
	void append_b_to_a(list<T>* a, const list<T>& b) {

		for (auto it = b.begin(); it != b.end(); it++)
			a->push_back(*it);

	};
	void Conditional_Random_Field::Get_Likelihood_estimation_observations(float* result, size_t* comb_observations, const std::list<Categoric_var*>& comb_var_order) {

		size_t* temp_mall;
		list<size_t> XY_MAP; list<Categoric_var*> XY_MAP_var;
		list<size_t> X_real_Y_MAP; list<Categoric_var*> X_real_Y_MAP_var;
		list<Categoric_var*> obs_var;
		this->Get_Actual_Observation_Set(&obs_var);

		this->Set_Observation_Set_var({});
		this->Set_Observation_Set_val({});
		this->MAP_on_Hidden_set(&XY_MAP);
		this->Get_Actual_Hidden_Set(&XY_MAP_var);
		temp_mall = list_2_malloc(XY_MAP);
		this->Get_Log_activation(result, temp_mall, XY_MAP_var);
		*result *= -1.f;
		free(temp_mall);


		list<size_t> pos_of_observed_var;
		find_observed_order(&pos_of_observed_var, obs_var, comb_var_order);
		list<size_t> obs_temp;
		extract_observations(&obs_temp, comb_observations, pos_of_observed_var);
		this->Set_Observation_Set_var(obs_var);
		this->Set_Observation_Set_val(obs_temp);
		this->Get_Actual_Hidden_Set(&X_real_Y_MAP_var);
		this->MAP_on_Hidden_set(&X_real_Y_MAP);
		append_b_to_a(&X_real_Y_MAP_var, obs_var);
		append_b_to_a(&X_real_Y_MAP, obs_temp);
		temp_mall = list_2_malloc(X_real_Y_MAP);
		float temp;
		this->Get_Log_activation(&temp, temp_mall, X_real_Y_MAP_var);
		*result += temp;
		free(temp_mall);

	}*/

	void Conditional_Random_Field::Get_Log_Z(float* Z) {

		this->Recompute_Log_Z(Z); //computations are done considering the current observations set

	}

}