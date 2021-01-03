/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_BASIC_PROP_H__
#define __EFG_BASIC_PROP_H__

#include "Propagator.h"

namespace EFG::node::bp {

    class BasicStrategy : public BeliefPropagator {
    public:
        bool operator()(std::list<std::unordered_set<EFG::node::Node*>>& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations 
        #ifdef THREAD_POOL_ENABLED
        ,thpl::equi::Pool* pl
        #endif
        ) override;
        std::unique_ptr<BeliefPropagator>	copy() const override { return std::make_unique<BasicStrategy>(); };
    protected:
        bool MessagePassing(std::unordered_set<EFG::node::Node*>& cluster, const bool& sum_or_MAP
        #ifdef THREAD_POOL_ENABLED
        ,thpl::equi::Pool* pl
        #endif
        );//return true when propagation terminated computing all the messages
        bool LoopyPropagation(std::unordered_set<EFG::node::Node*>& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations
        #ifdef THREAD_POOL_ENABLED
        ,thpl::equi::Pool* pl
        #endif
        ); //return true when propagation terminated within given iterations
    };

}

#endif