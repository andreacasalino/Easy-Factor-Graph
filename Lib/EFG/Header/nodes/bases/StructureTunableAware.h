/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_STRUCTURE_TUNABLE_AWARE_H
#define EFG_NODES_STRUCTURE_TUNABLE_AWARE_H

#include <nodes/bases/StructureAware.h>

namespace EFG::nodes {
    class StructureTunableAware : virtual public StructureAware {
    public:
        inline const std::vector<std::vector<std::shared_ptr<distribution::factor::cnst::FactorExponential>>>& getFactorsTunable() const { return this->factorstunable; };

    protected:
        std::vector<std::vector<std::shared_ptr<distribution::factor::cnst::FactorExponential>>> factorstunable;
    };
}

#endif
