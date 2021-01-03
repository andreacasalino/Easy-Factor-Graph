/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_VARIABLE_H__
#define __EFG_VARIABLE_H__

#include <util/ObservedSubject.h>
#include <util/Iterator.h>
#include <string>
#include <vector>

namespace EFG {

    /*!
    * \brief Describes a categoric variable
    * \details , having a finite set as domain,
    * assumed by default as {0,1,2,3,...,size}.
    A Categoric_var cannot be destroyed before destroying 
    all the potentials referring to it.
    */
    class CategoricVariable : public sbj::MultiObservable {
    public:
        /** \brief domain is assumed to be {0,1,2,3,...,size}
        * @param[in] size domain size of this variable
        * @param[in] name name to attach to this variable. It cannot be an empty string ""
        */
        CategoricVariable(const std::size_t& size, const std::string& name);

        CategoricVariable(const CategoricVariable& o);

        void operator=(const CategoricVariable&) = delete;

        inline const std::size_t& size() const { return this->Size; };
        inline const std::string& GetName() const { return this->Name; };

        /** \brief Returns true if there not exist clones of pointers in the passed set. 
         * \details Throws exception if an empty vector is passed
        */
        static bool AreAllVarsDifferent(const std::vector<CategoricVariable*>& vars);

        /** \brief Returns the size of the joint domain
        */
        static std::size_t GetJointDomainSize(const std::vector<CategoricVariable*>& vars);
    protected:
    // data
        size_t		Size; /** domain size */
        std::string Name; /** name associated to the variable */
    };

    /*!
    * \brief This object allows you to iterate all the elements in the joint domain of a group of variables, without precomputing all the elements in such a domain.
    * \details For example when having a domain made by variables = { A (size = 2), B (size = 3), C (size  = 2)  }, the elements in the
    * joint domain that will be iterated are:
    * <0,0,0>
    * <0,0,1>
    * <0,1,0>
    * <0,1,1>
    * <0,2,0>
    * <0,2,1>
    * <1,0,0>
    * <1,0,1>
    * <1,1,0>
    * <1,1,1>
    * <1,2,0>
    * <1,2,1>
    */
    class JointDomainIterator : public itr::Iterator {
    public:
        JointDomainIterator(const std::vector<CategoricVariable*>& vars);

        Iterator& operator++() final;
        const std::vector<size_t>& operator()() const;

        static void forEach(const std::vector<CategoricVariable*>& vars, const std::function<void(const std::vector<size_t>&)>& action);
    private:
        // data
        std::vector<size_t>		Sizes;
        std::vector<size_t>		Comb;
    };

}

#endif
