/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <train/TrainSet.h>
#include <Error.h>
#include <Parser.h>
#include <algorithm>
#include <fstream>
#include <math.h>

namespace EFG::train {
    TrainSet::TrainSet(const std::vector<CombinationPtr>& combinations) 
        : combinations(combinations) {
        if (combinations.empty()) {
            throw Error("empty train set");
        }
        auto it = combinations.begin();
        std::size_t size = (*it)->size();
        if (0 == size) {
            throw Error("invalid train set");
        }
        ++it;
        std::for_each(it, combinations.end(), [&size](const CombinationPtr& c) {
            if (c->size() != size) {
                throw Error("invalid train set");
            }
        });
    };

    std::vector<CombinationPtr> import(const std::vector<categoric::Combination>& combinations) {
        std::vector<CombinationPtr> temp;
        temp.reserve(combinations.size());
        std::for_each(combinations.begin(), combinations.end(), [&temp](const categoric::Combination& c){
            temp.emplace_back(std::make_shared<categoric::Combination>(c));
        });
        return temp;
    }
    TrainSet::TrainSet(const std::vector<categoric::Combination>& combinations)
        : TrainSet(import(combinations)) {
    }

    std::vector<CombinationPtr> import(const std::string& fileName) {
        std::ifstream reader(fileName);
        if (!reader.is_open()) {
            throw Error("invalid file");
        }
        auto convert = [](const std::list<std::string>& slices) {
            categoric::Combination comb(slices.size());
            std::size_t k = 0;
            auto* combData = comb.data();
            std::for_each(slices.begin(), slices.end(), [&](const std::string& v) {
                combData[k] = static_cast<std::size_t>(std::atoi(v.c_str()));
                ++k;
            });
            return comb;
        };
        std::vector<CombinationPtr> values;
        std::string line;
        while (!reader.eof()) {
            std::getline(reader, line);
            values.push_back(std::make_shared<categoric::Combination>(convert(xmlPrs::Parser::sliceFragments(line))));
        }
        return values;
    };
    TrainSet::TrainSet(const std::string& fileName) 
        : TrainSet(import(fileName)) {
    }

    TrainSet TrainSet::getRandomSubSet(const float& percentage) const {
        std::size_t numberOfCombinations = 1;
        numberOfCombinations = std::max(static_cast<std::size_t>(std::floor(this->combinations.size() * percentage)), numberOfCombinations);
        numberOfCombinations = std::min(this->combinations.size(), numberOfCombinations);
        std::vector<CombinationPtr> randomCombinations;
        randomCombinations.reserve(numberOfCombinations);
        for(std::size_t k=0; k<numberOfCombinations; ++k) {
            randomCombinations.emplace_back(this->combinations[rand() % this->combinations.size()]);
        }
        return TrainSet(randomCombinations);
    }

    void printTrainSet(const TrainSet& trainSet, const std::string& fileName) {
        std::ofstream f(fileName);
        if (!f.is_open()) {
            throw Error("invalid file");
        }
        auto printLine = [&f](const CombinationPtr& c) {
            f << c->data()[0];
            for (std::size_t k = 1; k < c->size(); ++k) {
                f << ' ' << c->data()[k];
            }
        };
        auto it = trainSet.getSet().begin();
        printLine(*it);
        ++it;
        std::for_each(it, trainSet.getSet().end(), [&](const CombinationPtr& c) {
            f << std::endl;
            printLine(c);
        });
    }
}
