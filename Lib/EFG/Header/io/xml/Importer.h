/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_IO_XML_IMPORTER_H
#define EFG_IO_XML_IMPORTER_H

#include <io/Importer.h>
#include <io/FilePath.h>

namespace EFG::io::xml {
    class Importer : public io::Importer {
    public:
        /**
         * @brief imports the structure (variables and factors) described in an xml file and add it to the passed model
         * @param the model receiving the parsed data
         * @param the path storing the xml to import
         * @return the set of evidences red from the file. Attention! this quantities are returned to allow the user
         * to set such evidence, since this is not automatically done when importing
         */
        template<typename Model>
        static std::map<std::string, std::size_t> importFromXml(Model& model, const FilePath& filePath) {
            return Importer().importComponents(filePath.getPath(), filePath.getFileName(), getComponents(model));
        };

    private:
        std::map<std::string, std::size_t> importComponents(const std::string& filePath, const std::string& fileName, const std::pair<nodes::InsertCapable*, nodes::InsertTunableCapable*>& components) final;
    };
}

#endif
