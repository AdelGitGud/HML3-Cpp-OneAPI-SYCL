#pragma once

#include <sycl/sycl.hpp>

// TODO - remove wait
 // TODO - complete

namespace onemtx {
	template <typename T, size_t N>
	struct SMatrix {
		sycl::queue* queue = nullptr;
		T* data = nullptr;
		std::array<size_t, N> dim = {};

		SMatrix(std::array<size_t, N> dimensions, sycl::queue& q) : dim(dimensions) {
			queue = &q;
			data = sycl::malloc_shared<T>(onemtx::MatrixSize(*this), *queue);
		};

		template <typename U>
		SMatrix(const U& arr, std::array<size_t, N> dimensions, sycl::queue& q) : dim(dimensions) {
			queue = &q;
			data = sycl::malloc_shared<T>(onemtx::MatrixSize(*this), *queue);
			queue->submit([&](sycl::handler& cgh) {
				cgh.memcpy(arr, this->data, i * j * sizeof(T));
			}).wait();
		};

		template <typename U>
		SMatrix(const SMatrix<U, N>& a, sycl::queue& q) : dim(a.dim) {
			queue = &q;
			data = sycl::malloc_shared<T>(onemtx::MatrixSize(*this), *queue);
			onemtx::Copy(*this, a, *queue);
		};

		template <typename U>
		SMatrix(const SMatrix<U, N>& a) : dim(a.dim) {
			queue = a.queue;
			data = sycl::malloc_shared<T>(onemtx::MatrixSize(*this), *queue);
			onemtx::Copy(*this, a, *queue);
		};

		~SMatrix() {
			sycl::free(data, *queue);
		};

		inline T operator()(size_t row, size_t col) {

		};

		inline T* operator[](size_t row) {

		};

		SMatrix<T, N> operator+(const SMatrix<T, N>& a) {

		};

		SMatrix<T, N> operator+(const T scalar) {

		};

		SMatrix<T, N> operator-(const SMatrix<T, N>& a) {

		};

		SMatrix<T, N> operator-(const T scalar) {

		};

		template <typename U>
		SMatrix<T, N> operator=(const SMatrix<U, N>& a) {
			onemtx::Copy(*this, a, *this->queue);
			return *this;
		};

		SMatrix<T, N> operator=(const T scalar) {

		};

		SMatrix<T, N> operator+=(const SMatrix<T, N>& a) {

		};

		SMatrix<T, N> operator+=(const T scalar) {

		};

		SMatrix<T, N> operator-=(const SMatrix<T, N>& a) {

		};

		SMatrix<T, N> operator-=(const T scalar) {

		};
	};

	template <typename T, size_t N>
	struct HMatrix : public SMatrix<T, N> {
		HMatrix(const size_t& rows, const size_t& cols, sycl::queue& q) : i(rows), j(cols) {
			queue = &q;
			data = sycl::malloc_host<T>(onemtx::MatrixSize(*this), *queue);
		};

		template <typename U>
		HMatrix(const U& arr, const size_t& rows, const size_t& cols, sycl::queue& q) : i(rows), j(cols) {
			queue = &q;
			data = sycl::malloc_host<T>(onemtx::MatrixSize(*this), *queue);
			queue->submit([&](sycl::handler& cgh) {
				cgh.memcpy(arr, this->data, onemtx::MatrixSize(*this) * sizeof(T));
			}).wait();
		};

		template <typename U>
		HMatrix(const SMatrix<U, N>& a, sycl::queue& q) : i(a.i), j(a.j) {
			queue = &q;
			data = sycl::malloc_host<T>(onemtx::MatrixSize(*this), *queue);
			onemtx::Copy(*this, a, *queue);
		};

		template <typename U>
		HMatrix(const SMatrix<U, N>& a) : i(a.i), j(a.j) {
			queue = a.queue;
			data = sycl::malloc_host<T>(onemtx::MatrixSize(*this), *queue);
			onemtx::Copy(*this, a, *queue);
		};
	};

	template <typename T, size_t N>
	struct DMatrix : public SMatrix<T, N> {
		DMatrix(const size_t& rows, const size_t& cols, sycl::queue& q) : i(rows), j(cols) {
			queue = &q;
			data = sycl::malloc_device<T>(onemtx::MatrixSize(*this), *queue);
		};

		template <typename U>
		DMatrix(const U& arr, const size_t& rows, const size_t& cols, sycl::queue& q) : i(rows), j(cols) {
			queue = &q;
			data = sycl::malloc_device<T>(onemtx::MatrixSize(*this), *queue);
			queue->submit([&](sycl::handler& cgh) {
				cgh.memcpy(arr, this->data, onemtx::MatrixSize(*this) * sizeof(T));
				}).wait();
		};

		template <typename U>
		DMatrix(const SMatrix<U, N>& a, sycl::queue& q) : i(a.i), j(a.j) {
			queue = &q;
			data = sycl::malloc_device<T>(onemtx::MatrixSize(*this), *queue);
			onemtx::Copy(*this, a, *queue);
		};

		template <typename U>
		DMatrix(const SMatrix<U, N>& a) : i(a.i), j(a.j) {
			queue = a.queue;
			data = sycl::malloc_device<T>(onemtx::MatrixSize(*this), *queue);
			onemtx::Copy(*this, a, *queue);
		};
	};

	template <typename T, size_t N>
	void Cross(SMatrix<T, N>& a, const SMatrix<T, N>& b) {

	};

	template <typename T, size_t N>
	void Cross(SMatrix<T, N>& newMatrix, const SMatrix<T, N>& a, const SMatrix<T, N>& b) {

	};
	
	template <typename T, size_t N>
	void Dot(T& scalar, const SMatrix<T, N>& a, const T& b) {

	};

	template <typename T, size_t N>
	void Dot(T& scalar, const SMatrix<T, N>& a, const SMatrix<T, N>& b) {

	};

	template <typename T, size_t N>
	void Transpose(SMatrix<T, N>& a) {

	};

	template <typename T, size_t N>
	void Invert(SMatrix<T, N>& a) {

	};

	template <typename T, typename U, size_t N>
	void Copy(SMatrix<T, N>& a, const SMatrix<U, N>& b, sycl::queue& q) {
		q.submit([&](sycl::handler& cgh) {
			cgh.memcpy((T*)b.data, a.data, onemtx::MatrixSize(b) * sizeof(T));
		}).wait();	
	};

	template <typename T, typename U, size_t N>
	void Copy(SMatrix<T, N>& a, const SMatrix<U, N>& b) {
		b.queue->submit([&](sycl::handler& cgh) {
			cgh.memcpy((T*)b.data, a.data, onemtx::MatrixSize(b) * sizeof(T));
		}).wait();
	};

	template <typename T, size_t N>
	constexpr size_t MatrixSize(const SMatrix<T, N>& a) noexcept {
		size_t size = a.dim[0];
		for (size_t i = 1; i < N; i++) {
			size *= a.dim[i];
		}
		return size;
	};

} // namespace onemtx
