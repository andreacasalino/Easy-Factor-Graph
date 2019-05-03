#include "../Header/Belief_propagation.h"
using namespace std;

namespace Segugio {

	void I_belief_propagation_strategy::Gather_incoming_messages(list<Potential*>* complete_set_of_message, Node::Neighbour_connection* outgoing_mex_to_compute) {

		complete_set_of_message->clear();
		for (auto itC = outgoing_mex_to_compute->Neighbourhood.begin();
			itC != outgoing_mex_to_compute->Neighbourhood.end(); itC++) {
#ifdef _DEBUG
			if ((*itC)->Message_to_this_node == NULL) {
				system("ECHO found NULL incoming message");
				abort();
			}
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
			(*outgoing_mex_to_compute->Message_to_neighbour_node)->Update(variation_to_previous, outgoing_mex_to_compute->Shared_potential, outgoing_mex_to_compute->This_Node->Get_var(), sum_or_MAP);
		else
			(*outgoing_mex_to_compute->Message_to_neighbour_node)->Update(variation_to_previous, outgoing_mex_to_compute->Shared_potential, complete_set_of_message, sum_or_MAP);

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
		for (itN; itN != nodes_in_subgraph.end(); itN++) {
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
			for (itN = nodes_in_subgraph.begin(); itN != nodes_in_subgraph.end(); itN++) {
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