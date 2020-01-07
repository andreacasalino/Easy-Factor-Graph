#ifndef __JSON_H__
#define __JSON_H__

#include <string>
#include <list>

class I_JSON_composite {
public:
	virtual ~I_JSON_composite();
	void stringify(std::string* result) { result->clear(); this->__stringify(result); };
protected:
	I_JSON_composite(const I_JSON_composite&) { abort(); };
	I_JSON_composite() {};

	virtual I_JSON_composite*  __copy() const = 0;
	virtual void __stringify(std::string* result) = 0;
	void __clone_nested(I_JSON_composite* to_clone);

	I_JSON_composite* ___copy(const I_JSON_composite* to_copy) { return to_copy->__copy(); };
	void							 ___stringify(std::string* result, I_JSON_composite* to_str) { to_str->__stringify(result); };

	std::list<I_JSON_composite*>	  Nested_elements;
};

class JSON_tag : public I_JSON_composite {
public:
	JSON_tag() {};

	void Add_field(const std::string& name, const std::string& value);
	void Add_nested(const std::string& name, const I_JSON_composite& to_nest);
private:
	virtual I_JSON_composite*  __copy() const;
	virtual void __stringify(std::string* result);

	struct __field {
		std::string name;
		std::string value;
	};

	std::list<std::string>	 Nested_names;
	std::list<__field>			 Nested_fields;
};

class JSON_array : public I_JSON_composite {
public:
	JSON_array() {};

	void Append(const I_JSON_composite& to_nest);
private:
	virtual I_JSON_composite*  __copy() const;
	virtual void __stringify(std::string* result);
};

//[1,2,3,5,2,4.4,6]
class JSON_numerical_array : public I_JSON_composite {
public:
	JSON_numerical_array() {};
	JSON_numerical_array(const std::list<float>& vals) { this->Values = vals; };

	void Append(const float& to_nest) { this->Values.push_back(to_nest); };
private:
	JSON_numerical_array(const JSON_numerical_array& to_clone) { this->Values = to_clone.Values; };

	virtual I_JSON_composite*  __copy() const { return new JSON_numerical_array(*this); };
	virtual void __stringify(std::string* result);

	std::list<float>			Values;
};

#endif