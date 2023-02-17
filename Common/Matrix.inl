#pragma once

// TODO - remove wait

namespace onemtx {
	template <typename T, size_t N>
	Matrix<T, N>::~Matrix() {
		if (data != nullptr) {
			sycl::free(data, *queue);
		}
	}

	template <typename T, size_t N>
	void MakeSMatrix(Matrix<T, N>& a, const std::array<size_t, N>& dimensions, sycl::queue& q) {
		a.data = sycl::malloc_shared<T>(MatrixSize(a), q);
		a.dim = dimensions;
		a.queue = &q;
	}
	template <typename T, size_t N>
	void MakeSMatrix(Matrix<T, N>& a, const T& arr, const std::array<size_t, N>& dimensions, sycl::queue& q) {
		a.data = sycl::malloc_shared<T>(MatrixSize(a), q);
		a.dim = dimensions;
		a.queue = &q;
		Copy(a, arr, *a.queue);
	}
	template <typename T, size_t N>
	void MakeSMatrix(Matrix<T, N>& a, const Matrix<T, N>& b, sycl::queue& q) {
		a.data = sycl::malloc_shared<T>(MatrixSize(a), q);
		a.dim = b.dim;
		a.queue = &q;
		Copy(a, b, *a.queue);
	}
	template <typename T, size_t N>
	void MakeSMatrix(Matrix<T, N>& a, const Matrix<T, N>& b) {
		a.data = sycl::malloc_shared<T>(MatrixSize(a), *b.queue);
		a.dim = b.dim;
		a.queue = b.queue;
		Copy(a, b);
	}

	template<typename T, size_t N>
	void MakeHMatrix(Matrix<T, N>& a, const std::array<size_t, N>& dimensions, sycl::queue& q) {
		a.data = sycl::malloc_host<T>(MatrixSize(a), q);
		a.dim = dimensions;
		a.queue = &q;
	}
	template<typename T, size_t N>
	void MakeHMatrix(Matrix<T, N>& a, const T& arr, const std::array<size_t, N>& dimensions, sycl::queue& q) {
		a.data = sycl::malloc_host<T>(MatrixSize(a), q);
		a.dim = dimensions;
		a.queue = &q;
		Copy(a, arr, *a.queue)
	}
	template<typename T, size_t N>
	void MakeHMatrix(Matrix<T, N>& a, const Matrix<T, N>& b, sycl::queue& q) {
		a.data = sycl::malloc_host<T>(MatrixSize(a), q);
		a.dim = b.dim;
		a.queue = &q;
		Copy(a, b, *a.queue);
	}
	template<typename T, size_t N>
	void MakeHMatrix(Matrix<T, N>& a, const Matrix<T, N>& b) {
		a.data = sycl::malloc_host<T>(MatrixSize(a), *b.queue);
		a.dim = b.dim;
		a.queue = b.queue;
		Copy(a, b);
	}

	template<typename T, size_t N>
	void MakeDMatrix(Matrix<T, N>& a, const std::array<size_t, N>& dimensions, sycl::queue& q) {
	}
	template<typename T, size_t N>
	void MakeDMatrix(Matrix<T, N>& a, const T& arr, const std::array<size_t, N>& dimensions, sycl::queue& q) {
	}
	template<typename T, size_t N>
	void MakeDMatrix(Matrix<T, N>& a, const Matrix<T, N>& b, sycl::queue& q) {
	}
	template<typename T, size_t N>
	void MakeDMatrix(Matrix<T, N>& a, const Matrix<T, N>& b) {
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
	void Copy(Matrix<T, N>& a, const Matrix<T, N>& b, sycl::queue& q) {
		q.submit([&](sycl::handler& cgh) {
			cgh.memcpy(a.data, b.data, MatrixSize(a) * sizeof(T));
			}).wait();
	}
	template<typename T, size_t N>
	void Copy(Matrix<T, N>& a, const Matrix<T, N>& b) {
		b.queue->submit([&](sycl::handler& cgh) {
			cgh.memcpy(a.data, b.data, MatrixSize(a) * sizeof(T));
			}).wait();
	}
	template<typename T, size_t N>
	void Copy(Matrix<T, N>& a, const T& arr, size_t size) {
		a.queue->submit([&](sycl::handler& cgh) {
			cgh.memcpy((a.data, arr, MatrixSize(a) * sizeof(T));
				}).wait();
	}

	template <typename T, size_t N>
	constexpr size_t MatrixSize(const onemtx::Matrix<T, N>& a) noexcept {
		size_t size = a.dim[0];
		for (size_t i = 1; i < N; i++) {
			size *= a.dim[i];
		}
		return size;
	};
}