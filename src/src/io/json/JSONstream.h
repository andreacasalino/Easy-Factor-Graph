// /**
//  * Author:    Andrea Casalino
//  * Created:   16.05.2019
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #ifndef GUI_JSON_STREAM_H_
// #define GUI_JSON_STREAM_H_

// #include <sstream>
// #include <memory>

// namespace EFG::io::json {
//     class streamJSON {
//     public:
//         std::string str() const;

//         void clear();

//         streamJSON(const streamJSON& o);
//         streamJSON& operator=(const streamJSON& o);

//         streamJSON(streamJSON&& o);
//         streamJSON& operator=(streamJSON&& o);

//         inline void addEndl() { (*this->stream) << std::endl; };

//         class Value;
//     protected:
//         streamJSON();

//         virtual char getDelimiterLeft() const = 0;
//         virtual char getDelimiterRight() const = 0;
//         void addElement(const std::string& value);

//     private:
//         bool                               firstElementFlag;
//         std::unique_ptr<std::stringstream> stream;
//     };

//     class streamJSON::Value {
//     public:
//         virtual std::string toString() const = 0;
//     };

//     class String : public streamJSON::Value {
//     public:
//         String(const std::string& rawValue);

//     private:
//         std::string toString() const final;
//         const std::string buffer;
//     };

//     template<typename T>
//     class Number : public streamJSON::Value {
//     public:
//         Number(const T& value) : value(value) {};

//     private:
//         inline std::string toString() const final { return
//         std::to_string(this->value); }; const T value;
//     };

//     class Null : public streamJSON::Value {
//     public:
//         Null() = default;

//     private:
//         inline std::string toString() const final { return "null"; };
//     };

//     class structJSON : public streamJSON {
//     public:
//         structJSON() = default;

//         void addElement(const std::string& name, const streamJSON& json);
//         void addElement(const std::string& name, const Value& value);

//     private:
//         inline char getDelimiterLeft() const final { return '{'; };
//         inline char getDelimiterRight() const final { return '}'; };
//     };

//     class arrayJSON : public streamJSON {
//     public:
//         arrayJSON() = default;

//         void addElement(const streamJSON& json);
//         void addElement(const Value& value);

//     private:
//         inline char getDelimiterLeft() const final { return '['; };
//         inline char getDelimiterRight() const final { return ']'; };

//     };
// }

// #endif
