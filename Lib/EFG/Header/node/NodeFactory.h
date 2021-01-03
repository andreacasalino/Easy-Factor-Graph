/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_NODE_FACTORY_H__
#define __EFG_NODE_FACTORY_H__

#include <node/Node.h>
#include <util/univocal_map.h>
#include <unordered_set>
#include <potential/ExpFactor.h>
#include <EquiPool.h>

namespace EFG::node {
	namespace bp {
		class BeliefPropagator;
	}

	class Node::NodeFactory : public EFG::pot::ExpFactor::Mover {
	public:
		NodeFactory(const NodeFactory&) = delete;
		void operator=(const NodeFactory&) = delete;

		virtual ~NodeFactory();

		/** \brief Returns a pointer to the variable in this graph with that name.
		* \details Returns NULL when the variable is not present in the graph.
		* @param[in] var_name name to search
		*/
		CategoricVariable* FindVariable(const std::string& var_name) const;
		/** \brief Returns the current set of hidden variables
		*/
		std::vector<CategoricVariable*> GetHiddenSet() const;
		/** \brief Returns the current set of observed variables
		*/
		std::vector<std::pair<CategoricVariable*, size_t>> GetObservationSet() const;

		std::vector<CategoricVariable*> GetObservationSetVars() const;

		/** \brief Returns the set of all variable contained in the net
		*/
		std::vector<CategoricVariable*> GetAllVariables() const;

		class EnergyEvaluator;
		inline EnergyEvaluator GetEnergyEvaluator(const std::vector<CategoricVariable*>& vars_order) const;

		typedef std::tuple<std::vector<pot::Factor*>, std::vector<std::vector<pot::ExpFactor*>>, std::vector<pot::ExpFactor*>> Structure;
		/*!
		* \brief Returns the list of potentials constituting the net.
		\details The potentials returned cannot be used for initializing a model. For performing such a task
		* you can build an empty  model and then use Absorb.
		*
		* @param[out] shapes list of Simple shapes contained in the  model
		* @param[out] learnable_exp list of Exponential tunable potentials contained in the model: every sub group share the same weight
		* @param[out] constant_exp list of Exponential constant potentials contained in the model
		*/
		virtual Structure GetStructure() const;

		/*!
		* \brief Returns the number of potentials constituting the graph, no matter of their type (simple shape, exponential shape fixed or exponential shape tunable)
		*/
		inline size_t					GetStructureSize() const { return (this->__SimpleShapes.size() + this->__ExpShapes.size()); };

		/** \brief Returns the current value adopted when performing a loopy belief propagation
		*/
		inline const unsigned int&		GetIteration4Propagation()  const { return this->PropagationMaxIter; };
		/** \brief Returns the value to adopt when performing a loopy belief propagation
		*/
		inline void					  	SetIteration4Propagation(const unsigned int& iter_to_use);

		inline bool						wasPropagationCompleted() const { if (this->LastPropagation != nullptr) return this->LastPropagation->TerminatedWithinIter; return true; };

		/** \brief Returns the marginal probabilty of the variable passed P(var|model, observations),
		* \details on the basis of the last observations set (see Node_factory::Set_Observation_Set_var)
		*/
		std::vector<float>				GetMarginalDistribution(const std::string& var);
		/** \brief Returns the Maximum a Posteriori estimation of the hidden set. @latexonly\label{MAP_method}@endlatexonly
		* \details Values are ordered as returned by Node_factory::Get_Actual_Hidden_Set.
		* Calculations are done considering the last last observations set (see Node_factory::Set_Observation_Set_var)
		*/
		size_t							GetMAP(const std::string& var);

		//returns the MAP of all the hidden variables, in the same order as returned with GetHiddenSet()
		std::vector<size_t>				GetMAP();

		/** \brief Returns a set of samples of the conditional distribution P(hidden variables | model, observed variables). @latexonly\label{Gibbs_method}@endlatexonly
		* \details Samples are obtained through Gibbs sampling.
		* Calculations are done considering the last last observations set (see Node_factory::Set_Observation_Set_var)
		* @param[in] N_samples number of desired samples
		* @param[in] initial_sample_to_skip number of samples to skip for performing Gibbs sampling
		* @param[out] result returned samples: every element of the list is a combination of values for the hidden set, with the same
		* order returned when calling Node_factory::Get_Actual_Hidden_Set
		*/
		std::list<std::vector<size_t>> GibbsSamplingHiddenSet(const unsigned int& N_samples, const unsigned int& initial_sample_to_skip, const int& seed = -1); //on the basis of last observed values set

		void							Reprint(const std::string& file_name) const;

