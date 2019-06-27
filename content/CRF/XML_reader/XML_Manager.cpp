// Author: Andrea Casalino
// mail: andrecasa91@gmail.com

#include "XML_Manager.h"
#include <iostream>
#include <sstream>
using namespace std;


void splitta_riga(string& riga, list<string>* slices) {

	istringstream iss(riga);
	slices->clear();
	while (true) {
		if (iss.eof()) {
			break;
		}
		slices->push_back(string());
		iss >> slices->back();
	}

}

XML_reader::XML_reader(string name_file) {

	int line = 0;
	ifstream f(name_file);

	if (!f.is_open()) {
		cout << "Error in XML_reader::XML_reader, cannot find " << name_file << endl;
		abort(); }

	string temp;
	list<string> slices;
	getline(f, temp); line++; //skip pramble 
	getline(f, temp); line++;
	splitta_riga(temp, &slices);

	this->Tree_content = new Tag(f, &line, slices);
	if (!f.eof()) {
		cout << "Error in XML_reader::XML_reader, multiple root tag founded \n";
		abort(); }
	f.close();

}



XML_reader::Tag_readable XML_reader::Get_root() {

	Tag_readable temp(this->Tree_content);
	return temp;
}

XML_reader::Tag::Tag(ifstream& f, int* line, std::list<std::string>& slices, Tag* generating_father) : father(generating_father) {

	int line_open = *line;	
	this->line_in_file = line_open;
	if (slices.front().front() != '<') {
		cout << "Error in XML_reader::XML_reader, line begin without < , line " << *line << "\n";
		abort();
	}
	//read tag name
	bool terminator_found = false;
	if (slices.size() == 1) {
		if (slices.front().back() != '>') {
			cout << "Error in XML_reader::XML_reader, tag terminator not found " << *line << "\n";
			abort();
		}
		this->name = string(slices.front(), 1, (int)slices.front().size() - 2);
		slices.pop_front();
		slices.push_back(">");
	}
	else {
		this->name = string(slices.front(), 1);
		slices.pop_front();
		if (slices.back().size() >= (this->name.size() + 3)) {
			//chek if terminator tag is contained in last slice
			string temp(slices.back(), (int)slices.back().size() - ((int)this->name.size() + 3));
			if (temp.compare("</" + this->name + ">") == 0) {
				terminator_found = true;
				for (int k = 0; k < ((int)this->name.size() + 3); k++)
					slices.back().pop_back();
				if (slices.back().empty())
					slices.pop_back();
			}
		}
	}

	if (slices.back().back() != '>') {
		cout << "Error in XML_reader::XML_reader, tag terminator not found " << *line << "\n";
		abort();
	}
	slices.back().pop_back();
	if (slices.back().empty())
		slices.pop_back();

	//read nested words
	for (list<string>::iterator it_word = slices.begin(); it_word != slices.end(); it_word++)
		this->Extract_word(*it_word, line);
	if (terminator_found)
		return;

	//read nested elements
	string line_new;
	list<string> slices_new;
	while (!f.eof()) {
		getline(f, line_new);
		(*line)++;
		splitta_riga(line_new, &slices_new);

		if (slices_new.front().compare("</" + this->name + ">") == 0)
			return;

		this->nested_tag.push_back(new Tag(f, line, slices_new, this));
	}

	cout << "Error in XML_reader::XML_reader, tag open at line " << line_open << " not properly closed \n";
	abort();

}

void XML_reader::Tag::Extract_word(string& raw, int* line) {

	this->fields.push_back(Field());
	int pos_equal = 0;
	for (int k = 0; k < raw.size(); k++) {
		if (raw[k] == '=') {
			pos_equal = k;
			break;
		}
	}

	if (pos_equal == 0) {
		cout << "Error in XML_reader::Tag::Extract_word, invalid word at line " << *line << endl;
		abort();
	}

	this->fields.back().name = string(raw, 0, pos_equal);
	string temp = string(raw, pos_equal + 1);

	if ((temp.front() != '\"') || (temp.back() != '\"')) {
		cout << "Error in XML_reader::Tag::Extract_word, word not delimited by \" at line " << *line << endl;
		abort();
	}

	this->fields.back().content = string(temp, 1, temp.size() - 2);

}

XML_reader::Tag::~Tag() {

	for (list<Tag*>::iterator it_t = this->nested_tag.begin(); it_t != this->nested_tag.end(); it_t++)
		delete *it_t;

}

void XML_reader::Reprint(const std::string& file_name) {

	ofstream f(file_name);
	if (!f.is_open()) {
		system("ECHO file not found");
		abort();
	}

	this->Reprint(f);
	f.close();

}

void XML_reader::Reprint(std::ostream& stream_to_use) {

	stream_to_use << "<?xml version=\"1.0\"?>\n";
	this->Tree_content->Reprint(stream_to_use, "", true);

}

void XML_reader::Tag::Reprint(std::ostream& stream_to_use, const std::string& space_to_use, const bool& is_the_root) {

	stream_to_use << space_to_use << "<" << this->name << "";
	for (auto it = this->fields.begin(); it != this->fields.end(); it++) {
		stream_to_use << " " << it->name << "=\"" << it->content << "\"";
	}
	stream_to_use << ">";

	if (!this->nested_tag.empty()) {
		stream_to_use << endl;
		for (auto it = this->nested_tag.begin(); it != this->nested_tag.end(); it++)
			(*it)->Reprint(stream_to_use, space_to_use + "  ", false);
		stream_to_use << space_to_use;
	}

	stream_to_use << "</" << this->name << ">";
	if (!is_the_root)
		stream_to_use << endl;

}







