/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <Presenter.h>
#include <iostream>

namespace EFG::sample {
    std::size_t Presenter::counter = 0;

    Presenter::Presenter(const std::string& name, const std::string& docSection, const std::string& results) {
        ++counter;
        std::cout << "--------------------------------------------------------\n\n";
        std::cout << "part " << counter;        
        if(!name.empty()) {
            std::cout << ", " << name;
        }
        if(!docSection.empty()) {
            std::cout << ", refer to the Section " << docSection << " of the documentation";
        }
        std::cout << "\n\n--------------------------------------------------------";
        std::cout << std::endl << std::endl;

        this->results = std::make_unique<std::string>(results);
    }

    Presenter::~Presenter() {
        if(nullptr != this->results) {
            std::cout << *this->results << std::endl;
        }
        std::cout << std::endl;
    }
}
