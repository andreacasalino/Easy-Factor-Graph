#include "../Header/Graphical_model.h"
using namespace std;

//#define ADD_REGULARIZATION

namespace Segugio {

	template<typename T>
	void clean_collection(list<T*>& coll) {

		for (auto it = coll.begin(); it != coll.end(); it++)
			delete *it;

	}

	Graph::Graph(const std::string& config_xml_file, const std::string& prefix_config_xml_file) :
		Node_factory(true) {

		XML_reader reader(prefix_config_xml_file + config_xml_file);
		this->Import_from_XML(&reader, prefix_config_xml_file); 

	};

	Graph::Graph(const std::list<Potential_Shape*>& potentials, const std::list<Potential_Exp_Shape*>& potentials_exp, const bool& use_cloning_Insert) :
		Node_factory(use_cloning_Insert) {

		list<_Pot_wrapper_4_Insertion*> temp;
		for (auto it = potentials.begin(); it != potentials.end(); it++)
			temp.push_back(new _Baseline_4_Insertion<Potential_Shape>(*it));
		for (auto it = potentials_exp.begin(); it != potentials_exp.end(); it++)
			temp.push_back(new _Baseline_4_Insertion<Potential_Exp_Shape>(*it));

		this->Node_factory::Insert(temp);
		clean_collection(temp);

	}





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

	Graph_Learnable::_Pot_wrapper_4_Insertion* Graph_Learnable::Get_Inserter(Potential_Exp_Shape* pot, const bool& weight_tunability) {

		struct Insertion_Handler : public _Baseline_4_Insertion<Potential_Exp_Shape> {
			Insertion_Handler(Potential_Exp_Shape* wrp, Graph_Learnable* graph, const bool& tunab) : 
				_Baseline_4_Insertion<Potential_Exp_Shape>(wrp), pt_to_graph(graph) , tunability(tunab){};
			virtual Potential*		Get_Potential_to_Insert(const std::list<Categoric_var*>& var_involved, const bool& get_cloned) {
				Potential_Exp_Shape* pot_exp_to_insert;
				if (get_cloned)
					pot_exp_to_insert = new Potential_Exp_Shape(this->wrapped, var_involved);
				else
					pot_exp_to_insert = this->wrapped;

				if (tunability) {
					auto vars = pot_exp_to_insert->Get_involved_var_safe();

					if (vars->size() == 1) {
						//new unary
						this->pt_to_graph->Model_handlers.push_back(new Unary_handler(this->pt_to_graph->Find_Node(vars->front()->Get_name()), pot_exp_to_insert));
					}
					else {
						//new binary
						Node* N1 = this->pt_to_graph->Find_Node(vars->front()->Get_name());
						Node* N2 = this->pt_to_graph->Find_Node(vars->back()->Get_name());

						this->pt_to_graph->Model_handlers.push_back(new Binary_handler(N1, N2, pot_exp_to_insert));
					}
				}

				return new Potential(pot_exp_to_insert);
			};
		private:
			Graph_Learnable* pt_to_graph;
			bool						   tunability;
		};

		return new Insertion_Handler(pot, this, weight_tunability);

	}

	Graph_Learnable::Graph_Learnable(const std::list<Potential_Exp_Shape*>& potentials_exp, const bool& use_cloning_Insert, const std::list<bool>& tunable_mask,
		const std::list<Potential_Shape*>& shapes) : Graph_Learnable(use_cloning_Insert) {

		list<bool>* tun_to_use = NULL;
		const	list<bool>* pt_tun_to_use = &tunable_mask;
		if (tunable_mask.empty()) {
			tun_to_use = new list<bool>();
			for (size_t k = 0; k < potentials_exp.size(); k++)
				tun_to_use->push_back(true);
			pt_tun_to_use = tun_to_use;
		}
		else {
			if (tunable_mask.size() != potentials_exp.size())
				abort();
		}

		list<_Pot_wrapper_4_Insertion*> temp;
		for (auto it = shapes.begin(); it != shapes.end(); it++)
			temp.push_back(new _Baseline_4_Insertion<Potential_Shape>(*it));
		auto it_tun = pt_tun_to_use->begin();
		for (auto it = potentials_exp.begin(); it != potentials_exp.end(); it++) {
			temp.push_back(this->Get_Inserter(*it, *it_tun));
			it_tun++;
		}
		if (tunable_mask.empty())
			delete tun_to_use;

		this->Node_factory::Insert(temp);
		clean_collection(temp);
	}

	void Graph_Learnable::Weights_Manager::Get_tunable_w(std::list<float>* w, Graph_Learnable* model) {

		w->clear();
		for (auto it = model->Model_handlers.begin(); it != model->Model_handlers.end(); it++) {
			w->push_back(float());
			(*it)->Get_weight(&w->back());
		}

	}

	void Graph_Learnable::Weights_Manager::Get_tunable_w_grad(std::list<float>* grad_w, Graph_Learnable* model, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order) {

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

	void Graph_Learnable::Weights_Manager::Set_tunable_w(const std::list<float>& w, Graph_Learnable* model) {

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

	void Graph_Learnable::Get_Likelihood_estimation(float* result, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order) {

		list<float> temp;
		this->Eval_Log_Energy_function_normalized(&temp, comb_train_set, comb_var_order);
		*result = 0.f;
		for (auto it = temp.begin(); it != temp.end(); it++)
			*result += *it;
		*result = *result / (float)comb_train_set.max_size();

	};




	size_t* list_2_malloc(const list<size_t>& l) {

		size_t* res = (size_t*)malloc(l.size() * sizeof(size_t));
		size_t k = 0;
		for (auto it = l.begin(); it != l.end(); it++) {
			res[k] = *it;
			k++;
		}
		return res;

	};

	Random_Field::Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file) : Graph_Learnable(true) {

		XML_reader reader(prefix_config_xml_file + config_xml_file);
		this->Import_from_XML(&reader, prefix_config_xml_file);

	};

	Random_Field::Random_Field(const std::list<Potential_Exp_Shape*>& potentials_exp, const bool& use_cloning_Insert, const std::list<bool>& tunable_mask,
		const std::list<Potential_Shape*>& shapes) :
		Graph_Learnable(potentials_exp, use_cloning_Insert, tunable_mask, shapes) { };

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




	void Conditional_Random_Field::__initialize() {

		list<size_t*> temp;
		list<Categoric_var*>::const_iterator it_temp;
		auto it_hnd = this->Model_handlers.begin();
		while (it_hnd != this->Model_handlers.end()) {
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

	}

	Conditional_Random_Field::Conditional_Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file) : Graph_Learnable(true) {

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

		this->__initialize();

	};

	Conditional_Random_Field::Conditional_Random_Field(const std::list<Potential_Exp_Shape*>& potentials, const std::list<Categoric_var*>& observed_var, const bool& use_cloning_Insert, const std::list<bool>& tunable_mask, 
		const std::list<Potential_Shape*>& shapes) :
		Graph_Learnable(potentials, use_cloning_Insert, tunable_mask, shapes) { 

		if (observed_var.empty()) {
			system("ECHO empty observed set for Conditional random field is not possible");
			abort();
		}

		list<Categoric_var*> vars_hidden;
		for (auto it = observed_var.begin(); it != observed_var.end(); it++)
			vars_hidden.push_back(this->Find_Variable(*it));

		this->Node_factory::Set_Observation_Set_var(vars_hidden);

		this->__initialize();

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

}