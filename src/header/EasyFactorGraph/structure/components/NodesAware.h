// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #ifndef EFG_STRUCTURE_NODES_AWARE_H
// #define EFG_STRUCTURE_NODES_AWARE_H

// #include <structure/Node.h>
// #include <Component.h>

// namespace EFG::strct {
//     class NodesAware : virtual public Component {
//     public:
//         /**
//          * @return all the variables (hidden or observed) in the model
//          */
//         std::set<categoric::VariablePtr> getVariables() const;

//         categoric::VariablePtr findVariable(const std::string& name) const;

//     protected:
//         /**
//          * @brief The set of variables part of the model, with the
//          connectivity information
//          */
//         std::map<categoric::VariablePtr, Node> nodes;
//     };
// }

// #endif
