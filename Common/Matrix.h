#pragma once

#include <sycl/sycl.hpp>

#include <sycl/detail/common.hpp>

 // TODO - complete

namespace onemtx {
	using AllocPtr = void* (*)(size_t, const sycl::queue&, const sycl::detail::code_location&);
	template <typename T, size_t U, std::array<size_t, U> D>
	struct MatrixData {
		sycl::queue* queue = nullptr;
		T* data = nullptr;
		std::array<size_t, U> dimLengths = D;

		MatrixData() = default;
		template <typename FuncPTR>
		MatrixData(sycl::queue& q, FuncPTR allocFunc);
		template <typename FuncPTR>
		MatrixData(const T& arr, sycl::queue& q, FuncPTR allocFunc);
		template <typename FuncPTR>
		MatrixData(const MatrixData<T, U, D>& a, sycl::queue& q, FuncPTR allocFunc);
		template <typename FuncPTR>
		MatrixData(const MatrixData<T, U, D>& a, FuncPTR allocFunc);

		~MatrixData();

		inline T* operator[](size_t row) {
			return data + (row * dimLengths[0]);
		}

		inline MatrixData<T, U, D> operator+(const MatrixData<T, U, D>& a) {

		}
		inline MatrixData<T, U, D> operator+(const T scalar) {

		}
		inline MatrixData<T, U, D> operator-(const MatrixData<T, U, D>& a) {

		}
		inline MatrixData<T, U, D> operator-(const T scalar) {

		}

		inline MatrixData<T, U, D> operator=(const MatrixData<T, U, D>& a) {
			if (a.data) {
				onemtx::Copy(*this, a);
			}
			return *this;
		}
		inline MatrixData<T, U, D> operator=(const T scalar) {

		}

		inline MatrixData<T, U, D> operator+=(const MatrixData<T, U, D>& a) {

		}
		inline MatrixData<T, U, D> operator+=(const T scalar) {

		}
		inline MatrixData<T, U, D> operator-=(const MatrixData<T, U, D>& a) {

		}
		inline MatrixData<T, U, D> operator-=(const T scalar) {

		}
	};

	class Matrix {};


	/*using Vec2HF = MatrixData < sycl::half, std::array<sycl::half, 2>{1, 1}>;
	using Vec2F = MatrixData<float, 2>;
	using Vec2D = MatrixData<double, 2>;
	using Vec2SC = MatrixData<sycl::schar, 2>;
	using Vec2UC = MatrixData<sycl::uchar, 2>;
	using Vec2SI = MatrixData<int32_t, 2>;
	using Vec2UI = MatrixData<uint32_t, 2>;

	using Vec3HF = MatrixData<sycl::half, 3>;
	using Vec3F = MatrixData<float, 3>;
	using Vec3D = MatrixData<double, 3>;
	using Vec3SC = MatrixData<sycl::schar, 3>;
	using Vec3UC = MatrixData<sycl::uchar, 3>;
	using Vec3SI = MatrixData<int32_t, 3>;
	using Vec3UI = MatrixData<uint32_t, 3>;*/

	template <typename T, size_t U, std::array<size_t, U> D>
	void Cross(MatrixData<T, U, D>& a, const MatrixData<T, U, D>& b);
	template <typename T, size_t U, std::array<size_t, U> D>
	void Cross(MatrixData<T, U, D>& newMatrix, const MatrixData<T, U, D>& a, const MatrixData<T, U, D>& b);
	template <typename T, size_t U, std::array<size_t, U> D>
	void Dot(T& scalar, const MatrixData<T, U, D>& a, const T& b);
	template <typename T, size_t U, std::array<size_t, U> D>
	void Dot(T& scalar, const MatrixData<T, U, D>& a, const MatrixData<T, U, D>& b);

	template <typename T, size_t U, std::array<size_t, U> D>
	void Transpose(MatrixData<T, U, D>& a);
	template <typename T, size_t U, std::array<size_t, U> D>
	void Invert(MatrixData<T, U, D>& a);

	template <typename T, size_t U, std::array<size_t, U> D>
	void Copy(MatrixData<T, U, D>& a, const MatrixData<T, U, D>& b);
	template <typename T, size_t U, std::array<size_t, U> D>
	void Copy(MatrixData<T, U, D>& a, const T& arr, size_t size);

	template <typename T, size_t U, std::array<size_t, U> D>
	constexpr size_t MatrixSizeOfT(const onemtx::MatrixData<T, U, D>& a) noexcept;

	template <typename T, size_t D>
	constexpr size_t arraySize(T (&)[D]) noexcept; // Might move this of a different file

} // namespace onemtx

#include "Matrix.inl"