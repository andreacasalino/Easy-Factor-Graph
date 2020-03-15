/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include "../Header/Graphical_model.h"
#include <iostream>
using namespace std;

namespace EFG {

	Graph::Graph(const std::string& config_xml_file, const std::string& prefix_config_xml_file) :
		Node_factory(true) {

		XML_reader* reader = NULL;
		try { reader = new XML_reader(prefix_config_xml_file + config_xml_file); }
		catch (int) { throw 0; }
		if (reader != NULL) {
			try { this->Import_from_XML(reader, prefix_config_xml_file); }
			catch (int) { throw 1; }
		}
		if (reader != NULL) delete reader;

	};

	Graph::Graph(const std::vector<Potential_Shape*>& potentials, const std::vector<Potential_Exp_Shape*>& potentials_exp, const bool& use_cloning_Insert) :
		Node_factory(use_cloning_Insert) {

		this->__Insert(potentials, {}, potentials_exp);

	}




	Graph_Learnable::~Graph_Learnable() {

		for (auto it = this->Model_handlers.begin(); it != this->Model_handlers.end(); it++)
			delete *it;

	}

	Potential_Exp_Shape* Graph_Learnable::__Insert(Potential_Exp_Shape* pot, const bool& weight_tunability) {

		auto pot_inserted = this->Node_factory::__Insert(pot);
		if (pot_inserted == NULL) return NULL;

		if (weight_tunability) {
			auto vars = pot_inserted->Get_involved_var();

			if (vars->size() == 1) {
				//new unary
				auto new_atomic = new Unary_handler(this, pot_inserted);
				this->Atomic_Learner.push_back(Graph_Learnable::Learner_info<atomic_Learning_handler>());
				this->Atomic_Learner.back().pos_in_Model_handlers = this->Model_handlers.size();
				this->Atomic_Learner.back().Ref_to_learner = new_atomic;
				this->Model_handlers.push_back(new_atomic);
			}
			else {
				//new binary
				Node* N1 = this->__Find_Node(vars->front());
				Node* N2 = this->__Find_Node(vars->back());

				auto new_atomic = new Binary_handler(this, pot_inserted);
				this->Atomic_Learner.push_back(Graph_Learnable::Learner_info<atomic_Learning_handler>());
				this->Atomic_Learner.back().pos_in_Model_handlers = this->Model_handlers.size();
				this->Atomic_Learner.back().Ref_to_learner = new_atomic;
				this->Model_handlers.push_back(new_atomic);
			}
		}
		return pot_inserted;

	}

	void Graph_Learnable::Share_weight(I_Learning_handler* pot_involved, const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share) {

		auto it_to_share = this->Atomic_Learner.begin();
		bool found = false;
		auto it_to_share_end = this->Atomic_Learner.end();
		for (it_to_share; it_to_share != it_to_share_end; it_to_share++) {
			if (it_to_share->Ref_to_learner == pot_involved) {
				found = true;
				break;
			}
		}
		if (!found)
			abort();

		size_t info = 0; //0 -> not found, 1 -> found in atomic, 2 -> found in composite
		auto info_it_atomic = this->Atomic_Learner.begin();
		auto info_it_composite = this->Composite_Learner.begin();
		for (auto it = this->Atomic_Learner.begin(); it != it_to_share_end; it++) {
			if (it->Ref_to_learner->is_here_Pot_to_share(vars_of_pot_whose_weight_is_to_share)) {
				info = 1;
				info_it_atomic = it;
				break;
			}
		}
		if (info == 0) {
			auto it_end = this->Composite_Learner.end();
			for (auto it = this->Composite_Learner.begin(); it != it_end; it++) {
				if (it->Ref_to_learner->is_here_Pot_to_share(vars_of_pot_whose_weight_is_to_share)) {
					info = 2;
					info_it_composite = it;
					break;
				}
			}
		}

		if (info == 0) throw 0; //was not able to find the potential whose weight is to share

		auto to_share = it_to_share->Ref_to_learner;
		auto it_M = this->Model_handlers.begin();
		advance(it_M, it_to_share->pos_in_Model_handlers);
		this->Model_handlers.erase(it_M);
		this->Atomic_Learner.erase(it_to_share);

		if (info == 1) {
			auto comp_temp = new composite_Learning_handler(info_it_atomic->Ref_to_learner, to_share);

			this->Composite_Learner.push_back(Learner_info<composite_Learning_handler>());
			this->Composite_Learner.back().Ref_to_learner = comp_temp;
			this->Composite_Learner.back().pos_in_Model_handlers = info_it_atomic->pos_in_Model_handlers;
			it_M = this->Model_handlers.begin();
			advance(it_M, info_it_atomic->pos_in_Model_handlers);
			*it_M = comp_temp;

			this->Atomic_Learner.erase(info_it_atomic);
		}
		else info_it_composite->Ref_to_learner->Append(to_share);

	}

