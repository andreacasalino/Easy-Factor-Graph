/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <print/DistributionPrint.h>
#include <distribution/DistributionIterator.h>
#include <print/GroupPrint.h>

void printValue(std::ostream& s, const EFG::distribution::DistributionIterator& iter) {
    const std::size_t* data = iter.getCombination().data();
    s << data[0];
    for(std::size_t k=1; k<iter.getCombination().size(); ++k) {
        s << ' ';
        s << data[k];
    }
    s << " -> ";
    s << iter.getImage();
    s << std::endl;
}

std::ostream& operator<<(std::ostream& s, const EFG::distribution::Distribution& distribution) {
    s << distribution.getGroup() << std::endl;

    auto it = distribution.getIterator();
    EFG::iterator::forEach(it, [&s](const EFG::distribution::DistributionIterator& it) {
        printValue(s, it);
    });

    return s;
}