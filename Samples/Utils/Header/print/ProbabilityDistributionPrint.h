/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_SAMPLE_PRINTER_PROBABILITY_DISTRIBUTION_H
#define EFG_SAMPLE_PRINTER_PROBABILITY_DISTRIBUTION_H

#include <vector>
#include <ostream>

template<typename T>
std::ostream& operator<<(std::ostream& s, const std::vector<T>& distribution) {
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

#endif
