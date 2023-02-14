#pragma once

template <typename T>
class Matrix {
public:
	Matrix() = default;
	Matrix(uint64_t rows, uint64_t cols) : m.rows(rows), m.cols(cols) {
		m.data = new T[rows * cols];
	};
	~Matrix() {
		delete[] m.data;
	};

	inline T& operator()(uint64_t row, uint64_t col) {

	};

	inline T* operator[](uint64_t row) {
	
	};

	inline void Add(Matrix<T>& other) {

	};

	inline void Subtract(Matrix<T>& other) {
		
	};

	inline void Multiply(Matrix<T>& other) {
		
	};

	inline void Multiply(T scalar) {

	};

	inline void Divide(T scalar) {

	};

	inline void Transpose() {

	};

	inline void Invert() {

	};

	inline size_t GetRows() { return m.rows; };
	inline size_t GetCols() { return m.cols; };
	inline T* GetData() { return m.data; };

private:
	struct Members {
		size_t rows = 0;
		size_t cols = 0;
		T* data = nullptr;
	}m;
};

