/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_MODEL_GRAPH_H
#define EFG_MODEL_GRAPH_H

#include <nodes/BeliefPropagator.h>
#include <nodes/EvidenceChanger.h>
#include <nodes/EvidenceSetter.h>
#include <nodes/GibbsSampler.h>
#include <nodes/InsertCapable.h>
#include <nodes/QueryHandler.h>

namespace EFG::model {
    class Graph
        : public nodes::BeliefPropagator
        , public nodes::EvidencesChanger
        , public nodes::EvidencesSetter
        , public nodes::GibbsSampler
        , public nodes::InsertCapable
        , public nodes::QueryHandler {
    public:
        Graph() = default;

        template<typename Model>
        Graph(const Model& o) {
            this->absorbModel(o);
        };

        Graph(const Graph& o) {
            this->absorbModel(o, true);
        };
    };
}

#endif
