#include "XML_importer.h"
#include <iostream>
#include <sstream>




//TODO: non metterle globali
std::string line_buffer;
std::list<std::string> slices;
int XML_reader::line = 0;

XML_reader::XML_reader(std::string name_file) {

	XML_reader::line = 0;
	std::ifstream f(name_file);

	if (!f.is_open()) {
		std::cout << "Error in XML_reader::XML_reader, cannot find " << name_file << std::endl;
		abort(); }

	line = 0;
	std::getline(f, line_buffer); line++; //skip pramble 
	std::getline(f, line_buffer); line++;
	XML_reader::splitta_riga(line_buffer, &slices);

	this->Tree_content = new Tag(f);
	if (!f.eof()) {
		std::cout << "Error in XML_reader::XML_reader, multiple root tag founded \n";
		abort(); }
	f.close();

}

void XML_reader::splitta_riga(std::string& riga, std::list<std::string>* slices) {

	std::istringstream iss(riga);
	slices->clear();
	while (true) {
		if (iss.eof()) {
			break;
		}
		slices->push_back(std::string());
		iss >> slices->back();
	}

}

XML_reader::Tag_readable XML_reader::Get_root() {

	Tag_readable temp(this->Tree_content);
	return temp;
}

XML_reader::Tag::Tag(std::ifstream& f) {

	int line_open = XML_reader::line;
	this->line_in_file = line_open;
	if (slices.front().front() != '<') {
		std::cout << "Error in XML_reader::XML_reader, line begin without < , line " << line << "\n";
		abort();
	}
	//read tag name
	bool terminator_found = false;
	if (slices.size() == 1) {
		if (slices.front().back() != '>') {
			std::cout << "Error in XML_reader::XML_reader, tag terminator not found " << line << "\n";
			abort();
		}
		this->name = std::string(slices.front(), 1, (int)slices.front().size() - 2);
		slices.pop_front();
		slices.push_back(">");
	}
	else {
		this->name = std::string(slices.front(), 1);
		slices.pop_front();
		if (slices.back().size() >= (this->name.size() + 3)) {
			//chek if terminator tag is contained in last slice
			std::string temp(slices.back(), (int)slices.back().size() - ((int)this->name.size() + 3));
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
		std::cout << "Error in XML_reader::XML_reader, tag terminator not found " << line << "\n";
		abort();
	}
	slices.back().pop_back();
	if (slices.back().empty())
		slices.pop_back();

	//read nested words
	for (std::list<std::string>::iterator it_word = slices.begin(); it_word != slices.end(); it_word++)
		this->Extract_word(*it_word);
	if (terminator_found)
		return;

	//read nested elements
	while (!f.eof()) {
		std::getline(f, line_buffer); XML_reader::line++;
		XML_reader::splitta_riga(line_buffer, &slices);

		if (slices.front().compare("</" + this->name + ">") == 0)
			return;

		this->nested_tag.push_back(new Tag(f));
	}

	std::cout << "Error in XML_reader::XML_reader, tag open at line " << line_open << " not properly closed \n";
	abort();

}

void XML_reader::Tag::Extract_word(std::string& raw) {

	this->fields.push_back(Field());
	int pos_equal = 0;
	for (int k = 0; k < raw.size(); k++) {
		if (raw[k] == '=') {
			pos_equal = k;
			break;
		}
	}

	if (pos_equal == 0) {
		std::cout << "Error in XML_reader::Tag::Extract_word, invalid word at line " << XML_reader::line << std::endl;
		abort();
	}

	this->fields.back().name = std::string(raw, 0, pos_equal);
	std::string temp = std::string(raw, pos_equal + 1);

	if ((temp.front() != '\"') || (temp.back() != '\"')) {
		std::cout << "Error in XML_reader::Tag::Extract_word, word not delimited by \" at line " << XML_reader::line << std::endl;
		abort();
	}

	this->fields.back().content = std::string(temp, 1, temp.size() - 2);

}

XML_reader::Tag::~Tag() {

	for (std::list<Tag*>::iterator it_t = this->nested_tag.begin(); it_t != this->nested_tag.end(); it_t++)
		delete *it_t;

}







bool XML_reader::Tag_readable::Exist_Nested_tag(const std::string& name_nested) {

	for (auto it = this->encapsulated->nested_tag.begin(); it != this->encapsulated->nested_tag.end(); it++) {
		if ((*it)->name.compare(name_nested) == 0) return true;
	}
	return false;

}

void XML_reader::Tag_readable::Get_Nested(const std::string& name_nested, std::list<Tag_readable>* nested) {

	nested->clear();
	for (std::list<Tag*>::iterator it = this->encapsulated->nested_tag.begin(); it != this->encapsulated->nested_tag.end(); it++) {
		if ((*it)->name.compare(name_nested) == 0) nested->push_back(Tag_readable(*it));
	}

}

std::list<XML_reader::Tag_readable>	XML_reader::Tag_readable::Get_Nested_fast(const std::string& name_nested) {

	std::list<Tag_readable> temp;
	this->Get_Nested(name_nested, &temp);
	return temp;

}

XML_reader::Tag_readable XML_reader::Tag_readable::Get_Nested(const std::string& name_nested) {

	XML_reader::Tag_readable res;
	for (std::list<Tag*>::iterator it = this->encapsulated->nested_tag.begin(); it != this->encapsulated->nested_tag.end(); it++) {
		if ((*it)->name.compare(name_nested) == 0) {
			res.encapsulated = *it;
			return res;
		}
	}
	std::cout << "not able to find nested tag " << name_nested << " in tag " << this->encapsulated->name << " at line " << this->encapsulated->line_in_file << std::endl;
	abort();
	return res;

}

XML_reader::Tag_readable XML_reader::Tag_readable::Get_Nested(std::list<std::string> path) {

	Tag* cursor = this->encapsulated;
	for (auto it = path.begin(); it != path.end(); it++) {
		Tag_readable attual(cursor);
		attual = attual.Get_Nested(*it);
		cursor = attual.encapsulated;
	}

	return Tag_readable(cursor);

}





bool XML_reader::Tag_readable::Exist_Field(const std::string& name_field) {

	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name.compare(name_field) == 0) return true;
	}
	return false;

}

