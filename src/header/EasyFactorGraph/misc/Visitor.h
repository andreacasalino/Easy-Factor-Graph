/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <functional>
#include <variant>

namespace EFG {
namespace detail {
template <typename T> using Predicate = std::function<void(T &)>;

template <typename T> struct Node { Predicate<T> pred_; };
} // namespace detail

template <typename... Ts> struct Visitor : public detail::Node<Ts>... {
  Visitor(detail::Predicate<Ts> &&...predicates) {
    this->template set<Ts...>(
        std::forward<detail::Predicate<Ts>>(predicates)...);
  }

  template <typename T> void operator()(T &subject) const {
    this->detail::Node<T>::pred_(subject);
  }

  void visit(std::variant<Ts...> &subject) const { std::visit(*this, subject); }

  template <typename T, typename... Tothers>
  void set(detail::Predicate<T> &&first,
           detail::Predicate<Tothers> &&...preds) {
    this->detail::Node<T>::pred_ = std::forward<detail::Predicate<T>>(first);
    this->template set<Tothers...>(
        std::forward<detail::Predicate<Tothers>>(preds)...);
  }

  template <typename T> void set(detail::Predicate<T> &&last) {
    this->detail::Node<T>::pred_ = std::forward<detail::Predicate<T>>(last);
  }
};

// TODO not ideal ... find something better to avoid code duplication

namespace detail {
template <typename T> using PredicateConst = std::function<void(const T &)>;

template <typename T> struct NodeConst { PredicateConst<T> pred_; };
} // namespace detail

template <typename... Ts>
struct VisitorConst : public detail::NodeConst<Ts>... {
  VisitorConst(detail::PredicateConst<Ts> &&...predicates) {
    this->template set<Ts...>(
        std::forward<detail::PredicateConst<Ts>>(predicates)...);
  }

  template <typename T> void operator()(const T &subject) const {
    this->detail::NodeConst<T>::pred_(subject);
  }

  void visit(const std::variant<Ts...> &subject) const {
    std::visit(*this, subject);
  }

  template <typename T, typename... Tothers>
  void set(detail::PredicateConst<T> &&first,
           detail::PredicateConst<Tothers> &&...preds) {
    this->detail::NodeConst<T>::pred_ =
        std::forward<detail::PredicateConst<T>>(first);
    this->template set<Tothers...>(
        std::forward<detail::PredicateConst<Tothers>>(preds)...);
  }

  template <typename T> void set(detail::PredicateConst<T> &&last) {
    this->detail::NodeConst<T>::pred_ =
        std::forward<detail::PredicateConst<T>>(last);
  }
};

} // namespace EFG
