/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_MODEL_GRAPH_H
#define EFG_MODEL_GRAPH_H

#include <structure/BeliefPropagator.h>
#include <structure/EvidenceChanger.h>
#include <structure/EvidenceSetter.h>
#include <structure/GibbsSampler.h>
#include <structure/InsertCapable.h>
#include <structure/QueryHandler.h>

namespace EFG::model {
    /**
     * @brief A simple graph object, that can't store tunable factors.
     */
    class Graph
        : public strct::BeliefPropagator
        , public strct::EvidencesChanger
        , public strct::EvidencesSetter
        , public strct::GibbsSampler
        , public strct::InsertCapable
        , public strct::QueryHandler {
    public:
        Graph() = default;

        template<typename Model>
        explicit Graph(const Model& o) {
            this->absorbModel(o);
        };

        Graph(const Graph& o) {
            this->absorbModel(o, true);
        };
    };
}

#endif
