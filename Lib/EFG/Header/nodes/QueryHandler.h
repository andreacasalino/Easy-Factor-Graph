/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_QUERY_HANDLER_H
#define EFG_NODES_QUERY_HANDLER_H

#include <nodes/bases/NodesAware.h>
#include <nodes/bases/EvidenceAware.h>
#include <nodes/bases/BeliefAware.h>
#include <distribution/factor/const/Factor.h>

namespace EFG::nodes {
    class QueryHandler
		: virtual public NodesAware
		, virtual public EvidenceAware
		, virtual public BeliefAware {
    public:
		/** \brief Returns the marginal probabilty of the variable passed P(var|model, observations),
		* \details on the basis of the last observations set (see Node_factory::Set_Observation_Set_var)
		*/
		std::vector<float> getMarginalDistribution(const std::string& var);

		// the returned shape has the variable in the same order as the ones passed
		distribution::factor::cnst::Factor getJointMarginalDistribution(const std::set<std::string>& subgroup);

		/** \brief Returns the Maximum a Posteriori estimation of the hidden set. @latexonly\label{MAP_method}@endlatexonly
		* \details Values are ordered as returned by Node_factory::Get_Actual_Hidden_Set.
		* Calculations are done considering the last last observations set (see Node_factory::Set_Observation_Set_var)
		*/
		std::size_t getMAP(const std::string& var);

		// returns the MAP of all the hidden variables, in the same order as returned with GetHiddenSet()
		std::vector<size_t> getHiddenSetMAP();
    };
}

#endif
