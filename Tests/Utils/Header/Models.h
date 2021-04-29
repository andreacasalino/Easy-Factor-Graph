// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #ifndef EFG_TEST_COMPLEX_POLITREE_H
// #define EFG_TEST_COMPLEX_POLITREE_H

// #include <gtest/gtest.h>
// #include <io/xml/Importer.h>

// namespace EFG::test {
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
// }

// #endif
