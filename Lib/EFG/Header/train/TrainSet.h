/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAIN_TRAINSET_H
#define EFG_TRAIN_TRAINSET_H

#include <Combination.h>
#include <string>
#include <memory>

namespace EFG::train {
    class TrainSet {
    public:
        TrainSet(const std::string& fileName);
        // check that all combinations have same size
        TrainSet(const std::vector<Combination>& combinations);

        inline const std::vector<Combination>& getSet() const { return this->combinations; };

    private:
        const std::vector<Combination> combinations;
    };

    typedef std::shared_ptr<TrainSet> TrainSetPtr;
}

#endif
