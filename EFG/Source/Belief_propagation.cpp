/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include "../Header/Belief_propagation.h"
#include <cmath>
#include <float.h>
using namespace std;

namespace EFG {



	I_belief_propagation_strategy::Message_Unary::Message_Unary(Categoric_var* var_involved) {

		Potential_Shape* ones = new Potential_Shape({ var_involved });
		ones->Set_ones();
		this->pwrapped = ones;

	}

	void I_belief_propagation_strategy::Message_Unary::eval_diff(float* diff, list<I_Potential::I_Distribution_value*>* F1, list<I_Potential::I_Distribution_value*>* F2) {

		if (F1->size() != F2->size()) {
			*diff = FLT_MAX;
			return;
		}

		*diff = 0.f;
		auto it2 = F2->begin();
		auto it1_end = F1->end();
		for (auto it1 = F1->begin(); it1 != it1_end; it1++) {
			*diff += abs((*it1)->Get_val() - (*it2)->Get_val());
			it2++;
		}

	}

	I_belief_propagation_strategy::Message_Unary::Message_Unary(Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP) {

		float fake_diff;
		this->Update(&fake_diff, binary_to_merge, potential_to_merge, Sum_or_MAP);

	}

	void I_belief_propagation_strategy::Message_Unary::Update(float* diff_to_previous, Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP) {

		if (potential_to_merge.empty())  throw 0; //  Message::Update: wrong method for managing an empty set of unary to merge

		Potential* unary_union = NULL;
		if (potential_to_merge.size() == 1)  unary_union = potential_to_merge.front();
		else unary_union = new Potential(potential_to_merge);

		auto merged_pot = this->merge_binary_and_unary(binary_to_merge, unary_union, Sum_or_MAP);
		merged_pot->Normalize_distribution();

		if (potential_to_merge.size() != 1) delete unary_union;

		if (pwrapped == NULL)
			*diff_to_previous = FLT_MAX;
		else {
			eval_diff(diff_to_previous, I_Potential::Get_distr_static(this->pwrapped), I_Potential::Get_distr_static(merged_pot));
			delete this->pwrapped;
		}
		this->pwrapped = merged_pot;

	}

	I_belief_propagation_strategy::Message_Unary::Message_Unary(Potential* binary_to_merge, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP) {

		float fake_diff;
		this->Update(&fake_diff, binary_to_merge, var_to_marginalize, Sum_or_MAP);

	}

	void I_belief_propagation_strategy::Message_Unary::Update(float* diff_to_previous, Potential* binary_to_merge, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP) {

		Message_Unary unary_temp(var_to_marginalize);

		auto merged_pot = this->merge_binary_and_unary(binary_to_merge, &unary_temp, Sum_or_MAP);
		merged_pot->Normalize_distribution();

		if (pwrapped == NULL)
			*diff_to_previous = FLT_MAX;
		else {
			eval_diff(diff_to_previous, I_Potential::Get_distr_static(this->pwrapped), I_Potential::Get_distr_static(merged_pot));
			delete this->pwrapped;
		}
		this->pwrapped = merged_pot;

	}

	Potential_Shape* I_belief_propagation_strategy::Message_Unary::merge_binary_and_unary(Potential* binary_to_merge, Potential* unary, const bool& Sum_or_MAP) {

		auto vars = binary_to_merge->Get_involved_var();
		Categoric_var* sender = vars->front(), * receiver = vars->back();
		size_t pos_sender = 0;
		if (sender != unary->Get_involved_var()->front()) {
			auto temp = sender;
			sender = receiver;
			receiver = temp;
			pos_sender = 1;
		}

		combinations comb_sender({ sender });
		vector<const I_Distribution_value*> val_sender_temp;
		comb_sender.Find_images_single_matches(&val_sender_temp, *unary);
		vector<float> val_sender;
		size_t k, K = val_sender_temp.size();
		val_sender.reserve(val_sender_temp.size());
		for (k = 0; k < K; k++) {
			if (val_sender_temp[k] == NULL) val_sender.push_back(0.f);
			else val_sender.push_back(val_sender_temp[k]->Get_val());
		}

		combinations comb_bin({ receiver });
		vector<list<const I_Distribution_value*>> val_bin;
		comb_bin.Find_images_multiple_matches(&val_bin, *binary_to_merge);

		Potential_Shape* shape = new Potential_Shape({ receiver });
		auto shape_Dist = Get_distr_static(shape);
		K = val_bin.size();
		float val, val_temp;
		list<const I_Distribution_value*>::iterator it_c, it_c_end;
		size_t* com;
		for (k = 0; k < K; k++) {
			val = 0.f;
			it_c_end = val_bin[k].end();
			for (it_c = val_bin[k].begin(); it_c != it_c_end; it_c++) {
				val_temp = (*it_c)->Get_val() * val_sender[(*it_c)->Get_indeces()[pos_sender]];
				if (Sum_or_MAP)
					val += val_temp;
				else {
					if (val_temp > val)
						val = val_temp;
				}
			}
			com = (size_t*)malloc(sizeof(size_t));
			com[0] = k;
			shape_Dist->push_back(new Distribution_value_concrete(com, val));
		}
		return shape;

	}





