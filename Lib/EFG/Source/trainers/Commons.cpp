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

	float dot(const float* bufferA, const float* buffeB, const std::size_t size) {
		float res = 0.f;
		for (std::size_t k = 0; k < size; ++k) {
			res += bufferA[k] * buffeB[k];
		}
		return res;
	}

	float dot(const Vect& a, const Vect& b) {
		if (a.size() != b.size()) {
			throw Error("Vectors with invalid sizes");
		}
		return dot(a.data(), b.data(), a.size());
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

	Matr::Matr(const std::size_t size)
		: buffer(size*size, 0.f) {
	}

	Matr::Matr(const Vect& a, const Vect& b) {
		if (a.size() != b.size()) {
			throw Error("Vectors with invalid sizes");
		}
		this->buffer.resize(a.size() * a.size());
		const float* dataA = a.data();
		const float* dataB = b.data();
		std::size_t posB;
		float* data = this->buffer.data();
		std::size_t pos = 0;
		for (std::size_t posA = 0; posA < a.size(); ++posA) {
			for (posB = 0; posB < a.size(); ++posB) {
				data[pos] = dataA[posA] * dataB[posB];
				++pos;
			}
		}
	}

	void Matr::addIdentity() {
		float* data = this->buffer.data();
		std::size_t size = this->getSize();
		for (std::size_t k = 0; k < size; ++k) {
			data[this->getPos(k, k)] += 1.f;
		}
	}

	Matr& Matr::operator-() {
		this->buffer = -this->buffer;
		return *this;
	}

	Matr& Matr::operator+=(const Matr& m) {
		this->buffer += m.buffer;
		return *this;
	}

	Vect Matr::operator*(const Vect& v) const {
		if (this->buffer.size() != v.size()* v.size()) {
			throw Error("Vector with invalid size");
		}
		Vect res;
		res.reserve(v.size());
		const float* data = this->buffer.data();
		std::size_t pos = 0;
		for (std::size_t k = 0; k < v.size(); ++k) {
			res.push_back(dot(&data[pos], v.data(), v.size()));
			pos += v.size();
		}
		return res;
	}

	Matr& Matr::operator*=(const float& m) {
		this->buffer *= m;
		return *this;
	}

	Matr Matr::operator*(const Matr& m) const {
		if (this->buffer.size() != m.buffer.size()) {
			throw Error("Matrices with invalid sizes");
		}
		std::size_t size = this->getSize();
		Vect temp(size, 0.f);
		Matr res(size);
		const float* mData = m.buffer.data();
		float* resData = res.buffer.data();
		std::size_t s;
		const float* thisData = this->buffer.data();
		std::size_t posThis;
		for (std::size_t k = 0; k < size; ++k) {
			for (s = 0; s < size; ++s) {
				temp[s] = mData[m.getPos(s , k)];
			}
			posThis = 0;
			for (s = 0; s < size; ++s) {
				resData[s, k] = dot(&thisData[posThis], temp.data(), size);
				posThis += size;
			}
		}
		return res;
	}
}
#endif
