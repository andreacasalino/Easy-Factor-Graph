/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/categoric/Range.h>
#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/SimpleCorrelations.h>
using namespace EFG::categoric;
using namespace EFG::misc;
using namespace EFG::factor;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

void print_probabilities(const FactorExponential<3> &factor);

int main() {
  SAMPLE_SECTION("Factors construction", "4.1.2.1", [] {
    // create factor domain in order to have for Phi_AB(a,b) = a + 2*b (just to
    // put some numbers)
    std::vector<SparseDomainPoint<2>> domain;
    for_each_generated<const Combination<2> *>(
        EFG::categoric::Range<2>{{4, 4}}, [&](const auto *comb) {
          float val = static_cast<float>((*comb)[0] + 2 * (*comb)[1]);
          domain.emplace_back(std::make_pair(*comb, val));
        });

    auto Phi_AB =
        Factor<2>::from_sparse_domain<true>({4, 4}, std::move(domain));
    print(cout, Phi_AB) << endl;
  });

  SAMPLE_SECTION("Simple Factor query", "4.1.2.2", [] {
    std::vector<float> prob;
    float weight =
        1.5f; // you can tune this value to see how the probabilities change

    // correlating factor
    auto Psi_C = FactorExponential<3>::from_sparse_domain_gen(
        {3, 3, 3}, SimplyCorrelatedDomainGen<3>{3});
    Psi_C.trsfm.setWeight(weight);
    print_probabilities(Psi_C);

    // anti-correlating factor
    auto Psi_A = FactorExponential<3>(
        {3, 3, 3}, Intervals::from_gen(SimplyAntiCorrelatedDomainGen<3>{3}));
    Psi_A.trsfm.setWeight(weight);
    getProbabilities(Psi_A, prob);
    print_probabilities(Psi_A);
  });

  SAMPLE_SECTION("Find specific combination in Factors", "", [] {
    std::vector<SparseDomainPoint<4>> domain;
    domain.emplace_back(std::make_pair(Combination<4>{0, 0, 0, 0}, 1.f));
    domain.emplace_back(std::make_pair(Combination<4>{0, 0, 1, 0}, 2.f));
    domain.emplace_back(std::make_pair(Combination<4>{1, 0, 1, 1}, 3.f));
    auto factor =
        Factor<4>::from_sparse_domain<true>({2, 2, 3, 3}, std::move(domain));

    cout << "current content of the distribution" << endl;
    print(cout, factor) << endl << endl;

    auto val = factor.get<true>(Combination<4>{0, 0, 1, 0});
    cout << "value found for 0 0 1 0  ->  " << val << endl;
  });

  return EXIT_SUCCESS;
}

void print_probabilities(const FactorExponential<3> &factor) {
  std::vector<float> prob;
  getProbabilities(factor, prob);
  Combination<3> sizes{factor.getVariables().sizes()};
  EFG::categoric::Range<3> rng{std::move(sizes)};
  Tabular table;
  table.addLine(Tabular::Line{"combination", "probability"});
  for_each_generated<const Combination<3> *>(
      rng, [&, idx = std::size_t{0}](const auto *comb) mutable {
        std::string comb_str{"["};
        comb_str += std::to_string(comb->front());
        std::for_each(comb->begin() + 1, comb->end(), [&](auto val) {
          comb_str.push_back(' ');
          comb_str += std::to_string(val);
        });
        comb_str.push_back(']');
        table.addLine(Tabular::Line{std::move(comb_str), prob[idx++]});
      });
  table.print(cout);
  cout << endl;
}
