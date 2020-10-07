/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_MODEL_LEARNABLE_H__
#define __EFG_MODEL_LEARNABLE_H__

#include <node/NodeFactory.h>
#include <node/belprop/BasicPropagator.h>
#include <distribution/Combinations.h>

namespace EFG::model {

	/*!
	 * \brief Interface for managing learnable graphs, i.e. graphs for which it is possible perform learning.
	 */
	class GraphLearnable : public node::Node::NodeFactory {
	public:
		~GraphLearnable();

		/*!
		 * \brief Returns the actual values of the tunable weights, those of the exponential shapes
		 whose weights can vary.
		 */
		std::vector<float> GetTunable() const;

		/*!
		 * \brief Set the values of the tunable weights, those of the exponential shapes
		 whose weights can vary.
		 */
		void SetTunable(const std::vector<float>& w_vector);

		/*!
		 * \brief Returns the gradient of the likelihood of the model w.r.t the tunable weights and a certain trainig set.
		 * @param[out] w_grad the computed gradient
		 * @param[in] training_set the training set to consider, i.e. the series of samples, wrapped in a combinations object
		 */
		std::vector<float> GetTunableGrad(const distr::Combinations& training_set, const bool& force_alpha_recomputation = false);

		/*!
		 * \brief Returns the model size, i.e. the number of tunable parameters of the model, i.e. the number of weigths that can vary.
		 */
		inline std::size_t GetModelSize() const { return this->AtomicLearner.size() + this->CompositeLearner.size(); };

		/* \brief Enables the regularization term when computing the gradient.
		\details The regularion term penalizes big values for the weights, adding to the
		function to maximise when performing learning the following term: -w'*w.
		When enabled, that term is added when computing the gradient.
		 */
		inline void ActivateRegularization() { this->UseRegularization = true; };

		/* \brief Disables the regularization term when computing the gradient,
		\details see also Activate_Regularization_term.
		When disabled, the regularization term is not considered when computing the gradient.
		 */
		inline void DeactivateRegularization() { this->UseRegularization = false; };

		const Structure GetStructure() const override;
	protected:
		GraphLearnable(const bool& use_cloning_Insert, const node::bp::BeliefPropagator& propagator) : NodeFactory(use_cloning_Insert, propagator), LastTrainingSetUsed(nullptr), UseRegularization(false) {};

		class LearningHandler;

		class AtomicHandler;
		class CompositeHandler;

		class UnaryHandler;
		class BinaryHandler;

		void 				 						 _Insert(const Structure& strct) override;
		pot::ExpFactor*								 _Insert(pot::ExpFactor* pot, const bool& weight_tunability);

		template<typename Handler, typename ... Args>
		inline void									 _Add(Args ... args);

		void										 _Share(const std::vector<CategoricVariable*>& set1, const std::vector<CategoricVariable*>& set2); //variable sets of the potetentials that must share the weight

		template<typename Handler, typename ... Args>
		void										 _Replace(const std::vector<CategoricVariable*>& vars, Args ... args);

		virtual std::vector<float>   				 _GetBetaPart(const distr::Combinations& training_set) = 0;

		inline const std::list<LearningHandler*>*    _GetLearnerList() const { return &this->LearnerList; };

		std::list<AtomicHandler*>			         _GetAllHandlers() const; //both the ones isolated and the ones inside the composites
	private:
		std::list<AtomicHandler*>::iterator														__FindInAtomics(const std::vector<CategoricVariable*>& vars);
		std::pair<std::list<CompositeHandler>::iterator, std::list<AtomicHandler*>::iterator>	__FindInComposites(const std::vector<CategoricVariable*>& vars);
		bool																					__Compare(const std::vector<CategoricVariable*>& set1, const std::vector<CategoricVariable*>& set2);
	
	// data
		std::list<AtomicHandler*>									AtomicLearner;
		std::list<CompositeHandler>									CompositeLearner;

		std::list<LearningHandler*>								    LearnerList; //the list of atomic and composite elements (atomic inside composite does not apper, since are contained in composite)

		bool														UseRegularization;
		const distr::Combinations*									LastTrainingSetUsed;
	};

	class GraphLearnable::LearningHandler {
	public:
		virtual ~LearningHandler() = default;

		virtual const float&	GetWeight() = 0;
		virtual void			SetWeight(const float& w_new) = 0;

		virtual void			RecomputeAlfaPart(const distr::Combinations& train_set) = 0;
		virtual const float&	GetAlfaPart() = 0;
		virtual float			GetBetaPart() = 0; //according to last performed belief propagation
	};

	class GraphLearnable::AtomicHandler : public LearningHandler, public pot::IPotentialDecorator, public pot::ExpFactor::Modifier {
	public:
		inline const float&		GetWeight() override { return this->GetWrappedExp()->GetWeight(); };
		inline void				SetWeight(const float& w_new) override { this->pot::ExpFactor::Modifier::SetWeight(w_new); };

		void					RecomputeAlfaPart(const distr::Combinations& train_set) override;
		const float&			GetAlfaPart() override { return this->alfaPart; };

		pot::ExpFactor*			GetWrappedExp() { return static_cast<pot::ExpFactor*>(this->GetWrapped()); };

		~AtomicHandler() { this->reset(); };
	protected:
		AtomicHandler(pot::ExpFactor* pot_to_handle);
		// cache
		float										  alfaPart;
	};

	class GraphLearnable::CompositeHandler : public  LearningHandler {
	public:
		~CompositeHandler();
		CompositeHandler(AtomicHandler* initial_A, AtomicHandler* initial_B);

		inline const float&		GetWeight() override { return this->Components.front()->GetWeight(); };
		void					SetWeight(const float& w_new) override;

		void					RecomputeAlfaPart(const distr::Combinations& train_set) override;
		inline const float&		GetAlfaPart() override { return this->alfaPart; };
		float					GetBetaPart() override;

		void 										Append(AtomicHandler* to_add);
		inline std::list<AtomicHandler*>*			GetComponents() { return &this->Components; };
		inline const std::list<AtomicHandler*>*		GetComponents() const { return &this->Components; };
		inline size_t								size() const { return this->Components.size(); };
	private:
		// data
		std::list<AtomicHandler*>					  Components;
		// cache
		float										  alfaPart;
	};

	template<typename Handler, typename ... Args>
	void	GraphLearnable::_Add(Args ... args) { //the arguments needed for building the atomic handler
		this->AtomicLearner.push_back(new Handler(args...));
		this->LearnerList.push_back(this->AtomicLearner.back());
	};

	template<typename Handler, typename ... Args>
	void	GraphLearnable::_Replace(const std::vector<CategoricVariable*>& vars, Args ... args) {

		auto a = this->__FindInAtomics(vars);
		if (a != this->AtomicLearner.end()) {
			pot::ExpFactor* pot = (*a)->GetWrappedExp();
			this->LearnerList.remove(*a);
			delete* a;
			*a = new Handler(pot, args...);
			this->LearnerList.push_back(*a);
			return;
		}

		auto c = this->__FindInComposites(vars);
		if (c.first != this->CompositeLearner.end()) {
			pot::ExpFactor* pot = (*c.second)->GetWrappedExp();
			delete* c.second;
			*c.second = new Handler(pot, args...);
			return;
		}

		throw std::runtime_error("inexistent potential");

	};

}

#endif