/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <print/RangePrint.h>

std::ostream& operator<<(std::ostream& s, const EFG::categoric::Range& range) {
    EFG::categoric::Range local = range;
    local.reset();

    EFG::iterator::forEach(local, [&s](EFG::categoric::Range& local){
        s << local.get().data()[0];
        for (std::size_t k=1; k<local.get().size(); ++k) {
            s << ' ' << local.get().data()[k];
        }
        s << std::endl;
    });

    return s;
}