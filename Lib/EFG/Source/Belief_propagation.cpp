/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <Belief_propagation.h>
#include <cmath>
#include <float.h>
using namespace std;

namespace EFG {

	bool Basic_strategy::operator()(std::list< std::unordered_set<Node*, std::function<size_t(const Node*)> > >& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations){

		Messagge_Passing MP;
		Loopy_belief_propagation LP;
		bool flag = true;
		for(auto it = cluster.begin(); it!=cluster.end(); ++it){
			if(!MP(*it, sum_or_MAP)) {
				if(!LP(*it , sum_or_MAP, max_iterations)) flag = false;
			}
		}
		return flag;

	};



	bool Basic_strategy::Messagge_Passing::operator()(std::unordered_set<Node*, std::function<size_t(const Node*)> >& cluster, const bool& sum_or_MAP) {

		list<Node::Neighbour_connection*> open_set;
		auto it = cluster.begin();
		auto it_end = cluster.end();
		list<Node::Neighbour_connection*>::const_iterator it_a;
		for(it; it!=it_end; ++it){
			for(it_a= (*it)->Get_Active_connections()->begin(); it_a != (*it)->Get_Active_connections()->end(); ++it_a) open_set.push_back(*it_a);
		} 

		bool advance_done;
		list<Node::Neighbour_connection*>::iterator it_o;
		while(!open_set.empty()){
			advance_done = false;
			it_o = open_set.begin();
			while(it_o != open_set.end()){
				if((*it_o)->Is_OutgoingMessage_Recomputation_possible()){
					advance_done = true;
					(*it_o)->Recompute_OutgoingMessage(sum_or_MAP);
					it_o = open_set.erase(it_o);
				}
				else ++it_o;
			}			
			if(!advance_done) return false;
		}
		return true;

	}



	bool Basic_strategy::Loopy_belief_propagation::operator()(std::unordered_set<Node*, std::function<size_t(const Node*)> >& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations) {

	//init message not computed
		list<Node::Neighbour_connection*>  mex_to_calibrate;
		auto it_end =cluster.end();
		list<Node::Neighbour_connection*>::const_iterator itc, itc_end;
		for(auto it=cluster.begin(); it!=it_end; ++it){
			itc_end = (*it)->Get_Active_connections()->end();
			for(itc = (*it)->Get_Active_connections()->begin(); itc!=itc_end; ++itc) {
				//if((*itc)->Get_Linked()->Get_IncomingMessage() == nullptr){
					(*itc)->Get_Linked()->Set_IncomingMessage_ones();
					mex_to_calibrate.push_back((*itc)->Get_Linked());
				//}
			}
		}

		float max_variation, temp;
		list<Node::Neighbour_connection*>::iterator it_cal, it_cal_end = mex_to_calibrate.end();
		for(unsigned int k=0; k<max_iterations; ++k){
			max_variation = 0.f; //it's a positive quantity
			for(it_cal = mex_to_calibrate.begin(); it_cal!=it_cal_end; ++it_cal){
				temp = (*it_cal)->Recompute_OutgoingMessage(sum_or_MAP);
				if(temp > max_variation) max_variation = temp;
			}
			if(max_variation < 1e-3) return true; //very little modifications were done -> convergence reached
		}
		return false;

	}

}