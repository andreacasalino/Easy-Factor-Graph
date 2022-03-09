// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #include <DistributionSamples.h>
// #include <categoric/Range.h>

// namespace EFG::test {
//     std::vector<categoric::Combination> getCombinations(const
//     categoric::Group& group) {
//         std::vector<categoric::Combination> combinations;
//         combinations.reserve(group.size());
//         categoric::Range range(group.getVariables());
//         iterator::forEach(range, [&combinations](const categoric::Range&
//         range){
//             combinations.emplace_back(range.get());
//         });
//         return combinations;
//     }
//     FactorTest::FactorTest(const std::set<categoric::VariablePtr>& variables)
//         : distribution::factor::cnst::Factor(variables)
//         , combinations(getCombinations(*this->group)) {
//     }

//     FactorUnaryTest::FactorUnaryTest()
//         : FactorTest({categoric::makeVariable(3, "A")}) {
//         this->values->emplace(combinations[0], 1.f);
//         this->values->emplace(combinations[2], 2.f);
//     };

//     FactorBinaryTest::FactorBinaryTest()
//         : FactorTest({categoric::makeVariable(3, "A"),
//         categoric::makeVariable(4, "B")}) {
//         this->values->emplace(combinations[0], 1.f);
//         this->values->emplace(combinations[2], 2.f);
//         this->values->emplace(combinations[4], 3.f);
//     };

//     FactorTernaryTest::FactorTernaryTest()
//         : FactorTest({categoric::makeVariable(3, "A"),
//         categoric::makeVariable(4, "B"), categoric::makeVariable(2, "C")}) {
//         this->values->emplace(combinations[0], 1.f);
//         this->values->emplace(combinations[2], 2.f);
//         this->values->emplace(combinations[4], 3.f);
//         this->values->emplace(combinations[6], 1.5f);
//     };
// }
