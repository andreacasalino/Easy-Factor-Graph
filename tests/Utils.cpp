#include "Utils.h"

#include <math.h>
#include <sstream>

namespace EFG::test {
distribution::Factor make_corr_factor(const categoric::VariablePtr &first,
                                      const categoric::VariablePtr &second) {
  return distribution::Factor(categoric::Group{first, second},
                              distribution::USE_SIMPLE_CORRELATION_TAG);
}

std::shared_ptr<distribution::Factor>
make_corr_factor2(const categoric::VariablePtr &first,
                  const categoric::VariablePtr &second) {
  return std::make_shared<distribution::Factor>(
      categoric::Group{first, second},
      distribution::USE_SIMPLE_CORRELATION_TAG);
}

distribution::FactorExponential
make_corr_expfactor(const categoric::VariablePtr &first,
                    const categoric::VariablePtr &second, const float w) {
  auto factor = make_corr_factor(first, second);
  return distribution::FactorExponential(factor, w);
}

std::shared_ptr<distribution::FactorExponential>
make_corr_expfactor2(const categoric::VariablePtr &first,
                     const categoric::VariablePtr &second, const float w) {
  auto factor = make_corr_factor(first, second);
  return std::make_shared<distribution::FactorExponential>(factor, w);
}

std::string make_graph_path(const std::string &file_name) {
  std::stringstream stream;
  stream << SAMPLE_FOLDER << "Sample03-BeliefPropagation-B/" << file_name;
  return stream.str();
}

bool almost_equal(const float a, const float b, const float tollerance) {
  return fabs(a - b) < tollerance;
}

bool almost_equal(const std::vector<float> &a, const std::vector<float> &b,
                  const float tollerance) {
  if (a.size() != b.size()) {
    return false;
  }
  for (std::size_t k = 0; k < a.size(); ++k) {
    if (!almost_equal(a[k], b[k], tollerance)) {
      return false;
    }
  }
  return true;
}

namespace {
std::vector<float> make_normalized(const std::vector<float> &values) {
  float coeff = 0;
  for (const auto &val : values) {
    coeff += val;
  }
  coeff = 1.f / coeff;
  std::vector<float> result = values;
  for (auto &val : result) {
    val *= coeff;
  }
  return result;
}
} // namespace

ProbDistribution::ProbDistribution(const std::vector<float> &values)
    : values_normalized(make_normalized(values)) {}
} // namespace EFG::test
