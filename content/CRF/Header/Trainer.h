//Author: Andrea Casalino
//mail: andrecasa91@gmail.com

#pragma once
#ifndef __CRF_TRAINER_H__
#define __CRF_TRAINER_H__

#include "Training_set.h"
#include "Graphical_model.h"

namespace Segugio {

	class I_Trainer {
	public:
		static I_Trainer* Get_fixed_step(const float& step_size = 0.1f, const float& stoch_grad_percentage = 1.f);
		static I_Trainer* Get_BFGS(const float& stoch_grad_percentage = 1.f);

		virtual void Train(Graph_Learnable* model_to_train, Training_set* Train_set, const unsigned int& Max_Iterations = 100, std::list<float>* descend_story = NULL) = 0;
	protected:
		virtual void Clean_Up() {};
		static void Clean_Up(I_Trainer* to_Clean) { to_Clean->Clean_Up(); };

		void Get_w_grad(Graph_Learnable* model, std::list<float>* grad_w, std::list<size_t*>* comb_in_train_set, std::list<Categoric_var*>* comb_var) {
			Graph_Learnable::Weights_Manager::Get_w_grad(grad_w, model, comb_in_train_set, comb_var); };
		void Set_w(const std::list<float>& w, Graph_Learnable* model) { Graph_Learnable::Weights_Manager::Set_w(w, model); };
	};

}

#endif