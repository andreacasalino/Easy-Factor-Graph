/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_ERROR_H
#define EFG_ERROR_H

#include <stdexcept>
#include <sstream>

namespace EFG {
    /** @brief A runtime error that can be raised when using any object in EFG::
	 */
    class Error : public std::runtime_error {
    public:
        Error(const std::string& what);

        explicit Error(const std::string& objectName, const std::string& what);

        template<typename ... Args>
        Error(const std::string& objectName, Args ... args)
            : Error(objectName, MergeArgs(args...)) {
        };
    private:
        template<typename ... Args>
        static std::string MergeArgs(Args ... args){
            std::stringstream stream;
            mergeArgs(stream, args...);
            return stream.str();
        };
        template<typename ... Args>
        static void mergeArgs(std::stringstream& stream, const std::string& a, Args ... args){
            stream << a;
            mergeArgs(stream, args...);
        };
        static void mergeArgs(std::stringstream& stream, const std::string& a);
        template<typename ... Args>
        static void mergeArgs(std::stringstream& stream, const char* a, Args ... args){
            stream << a;
            mergeArgs(stream, args...);
        };
        static void mergeArgs(std::stringstream& stream, const char* a);
    };
}

#endif