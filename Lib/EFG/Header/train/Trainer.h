/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/


#pragma once
#ifndef __EFG_TRAINER_H__
#define __EFG_TRAINER_H__

#include <train/TrainingSet.h>
#include <model/Learnable.h>
#include <node/EnergyEvaluator.h>

//#define COMPILE_BFGS


namespace EFG::train {

	/*!
	 * \brief This class is used by a Graph_Learnable, to perform training with an instance of a Training_set.
	 * \details Instantiate a particular class of trainer to use by calling Get_fixed_step or Get_BFGS. That methods 
	 * allocate in the heap a trainer to use later, for multiple tranining sessions. Remember to delete the instantiated
	 * trainer.
	 */
	class Trainer {
	public:
		/**
		* \brief Creates a fixed step gradient descend solver.
		* @param[in] step_size learinig degree
		* @param[in] stoch_grad_percentage percentage of the training set to use every time for evaluating the gradient
		*/
		static std::unique_ptr<Trainer> GetFixedStep(const float& step_size = 0.1f, const float& stoch_grad_percentage = 1.f);

#ifdef COMPILE_BFGS
		/**
		* \brief Creates a BFGS gradient descend solver (https://en.wikipedia.org/wiki/Broyden%E2%80%93Fletcher%E2%80%93Goldfarb%E2%80%93Shanno_algorithm)
		* @param[in] stoch_grad_percentage percentage of the training set to use every time for evaluating the gradient
		*/
		static std::unique_ptr<I_Trainer> GetBFGS(const float& stoch_grad_percentage = 1.f);
#endif

		/* \brief Performs training of the passed model, considering the passed training set.
		*
		* @param[in] model_to_train the model to train
		* @param[in] training_set the training set to consider 
		* @param[in] Max_Iterations maximum number of iterations assumed by the trainer
		* @param[in] descend_story when passed different from NULL, will contains the evolution of the model likelihood during the training 
		*/
		virtual void operator()(model::GraphLearnable& model_to_train, train::TrainingSet& training_set, const unsigned int& Max_Iterations = 100, std::list<float>* descend_story = nullptr) = 0;
	protected:

		class LikelihhodEvaluator{
		public:
			LikelihhodEvaluator(model::GraphLearnable& model) : Evaluator(model.GetEnergyEvaluator(model.GetAllVariables())) {};
			
			float operator()(const train::TrainingSet::UsableTrainingSet& set);
		private:
			node::Node::NodeFactory::EnergyEvaluator			Evaluator;
		};	

	private:
		class AdvancerStrategy {
		public:
			virtual ~AdvancerStrategy() {};
			virtual void Reset() = 0;
			virtual float advance(model::GraphLearnable* model_to_advance, const train::TrainingSet::UsableTrainingSet& train_set, const bool& force_alfa_rec) = 0; //return L_infinity norm of weigth variation
		};

		class FixedStep;
#ifdef COMPILE_BFGS
		class BFGS;
#endif

		class EntireSet;
		class StochSet;

		Trainer(std::unique_ptr<AdvancerStrategy> strtgy) : advStrtgy(std::move(strtgy)) {};

	// data
		std::unique_ptr<AdvancerStrategy> advStrtgy;
	};

}

#endif