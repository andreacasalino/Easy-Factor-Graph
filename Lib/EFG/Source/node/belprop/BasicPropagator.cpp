/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <node/belprop/BasicPropagator.h>
#include <cmath>
#include <float.h>
#include "../NeighbourConnection.h"
#include <algorithm>
using namespace std;

namespace EFG::node::bp {

	bool BasicStrategy::operator()(std::list< std::unordered_set<Node*> >& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations) {

		Messagge_Passing MP;
		Loopy_belief_propagation LP;
		bool flag = true;
		for (auto it = cluster.begin(); it != cluster.end(); ++it) {
			if (!MP(*it, sum_or_MAP)) {
				if (!LP(*it, sum_or_MAP, max_iterations)) flag = false;
			}
		}
		return flag;

	};



	bool BasicStrategy::Messagge_Passing::operator()(std::unordered_set<Node*>& cluster, const bool& sum_or_MAP) {

		list<Node::NeighbourConnection*> open_set;
		auto it = cluster.begin();
		auto it_end = cluster.end();
		list<Node::NeighbourConnection*>::const_iterator it_a;
		for (it; it != it_end; ++it) {
			for (it_a = (*it)->GetActiveConnections()->begin(); it_a != (*it)->GetActiveConnections()->end(); ++it_a) open_set.push_back(*it_a);
		}

		bool advance_done;
		list<Node::NeighbourConnection*>::iterator it_o;
		while (!open_set.empty()) {
			advance_done = false;
			it_o = open_set.begin();
			while (it_o != open_set.end()) {
				if ((*it_o)->isOutgoingRecomputationPossible()) {
					advance_done = true;
					(*it_o)->RecomputeOutgoing(sum_or_MAP);
					it_o = open_set.erase(it_o);
				}
				else ++it_o;
			}
			if (!advance_done) return false;
		}
		return true;

	}



	bool BasicStrategy::Loopy_belief_propagation::operator()(std::unordered_set<Node*>& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations) {

		//init message not computed
		list<Node::NeighbourConnection*>  mex_to_calibrate;
		auto it_end = cluster.end();
		list<Node::NeighbourConnection*>::const_iterator itc, itc_end;
		for (auto it = cluster.begin(); it != it_end; ++it) {
			itc_end = (*it)->GetActiveConnections()->end();
			for (itc = (*it)->GetActiveConnections()->begin(); itc != itc_end; ++itc) {
				//if((*itc)->Get_Linked()->Get_IncomingMessage() == nullptr){
				(*itc)->GetLinked()->SetIncoming2Ones();
				mex_to_calibrate.push_back((*itc)->GetLinked());
				//}
			}
		}

		float max_variation, temp;
		auto recalibrate_all = [&mex_to_calibrate, &max_variation, &temp, &sum_or_MAP]() {
			std::for_each(mex_to_calibrate.begin(), mex_to_calibrate.end(), [&max_variation, &temp, &sum_or_MAP](Node::NeighbourConnection* c) {
				temp = c->RecomputeOutgoing(sum_or_MAP);
				if (temp > max_variation) max_variation = temp;
			});
		};

		list<Node::NeighbourConnection*>::iterator it_cal, it_cal_end = mex_to_calibrate.end();
		for (unsigned int k = 0; k < max_iterations; ++k) {
			max_variation = 0.f; //it's a positive quantity
			recalibrate_all();
			if (max_variation < 1e-3) return true; //very little modifications were done -> convergence reached
		}
		return false;

	}

}