	Graph_Learnable::Graph_Learnable(const std::vector<Potential_Shape*>& shapes, const std::vector<std::list<Potential_Exp_Shape*>>& learnable_exp, const std::vector<Potential_Exp_Shape*>& constant_exp, const bool& use_cloning_Insert)
		: Graph_Learnable(use_cloning_Insert) {

		this->__Insert(shapes, learnable_exp, constant_exp);

	}

	void Graph_Learnable::Get_tunable(std::vector<float>* w) const {

		w->clear();
		w->reserve(this->Model_handlers.size());
		auto it_end = this->Model_handlers.end();
		for (auto it = this->Model_handlers.begin(); it != it_end; it++)
			w->push_back((*it)->Get_weight());

	}

	void Graph_Learnable::Get_tunable_grad(std::vector<float>* w_grad, const I_Potential::combinations& training_set, const bool& force_alpha_recomputation) {

		bool recompute_alfa = false;
		if (force_alpha_recomputation) recompute_alfa = true;
		else {
			if (this->Last_training_set_used != &training_set) recompute_alfa = true;
		}
		this->Last_training_set_used = &training_set; 

		w_grad->clear();
		w_grad->reserve(this->Model_handlers.size());

		std::list<I_Learning_handler*>::iterator it, it_end = this->Model_handlers.end();
		if (recompute_alfa) {
			for (it = this->Model_handlers.begin(); it != it_end; it++) 
				(*it)->Recompute_grad_alfa_part(*this->Last_training_set_used);
		}
		for (it = this->Model_handlers.begin(); it != it_end; it++)	w_grad->push_back((*it)->Get_grad_alfa_part());
		vector<float> beta_part;
		this->__Get_beta_part(&beta_part, training_set);
		size_t k, K = w_grad->size();
		for (k = 0; k < K; k++)
			(*w_grad)[k] -= beta_part[k];

		if (this->Use_regularization) {
			k = 0;
			for (it = this->Model_handlers.begin(); it != it_end; it++) {
				(*w_grad)[k] -= 2.f * (*it)->Get_weight();
				k++;
			}
		}

	}

	void Graph_Learnable::Set_tunable(const std::vector<float>& w_vector) {

		if (this->Model_handlers.size() != w_vector.size()) throw 0;
		size_t k = 0;
		auto it_end = this->Model_handlers.end();
		for (auto it = this->Model_handlers.begin(); it != it_end; it++) {
			(*it)->Set_weight(w_vector[k]);
			k++;
		}

	}

	void Graph_Learnable::Get_TrainingSet_Likelihood(float* result, const I_Potential::combinations& training_set) const {

		vector<float> L;
		this->Eval_Energy_function_normalized(&L, training_set);
		*result = 0.f;
		float coeff = 1.f / (float)training_set.get_number_of_combinations();
		auto it_end = L.end();
		for (auto it = L.begin(); it != it_end; it++)
			*result += coeff * *it;

	};

