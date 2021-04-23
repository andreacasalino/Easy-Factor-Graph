/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_IO_XML_IMPORTER_H
#define EFG_IO_XML_IMPORTER_H

#include <io/Importer.h>

namespace EFG::io::xml {
    class Importer : public io::Importer {
    public:
        template<typename Model>
        static std::map<std::string, std::size_t> importFromXml(Model& model, const std::string& filePath, const std::string& fileName) {
            return Importer().importComponents(filePath, fileName, getComponents(model));
        };

    private:
        std::map<std::string, std::size_t> importComponents(const std::string& filePath, const std::string& fileName, const std::pair<nodes::InsertCapable*, nodes::InsertTunableCapable*>& components) final;
    };
}

#endif
