/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_DISTRIBUTION_INSTANTIABLE_H
#define EFG_DISTRIBUTION_DISTRIBUTION_INSTANTIABLE_H

#include <distribution/Distribution.h>

namespace EFG::distribution {
    class DistributionInstantiable : public virtual Distribution {
    protected:
        DistributionInstantiable(const categoric::Group& group, EvaluatorPtr evaluator);

        DistributionInstantiable(const DistributionInstantiable& o);
        DistributionInstantiable& operator=(const DistributionInstantiable& o);

        DistributionInstantiable(DistributionInstantiable&& o);
        DistributionInstantiable& operator=(DistributionInstantiable&& o);
    };
}

#endif
