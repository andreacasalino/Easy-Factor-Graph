/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_IO_EXPORTER_H
#define EFG_IO_EXPORTER_H

#include <nodes/bases/EvidenceAware.h>
#include <nodes/bases/StructureAware.h>
#include <nodes/bases/StructureTunableAware.h>
#include <Error.h>

namespace EFG::io {
    class Exporter {
    protected:
        Exporter() = default;

        template<typename Model>
        static std::tuple<const nodes::EvidenceAware*, const nodes::StructureAware*, const nodes::StructureTunableAware*> getComponents(const Model& model) {
            const nodes::EvidenceAware* evidences = dynamic_cast<const nodes::EvidenceAware*>(&model);
            if (nullptr == evidences) {
                throw Error("the model should be evidence aware");
            }

            const nodes::StructureAware* structure = dynamic_cast<const nodes::StructureAware*>(&model);
            if (nullptr == structure) {
                throw Error("the model should be at least structure aware");
            }
            return std::make_tuple(evidences, structure, dynamic_cast<const nodes::StructureTunableAware*>(&model));
        };

        virtual void exportComponents(const std::string& filePath, const std::string& modelName, const std::tuple<const nodes::EvidenceAware*, const nodes::StructureAware*, const nodes::StructureTunableAware*>& components) = 0;
    };
}

#endif
