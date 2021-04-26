/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_SAMPLE_PRESENTER_H
#define EFG_SAMPLE_PRESENTER_H

#include <string>
#include <memory>

namespace EFG::sample {
    class Presenter {
    public:
        ~Presenter();

        Presenter(const std::string& name, const std::string& docSection, const std::string& results);

    private:
        static std::size_t counter;

        std::unique_ptr<std::string> results;
    };

    template<typename Example>
    void samplePart(const Example& example, const std::string& name, const std::string& docSection = "", const std::string& results = "") {
        EFG::sample::Presenter presenter(name, docSection, results);
        example();
    };
}

#endif
