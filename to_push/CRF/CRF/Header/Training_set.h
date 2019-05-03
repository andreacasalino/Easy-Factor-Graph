//Author: Andrea Casalino
//mail: andrecasa91@gmail.com

#pragma once
#ifndef __CRF_TRAINING_SET_H__
#define __CRF_TRAINING_SET_H__

#include "Potential.h"

namespace Segugio {

	class Training_set {
	public:
		Training_set(const std::string& file_to_import);
		Training_set(const std::string& file_to_import, const bool& display_after_parsing); //mainly for debug
		~Training_set();

		struct subset {
		public:
			subset(Training_set* set, const float& size_percentage = 1.f);

			struct Handler {
			protected:
				static std::list<size_t*>*		Get_list(subset* sub_set) { return &sub_set->Sub_Set; };
				static std::list<std::string>*	Get_names(subset* sub_set) { return sub_set->pVariable_names; };
				static std::list<std::string>*	Get_names(Training_set* set) { return &set->Variable_names; };
			};

			//void Get_pos_of_var_in_set(std::list<size_t>* result, const std::list<Categoric_var*>& variables);
		private:
			std::list<std::string>*		pVariable_names;
			std::list<size_t*>			Sub_Set;
		};
	private:
	// data
		std::list<std::string>	Variable_names;
		std::list<size_t*>		Set;
	};

}

#endif