/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <ModelTest.h>

namespace EFG::test {
    io::FilePath getModelPath(const std::string& modelName, const std::string& folder) {
        return io::FilePath(std::string(SAMPLE_FOLDER) + folder, modelName);
    };
}
