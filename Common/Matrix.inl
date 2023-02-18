#include "Matrix.h"
#pragma once

// TODO - remove wait

namespace onemtx {
	template<typename T, size_t N>
	inline Matrix<T, N>::Matrix(const std::array<size_t, N>& dimensions, sycl::queue& q, void* allocFunc) {
		queue = &q;
		dim = dimensions;
		data = (T*)sycl::malloc_shared(MatrixSizeOfT(*this), *queue);
	}
	template<typename T, size_t N>
	inline Matrix<T, N>::Matrix(const T& arr, const std::array<size_t, N>& dimensions, sycl::queue& q, void* allocFunc) {
		queue = &q;
		dim = dimensions;
		data = (T*)sycl::malloc_shared(MatrixSizeOfT(*this), *queue);
		Copy(*this, arr);
	}
	template<typename T, size_t N>
	inline Matrix<T, N>::Matrix(const Matrix<T, N>& a, sycl::queue& q, void* allocFunc) {
		queue = &q;
		dim = a.dim;
		data = (T*)sycl::malloc_shared(MatrixSizeOfT(*this), *queue);
		Copy(*this, a);
	}
	template<typename T, size_t N>
	inline Matrix<T, N>::Matrix(const Matrix<T, N>& a, void* allocFunc) {
		queue = a.queue;
		dim = a.dim;
		data = (T*)sycl::malloc_shared(MatrixSizeOfT(*this), *queue);
		Copy(*this, a);
	}
	template <typename T, size_t N>
	Matrix<T, N>::~Matrix() {
		if (data != nullptr) {
			sycl::free(data, *queue);
		}
	}

	template<typename T, size_t N>
	void Cross(Matrix<T, N>& a, const Matrix<T, N>& b) {
	}
	template<typename T, size_t N>
	void Cross(Matrix<T, N>& newMatrix, const Matrix<T, N>& a, const Matrix<T, N>& b) {
	}
	template<typename T, size_t N>
	void Dot(T& scalar, const Matrix<T, N>& a, const T& b) {
	}
	template<typename T, size_t N>
	void Dot(T& scalar, const Matrix<T, N>& a, const Matrix<T, N>& b) {
	}

	template<typename T, size_t N>
	void Transpose(Matrix<T, N>& a) {
	}
	template<typename T, size_t N>
	void Invert(Matrix<T, N>& a) {
	}

	template<typename T, size_t N>
	void Copy(Matrix<T, N>& a, const Matrix<T, N>& b) {
		a.queue->submit([&](sycl::handler& cgh) {
			cgh.memcpy(a.data, b.data, MatrixSizeOfT(a));
		}).wait();
	}
	template<typename T, size_t N>
	void Copy(Matrix<T, N>& a, const T& arr, size_t size) {
		a.queue->submit([&](sycl::handler& cgh) {
			cgh.memcpy(a.data, arr, MatrixSizeOfT(a));
		}).wait();
	}

	template <typename T, size_t N>
	constexpr size_t MatrixSizeOfT(const onemtx::Matrix<T, N>& a) noexcept {
		size_t size = a.dim[0];
		for (size_t i = 1; i < N; i++) {
			size *= a.dim[i];
		}
		return size * sizeof(T);
	};
}