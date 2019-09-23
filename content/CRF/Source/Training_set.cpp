#include "../Header/Training_set.h"
#include "../../thirdy_part_lib/XML_reader/XML_Manager.h"
#include <string>
#include <fstream>
using namespace std;

namespace Segugio {

	std::list<std::string> extract_names(const std::list<Categoric_var*>& variable_in_the_net) {

		list<string> names;
		for (auto it = variable_in_the_net.begin(); it != variable_in_the_net.end(); it++)
			names.push_back((*it)->Get_name());
		return names;

	}

	Training_set::~Training_set() {

		for (auto it = this->Set.begin(); it != this->Set.end(); it++)
			free(*it);

	}

	Training_set::Training_set(const std::string& file_to_import) {

		this->Is_training_set_valid = false;

		ifstream f_set(file_to_import);
		if (!f_set.is_open()) {
#ifdef _DEBUG
			system("ECHO impossible to open training set file");
#endif // DEBUG
			f_set.close();
			return;
		}

		string line;
		list<string> slices;

		if (f_set.eof()) {
#ifdef _DEBUG
			system("ECHO found empty training set");
#endif // DEBUG
			f_set.close();
			return;
		}

		getline(f_set, line);
		splitta_riga(line, &this->Variable_names);
		size_t N_vars = this->Variable_names.size();

		if (f_set.eof()) {
#ifdef _DEBUG
			system("ECHO found empty training set");
#endif // DEBUG
			f_set.close();
			return;
		}

		size_t line_cont = 2, k;
		while (!f_set.eof()) {
			getline(f_set, line);
			slices.clear();
			splitta_riga(line, &slices);

			if (slices.size() != N_vars) {
#ifdef _DEBUG
				system(string("ECHO inconsistent data when reading training set at line " + to_string(line_cont)).c_str());
#endif // DEBUG
				f_set.close();
				return;
			}

			this->Set.push_back(NULL);
			this->Set.back() = (size_t*)malloc(sizeof(size_t)*N_vars);
			k = 0;
			while (!slices.empty()) {
				this->Set.back()[k] = (size_t)atoi(slices.front().c_str());
				slices.pop_front();
				k++;
			}
			line_cont++;
		}

		f_set.close();

		if (this->Set.empty()) {
#ifdef _DEBUG
			system("ECHO found empty training set");
#endif // DEBUG
			return;
		}

		this->Is_training_set_valid = true;

	}

	void Training_set::Print(const std::string& file_name) {

		ofstream f(file_name);
		if (!f.is_open()) {
#ifdef _DEBUG
			system("ECHO inexistent file to print training set");
#endif
			return;
			f.close();
		}

		size_t N_vars = this->Variable_names.size(), k;

		auto itn = this->Variable_names.begin();
		f << *itn;
		itn++;
		for (itn; itn != this->Variable_names.end(); itn++)
			f << " " << *itn;
		f << endl;
		auto it_end = this->Set.end(); 
		it_end--;
		for (auto it = this->Set.begin(); it != this->Set.end(); it++) {
			f << (*it)[0];
			for (k = 1; k < N_vars; k++)
				f << " " << (*it)[k];

			if(it != it_end)
				f << endl;
		}

		f.close();

	}

	Training_set::subset::subset(Training_set* set, const float& size_percentage):
		pVariable_names(&set->Variable_names), Is_sub_set_valid(set->Is_training_set_valid) {

		if (!this->Is_sub_set_valid) {
#ifdef _DEBUG
			system("ECHO asked to create a sub set from an invalid training set");
#endif // DEBUG
			return;
		}

		float percentage_to_use = size_percentage;
		if (size_percentage < 0.f) {
#ifdef _DEBUG
			system("ECHO percentage for computing training subset must be at least equal to 0.0001");
#endif // DEBUG
			percentage_to_use = 0.0001f;
		}
		if (size_percentage > 1.f) {
#ifdef _DEBUG
			system("ECHO percentage for computing training subset must be lower than 1.0, 1.0 is assumed");
#endif // DEBUG
			percentage_to_use = 1.f;
		}

		if (percentage_to_use == 1.f)
			Sub_Set = set->Set;
		else {
			size_t subset_size = (size_t)floor(set->Set.size() * percentage_to_use);
			if (subset_size == 0) subset_size = 1;

			auto open_set = set->Set;
			size_t pos;
			list<size_t*>::iterator it_set;
			for (size_t k = 0; k < subset_size; k++) {
				pos = (size_t)(rand() % (int)open_set.size());
				it_set = open_set.begin();
				advance(it_set, pos);
				this->Sub_Set.push_back(*it_set);
				open_set.erase(it_set);
			}
		}

	}

}

