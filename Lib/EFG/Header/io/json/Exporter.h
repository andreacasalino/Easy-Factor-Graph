/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_IO_JSON_EXPORTER_H
#define EFG_IO_JSON_EXPORTER_H

#include <io/Exporter.h>

namespace EFG::io::json {
    class Exporter : public io::Exporter {
    public:
        template<typename Model>
        static void exportToJson(const Model& model, const std::string& filePath, const std::string& modelName = "") {
            Exporter().exportComponents(filePath, modelName, getComponents(model));
        };

    private:
        void exportComponents(const std::string& filePath, const std::string& modelName, const std::tuple<const nodes::EvidenceAware*, const nodes::StructureAware*, const nodes::StructureTunableAware*>& components) final;
    };
}

#endif
