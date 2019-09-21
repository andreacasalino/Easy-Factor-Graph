#include "../Header/Trainer.h"
using namespace std;

#include "../../thirdy_part_lib/Eigen/Dense"
using namespace Eigen;

namespace Segugio {

	class Advancer_Concrete : public I_Trainer {
	public:
		virtual ~Advancer_Concrete() {};
		virtual void Reset() {};

		void Train(Graph_Learnable* model_to_train, Training_set* Train_set, const unsigned int& Max_Iterations, std::list<float>* descend_story) { abort(); };
		virtual float _advance(Graph_Learnable* model_to_advance, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var) = 0; //return L_infinity norm of weigth variation
	};





	class Fixed_step : public Advancer_Concrete {
	public:
		Fixed_step(const float& step) : Step_to_advance(step) {};
	private:
		float _advance(Graph_Learnable* model_to_advance, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var);
	// data
		float Step_to_advance;
	};

	class BFGS : public Advancer_Concrete {
	public:
		BFGS() : pData(NULL) {};
		~BFGS() { this->Reset(); };

		void Reset();
	private:
		float _advance(Graph_Learnable* model_to_advance,const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var);
		void __line_search(Graph_Learnable* model_to_advance, VectorXf& S, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var);

		struct Cache {
			MatrixXf invB;
			VectorXf Grad_old;
		};
	// data
		Cache*		pData;
	};



	class Trainer_Decorator : public I_Trainer, public Training_set::subset::Handler {
	public:
		Trainer_Decorator(Advancer_Concrete* to_wrap) : Wrapped(to_wrap) {};
		~Trainer_Decorator() { this->I_Trainer::Clean_Up(Wrapped); delete this->Wrapped; };

		void Clean_Up() { this->I_Trainer::Clean_Up(Wrapped); };
	protected:
		void __check_tunable_are_present(Graph_Learnable* model_to_train);
	// data
		Advancer_Concrete*		  Wrapped;
	};



	void Trainer_Decorator::__check_tunable_are_present(Graph_Learnable* model_to_train) {

		list<float> w;
		Graph_Learnable::Weights_Manager::Get_tunable_w(&w, model_to_train);
		if (w.empty()) {
			system("ECHO no tunable paramters are present in the model");
			abort();
		}

	}

	class Entire_Set : public Trainer_Decorator {
	public:
		Entire_Set(Advancer_Concrete* to_wrap) : Trainer_Decorator(to_wrap) {};
		void Train(Graph_Learnable* model_to_train, Training_set* Train_set, const unsigned int& Max_Iterations, std::list<float>* descend_story);
	};

	class Stoch_Set_variation : public Trainer_Decorator {
	public:
		Stoch_Set_variation(Advancer_Concrete* to_wrap, const float& percentage_to_use) : Trainer_Decorator(to_wrap), Percentage(percentage_to_use) {};
		void Train(Graph_Learnable* model_to_train, Training_set* Train_set, const unsigned int& Max_Iterations, std::list<float>* descend_story);
	private:
		float Percentage;
	};




	void compute_order_of_variable_in_train_set(list<Categoric_var*>* result, Graph_Learnable* model, list<string>* var_names_from_training_set) {

		result->clear();
		for (auto it = var_names_from_training_set->begin(); it != var_names_from_training_set->end(); it++)
			result->push_back(model->Find_Variable(*it));

	}

	void Entire_Set::Train(Graph_Learnable* model_to_train, Training_set* Train_set, const unsigned int& Max_Iterations, std::list<float>* descend_story) {

		if (!Train_set->Get_validity_flag()) return;

		this->__check_tunable_are_present(model_to_train);

		this->Wrapped->Reset();
		Training_set::subset Set(Train_set);
		list<Categoric_var*>	  order_of_variable_in_train_set;
		compute_order_of_variable_in_train_set(&order_of_variable_in_train_set, model_to_train, this->Get_names(&Set));
		float variation;
		for (unsigned int k = 0; k < Max_Iterations; k++) {
			variation = this->Wrapped->_advance(model_to_train, *this->Get_list(&Set), order_of_variable_in_train_set);
			if (descend_story != NULL) {
				float temp;
				model_to_train->Get_Likelihood_estimation(&temp, *this->Get_list(&Set), order_of_variable_in_train_set);
				descend_story->push_back(temp);
			}

			if (variation <= 1e-3) break;
		}

		this->Clean_Up();

	}

	void Stoch_Set_variation::Train(Graph_Learnable* model_to_train, Training_set* Train_set, const unsigned int& Max_Iterations, std::list<float>* descend_story) {

		if (!Train_set->Get_validity_flag()) return;

		this->__check_tunable_are_present(model_to_train);

		this->Wrapped->Reset();
		list<Categoric_var*>	  order_of_variable_in_train_set;
		compute_order_of_variable_in_train_set(&order_of_variable_in_train_set, model_to_train, this->Get_names(Train_set));
		float variation;
		for (unsigned int k = 0; k < Max_Iterations; k++) {
			Training_set::subset Set(Train_set, this->Percentage);
			variation = this->Wrapped->_advance(model_to_train, *this->Get_list(&Set), order_of_variable_in_train_set);
			if (descend_story != NULL) {
				float temp;
				model_to_train->Get_Likelihood_estimation(&temp, *this->Get_list(&Set), order_of_variable_in_train_set);
				descend_story->push_back(temp);
			}

			if (variation <= 1e-3) break;
		}

		this->Clean_Up();

	}

