#pragma once
#include "numcpp.h"

struct grad
{
	vector<Matf> dEdW;
	vector<Matf> dEdB;
	float loss;
	int cnt = 0;
	grad& operator+= (const grad& rhs)
	{
		loss += rhs.loss;
		cnt += rhs.cnt;
		for (int i = 0; i < dEdW.size(); i++)
		{
			dEdW[i] += rhs.dEdW[i];
			dEdB[i] += rhs.dEdB[i];
		}
		return *this;
	}
};

struct SimpleNN
{
public:
	
	/*
	*   X - - - ->T1 -> sigm-> T1s -----    ----- Tn -> softmax -> E (crossEntropy loss) 
	*	  W1 B1						W2 B2	Wn Bn
	*/
	/*
	X is colunm-matrix
	
	*/
	vector<Matf> W;
	vector<Matf> B;
	
	float gsdn = 1;

	SimpleNN() {}
	void addLayer(size_t prev, size_t next)
	{
		W.push_back(Matf(next, prev));
		W.back().fillRnd(-1, 1); 

		B.push_back(Matf(next));
		B.back().fillRnd(-1, 1);
	}

	bool check() {
		if (W.size() == 0)
		{
			cout << "Add more layers\n";
			return false;
		}
		for (int i = 1; i < W.size(); i++) {
			if (W[i - 1].rows != W[i].cols)
			{
				cout << "Wrong sizes: " << W[i - 1].rows << " * " << W[i - 1].cols << " " << W[i].rows << " * " << W[i].cols << "\n";
				return false;
			}
		}
		return true;
	}

	Matrix<float> forward(Matrix<float> X)
	{
		for (int i = 0; i < W.size() - 1; i++)
		{
			X = (W[i] * X + B[i]).transform(&sigmoid);
		}
		X = (W.back() * X + B.back()).softmax();
		return X;
	}

	grad trainStep(Matrix<float> X, Matrix<float> ans)
	{
		vector<Matf> T;
		vector<Matf> Ts;

		Ts.push_back(X);
		T.push_back(W[0] * X + B[0]);
		Ts.push_back(T[0].transform(&sigmoid));

		for (int i = 1; i < W.size() - 1; i++)
		{
			T.push_back(W[i] * Ts[i] + B[i]);
			Ts.push_back(T[i].transform(&sigmoid));
		}
		T.push_back(W.back() * Ts.back() + B.back());
		Matf E = T.back().softmax();

		grad res;
		res.dEdW.resize(W.size());
		res.dEdB.resize(B.size());
		res.cnt = 1;
		res.loss = E.crossEntropy(ans);

		Matf dEdT = E - ans;
		res.dEdW.back() = dEdT * Ts.back().T();
		res.dEdB.back() = dEdT;

		for (int i = W.size() - 2; i >= 0; --i)
		{
			dEdT = W[i+1].T() * dEdT;
			dEdT.adm(T[i].transform(&dirsigm));

			res.dEdW[i] = dEdT * Ts[i].T();
			res.dEdB[i] = dEdT;
		}
		return res;
	}

	void updateWeights(grad gr)
	{
		for (int i = 0; i < W.size(); i++)
		{
			W[i] -= gsdn / float(gr.cnt) * gr.dEdW[i];
			B[i] -= gsdn / float(gr.cnt) * gr.dEdB[i];
		}
		gsdn *= 0.999;
	}
	grad emptyGrad()
	{
		grad res;
		res.dEdW.resize(W.size());
		res.dEdB.resize(B.size());
		for (int i = 0; i < W.size(); ++i)
		{
			res.dEdW[i].zeros_like(W[i]);
			res.dEdB[i].zeros_like(B[i]);
		}
		return res;
	}

};

