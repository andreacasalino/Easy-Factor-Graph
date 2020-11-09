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
        bool operator()(std::list<std::unordered_set<EFG::node::Node*>>& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations, thpl::equi::Pool* pl) override;
        std::unique_ptr<BeliefPropagator>	copy() const override { return std::make_unique<BasicStrategy>(); };
    protected:
        bool MessagePassing(thpl::equi::Pool* pool, std::unordered_set<EFG::node::Node*>& cluster, const bool& sum_or_MAP);//return true when propagation terminated computing all the messages
        bool LoopyPropagation(thpl::equi::Pool* pool, std::unordered_set<EFG::node::Node*>& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations); //return true when propagation terminated within given iterations
    };

}

#endif