#pragma once
#include "numcpp.h"

struct SimpleNN
{
public:
	/*
		   W1 B1
	input ---  layer 1 ---- layer 2  --- layer 3 --- output
	960+2   ---	960    ----	  300    ---   30    ---   4

	*/
	Matrix<float> W1;
	Matrix<float> B1;
	Matrix<float> W2;
	Matrix<float> B2;
	Matrix<float> W3;
	Matrix<float> B3;
	Matrix<float> W4;
	Matrix<float> B4;
	float gsdn = 1;

	SimpleNN(int inputSize)
	{
		W1.resize(inputSize, inputSize);
		W1.fillRnd(-1, 1);
		B1.resize(inputSize);
		B1.fillRnd(-1, 1);

		W2.resize(300, inputSize);
		W2.fillRnd(-1, 1);
		B2.resize(300);
		B2.fillRnd(-1, 1);

		W3.resize(30, 300);
		W3.fillRnd(-1, 1);
		B3.resize(30);
		B3.fillRnd(-1, 1);

		W4.resize(4, 30);
		W4.fillRnd(-1, 1);
		B4.resize(4);
		B4.fillRnd(-1, 1);
	}
	Matrix<float> forward(Matrix<float> X)
	{
		X = (W1 * X + B1).transform(&sigmoid);

		X = (W2 * X + B2).transform(&sigmoid);

		X = (W3 * X + B3).transform(&sigmoid);

		X = (W4 * X + B4).softmax();

		return X;
	}

	float trainStep(Matrix<float> X, Matrix<float> ans)
	{
		Matf X1 = (W1 * X + B1);
		Matf X1sigm = X1.transform(&sigmoid);

		Matf X2 = (W2 * X1sigm + B2);
		Matf X2sigm = X2.transform(&sigmoid);

		Matf X3 = (W3 * X2sigm + B3);
		Matf X3sigm = X3.transform(&sigmoid);

		Matf X4 = (W4 * X3sigm + B4);
		Matf E = X4.softmax();
		float loss = ans.CrossEntropy(E);

		//backprop
		//layer 3-output
		Matf dEt = E - ans;
		Matf dEW4 = dEt * X3sigm.T();
		Matf dEB4 = dEt;
		W4 -= gsdn * dEW4;
		B4 -= gsdn * dEB4;
		//layer 2-3
		dEt = W4.T() * dEt;
		dEt.transform(&dirsigm);
		Matf dEW3 = dEt * X2sigm.T();
		Matf dEB3 = dEt;
		W3 -= gsdn * dEW3;
		B3 -= gsdn * dEB3;
		//layer 1-2
		dEt = W3.T() * dEt;
		dEt.transform(&dirsigm);
		Matf dEW2 = dEt * X1sigm.T();
		Matf dEB2 = dEt;
		W2 -= gsdn * dEW2;
		B2 -= gsdn * dEB2;
		//layer input - 1
		dEt = W2.T() * dEt;
		dEt.transform(&dirsigm);
		Matf dEW1 = dEt * X1sigm.T();
		Matf dEB1 = dEt;
		W1 -= gsdn * dEW1;
		B1 -= gsdn * dEB1;

		//gsdn *= 0.99;
		return loss;
	}

};
