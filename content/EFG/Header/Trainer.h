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
		static I_Trainer* Get_fixed_step(const float& step_size = 0.1f, const float& stoch_grad_percentage = 1.f);

#ifdef COMPILE_BFGS
		/**
		* \brief Creates a BFGS gradient descend solver (https://en.wikipedia.org/wiki/Broyden%E2%80%93Fletcher%E2%80%93Goldfarb%E2%80%93Shanno_algorithm)
		* @param[in] stoch_grad_percentage percentage of the training set to use every time for evaluating the gradient
		*/
		static I_Trainer* Get_BFGS(const float& stoch_grad_percentage = 1.f);
#endif

		virtual ~I_Trainer() {};

		virtual void Train(Graph_Learnable* model_to_train, Training_set* Train_set, const unsigned int& Max_Iterations = 100, std::list<float>* descend_story = NULL) = 0;
	protected:
		virtual void Clean_Up() {};
		static void Clean_Up(I_Trainer* to_Clean) { to_Clean->Clean_Up(); };

		void Get_w_grad(Graph_Learnable* model, std::list<float>* grad_w, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var) {
			Graph_Learnable::Weights_Manager::Get_tunable_w_grad(grad_w, model, comb_in_train_set, comb_var); };
		void Set_w(const std::list<float>& w, Graph_Learnable* model) { Graph_Learnable::Weights_Manager::Set_tunable_w(w, model); };
	};

}

#endif