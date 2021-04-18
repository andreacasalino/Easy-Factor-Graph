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

namespace EFG::train {
    TrainSet::TrainSet(const std::vector<Combination>& combinations) {
        if (combinations.empty()) {
            throw Error("empty train set");
        }
        auto it = combinations.begin();
        std::size_t size = it->size();
        if (0 == size) {
            throw Error("invalid train set");
        }
        ++it;
        std::for_each(it, combinations.end(), [&size](const Combination& c) {
            if (c.size() != size) {
                throw Error("invalid train set");
            }
        });
    }

    std::vector<Combination> import(const std::string& fileName) {
        std::ifstream reader(fileName);
        if (!reader.is_open()) {
            throw Error("invalid file");
        }
        auto convert = [](const std::list<std::string>& slices) {
            std::vector<std::size_t> vals;
            vals.reserve(slices.size());
            std::for_each(slices.begin(), slices.end(), [&vals](const std::string& v) {
                vals.emplace_back(std::atoi(v.c_str()));
            });
            return vals;
        };
        std::vector<Combination> values;
        std::string line;
        while (!reader.eof()) {
            std::getline(reader, line);
            values.push_back(Combination(convert(xmlPrs::Parser::sliceFragments(line))));
        }
        return values;
    };
    TrainSet::TrainSet(const std::string& fileName) 
        : TrainSet(import(fileName)) {
    }
}
