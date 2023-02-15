#pragma once

#include <sycl/sycl.hpp>

// TODO - remove wait
 // TODO - complete

namespace onemtx {
	template <typename T>
	struct Matrix {
		size_t x = 0;
		size_t y = 0;
		T* data = nullptr;
		sycl::queue* queue = nullptr;

		Matrix(const size_t& rows, const size_t& cols, sycl::queue& q) : x(rows), y(cols) {
			queue = &q;
			data = sycl::malloc_device<T>(x * y, q);
		};

		Matrix(const T& arr, const size_t& rows, const size_t& cols, sycl::queue& q) : x(rows), y(cols) {
			queue = &q;
			data = sycl::malloc_device<T>(x * y, q);
			queue->submit([&](sycl::handler& cgh) {
				cgh.memcpy(arr, this->data, x * y * sizeof(T));
			}).wait();
		};

		template <typename U>
		Matrix(const Matrix<U>& a, sycl::queue& q) : x(a.x), y(a.y) {
			queue = &q;
			data = sycl::malloc_device<U>(x * y, *queue);
			onemtx::Copy(*this, a, *queue);
		};

		template <typename U>
		Matrix(const Matrix<U>& a) : x(a.x), y(a.y) {
			queue = *a.queue;
			data = sycl::malloc_device<U>(x * y, *queue);
			onemtx::Copy(*this, a, *queue);
		};

		~Matrix() {
			sycl::free(data, *queue);
		};

		inline T operator()(size_t row, size_t col) {

		};

		inline T* operator[](size_t row) {

		};

		Matrix<T> operator+(const Matrix<T>& a) {

		};

		Matrix<T> operator+(const T scalar) {

		};

		Matrix<T> operator-(const Matrix<T>& a) {

		};

		Matrix<T> operator-(const T scalar) {

		};

		Matrix<T> operator=(const Matrix<T>& a) {
			onemtx::Copy(*this, a);
			return *this;
		};

		Matrix<T> operator=(const T scalar) {

		};

		Matrix<T> operator+=(const Matrix<T>& a) {

		};

		Matrix<T> operator+=(const T scalar) {

		};

		Matrix<T> operator-=(const Matrix<T>& a) {

		};

		Matrix<T> operator-=(const T scalar) {

		};
	};

	template <typename T>
	void Cross(Matrix<T>& a, const Matrix<T>& b) {

	};

	template <typename T>
	void Cross(Matrix<T>& newMatrix, const Matrix<T>& a, const Matrix<T>& b) {

	};
	
	template <typename T>
	void Dot(T& scalar, const Matrix<T>& a, const T& b) {

	};

	template <typename T>
	void Dot(T& scalar, const Matrix<T>& a, const Matrix<T>& b) {

	};

	template <typename T>
	void Transpose(Matrix<T>& a) {

	};

	template <typename T>
	void Invert(Matrix<T>& a) {

	};

	template <typename T>
	void Copy(Matrix<T>& a, const Matrix<T>& b, sycl::queue& q) {
		q.submit([&](sycl::handler& cgh) {
			cgh.memcpy(b.data, a.data, b.x * b.y * sizeof(T));
		}).wait();	
	};

	template <typename T>
	void Copy(Matrix<T>& a, const Matrix<T>& b) {
		b.queue->submit([&](sycl::handler& cgh) {
			cgh.memcpy(b.data, a.data, b.x * b.y * sizeof(T));
		}).wait();
	};

} // namespace onemtx