	void Graph_Learnable::Remove(atomic_Learning_handler* to_remove) {

		auto it_end = this->Atomic_Learner.end();
		for (auto it = this->Atomic_Learner.begin(); it != it_end; it++) {
			if (to_remove == it->Ref_to_learner) {
				delete it->Ref_to_learner;
				auto it_M = this->Model_handlers.begin();
				advance(it_M, it->pos_in_Model_handlers);
				this->Model_handlers.erase(it_M);
				this->Atomic_Learner.erase(it);
				return;
			}
		}

		list<atomic_Learning_handler*>* temp;
		list<atomic_Learning_handler*>::iterator it_temp;
		auto it_end2 = this->Composite_Learner.end();
		for (auto it = this->Composite_Learner.begin(); it != it_end2; it++) {
			temp = it->Ref_to_learner->Get_Components();
			for (it_temp = temp->begin(); it_temp != temp->end(); it_temp++) {
				if (to_remove == *it_temp) {
					delete to_remove;
					temp->erase(it_temp);

					if (temp->empty()) {
						auto it_M = this->Model_handlers.begin();
						advance(it_M, it->pos_in_Model_handlers);
						this->Model_handlers.erase(it_M);
						this->Composite_Learner.erase(it);
					}

					return;
				}
			}
		}

	}

	void Graph_Learnable::__Get_structure(std::vector<Potential_Shape*>* shapes, std::vector<std::list<Potential_Exp_Shape*>>* learnable_exp, std::vector<Potential_Exp_Shape*>* constant_exp) const {

		this->Node_factory::__Get_structure(shapes, learnable_exp, constant_exp);
		list<Potential_Exp_Shape*> to_remove;
		learnable_exp->reserve(this->Atomic_Learner.size() + this->Composite_Learner.size());
		auto itA_end = this->Atomic_Learner.end();
		for (auto itA = this->Atomic_Learner.begin(); itA != itA_end; itA++) {
			learnable_exp->push_back({ itA->Ref_to_learner->Get_wrapped() });
			to_remove.push_back(itA->Ref_to_learner->Get_wrapped());
		}
		list<atomic_Learning_handler*>::iterator it, it_end;
		auto itC_end = this->Composite_Learner.end();
		for (auto itC = this->Composite_Learner.begin(); itC != itC_end; itC++) {
			it_end = itC->Ref_to_learner->Get_Components()->end(); 
			learnable_exp->push_back(list<Potential_Exp_Shape*>());
			for (it = itC->Ref_to_learner->Get_Components()->begin(); it != it_end; it++) {
				learnable_exp->back().push_back((*it)->Get_wrapped());
				to_remove.push_back((*it)->Get_wrapped());
			}
		}

		vector<Potential_Exp_Shape*>::iterator itP, itP_end;
		auto r_end = to_remove.end();
		for (auto r = to_remove.begin(); r != r_end; r++) {
			itP_end = constant_exp->end();
			for (itP = constant_exp->begin(); itP != itP_end; itP++) {
				if (*itP == *r) {
					constant_exp->erase(itP);
					break;
				}
			}
		}

	}

	void Graph_Learnable::__Insert(const std::vector<Potential_Shape*>& shapes, const std::vector<std::list<Potential_Exp_Shape*>>& learnable_exp, const std::vector<Potential_Exp_Shape*>& constant_exp) {

		size_t k, K = shapes.size();
		for (k = 0; k < K; k++)
			this->Node_factory::__Insert(shapes[k]);

		K = constant_exp.size();
		for (k = 0; k < K; k++)
			this->__Insert(constant_exp[k], false);

		K = learnable_exp.size();
		list<Potential_Exp_Shape*>::const_iterator it, it_end;
		Potential_Exp_Shape* first_inserted = NULL;
		const list<Categoric_var*>* vars_to_share;
		for (k = 0; k < K; k++) {
			it_end = learnable_exp[k].end();
			it = learnable_exp[k].begin();
			first_inserted = this->__Insert(*it, true);
			vars_to_share = first_inserted->Get_involved_var();
			it++;
			for (it; it != it_end; it++) {
				this->__Insert(*it, true);
				this->Share_weight(this->Model_handlers.back(), *vars_to_share);
			}
		}

	}

