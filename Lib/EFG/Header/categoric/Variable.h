/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_CATEGORIC_VARIABLE_H
#define EFG_CATEGORIC_VARIABLE_H

#include <string>
#include <memory>

namespace EFG::categoric {
    /**
     * @brief An object representing an immutable categoric variable.
     */
    class Variable {
    public:
       /** 
        * @param domain size of this variable
        * @param name used to label this varaible. 
        * @throw passing an empty string 
        */
        Variable(const std::size_t& size, const std::string& name);

        Variable(const Variable& ) = default;
        Variable& operator=(const Variable& ) = delete;

        inline bool operator==(const Variable& o) const { return (this->Size == o.Size) && (this->Name == o.Name); };

        inline std::size_t size() const { return this->Size; };
        inline const std::string& name() const { return this->Name; };
        
    protected:
        const size_t	  Size;
        const std::string Name;
    };

    typedef std::shared_ptr<Variable> VariablePtr;

    inline VariablePtr makeVariable(const std::size_t& size, const std::string& name) { return std::make_shared<Variable>(size, name); };
}

#endif
