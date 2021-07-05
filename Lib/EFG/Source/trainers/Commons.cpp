/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef ADVANCED_TRAINERS_ENABLED
#include <trainers/Commons.h>
#include <Error.h>

namespace EFG::train {
	Vect& operator-(Vect& v) {
		float* data = v.data();
		for (std::size_t k = 0; k < v.size(); ++k) {
			data[k] = -data[k];
		}
		return v;
	};

	Vect& operator+=(Vect& a, const Vect& b) {
		if (a.size() != b.size()) {
			throw Error("Vectors with invalid sizes");
		}
		float* dataA = a.data();
		const float* dataB = b.data();
		for (std::size_t k = 0; k < a.size(); ++k) {
			dataA[k] += dataB[k];
		}
		return a;
	}

	Vect operator+(const Vect& a, const Vect& b) {
		Vect res = a;
		res += b;
		return res;
	}

	Vect& operator-=(Vect& a, const Vect& b) {
		if (a.size() != b.size()) {
			throw Error("Vectors with invalid sizes");
		}
		float* dataA = a.data();
		const float* dataB = b.data();
		for (std::size_t k = 0; k < a.size(); ++k) {
			dataA[k] -= dataB[k];
		}
		return a;
	}

	Vect operator-(const Vect& a, const Vect& b) {
		Vect res = a;
		res -= b;
		return res;
	}

	float dot(const Vect& a, const Vect& b) {
		if (a.size() != b.size()) {
			throw Error("Vectors with invalid sizes");
		}
		const float* dataA = a.data();
		const float* dataB = b.data();
		float res = 0.f;
		for (std::size_t k = 0; k < a.size(); ++k) {
			res += dataA[k] * dataB[k];
		}
		return res;
	}

	Vect& operator*=(Vect& v, const float& c) {
		float* data = v.data();
		for (std::size_t k = 0; k < v.size(); ++k) {
			data[k] *= c;
		}
		return v;
	};

	Vect operator*(const Vect& v, const float& c) {
		Vect res = v;
		res *= c;
		return res;
	};

	Vect operator*(const Matr& a, const Vect& b) {
		if (a.size() != b.size()) {
			throw Error("Vectors with invalid sizes");
		}
		Vect res;
		res.reserve(a.size());
		for (std::size_t k = 0; k < a.size(); ++k) {
			res[k] = dot(a[k], b);
		}
		return res;
	}
}
#endif
