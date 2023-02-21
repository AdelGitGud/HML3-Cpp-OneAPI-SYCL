#include "Matrix.h"
#pragma once

// TODO - remove wait

namespace onemtx {
	template <typename T, size_t U, std::array<size_t, U> D>
	template<typename FuncPTR>
	inline MatrixData<T, U, D>::MatrixData(sycl::queue& q, FuncPTR allocFunc)
	: queue(&q) {
		data = (T*)allocFunc(MatrixSizeOfT(*this), *queue, sycl::detail::code_location::current());
	}
	template <typename T, size_t U, std::array<size_t, U> D>
	template<typename FuncPTR>
	inline MatrixData<T, U, D>::MatrixData(const T& arr, sycl::queue& q, FuncPTR allocFunc)
	: queue(&q) {
		data = (T*)allocFunc(MatrixSizeOfT(*this), *queue, sycl::detail::code_location::current());
		Copy(*this, arr);
	}
	template <typename T, size_t U, std::array<size_t, U> D>
	template <typename FuncPTR>
	inline MatrixData<T, U, D>::MatrixData(const MatrixData<T, U, D>& a, sycl::queue& q, FuncPTR allocFunc)
	: queue(&q) {
		data = (T*)allocFunc(MatrixSizeOfT(*this), *queue, sycl::detail::code_location::current());
		Copy(*this, a);
	}
	template <typename T, size_t U, std::array<size_t, U> D>
	template <typename FuncPTR>
	inline MatrixData<T, U, D>::MatrixData(const MatrixData<T, U, D>& a, FuncPTR allocFunc)
	: queue(a.queue) {
		data = (T*)allocFunc(MatrixSizeOfT(*this), *queue, sycl::detail::code_location::current());
		Copy(*this, a);
	}
	template <typename T, size_t U, std::array<size_t, U> D>
	MatrixData<T, U, D>::~MatrixData() {
		if (data != nullptr) {
			sycl::free(data, *queue);
		}
	}

	template <typename T, size_t U, std::array<size_t, U> D>
	void Cross(MatrixData<T, U, D>& a, const MatrixData<T, U, D>& b) {
	}
	template <typename T, size_t U, std::array<size_t, U> D>
	void Cross(MatrixData<T, U, D>& newMatrix, const MatrixData<T, U, D>& a, const MatrixData<T, U, D>& b) {
	}
	template <typename T, size_t U, std::array<size_t, U> D>
	void Dot(T& scalar, const MatrixData<T, U, D>& a, const T& b) {
	}
	template <typename T, size_t U, std::array<size_t, U> D>
	void Dot(T& scalar, const MatrixData<T, U, D>& a, const MatrixData<T, U, D>& b) {
	}

	template <typename T, size_t U, std::array<size_t, U> D>
	void Transpose(MatrixData<T, U, D>& a) {
	}
	template <typename T, size_t U, std::array<size_t, U> D>
	void Invert(MatrixData<T, U, D>& a) {
	}

	template <typename T, size_t U, std::array<size_t, U> D>
	void Copy(MatrixData<T, U, D>& a, const MatrixData<T, U, D>& b) {
		a.queue->submit([&](sycl::handler& cgh) {
			cgh.memcpy(a.data, b.data, MatrixSizeOfT(a));
		}).wait();
	}
	template <typename T, size_t U, std::array<size_t, U> D>
	void Copy(MatrixData<T, U, D>& a, const T& arr, size_t size) {
		a.queue->submit([&](sycl::handler& cgh) {
			cgh.memcpy(a.data, arr, MatrixSizeOfT(a));
		}).wait();
	}

	template <typename T, size_t U, std::array<size_t, U> D>
	constexpr size_t MatrixSizeOfT(const onemtx::MatrixData<T, U, D>& a) noexcept {
		size_t size = a.dimLengths[0];
		for (size_t i = 1; i < U; i++) {
			size *= a.dimLengths[i];
		}
		return size * sizeof(T);
	}

	template <typename T, size_t U, std::array<size_t, U> D>
	constexpr size_t arraySize(T (&)[D]) noexcept {
		return D;
	}
}