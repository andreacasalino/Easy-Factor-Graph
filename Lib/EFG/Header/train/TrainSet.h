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
    typedef std::shared_ptr<Combination> CombinationPtr;

    class TrainSet {
    public:
        // check that all combinations have same size
        TrainSet(const std::vector<Combination>& combinations);
        TrainSet(const std::string& fileName);

        TrainSet getRandomSubSet(const float& percentage) const;

        inline const std::vector<CombinationPtr>& getSet() const { return this->combinations; };

    private:
        TrainSet(const std::vector<CombinationPtr>& combinations);

        const std::vector<CombinationPtr> combinations;
    };

    typedef std::shared_ptr<TrainSet> TrainSetPtr;
}

#endif