		pot::Factor 					GetJointMarginalDistribution(const std::vector<std::string>& subgroup); //the returned shape has the variable in the same order as the ones passed

		// use only for big graphs
		void							SetThreadPoolSize(const std::size_t& poolSize); //when passing <= 1 the actual pool is destroyed. When building the object, a defualt 0 size value is assumed
	protected:
		NodeFactory(const bool& use_cloning_Insert);

		class XmlStructureImporter;

		Node*							_FindNode(const std::string& var_name);

		size_t*							_FindObservation(const std::string& var_name);

		void		  					_Insert(pot::Factor& shape);
		void		  					_Insert(pot::Factor&& shape);

		pot::ExpFactor*					_Insert(pot::ExpFactor& exp_shape); //returns the potential actually inserted in this graph.
		pot::ExpFactor*					_Insert(pot::ExpFactor&& exp_shape); //returns the potential actually inserted in this graph. !!! the passed exp shape is emptied and should be not used anymore

		virtual void 					_Insert(const Structure& strct, const bool& useMove); //when useMove passed as true, every element in strct is transferred into this graph using move, empty the original factors

		void							_Copy(const NodeFactory& o); // copy structure and clone propagator

		/** \brief Set the evidences: identify the variables in the hidden set and the values assumed
		\details When passing both input as empty list, all the evidences are deleted.
			*
			* @param[in] new_observed_vars the set of variables to consider as evidence
			* @param[in] new_observed_vals the values assumed as evidences
		*/
		void					  		_SetEvidences(const std::vector<std::pair<std::string, size_t>>& new_observations);

		/** \brief Similar to Node_factory::Set_Evidences(const std::list<CategoricVariable*>& new_observed_vars, const std::list<size_t>& new_observed_vals)
		\details Here only the evidence values are set, assuming the last oberved set imposed
		*/
		void					  		_SetEvidences(const std::vector<size_t>& new_observations);

		static const bp::BeliefPropagator& _GetPropagator(const NodeFactory& model);
		void 							   _SetPropagator(std::unique_ptr<bp::BeliefPropagator> prop);

		void					  		_BeliefPropagation(const bool& sum_or_MAP); //in the protected version, the propagation is forced to be always re-done
	private:
		inline std::unique_ptr<EFG::pot::Factor> createMoving(EFG::pot::Factor&& o) { return std::make_unique<EFG::pot::Factor>(std::move(o)); };
		using Mover::createMoving;

		template<typename P>
		P*								__Insert(P* to_insert, const bool& use_move = false);

		void					  		__RecomputeClusters();
		void					  		__BeliefPropagation(const bool& sum_or_MAP);

		class ObsvContainer : public ump::univocal_map< Node*, std::pair<Node*, size_t>, std::function<Node*(std::pair<Node*, size_t>&)> > {
			std::vector<Node*>  order;
		public:
			ObsvContainer();

			void setVars(const std::vector<Node*>& vars);
			void setVals(const std::vector<size_t>& vals);
			inline const std::vector<Node*>& getOrder() const { return this->order; };
		};

		struct PropagationInfo {
			bool						TerminatedWithinIter;
			bool						WasSum_or_MAP;
		};

	// data
		sbj::MultiObservable															subject;

		bool																			bDestroyPotentials_and_Variables;

		unsigned int 																	PropagationMaxIter;

		std::unique_ptr<PropagationInfo>												LastPropagation;

		std::unique_ptr<bp::BeliefPropagator>											Propagator;

		std::list<std::unordered_set<Node*>>											HiddenClusters;

		ObsvContainer																	Observations;

		ump::univocal_map<const std::string*, Node,
			std::function<const std::string* (Node&)>,  std::function<size_t(const std::string*)>, 
			std::function<bool(const std::string*, const std::string*)>>				Nodes;

		ump::univocal_map< std::pair<const std::string*, const std::string*>, const pot::IPotential*,
			std::function<std::pair<const std::string*, const std::string*>(const pot::IPotential*)>,
			std::function<size_t(const std::pair<const std::string*, const std::string*>&)>,
			std::function<bool(const std::pair<const std::string*, const std::string*>& , const std::pair<const std::string*, const std::string*>&)>>
																						BinaryPotentials;

		std::list<sbj::Subject::Observer>												PotentialObservers;

	//these lists are used only for having a direct reference to the differet kind of potentials in the model
		std::list<pot::Factor*>															__SimpleShapes;
		std::list<pot::ExpFactor*>														__ExpShapes;

	protected:
	//this thread pool can be exploited to speed up different kinds of computations
		std::unique_ptr<thpl::equi::Pool>												ThPool;
	};

}

#endif