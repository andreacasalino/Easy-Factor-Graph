/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TEST_MODELTEST_H
#define EFG_TEST_MODELTEST_H

#include <gtest/gtest.h>
#include <io/xml/Importer.h>

namespace EFG::test {
    io::FilePath getModelPath(const std::string& modelName, const std::string& folder = "Sample03-BeliefPropagation-B/");

    template<typename Model>
    class ModelTest
        : public ::testing::Test
        , virtual public Model {
    protected:
        void SetUp() override {
            io::xml::Importer::importFromXml(*this, getModelPath(this->getName(), this->getFolder()));
        }

        virtual std::string getName() const = 0;
        virtual inline std::string getFolder() const { return "Sample03-BeliefPropagation-B/"; };
    };
}

#endif
