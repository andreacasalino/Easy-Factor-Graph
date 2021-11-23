/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_IO_IMPORTER_H
#define EFG_IO_IMPORTER_H

#include <structure/InsertCapable.h>
#include <structure/InsertTunableCapable.h>
#include <Error.h>

namespace EFG::io {
    class Importer {
    protected:
        Importer() = default;

        template<typename Model>
        static std::pair<strct::InsertCapable*, strct::InsertTunableCapable*> getComponents(Model& model) {
            strct::InsertCapable* structure = dynamic_cast<strct::InsertCapable*>(&model);
            if (nullptr == structure) {
                throw Error("the model should be at least structure insert capable");
            }
            return std::make_pair(structure, dynamic_cast<strct::InsertTunableCapable*>(&model));
        };

        virtual std::map<std::string, std::size_t> importComponents(const std::string& filePath, const std::string& fileName, const std::pair<strct::InsertCapable*, strct::InsertTunableCapable*>& components) = 0;
    };
}

#endif
