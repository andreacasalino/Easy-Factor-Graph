/**
 * Author:    Andrea Casalino
 * Created:   16.05.2019
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "JSONstream.h"

namespace EFG::io::json {
    streamJSON::streamJSON()
        : firstElementFlag(true) {
        this->stream = std::make_unique<std::stringstream>();
    }

    streamJSON::streamJSON(const streamJSON& o)
        : streamJSON() {
        *this = o;
    };

    streamJSON& streamJSON::operator=(const streamJSON& o) {
        this->firstElementFlag = o.firstElementFlag;
        *this->stream << o.stream->str();
        return *this;
    };

    streamJSON::streamJSON(streamJSON&& o)
        : streamJSON() {
        *this = std::move(o);
    };

    streamJSON& streamJSON::operator=(streamJSON&& o) {
        this->firstElementFlag = o.firstElementFlag;
        this->stream = std::move(o.stream);
        o.stream = std::make_unique<std::stringstream>();
        return *this;
    };

    std::string streamJSON::str() const {
        std::stringstream temp;
        temp << this->getDelimiterLeft() << this->stream->str() << this->getDelimiterRight();
        return temp.str();
    }

    void streamJSON::clear() {
        this->stream = std::make_unique<std::stringstream>();
        this->firstElementFlag = true;
    }

    void streamJSON::addElement(const std::string& value) {
        if (this->firstElementFlag) {
            this->firstElementFlag = false;
        }
        else {
            *this->stream << ",";
        }
        *this->stream << value;
    }


    String::String(const std::string& rawValue)
        : buffer(rawValue) {
    }

    std::string String::toString() const {
        std::stringstream temp;
        temp << "\"" << this->buffer << "\"";
        return temp.str();
    }


    void structJSON::addElement(const std::string& name, const streamJSON& json) {
        std::stringstream temp;
        temp << "\"" << name << "\"" << ":" << json.str();
        this->streamJSON::addElement(temp.str());
    }

    void structJSON::addElement(const std::string& name, const Value& value) {
        std::stringstream temp;
        temp << "\"" << name << "\"" << ":" << value.toString();
        this->streamJSON::addElement(temp.str());
    }


    void arrayJSON::addElement(const streamJSON& json) {
        this->streamJSON::addElement(json.str());
    }

    void arrayJSON::addElement(const Value& value) {
        this->streamJSON::addElement(value.toString());
    }
}
