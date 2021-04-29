// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #ifndef EFG_TEST_MODELS_H
// #define EFG_TEST_MODELS_H

// #include <gtest/gtest.h>
// #include <io/xml/Importer.h>

// namespace EFG::test {
//     template<typename Model>
//     class SimplePolitree 
//         : public ::testing::Test
//         , public Model { 
//     public: 
//         SimplePolitree(); 

//         void SetUp() override {
//             // xml::Importer::importFromXml(*this, std::string(TEST_FOLDER) + std::string("../Samples/Sample03-BeliefPropagation-B/"), "graph_2.xml");
//         };
//     };

//     template<typename Model>
//     class ComplexPolitree 
//         : public ::testing::Test
//         , public Model { 
//     public: 
//         ComplexPolitree(); 

//         void SetUp() override {
//             xml::Importer::importFromXml(*this, std::string(TEST_FOLDER) + std::string("../Samples/Sample03-BeliefPropagation-B/"), "graph_2.xml");
//         };
//     };

//     template<typename Model>
//     class SimpleLoopy 
//         : public ::testing::Test
//         , public Model { 
//     public: 
//         SimpleLoopy(); 

//         void SetUp() override {
//             // xml::Importer::importFromXml(*this, std::string(TEST_FOLDER) + std::string("../Samples/Sample03-BeliefPropagation-B/"), "graph_2.xml");
//         };
//     };

//     template<typename Model>
//     class ComplexLoopy 
//         : public ::testing::Test
//         , public Model { 
//     public: 
//         ComplexLoopy(); 

//         void SetUp() override {
//             xml::Importer::importFromXml(*this, std::string(TEST_FOLDER) + std::string("../Samples/Sample03-BeliefPropagation-B/"), "graph_2.xml");
//         };
//     };

//     class TunableAndNotModel 
//         : public ::testing::Test
//         , public Model { 
//     public: 
//         ComplexLoopy(); 

//         void SetUp() override {
//         };
//     };

//     class SharedWeightsModel 
//         : public ::testing::Test
//         , public Model { 
//     public: 
//         ComplexLoopy(); 

//         void SetUp() override {
//         };
//     };
// }

// #endif