bool XML_reader::Tag_readable::Exist_Nested_tag(const string& name_nested) {

	for (auto it = this->encapsulated->nested_tag.begin(); it != this->encapsulated->nested_tag.end(); it++) {
		if ((*it)->name.compare(name_nested) == 0) return true;
	}
	return false;

}

void XML_reader::Tag_readable::Get_Nested(const string& name_nested, list<Tag_readable>* nested) {

	nested->clear();
	for (list<Tag*>::iterator it = this->encapsulated->nested_tag.begin(); it != this->encapsulated->nested_tag.end(); it++) {
		if ((*it)->name.compare(name_nested) == 0) nested->push_back(Tag_readable(*it));
	}

}

list<XML_reader::Tag_readable>	XML_reader::Tag_readable::Get_Nested_fast(const string& name_nested) {

	list<Tag_readable> temp;
	this->Get_Nested(name_nested, &temp);
	return temp;

}

XML_reader::Tag_readable XML_reader::Tag_readable::Get_Nested(const string& name_nested) {

	XML_reader::Tag_readable res;
	for (list<Tag*>::iterator it = this->encapsulated->nested_tag.begin(); it != this->encapsulated->nested_tag.end(); it++) {
		if ((*it)->name.compare(name_nested) == 0) {
			res.encapsulated = *it;
			return res;
		}
	}
	cout << "not able to find nested tag " << name_nested << " in tag " << this->encapsulated->name << " at line " << this->encapsulated->line_in_file << endl;
	abort();
	return res;

}

XML_reader::Tag_readable XML_reader::Tag_readable::Get_Nested(list<string> path) {

	Tag* cursor = this->encapsulated;
	for (auto it = path.begin(); it != path.end(); it++) {
		Tag_readable attual(cursor);
		attual = attual.Get_Nested(*it);
		cursor = attual.encapsulated;
	}

	return Tag_readable(cursor);

}





bool XML_reader::Tag_readable::Exist_Field(const string& name_field) {

	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name.compare(name_field) == 0) return true;
	}
	return false;

}

string XML_reader::Tag_readable::Get_value(const string& name) {

	for (list<Tag::Field>::iterator it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name.compare(name) == 0) {
			return it->content;
		}
	}
	cout << "not able to find field " << name << " in tag " << this->encapsulated->name << " at line " << this->encapsulated->line_in_file << endl;
	abort();
	return string("");

}

list<string>  XML_reader::Tag_readable::Get_values(list<string>& field_names) {
	
	list<string> values;
	this->Get_values(field_names, &values);
	return values;

}

void  XML_reader::Tag_readable::Get_values(list<string>& field_names, list<string>* fields) {

	fields->clear();
	for (list<string>::iterator it = field_names.begin(); it != field_names.end(); it++) {
		fields->push_back(this->Get_value(*it));
	}

}

void  XML_reader::Tag_readable::Get_all_values(list<string>* field_names, list<string>* fields) {
	
	field_names->clear();
	fields->clear();
	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		field_names->push_back(it->name);
		fields->push_back(it->content);
	}

}

list<string>  XML_reader::Tag_readable::Get_values_specific_field_name(const string& field_name) {

	list<string> vals;
	this->Get_values_specific_field_name(field_name, &vals);
	return vals;

}

void  XML_reader::Tag_readable::Get_values_specific_field_name(const string& field_name, list<string>* results) {

	results->clear();
	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name.compare(field_name) == 0) {
			results->push_back(it->content);
		}
	}

}






void XML_reader::Tag_readable::Set_field_name(const std::string& field_name, const std::string& new_name) {

	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name == field_name)
			it->name = new_name;
	}

}

void XML_reader::Tag_readable::Set_field_content(const std::string& field_name, const std::list<std::string>& new_vals) {

	list<string*> val_to_change;
	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name == field_name) 
			val_to_change.push_back(&it->content);
	}

	if (val_to_change.size() != new_vals.size()) {
		system("ECHO wrong number of values");
		abort();
	}

	auto it = new_vals.begin();
	for (auto it_v = val_to_change.begin(); it_v != val_to_change.end(); it_v++) {
		**it_v = *it;
		it++;
	}

}

void XML_reader::Tag_readable::Remove_field(const std::string& field_name, const std::string& value) {

	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if ((it->name == field_name) && (it->content == value)) {
			it = this->encapsulated->fields.erase(it);
			return;
		}
	}

	system("ECHO inexostent field");
	abort();

}

void XML_reader::Tag_readable::Remove_field(const std::string& field_name) {

	auto it = this->encapsulated->fields.begin();
	while (it != this->encapsulated->fields.end()) {
		if (it->name == field_name)
			it = this->encapsulated->fields.erase(it);
		else
			it++;
	}

}

void XML_reader::Tag_readable::Add_field(const std::string& field_name, const std::string& content) {

	this->encapsulated->fields.push_back(Tag::Field());
	this->encapsulated->fields.back().name = field_name;
	this->encapsulated->fields.back().content = content;

}

XML_reader::Tag_readable XML_reader::Tag_readable::Add_Nested_and_return_created(const std::string& tag_name) {

	Tag* created = new Tag(tag_name);
	this->encapsulated->nested_tag.push_back(created);
	return Tag_readable(created);

}

void XML_reader::Tag_readable::Add_Nested(const std::string& tag_name) {

	this->Add_Nested_and_return_created(tag_name);

}

void XML_reader::Tag_readable::Remove() {

	if (this->encapsulated->father == NULL) {
		system("ECHO You cannot remove the root");
		abort();
	}

	for (auto it = this->encapsulated->father->nested_tag.begin();
		it != this->encapsulated->father->nested_tag.end(); it++) {
		if (*it == this->encapsulated) {
			it = this->encapsulated->father->nested_tag.erase(it);
			break;
		}
	}

	delete this->encapsulated;

	this->encapsulated = NULL;

}