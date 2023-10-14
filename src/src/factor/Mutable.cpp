/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/factor/Mutable.h>

namespace EFG::factor {
Mutable::Mutable(FunctionPtr data) : function_{data} {
  if (data == nullptr) {
    throw Error{"null data"};
  }
}

void Mutable::set(const std::vector<std::size_t> &comb, float value) {
  if (value < 0.f) {
    throw Error("negative value is not possible");
  }
  const auto &vars = function_->vars().getVariables();
  if (comb.size() != vars.size()) {
    throw Error{"Invalid combination"};
  }
  for (std::size_t k = 0; k < comb.size(); ++k) {
    if (vars[k]->size() <= comb[k]) {
      throw Error{"Invalid combination"};
    }
  }
  function_->set(comb, value);
}
} // namespace EFG::factor
