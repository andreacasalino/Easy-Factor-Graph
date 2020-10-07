/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_BELIEF_PROP_H__
#define __EFG_BELIEF_PROP_H__

#include <node/Node.h>
#include <list>
#include <util/univocal_map.h>
#include <unordered_set>

namespace EFG::node::bp {

    class BeliefPropagator {
    public:
        virtual ~BeliefPropagator() = default;

        virtual bool operator()(std::list< std::unordered_set<Node*> >& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations) = 0; //returns true in case the propagation has completely terminated, i.e. all messagges have been computed

        virtual std::unique_ptr<BeliefPropagator>	copy() const = 0;
    };

}

#endif