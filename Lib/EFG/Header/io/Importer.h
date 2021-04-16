/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_IO_IMPORTER_H
#define EFG_IO_IMPORTER_H

#include <nodes/InsertCapable.h>
#include <nodes/InsertTunableCapable.h>
#include <Error.h>

namespace EFG::io {
    class Importer {
    protected:
        Importer() = default;

        template<typename Model>
        static std::pair<nodes::InsertCapable*, nodes::InsertTunableCapable*> getComponents(Model& model) {
            nodes::InsertCapable* structure = dynamic_cast<nodes::InsertCapable*>(&model);
            if (nullptr == structure) {
                throw Error("the model should be at least structure insert capable");
            }
            return std::make_pair(structure, dynamic_cast<nodes::InsertTunableCapable*>(&model));
        };

        virtual void importComponents(const std::string& filePath, const std::string& fileName, const std::pair<nodes::InsertCapable*, nodes::InsertTunableCapable*>& components) = 0;
    };
}

#endif
