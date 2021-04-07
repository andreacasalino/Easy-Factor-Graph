/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_CATEGORIC_VARIABLE_H
#define EFG_CATEGORIC_VARIABLE_H

#include <observers/MultiObservable.h>
#include <string>
#include <memory>

namespace EFG::categoric {
    class Variable : public observer::MultiObservable {
    public:
        /** \brief domain is assumed to be {0,1,2,3,...,size}
        * @param[in] size domain size of this variable
        * @param[in] name name to attach to this variable. It cannot be an empty string ""
        */
        Variable(const std::size_t& size, const std::string& name);

        Variable(const Variable& ) = default;
        Variable& operator=(const Variable& ) = delete;

        inline bool operator==(const Variable& o) const { return (this->Size == o.Size) && (this->Name == o.Name); };

        inline std::size_t size() const { return this->Size; };
        inline const std::string& name() const { return this->Name; };
        
    protected:
    // data
        const size_t	  Size; /** domain size */
        const std::string Name; /** name associated to the variable */
    };

    typedef std::shared_ptr<Variable> VariablePtr;

    inline VariablePtr makeVariable(const std::size_t& size, const std::string& name) { return std::make_shared<Variable>(size, name); };
}

#endif