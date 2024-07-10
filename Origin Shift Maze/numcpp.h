#pragma once
#include <vector>
#include <math.h>
#include <iostream>
#include "EasyRandom.h"
using namespace std;

template< typename Y>
class Matrix
{
public:
	vector<Y> data;
	size_t rows, cols;
	/*
	                cols
			. . . . . . . . . .
			. . . . . . . . . .
	rows	. . . . . . . . . .
			. . . . . . . . . .
			. . . . . . . . . .
	*/
	Matrix()
	{
		rows = 0;
		cols = 0;
	}
	Matrix(size_t s)
	{
		rows = s;
		cols = 1;
		data.resize(s, 0);
	}
	Matrix(size_t s, vector<Y> vec)
	{
		rows = s;
		cols = 1;
		data = vec;
	}
	Matrix(size_t h, size_t w)
	{
		rows = h;
		cols = w;
		data.resize(w * h, 0);
	}
	Matrix(size_t h, size_t w, vector<Y> vec)
	{
		rows = h;
		cols = w;
		data = vec;
	}
	Matrix& zeros_like(const Matrix& rhs)
	{
		resize(rhs.rows, rhs.cols);
		return *this;
	}
	void resize(size_t h, size_t w) {
		rows = h;
		cols = w;
		data.resize(w * h, 0);
	}
	void resize(size_t s) {
		rows = s;
		cols = 1;
		data.resize(s, 0);
	}
	void clear() {
		data.clear();
	}
	void fillRnd(float a, float b) {
		if (data.size() > 0)
		{
			EasyRandomf gen(a, b);
			for (Y& el : data) el = gen.get();
		}
	}
	Y& operator[](size_t idx)
	{
		if (idx >= data.size()) cout << "Index error\n";
		return data[idx];
	}
	Y operator[](size_t idx) const
	{
		return data[idx];
	}
	Y& operator[](pair<size_t, size_t> idx)
	{
		return data[idx.first * cols + idx.second];
	}
	Y operator[](pair<size_t, size_t> idx) const
	{
		return data[idx.first * cols + idx.second];
	}
	Matrix& transform(Y(*func)(Y))
	{
		for (Y& element : data)
			element = (*func)(element);
		return *this;
	}
	Matrix T()
	{
		Matrix<Y> mat(cols, rows);
		for (int i = 0; i < cols; ++i)
		{
			for (int j = 0; j < rows; ++j)
			{
				mat[{i, j}] = (*this)[{j, i}];
			}
		}
		return mat;
	}
	Matrix& operator+= (const Matrix& rhs)
	{
		if (rhs.cols != cols || rhs.rows != rows)
		{
			cout << "Matrix operator+ exception: wrong size matrix\n";
			return *this;
		} 
		for (size_t i = 0; i < rows * cols; i++)
		{
			data[i] += rhs[i];
		}
	}
	Matrix& operator-= (const Matrix& rhs)
	{
		if (rhs.cols != cols || rhs.rows != rows)
		{
			cout << "Matrix operator- exception: wrong size matrix\n";
			return *this;
		}
		for (size_t i = 0; i < rows * cols; i++)
		{
			data[i] -= rhs[i];
		}
	}
	friend Matrix operator+(Matrix lhs, const Matrix& rhs) 
	{
		lhs += rhs;
		return lhs; 
	}
	friend Matrix operator-(Matrix lhs, const Matrix& rhs)
	{
		lhs -= rhs;
		return lhs;
	}
	Matrix& operator *= (const Y& k)
	{
		for (auto& el : data)
			el *= k;
		return *this;
	}

	Matrix& adm(const Matrix& rhs)
	{
		if (rhs.cols != cols || rhs.rows != rows)
		{
			cout << "Matrix adm* exception: wrong size matrix\n";
			return *this;
		}
		for (size_t i = 0; i < data.size(); ++i)
		{
			data[i] *= rhs[i];
		}
		return *this;
	}
	friend Matrix operator *(Matrix lhs, const Y& k)
	{
		return lhs *= k;
	}
	friend Matrix operator *(Y k, Matrix rhs)
	{
		return rhs *= k;
	}
	friend Matrix operator*(Matrix lhs, const Matrix& rhs)
	{
		if (lhs.cols != rhs.rows) {
			cout << "Matrix operator* exception: wrong size\n";
			return lhs;
		}
		Matrix<Y> res(lhs.rows, rhs.cols);
		for (size_t i = 0; i < lhs.rows; ++i)
		{
			for (size_t j = 0; j < rhs.cols; ++j)
			{
				for (size_t k = 0; k < lhs.cols; ++k) {
					res[{i, j}] += lhs[{i, k}] * rhs[{k, j}];
				}
			}
		}
		return res;
	}
	friend ostream& operator<<(std::ostream& out, const Matrix& f)
	{
		for (size_t i = 0; i < f.rows; ++i)
		{
			for (size_t j = 0; j < f.cols; ++j) {
				out << f[{i, j}] << " ";
			}
			out << "\n";
		}
		return out;
	}
	Matrix& softmax()
	{
		float norm = 0;
		for (size_t i = 0; i < rows * cols; i++)
		{
			norm += expf(data[i]);
		}
		for (size_t i = 0; i < rows * cols; i++)
		{
			data[i] = expf(data[i]) / norm;
		}
		return *this;
	}
	float crossEntropy(const Matrix& rhs)
	{
		if (rhs.cols != cols || rhs.rows != rows)
		{
			cout << "Matrix ce exception: wrong size matrix\n";
			return -1;
		}
		float res = 0;
		for (size_t i = 0; i < data.size(); ++i)
		{
			res -= rhs[i] * logf(data[i] + 0.01);
		}
		return res;
	}
	size_t indexOfMax()
	{
		size_t res = 0;
		for (size_t i = 0; i < data.size(); ++i)
		{
			if (data[i] > data[res])
				res = i;
		}
		return res;
	}
};

typedef Matrix<float> Matf;

float sigmoid(float x)
{
	return 1.0f / (1.0f + expf(-x));
}
float dirsigm(float x)
{
	return sigmoid(x) * (1 - sigmoid(x));
}
float sqr(float x)
{
	return x * x;
}