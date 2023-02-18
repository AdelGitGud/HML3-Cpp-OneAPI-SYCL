#pragma once

#include <sycl/sycl.hpp>

#include <sycl/detail/common.hpp>

 // TODO - complete

// Might switch away from #define
#define Vec2HF Matrix<sycl::half, 1>
#define Vec2F Matrix<float, 1>
#define Vec2D Matrix<double, 1>
#define Vec2SC Matrix<sycl::schar, 1>
#define Vec2UC Matrix<sycl::uchar, 1>
#define Vec2SI Matrix<int32_t, 1>
#define Vec2UI Matrix<uint32_t, 1>

#define Vec3HF Matrix<sycl::half, 1>
#define Vec3F Matrix<float, 1>
#define Vec3D Matrix<double, 1>
#define Vec3SC Matrix<sycl::schar, 1>
#define Vec3UC Matrix<sycl::uchar, 1>
#define Vec3SI Matrix<int32_t, 1>
#define Vec3UI Matrix<uint32_t, 1>

namespace onemtx {
	using AllocPtr = void* (*)(size_t, const sycl::queue&, const sycl::detail::code_location&);

	template <typename T, size_t N>
	struct Matrix {
		sycl::queue* queue = nullptr;
		std::array<size_t, N> dim = {};
		T* data = nullptr;

		Matrix() = default;
		Matrix(const std::array<size_t, N>& dimensions, sycl::queue& q, void* allocFunc);
		Matrix(const T& arr, const std::array<size_t, N>& dimensions, sycl::queue& q, void* allocFunc);
		Matrix(const Matrix<T, N>& a, sycl::queue& q, void* allocFunc);
		Matrix(const Matrix<T, N>& a, void* allocFunc);

		~Matrix();

		inline T* operator[](size_t row) {
			return data + (row * dim[0]);
		}

		inline Matrix<T, N> operator+(const Matrix<T, N>& a) {

		}
		inline Matrix<T, N> operator+(const T scalar) {

		}
		inline Matrix<T, N> operator-(const Matrix<T, N>& a) {

		}
		inline Matrix<T, N> operator-(const T scalar) {

		}

		inline Matrix<T, N> operator=(const Matrix<T, N>& a) {
			if (a.data)
				onemtx::Copy(*this, a);
			return *this;
		}
		inline Matrix<T, N> operator=(const T scalar) {

		}

		inline Matrix<T, N> operator+=(const Matrix<T, N>& a) {

		}
		inline Matrix<T, N> operator+=(const T scalar) {

		}
		inline Matrix<T, N> operator-=(const Matrix<T, N>& a) {

		}
		inline Matrix<T, N> operator-=(const T scalar) {

		}
	};

	template<typename T, size_t N>
	void Cross(Matrix<T, N>& a, const Matrix<T, N>& b);
	template<typename T, size_t N>
	void Cross(Matrix<T, N>& newMatrix, const Matrix<T, N>& a, const Matrix<T, N>& b);
	template<typename T, size_t N>
	void Dot(T& scalar, const Matrix<T, N>& a, const T& b);
	template<typename T, size_t N>
	void Dot(T& scalar, const Matrix<T, N>& a, const Matrix<T, N>& b);

	template<typename T, size_t N>
	void Transpose(Matrix<T, N>& a);
	template<typename T, size_t N>
	void Invert(Matrix<T, N>& a);

	template<typename T, size_t N>
	void Copy(Matrix<T, N>& a, const Matrix<T, N>& b);
	template<typename T, size_t N>
	void Copy(Matrix<T, N>& a, const T& arr, size_t size);

	template <typename T, size_t N>
	constexpr size_t MatrixSizeOfT(const onemtx::Matrix<T, N>& a) noexcept;

} // namespace onemtx

#include "Matrix.inl"