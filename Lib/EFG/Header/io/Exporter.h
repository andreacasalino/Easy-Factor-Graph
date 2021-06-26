/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_IO_EXPORTER_H
#define EFG_IO_EXPORTER_H

#include <structure/components/EvidenceAware.h>
#include <structure/components/StructureAware.h>
#include <structure/components/StructureTunableAware.h>
#include <Error.h>

namespace EFG::io {
    class Exporter {
    protected:
        Exporter() = default;

        template<typename Model>
        static std::tuple<const strct::EvidenceAware*, const strct::StructureAware*, const strct::StructureTunableAware*> getComponents(const Model& model) {
            const strct::EvidenceAware* evidences = dynamic_cast<const strct::EvidenceAware*>(&model);
            if (nullptr == evidences) {
                throw Error("the model should be evidence aware");
            }

            const strct::StructureAware* structure = dynamic_cast<const strct::StructureAware*>(&model);
            if (nullptr == structure) {
                throw Error("the model should be at least structure aware");
            }
            return std::make_tuple(evidences, structure, dynamic_cast<const strct::StructureTunableAware*>(&model));
        };

        virtual void exportComponents(const std::string& filePath, const std::string& modelName, const std::tuple<const strct::EvidenceAware*, const strct::StructureAware*, const strct::StructureTunableAware*>& components) = 0;
    };
}

#endif
