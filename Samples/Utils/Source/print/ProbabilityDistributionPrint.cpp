/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <print/ProbabilityDistributionPrint.h>

std::ostream& operator<<(std::ostream& s, const std::vector<float>& distribution) {
    if (distribution.empty()) {
        return s;
    }
    auto it = distribution.begin();
    s << *it;
    ++it;
    for (it; it != distribution.end(); ++it) {
        s << " " << *it;
    }
    return s;
}
