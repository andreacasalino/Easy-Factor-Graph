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
		/** 
		 * @return the marginal probabilty of the passed variable, i.e. P(var|model, observations),
		 * conditioned to the last set evidences.
		 * @throw when the passed variable name is not found
		 */
		std::vector<float> getMarginalDistribution(const std::string& var);

		/**
		 * @return a factor representing the joint distribution of the subgraph described by the passed variables.
		 * @throw when some of the passed variable names is not found
		 */
		distribution::factor::cnst::Factor getJointMarginalDistribution(const std::set<std::string>& subgroup);

		/** 
		 * @return the Maximum a Posteriori estimation of a specific variable in the model, 
		 * conditioned to the last set evidences.
		 * @throw when the passed variable name is not found
		 */
		std::size_t getMAP(const std::string& var);

		/**
		 * @return the Maximum a Posteriori estimation of the hidden variables, conditioned to the last set evidences.
		 * values are ordered in the same way the variables in the hiddden set can be ordered (alfabetic order)
		 */
		std::vector<size_t> getHiddenSetMAP();
    };
}

#endif
