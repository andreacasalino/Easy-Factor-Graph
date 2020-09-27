/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_BELIEF_H__
#define __EFG_BELIEF_H__

#include <Node.h>

namespace EFG {

    class Basic_strategy : public I_belief_propagation_strategy {
    public:
        bool operator()(std::list< std::unordered_set<Node*, std::function<size_t(const Node*)> > >& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations) override;
		std::unique_ptr<I_belief_propagation_strategy>	copy() const override { return std::unique_ptr<I_belief_propagation_strategy>(new Basic_strategy()); };
    protected:
        struct Messagge_Passing {
            bool operator()(std::unordered_set<Node*, std::function<size_t(const Node*)> >& cluster, const bool& sum_or_MAP); //return true when propagation is over after the call  to this function
        };

        struct  Loopy_belief_propagation {
            bool operator()(std::unordered_set<Node*, std::function<size_t(const Node*)> >& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations); //return true when propagation terminated within given iterations
        };
    };

}


#endif