	float Fixed_step::_advance(Graph_Learnable* model_to_advance, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var) {

		list<float> grad_att;
		this->Get_w_grad(model_to_advance, &grad_att, comb_in_train_set, comb_var);

		list<float> w_att;
		Graph_Learnable::Weights_Manager::Get_tunable_w(&w_att, model_to_advance);

		auto it_w_grad = grad_att.begin();
		float delta_w = 0.f, temp;
		for (auto it_w = w_att.begin(); it_w != w_att.end(); it_w++) {
			temp = this->Step_to_advance * *it_w_grad;
			*it_w += temp;
			it_w_grad++;

			temp = abs(temp);
			if (temp > delta_w) delta_w = temp;
		}

		this->Set_w(w_att, model_to_advance);

		return delta_w;

	}

	void list_2_VectorXf(VectorXf* result, const list<float>& L) {

		*result = VectorXf(L.size());
		size_t k = 0;
		for (auto it = L.begin(); it != L.end(); it++) {
			(*result)(k) = *it;
			k++;
		}

	}

	void BFGS::__line_search(Graph_Learnable* model_to_advance, VectorXf& S, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var) {
 
		list<float> w_initial, w_new;
		Graph_Learnable::Weights_Manager::Get_tunable_w(&w_initial, model_to_advance);
		list<float>::iterator it_w, it_w2;
		for (it_w = w_initial.begin(); it_w != w_initial.end(); it_w++)
			w_new.push_back(1.f);
		list<size_t*>::const_iterator it_comb;

		float E_old = 0.f, E_new;
		float temp;
		for (it_comb = comb_in_train_set.begin(); it_comb != comb_in_train_set.end(); it_comb++) {
			model_to_advance->Eval_Log_Energy_function_normalized(&temp, *it_comb, comb_var);
			E_old += temp;
		}

		size_t k2;
		for (size_t k = 0; k < 5; k++) {
			S *= 0.5f;
			it_w2 = w_new.begin();
			k2 = 0;
			for (it_w = w_initial.begin(); it_w != w_initial.end(); it_w++) {
				*it_w2 = *it_w + S(k2);
				k2++;
				it_w2++;
			}
			this->Set_w(w_new, model_to_advance);

			E_new = 0.f;
			for (it_comb = comb_in_train_set.begin(); it_comb != comb_in_train_set.end(); it_comb++) {
				model_to_advance->Eval_Log_Energy_function_normalized(&temp, *it_comb, comb_var);
				E_new += temp;
			}

			if (E_new < E_old) {
				S *= 2.f;
				break;
			}
			E_old = E_new;
		}

		it_w2 = w_new.begin();
		k2 = 0;
		for (it_w = w_initial.begin(); it_w != w_initial.end(); it_w++) {
			*it_w2 = *it_w + S(k2);
			k2++;
			it_w2++;
		}
		this->Set_w(w_new, model_to_advance);

	}

	float BFGS::_advance(Graph_Learnable* model_to_advance, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var) {

		if (this->pData == NULL) {
			size_t model_size = model_to_advance->Get_model_size();
			this->pData = new Cache();

			this->pData->invB = MatrixXf::Identity(model_size, model_size);
			
			list<float> temp;
			this->Get_w_grad(model_to_advance, &temp, comb_in_train_set, comb_var);
			list_2_VectorXf(&this->pData->Grad_old, temp);
		}

		VectorXf S = this->pData->invB * this->pData->Grad_old; 
		this->__line_search(model_to_advance, S, comb_in_train_set, comb_var);

		list<float> grad_att;
		this->Get_w_grad(model_to_advance, &grad_att, comb_in_train_set, comb_var);
		VectorXf Grad_att;
		list_2_VectorXf(&Grad_att, grad_att);

		VectorXf Y = Grad_att - this->pData->Grad_old;
		this->pData->Grad_old = Grad_att;

		float dot_temp = Y.dot(S);
		MatrixXf temp = (1.f / dot_temp) * S * Y.transpose();
		temp *= -1.f;
		size_t k;
		for (k = 0; k < (int)temp.cols(); k++)
			temp(k, k) += 1.f;

		this->pData->invB = temp * this->pData->invB * temp.transpose();
		this->pData->invB += (1.f / dot_temp) * S * S.transpose();


		//EigenSolver<MatrixXf> solver(this->pData->invB);
		//VectorXcf eig_vals = solver.eigenvalues();
		//float max_eig = abs(eig_vals(0).real()), eig_temp;
		//for (k = 1; k < (size_t)eig_vals.size(); k++) {
		//	eig_temp = abs(eig_vals(k).real());
		//	if (eig_temp > max_eig)
		//		max_eig = eig_temp;
		//}
		//return max_eig;

		float min_S = abs(S(0)), temp_S;
		for (k = 1; k < (size_t)S.size(); k++) {
			temp_S = abs(S(k));
			if (S(k) < temp_S)
				min_S = S(k);
		}
		return 1000.f;

	}

	void BFGS::Reset() {

		if (this->pData != NULL)
			delete this->pData;
		this->pData = NULL;

	}





	I_Trainer* I_Trainer::Get_fixed_step(const float& step_size, const float& stoch_grad_percentage) {

		Advancer_Concrete* created = new Fixed_step(step_size);
		if (stoch_grad_percentage == 1.f) return new Entire_Set(created);
		else							  return new Stoch_Set_variation(created, stoch_grad_percentage);

	}

	I_Trainer* I_Trainer::Get_BFGS(const float& stoch_grad_percentage) {

		Advancer_Concrete* created = new BFGS();
		if (stoch_grad_percentage == 1.f) return new Entire_Set(created);
		else							  return new Stoch_Set_variation(created, stoch_grad_percentage);

	}

}

