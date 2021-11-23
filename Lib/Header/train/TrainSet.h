/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAIN_TRAINSET_H
#define EFG_TRAIN_TRAINSET_H

#include <categoric/Combination.h>
#include <string>
#include <memory>

namespace EFG::train {
    typedef std::shared_ptr<categoric::Combination> CombinationPtr;

    class TrainSet {
    public:
        /**
         * @param the set of combinations that will be part of the train set.
         * @throw if the combinations don't have all the same size
         * @throw if the combinations container is empty
         */
        explicit TrainSet(const std::vector<categoric::Combination>& combinations);

        /**
         * @param import the combinations from a textual file where each row represent a combination
         * @throw if the file is not readable
         * @throw if the parsed combinations don't have all the same size
         * @throw if the file is empty
         */
        explicit TrainSet(const std::string& fileName);

        /**
         * @return a TrainSet containg some of the combinations stored into this object. 
         * The combination to take are randomly decided.
         * @param the percentage of combinations to extract from this object.
         */
        TrainSet getRandomSubSet(const float& percentage) const;

        inline const std::vector<CombinationPtr>& getSet() const { return this->combinations; };

    private:
        TrainSet(const std::vector<CombinationPtr>& combinations);

        const std::vector<CombinationPtr> combinations;
    };

    typedef std::shared_ptr<TrainSet> TrainSetPtr;

    void printTrainSet(const TrainSet& trainSet, const std::string& fileName);
}

#endif