std::string XML_reader::Tag_readable::Get_value(const std::string& name) {

	for (std::list<Tag::Field>::iterator it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name.compare(name) == 0) {
			return it->content;
		}
	}
	std::cout << "not able to find field " << name << " in tag " << this->encapsulated->name << " at line " << this->encapsulated->line_in_file << std::endl;
	abort();
	return std::string("");

}

std::list<std::string>  XML_reader::Tag_readable::Get_values(std::list<std::string>& field_names) {
	
	std::list<std::string> values;
	this->Get_values(field_names, &values);
	return values;

}

void  XML_reader::Tag_readable::Get_values(std::list<std::string>& field_names, std::list<std::string>* fields) {

	fields->clear();
	for (std::list<std::string>::iterator it = field_names.begin(); it != field_names.end(); it++) {
		fields->push_back(this->Get_value(*it));
	}

}

void  XML_reader::Tag_readable::Get_all_values(std::list<std::string>* field_names, std::list<std::string>* fields) {
	
	field_names->clear();
	fields->clear();
	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		field_names->push_back(it->name);
		fields->push_back(it->content);
	}

}

std::list<std::string>  XML_reader::Tag_readable::Get_values_specific_field_name(const std::string& field_name) {

	std::list<std::string> vals;
	this->Get_values_specific_field_name(field_name, &vals);
	return vals;

}

void  XML_reader::Tag_readable::Get_values_specific_field_name(const std::string& field_name, std::list<std::string>* results) {

	results->clear();
	for (auto it = this->encapsulated->fields.begin(); it != this->encapsulated->fields.end(); it++) {
		if (it->name.compare(field_name) == 0) {
			results->push_back(it->content);
		}
	}

}