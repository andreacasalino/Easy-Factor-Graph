/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_BASIC_PROP_H__
#define __EFG_BASIC_PROP_H__

#include <node/belprop/BeliefPropagator.h>

namespace EFG::node::bp {

    class BasicStrategy : public BeliefPropagator {
    public:
        bool operator()(std::list< std::unordered_set<Node*> >& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations) override;
        std::unique_ptr<BeliefPropagator>	copy() const override { return std::make_unique<BasicStrategy>(); };
    protected:
        struct Messagge_Passing {
            bool operator()(std::unordered_set<Node*>& cluster, const bool& sum_or_MAP); //return true when propagation is over after the call  to this function
        };

        struct  Loopy_belief_propagation {
            bool operator()(std::unordered_set<Node*>& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations); //return true when propagation terminated within given iterations
        };
    };

}

#endif