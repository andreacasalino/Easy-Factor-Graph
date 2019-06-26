#include "../Header/Training_set.h"
#include "../XML_reader/XML_importer.h"
#include <string>
#include <fstream>
using namespace std;

namespace Segugio {

	Training_set::~Training_set() {

		for (auto it = this->Set.begin(); it != this->Set.end(); it++)
			free(*it);

	}

	Training_set::Training_set(const std::string& file_to_import) {

		ifstream f_set(file_to_import);
		if (!f_set.is_open()) {
			system("ECHO impossible to open training set file");
			abort();
		}

		string line;
		list<string> slices;

		if (f_set.eof()) {
			system("ECHO empty training set");
			abort();
		}

		getline(f_set, line);
		XML_reader::splitta_riga(line, &this->Variable_names);
		size_t N_vars = this->Variable_names.size();
		
		if (f_set.eof()) {
			system("ECHO empty training set");
			abort();
		}

		size_t line_cont = 2, k;
		while (!f_set.eof()) {
			getline(f_set, line);
			slices.clear();
			XML_reader::splitta_riga(line, &slices);

			if (slices.size() != N_vars) {
				system(string("ECHO inconsistent data at line " + to_string(line_cont)).c_str());
				abort();
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
			system("ECHO empty train set parsed");
			abort();
		}

	}

	void Training_set::Print(const std::string& file_name) {

		ofstream f(file_name);
		if (!f.is_open()) {
			system("ECHO inexistent file");
			abort();
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
		pVariable_names(&set->Variable_names) {

		if ((size_percentage < 0.f) || (size_percentage > 1.f)) {
			system("ECHO invalid percentage for computing training subset");
			abort();
		}

		if (size_percentage == 1.f)
			Sub_Set = set->Set;
		else {
			size_t subset_size = (size_t)floor(set->Set.size() * size_percentage);
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

