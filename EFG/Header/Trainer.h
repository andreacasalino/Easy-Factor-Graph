/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/


#pragma once
#ifndef __CRF_TRAINER_H__
#define __CRF_TRAINER_H__

#include "Training_set.h"
#include "Graphical_model.h"
#include <memory>

//#define COMPILE_BFGS


namespace EFG {

	/*!
	 * \brief This class is used by a Graph_Learnable, to perform training with an instance of a Training_set.
	 * \details Instantiate a particular class of trainer to use by calling Get_fixed_step or Get_BFGS. That methods 
	 * allocate in the heap a trainer to use later, for multiple tranining sessions. Remember to delete the instantiated
	 * trainer.
	 */
	class I_Trainer {
	public:
		/**
		* \brief Creates a fixed step gradient descend solver.
		* @param[in] step_size learinig degree
		* @param[in] stoch_grad_percentage percentage of the training set to use every time for evaluating the gradient
		*/
		static std::unique_ptr<I_Trainer> Get_fixed_step(const float& step_size = 0.1f, const float& stoch_grad_percentage = 1.f);

#ifdef COMPILE_BFGS
		/**
		* \brief Creates a BFGS gradient descend solver (https://en.wikipedia.org/wiki/Broyden%E2%80%93Fletcher%E2%80%93Goldfarb%E2%80%93Shanno_algorithm)
		* @param[in] stoch_grad_percentage percentage of the training set to use every time for evaluating the gradient
		*/
		static std::unique_ptr<I_Trainer> Get_BFGS(const float& stoch_grad_percentage = 1.f);
#endif

		virtual ~I_Trainer() { delete this->adv_strtgy; };

		/* \brief Performs training of the passed model, considering the passed training set.
		*
		* @param[in] model_to_train the model to train
		* @param[in] training_set the training set to consider 
		* @param[in] Max_Iterations maximum number of iterations assumed by the trainer
		* @param[in] descend_story when passed different from NULL, will contains the evolution of the model likelihood during the training 
		*/
		virtual void Train(Graph_Learnable& model_to_train, Training_set& training_set, const unsigned int& Max_Iterations = 100, std::list<float>* descend_story = NULL) = 0;
	private:
		class I_Advancer_Strategy {
		public:
			virtual ~I_Advancer_Strategy() {};
			virtual void Reset() = 0;
			virtual float advance(Graph_Learnable* model_to_advance, const I_Potential::combinations& train_set, const bool& force_alfa_rec) = 0; //return L_infinity norm of weigth variation
		};

		class Fixed_step;
#ifdef COMPILE_BFGS
		class BFGS;
#endif

		class Entire_Set;
		class Stoch_Set;

		I_Trainer(I_Advancer_Strategy* strtgy) : adv_strtgy(strtgy) {};

	// data
		I_Advancer_Strategy*					adv_strtgy;

	};

}

#endif