	void Graph_Learnable::__Get_Atomic_Learner_complete_list(std::list<atomic_Learning_handler**>* atomic_list) {

		atomic_list->clear();
		auto itA_end = this->Atomic_Learner.end();
		for (auto it = this->Atomic_Learner.begin(); it != itA_end; it++)
			atomic_list->push_back(&it->Ref_to_learner);
		auto itC_end = this->Composite_Learner.end();
		for (auto it = this->Composite_Learner.begin(); it != itC_end; it++) {
			auto wrapped = it->Ref_to_learner->Get_Components();
			for (auto it2 = wrapped->begin(); it2 != wrapped->end(); it2++)
				atomic_list->push_back(&(*it2));
		}

	}




	Random_Field::Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file) : Graph_Learnable(true) {

		XML_reader* reader = NULL;
		try { reader = new XML_reader(prefix_config_xml_file + config_xml_file); }
		catch (int) { throw 0; }
		if (reader != NULL) {
			try { this->Import_from_XML(reader, prefix_config_xml_file); }
			catch (int) { throw 1; }
		}
		if (reader != NULL) delete reader;

	};

	Random_Field::Random_Field(const std::vector<Potential_Shape*>& shapes, const std::vector<std::list<Potential_Exp_Shape*>>& learnable_exp, const std::vector<Potential_Exp_Shape*>& constant_exp, const bool& use_cloning_Insert) :
		Graph_Learnable(shapes, learnable_exp, constant_exp, use_cloning_Insert) { };

	void Random_Field::__Get_beta_part(std::vector<float>* betas, const I_Potential::combinations& training_set) {

		this->Set_Evidences({}, {});
		this->Belief_Propagation(true);

		betas->clear();
		betas->reserve(this->Model_handlers.size());
		auto it_end = this->Model_handlers.end();
		for (auto it = this->Model_handlers.begin(); it != it_end; it++)
			betas->push_back((*it)->Get_grad_beta_part());

	}

	void Random_Field::Insert(Potential_Exp_Shape& pot, const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share) {

		if (this->Graph_Learnable::__Insert(&pot, true) != NULL) 
			this->Share_weight(this->Model_handlers.back(), vars_of_pot_whose_weight_is_to_share);

	};




	Conditional_Random_Field::Conditional_Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file) : Graph_Learnable(true) {

		XML_reader* reader = NULL;
		try { reader = new XML_reader(prefix_config_xml_file + config_xml_file); }
		catch (int) { throw 0; }
		if (reader != NULL) {
			try { this->Import_from_XML(reader, prefix_config_xml_file); }
			catch (int) { throw 1; }
		}
		if (reader != NULL) delete reader;

		this->__remove_redudant();

		this->vars_order_training_set = NULL;
		this->pos_observations_in_training_set = NULL;

	};

	Conditional_Random_Field::Conditional_Random_Field(const std::vector<Potential_Shape*>& shapes, const std::vector<std::list<Potential_Exp_Shape*>>& learnable_exp, const std::vector<Potential_Exp_Shape*>& constant_exp, const std::list<string>& observed_var, const bool& use_cloning_Insert) :
		Graph_Learnable(shapes, learnable_exp, constant_exp, use_cloning_Insert) {

		list<Categoric_var*> hidden_vars;
		list<size_t>		 hidden_vals;
		auto it_end = observed_var.end();
		for (auto it = observed_var.begin(); it != it_end; it++) {
			hidden_vars.push_back(this->Find_Variable(*it));
			hidden_vals.push_back(0);
		}
		this->Node_factory::Set_Evidences(hidden_vars, hidden_vals);

		this->__remove_redudant();

		this->vars_order_training_set = NULL;
		this->pos_observations_in_training_set = NULL;

	}

	Conditional_Random_Field::Conditional_Random_Field(const Conditional_Random_Field& o) : Graph_Learnable(o) {

		this->__copy(o);

		this->__remove_redudant();

		this->vars_order_training_set = NULL;
		this->pos_observations_in_training_set = NULL;

	};

	void Conditional_Random_Field::__remove_redudant() {

		list<Categoric_var*> hidden_set;
		this->Get_Actual_Hidden_Set(&hidden_set);
		if (hidden_set.empty()) throw 0;

		list<atomic_Learning_handler**> atomic_list;
		this->__Get_Atomic_Learner_complete_list(&atomic_list);

		list<size_t*> temp;
		size_t* temp_new = NULL;
		list<Categoric_var*>::const_iterator it_temp;
		auto it_hnd = atomic_list.begin();
		auto it_hnd2 = this->Model_handlers.begin();
		auto it_hnd_end = atomic_list.end();
		auto it_Model_end = this->Model_handlers.end();
		for (it_hnd; it_hnd != it_hnd_end; it_hnd++) {
			temp.clear();
			auto vars = (**it_hnd)->Get_involved_var();
			for (it_temp = vars->begin(); it_temp != vars->end(); it_temp++)
				temp.push_back(this->__Get_observed_val(*it_temp));

			if (temp.size() == 1) {
				if (temp.front() != NULL) {
					//remove this unary handler since is attached to an observation
					this->Remove(**it_hnd);					
#ifdef _DEBUG
					cout << "warning: detected redundant potentials attached to observed variables";
#endif
				}
			}
			else {
				if ((temp.front() != NULL) && (temp.back() != NULL)) {
					//remove this binary handler since is attached to an observation
					this->Remove(**it_hnd);
#ifdef _DEBUG
					cout << "warning: detected redundant potentials attached to observed variables";
#endif
				}
				else if (temp.front() != NULL) {
					for (it_hnd2 = this->Model_handlers.begin(); it_hnd2 != it_Model_end; it_hnd2++) {
						if (*it_hnd2 == **it_hnd) break;
					}
					Binary_handler_with_Observation::Create(this->__Find_Node(vars->back()), temp.front(), *it_hnd);
					*it_hnd2 = **it_hnd;
				}
				else if (temp.back() != NULL) {
					for (it_hnd2 = this->Model_handlers.begin(); it_hnd2 != it_Model_end; it_hnd2++) {
						if (*it_hnd2 == **it_hnd) break;
					}
					Binary_handler_with_Observation::Create(this->__Find_Node(vars->front()), temp.back(), *it_hnd);
					*it_hnd2 = **it_hnd;
				}
			}
		}

	}

	void Conditional_Random_Field::__Get_beta_part(std::vector<float>* betas, const I_Potential::combinations& training_set) {

		betas->clear();
		betas->reserve(this->Model_handlers.size());
		size_t k, K = this->Model_handlers.size();
		for (k = 0; k < K; k++) betas->push_back(0.f);
		float coeff = 1.f / (float)training_set.get_number_of_combinations();

		//recompute pos_observations if needed
		if (&training_set.Get_variables() != this->vars_order_training_set) {
			this->vars_order_training_set = &training_set.Get_variables();
			list<Categoric_var*> obsv_set;
			this->Get_Actual_Observation_Set_Var(&obsv_set);
			
			if (this->pos_observations_in_training_set == NULL) {
				this->pos_observations_size = obsv_set.size();
				this->pos_observations_in_training_set = (size_t*)malloc(this->pos_observations_size * sizeof(size_t));
			}
			list<Categoric_var*>::const_iterator it, it_end = this->vars_order_training_set->end();
			list<Categoric_var*>::iterator ito_end = obsv_set.end();
			k = 0;
			size_t o;
			for (auto ito = obsv_set.begin(); ito != ito_end; ito++) {
				o = 0;
				for (it = this->vars_order_training_set->begin(); it != it_end; it++) {
					if (*it == *ito) {
						this->pos_observations_in_training_set[k] = o;
						break;
					}
					o++;
				}
				k++;
			}
		}

		list<size_t> observations;
		I_Potential::combinations::iterator it_set(training_set);
		list<I_Learning_handler*>::iterator it_h;
		const size_t* cc = NULL;
		while (it_set.is_not_at_end()) {
			observations.clear();
			cc = *it_set;
			for (k = 0; k < this->pos_observations_size; k++)
				observations.push_back(cc[this->pos_observations_in_training_set[k]]);

			this->Set_Evidences(observations);
			this->Belief_Propagation(true);

			it_h = this->Model_handlers.begin();
			for (k = 0; k < K; k++) {
				(*betas)[k] += coeff * (*it_h)->Get_grad_beta_part();
				it_h++;
			}
			++it_set;
		}

	}

}