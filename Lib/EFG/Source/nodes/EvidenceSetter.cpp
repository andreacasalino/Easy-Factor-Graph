/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/EvidenceSetter.h>
#include <Error.h>
#include "Commons.h"

namespace EFG::nodes {
    void EvidencesSetter::setEvidences(const std::vector<std::size_t>& observations) {
        if (this->evidences.size() != observations.size()) {
            throw Error("invalid number of observations");
        }
        // check size
        auto itOb = observations.begin();
        for (auto it = this->evidences.begin(); it != this->evidences.end(); ++it) {
            if (*itOb >= it->first->size()) {
                throw Error("invalid evidence value for variable " + it->first->name());
            }
            ++itOb;
        }
        itOb = observations.begin();
        for (auto it = this->evidences.begin(); it != this->evidences.end(); ++it) {
            auto itN = this->nodes.find(it->first);
            reconnnect(itN->second);
            disconnect(itN->second, *itOb);
            ++itOb;
        }
        this->lastPropagationDone = BeliefPropagationInfo::NotDone;
    }
}
