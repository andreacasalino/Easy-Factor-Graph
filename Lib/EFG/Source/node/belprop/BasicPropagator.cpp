/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include "BasicPropagator.h"
#include <cmath>
#include <float.h>
#include "../NeighbourConnection.h"
#include <algorithm>
#include <set>
using namespace std;

namespace EFG::node::bp {

	bool BasicStrategy::operator()(std::list<std::unordered_set<EFG::node::Node*>>& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations 
        #ifdef THREAD_POOL_ENABLED
        ,thpl::equi::Pool* pl
        #endif
        ) {

		bool flag = true;
		for (auto it = cluster.begin(); it != cluster.end(); ++it) {
			if (!this->MessagePassing(*it, sum_or_MAP 
			#ifdef THREAD_POOL_ENABLED
        	,pl
        	#endif
			)) {
				if (!this->LoopyPropagation(*it, sum_or_MAP, max_iterations 
				#ifdef THREAD_POOL_ENABLED
				,pl
				#endif
				)) flag = false;
			}
		}
		return flag;
	};



	bool BasicStrategy::MessagePassing(std::unordered_set<EFG::node::Node*>& cluster, const bool& sum_or_MAP 
        #ifdef THREAD_POOL_ENABLED
        ,thpl::equi::Pool* pool
        #endif
        ) {
		list<Node::NeighbourConnection*> open_set;
		auto it = cluster.begin();
		auto it_end = cluster.end();
		list<Node::NeighbourConnection*>::const_iterator it_a;
		for (it; it != it_end; ++it) {
			for (it_a = (*it)->GetActiveConnections()->begin(); it_a != (*it)->GetActiveConnections()->end(); ++it_a) open_set.push_back(*it_a);
		}

		bool advance_done;
		list<Node::NeighbourConnection*>::iterator it_o;
        #ifdef THREAD_POOL_ENABLED
		if (pool == nullptr) {
		#endif
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
        #ifdef THREAD_POOL_ENABLED
		}
		else {
			while (!open_set.empty()) {
				advance_done = false;
				it_o = open_set.begin();
				while (it_o != open_set.end()) {
					if ((*it_o)->isOutgoingRecomputationPossible()) {
						advance_done = true;
						Node::NeighbourConnection* temp = *it_o;
						pool->push([temp, &sum_or_MAP]() { temp->RecomputeOutgoing(sum_or_MAP); });
						it_o = open_set.erase(it_o);
					}
					else ++it_o;
				}
				pool->wait();
				if (!advance_done) return false;
			}
		}
		#endif
		return true;
	}



	bool BasicStrategy::LoopyPropagation(std::unordered_set<EFG::node::Node*>& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations
        #ifdef THREAD_POOL_ENABLED
        ,thpl::equi::Pool* pool
        #endif
        ) {
		//init message not computed
		list<Node::NeighbourConnection*>  mex_to_calibrate;
		auto it_end = cluster.end();
		list<Node::NeighbourConnection*>::const_iterator itc, itc_end;
		for (auto it = cluster.begin(); it != it_end; ++it) {
			itc_end = (*it)->GetActiveConnections()->end();
			for (itc = (*it)->GetActiveConnections()->begin(); itc != itc_end; ++itc) {
				(*itc)->SetIncoming2Ones();
				mex_to_calibrate.push_back(*itc);
			}
		}

        #ifdef THREAD_POOL_ENABLED
		if (pool == nullptr) {
		#endif
			float max_variation, temp;
			for (unsigned int k = 0; k < max_iterations; ++k) {
				max_variation = 0.f; //it's a positive quantity
				std::for_each(mex_to_calibrate.begin(), mex_to_calibrate.end(), [&max_variation, &temp, &sum_or_MAP](Node::NeighbourConnection* c) {
					temp = c->RecomputeOutgoing(sum_or_MAP);
					if (temp > max_variation) max_variation = temp;
				});
				if (max_variation < 1e-3) return true; //very little modifications were done -> convergence reached
			}
        #ifdef THREAD_POOL_ENABLED
		}
		else {
			float max_variation;
			list<Node::NeighbourConnection*> mex_remaining;
			set<Node::NeighbourConnection*> mex_to_recompute, mex_locked;
			bool recomputation_possible;
			for (unsigned int k = 0; k < max_iterations; ++k) {
				max_variation = 0.f; //it's a positive quantity
				mex_remaining = mex_to_calibrate;
				//recompute all the messages
				while (!mex_remaining.empty()) {
					mex_locked.clear();
					mex_to_recompute.clear();
					auto rr = mex_remaining.begin();
					while (rr != mex_remaining.end()) {
						// check that the message to recompute is not locked
						if (mex_locked.find(*rr) == mex_locked.end()) {
							//check that none of the element in the neighbourhood will be recomputed
							recomputation_possible = true;
							auto required = (*rr)->GetLinked()->GetNeighbourhood();
							for (auto nn = required->begin(); nn != required->end(); ++nn) {
								if (mex_to_recompute.find(*nn) != mex_to_recompute.end()) {
									recomputation_possible = false;
									break;
								}
							}
							if (recomputation_possible) {
								mex_to_recompute.emplace(*rr);
								// add the neighbourhood to the locked group
								for (auto nn = required->begin(); nn != required->end(); ++nn) mex_locked.emplace(*nn);
								Node::NeighbourConnection* c = *rr;
								rr = mex_remaining.erase(rr);
								pool->push([&max_variation, &sum_or_MAP, c]() {
									float temp = c->GetLinked()->RecomputeOutgoing(sum_or_MAP);
									if (temp > max_variation) max_variation = temp;
								});
							}
							else ++rr;
						}
						else ++rr;
					}
					pool->wait();
				}
				if (max_variation < 1e-3) return true; //very little modifications were done -> convergence reached
			}
		}
		#endif
		return false;
	}

}