#include "../Header/Command.h"
using namespace std;


Command::Command(const std::string& raw) : Name('-') {

	list<size_t> positions;
	this->__find_separators(&positions, raw);
	if (positions.empty()) {
		if (raw.size() == 1)
			this->Name = raw[0];
	}
	else {
		if (positions.size() % 2 != 0)
			return;
		if (positions.front() != 1)
			return;

		this->Name = raw[0];

		size_t p1, p2, p3, k;
		auto it_opt = this->Options.begin();
		__option* existing_opt = NULL;
		string value;
		while (!positions.empty()) {
			p1 = positions.front(); positions.pop_front();
			p2 = positions.front(); positions.pop_front();
			if ((p2 - p1) == 2) {
				existing_opt = NULL;
				for (it_opt = this->Options.begin(); it_opt != this->Options.end(); it_opt++) {
					if (it_opt->name == raw[p1 + 1]) {
						existing_opt = &(*it_opt);
						break;
					}
				}

				if (existing_opt == NULL) {
					this->Options.push_back(__option());
					this->Options.back().name = raw[p1 + 1];
					existing_opt = &this->Options.back();
				}

				if (positions.empty()) p3 = raw.size();
				else p3 = positions.front();

				value.clear();
				k = p2;
				k++;
				for (k; k < p3; k++) {
					if ((raw[k] != '\n') && (raw[k] != ' '))
						value.push_back(raw[k]);
				}
				existing_opt->values.push_back(value);
			}
		}
	}

}

const std::vector<std::string>* Command::Get_values(const char& option_name) const {

	for (auto it = this->Options.begin(); it != this->Options.end(); it++) {
		if (it->name == option_name)
			return &it->values;
	}
	return NULL;

}

const std::string*	 Command::Get_value(const char& option_name) const {

	auto vals = this->Get_values(option_name);
	if (vals == NULL) return NULL;
	return &vals->front();

}

void Command::__find_separators(std::list<std::size_t>* positions, const std::string& word) {

	positions->clear();
	for (size_t k = 0; k < word.size(); k++) {
		if (word[k] == '$')
			positions->push_back(k);
	}

}

void Command::Print() const {

	string to_print;

	to_print = "Name:";
	to_print += " ";
	to_print.back() = this->Name;
	system(string("ECHO " + to_print).c_str());

	for (auto it = this->Options.begin(); it != this->Options.end(); it++) {
		to_print.clear();
		to_print = "Option:";
		to_print += " ";
		to_print.back() = it->name;
		to_print += ",[";
		if (!it->values.empty()) {
			auto itt = it->values.begin();
			to_print += *itt;
			itt++;
			for (itt; itt != it->values.end(); itt++)
				to_print += "," + *itt;
		}
		to_print += "]";
		system(string("ECHO " + to_print).c_str());
	}

}




std::string char_pt_2_string(char* buffer, const size_t& Size) {

	string message;
	for (size_t k = 0; k < Size; k++)
		message.push_back(buffer[k]);
	return message;

}