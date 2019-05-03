//Author: Andrea Casalino
//mail: andrecasa91@gmail.com

#pragma once
#ifndef __CRF_BELIEF_PROPAGATION_H__
#define __CRF_BELIEF_PROPAGATION_H__

#include "Node.h"

namespace Segugio {

	class Messagge_Passing : public I_belief_propagation_strategy {
	public:
		bool _propagate(std::list<Node*>& cluster, const bool& sum_or_MAP); //return true when inference is over after the call  to this function
	private:
		void _propagate(Node* N, bool* all_outgoing_done, int* mex_computed, const bool& sum_or_MAP);
	};


	class Loopy_belief_propagation : public I_belief_propagation_strategy {
	public:
		Loopy_belief_propagation(const int& max_iter) : Iter(max_iter) {};

		bool _propagate(std::list<Node*>& cluster, const bool& sum_or_MAP); //return true when inference terminated within given iterations
	protected:
		unsigned int Iter;
	};

}

#endif