/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_VARIABLE_H__
#define __EFG_VARIABLE_H__

#include <Observed_Subject.h>
#include <string>
#include <vector>
#include <stdexcept>

namespace EFG {

    /*!
    * \brief Describes a categoric variable
    * \details , having a finite set as domain,
    * assumed by default as {0,1,2,3,...,size}.
    A Categoric_var cannot be destroyed before destroying 
    all the potentials referring to it.
    */
    class Categoric_var : public Subject_multiObservers {
    public:
        /** \brief domain is assumed to be {0,1,2,3,...,size}
        * @param[in] size domain size of this variable
        * @param[in] name name to attach to this variable. It cannot be an empty string ""
        */
        Categoric_var(const size_t& size, const std::string& name);

        Categoric_var() = delete;
        void operator=(const Categoric_var& ) = delete;

        inline const size_t& size() const { return this->Size; };
        inline const std::string& Get_name() const { return this->Name; };

        /** \brief Returns true if there not exist clones of pointers in the passed set. 
         * \details Throws exception if an empty vector is passed
        */
        static bool check_all_vars_are_different(const std::vector<Categoric_var*>& vars);

        static size_t Get_joint_domain_size(const std::vector<Categoric_var*>& vars);
    protected:
    // data
        size_t		Size; /** domain size */
        std::string Name; /** name associated to the variable */
    };



    /*!
    * \brief This object allows you to iterate all the elements in a discrete domain, without precomputing all the elements in such a domain.
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
	class Domain_iterator {
	public:
		Domain_iterator(const std::vector<Categoric_var*>& vars);

		Domain_iterator& 								operator++();
		inline const std::vector<size_t>&				operator()() const { if(!this->flag) throw std::runtime_error("iterator not incrementable"); return this->Comb; };
		inline bool										is_not_at_end() const { return this->flag; };
	private:
		void											init(const std::vector<Categoric_var*>& vars);
	// data
		std::vector<size_t>		Sizes;
		std::vector<size_t>		Comb;
		bool					flag;
	};

}

#endif
