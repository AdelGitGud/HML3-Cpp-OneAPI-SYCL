#pragma once

#include <sycl/sycl.hpp>

#include <sycl/detail/common.hpp>

 // TODO - complete

namespace onemtx {
	//using AllocPtr = void* (*)(size_t, const sycl::queue&, const sycl::detail::code_location&);
	template<typename T, size_t ... VarD> struct Matrix;

	template<typename T, size_t D>
	struct Matrix<T, D> {
		static constexpr size_t size = D;
		using type = T[D];
		type data;

		T& operator[](size_t i) { return data[i]; }
	};

	template<typename T, size_t D, size_t ... VarD>
	struct Matrix<T, D, VarD...> {
		static constexpr size_t size = sizeof...(VarD);
		using DType = typename Matrix<T, VarD...>::type;
		using type = DType[D];
		type data;

		DType& operator[](size_t i) { return data[i]; }
	};

} // namespace onemtx