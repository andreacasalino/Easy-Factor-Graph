/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

namespace EFG {
template<typename T, typename M, typename Predicate>
void dynamic_const_predicate(const M& subject, const Predicate& predicate) {
	const auto* as_T_ptr = dynamic_cast<const T*>(&subject);
	if (nullptr != as_T_ptr) {
		predicate(*as_T_ptr);
	}
}

template<typename T, typename M, typename Predicate>
void dynamic_predicate(M& subject, const Predicate& predicate) {
	auto* as_T_ptr = dynamic_cast<T*>(&subject);
	if (nullptr != as_T_ptr) {
		predicate(*as_T_ptr);
	}
}
} // namespace EFG
