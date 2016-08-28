#ifndef MATRIX_H
#define MATRIX_H

#include<vector>
using namespace std;

class Matrix {
public:
	enum LIMITS { MaxNumOfRows = 6, MaxNumOfCols = 6 };
	enum STATUS { Normal, RREF, REF };

	Matrix(double*, int, int);//start from 0
	Matrix(const vector<int>&, int, int);
	~Matrix();

	void print() const;
	int getRank() const;
	void getRowEchelonForm(bool = false);
	bool canGetSolution();
	const double* getSolution() const;

private:
	double matrix[MaxNumOfRows][MaxNumOfCols];
	int numOfRows;
	int numOfCols;
	int status;
	int rank;
	bool pivotColumns[MaxNumOfCols];
	bool pivotRows[MaxNumOfRows];
	double *solution;

	void rowExchange(int, int);
	void rowSubTraction(int, int, int = -1);
	void getRREF();
};



#endif // !MATRIX_H