	void I_belief_propagation_strategy::Gather_incoming_messages(list<Potential*>* complete_set_of_message, Node::Neighbour_connection* outgoing_mex_to_compute) {

		complete_set_of_message->clear();
		auto itC_end = outgoing_mex_to_compute->Neighbourhood.end();
		for (auto itC = outgoing_mex_to_compute->Neighbourhood.begin();
			itC != itC_end; itC++) {
#ifdef _DEBUG
			if ((*itC)->Message_to_this_node == NULL)  abort(); // found NULL incoming message, report this bug to andrecasa91@gmail.com
#endif // _DEBUG

			complete_set_of_message->push_back((*itC)->Message_to_this_node);
		}

		outgoing_mex_to_compute->This_Node->Append_temporary_permanent_Unaries(complete_set_of_message);

	}

	void I_belief_propagation_strategy::Instantiate_message(Node::Neighbour_connection* outgoing_mex_to_compute, const bool& sum_or_MAP) {

		list<Potential*> complete_set_of_message;
		this->Gather_incoming_messages(&complete_set_of_message, outgoing_mex_to_compute);

		if (complete_set_of_message.empty())
			*outgoing_mex_to_compute->Message_to_neighbour_node = new Message_Unary(outgoing_mex_to_compute->Shared_potential, outgoing_mex_to_compute->This_Node->Get_var(), sum_or_MAP);
		else
			*outgoing_mex_to_compute->Message_to_neighbour_node = new Message_Unary(outgoing_mex_to_compute->Shared_potential, complete_set_of_message, sum_or_MAP);

	}

	void I_belief_propagation_strategy::Update_message(float* variation_to_previous, Node::Neighbour_connection* outgoing_mex_to_compute, const bool& sum_or_MAP) {

		list<Potential*> complete_set_of_message;
		this->Gather_incoming_messages(&complete_set_of_message, outgoing_mex_to_compute);

		if (complete_set_of_message.empty())
			static_cast<Message_Unary*>(*outgoing_mex_to_compute->Message_to_neighbour_node)->Update(variation_to_previous, outgoing_mex_to_compute->Shared_potential, outgoing_mex_to_compute->This_Node->Get_var(), sum_or_MAP);
		else
			static_cast<Message_Unary*>(*outgoing_mex_to_compute->Message_to_neighbour_node)->Update(variation_to_previous, outgoing_mex_to_compute->Shared_potential, complete_set_of_message, sum_or_MAP);

	}

	bool I_belief_propagation_strategy::Propagate(std::list<Node*>& cluster, const bool& sum_or_MAP, const unsigned int& Iterations) {

			Messagge_Passing phase_1;
			bool infer_done = phase_1._propagate(cluster, sum_or_MAP);

			if (!infer_done) {
				Loopy_belief_propagation phase_2(Iterations);
				return phase_2._propagate(cluster, sum_or_MAP);
			}
			else return true;

	}



	bool Messagge_Passing::_propagate(std::list<Node*>& cluster, const bool& sum_or_MAP) {

		list<Node*> open_set = cluster;
		list<Node*>::iterator it_open;
		int mex_computed;
		bool temp;
		while (true) {
			mex_computed = 0;
			it_open = open_set.begin();
			while (it_open != open_set.end()) {
				this->_propagate(*it_open, &temp, &mex_computed, sum_or_MAP);

				if (temp) it_open = open_set.erase(it_open);
				else it_open++;
			}

			if (open_set.empty()) break;

			if (mex_computed == 0) return false;

		}
		return true;

	}

	void Messagge_Passing::_propagate(Node* N, bool* all_outgoing_done, int* mex_computed, const bool& sum_or_MAP) {

		*all_outgoing_done = false;

		Node::Neighbour_connection* mex_to_compute = NULL;
		auto itC = N->Get_Active_connections()->begin();
		for (itC; itC != N->Get_Active_connections()->end(); itC++) {
			if (*this->Get_Mex_to_Neigh(*itC) == NULL) {
				mex_to_compute = *itC;
				break;
			}
		}
		if (mex_to_compute == NULL) {
			*all_outgoing_done = true;
			return;
		}

		//check that in set of other mex all data was computed
		for (itC = this->Get_Neighbourhood(mex_to_compute)->begin();
			itC != this->Get_Neighbourhood(mex_to_compute)->end(); itC++) {
			if (*this->Get_Mex_to_This(*itC) == NULL) {
				return;
			}
		}

		this->Instantiate_message(mex_to_compute, sum_or_MAP);
		(*mex_computed)++;

	}



	bool Loopy_belief_propagation::_propagate(std::list<Node*>& nodes_in_subgraph, const bool& sum_or_MAP) {

		//init message not computed
		auto itN = nodes_in_subgraph.begin();
		list<Node::Neighbour_connection*>::const_iterator it_conn;
		auto itN_end = nodes_in_subgraph.end();
		for (itN; itN != itN_end; itN++) {
			for (it_conn = (*itN)->Get_Active_connections()->begin();
				it_conn != (*itN)->Get_Active_connections()->end(); it_conn++) {
				if (*this->Get_Mex_to_This(*it_conn) == NULL)
					*this->Get_Mex_to_This(*it_conn) = new Message_Unary((*itN)->Get_var());
			}
		}

		float temp_variation;
		bool not_changed;
		for (unsigned int k = 0; k < this->Iter; k++) {
			not_changed = true;
			for (itN = nodes_in_subgraph.begin(); itN != itN_end; itN++) {
				for (it_conn = (*itN)->Get_Active_connections()->begin();
					it_conn != (*itN)->Get_Active_connections()->end(); it_conn++) {
					this->Update_message(&temp_variation, *it_conn, sum_or_MAP);

					if (temp_variation > 1e-3) not_changed = false;
				}
			}

			if (not_changed) return true;
		}
		return false;

	}

}