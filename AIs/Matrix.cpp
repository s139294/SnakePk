#include "Matrix.h"
#include<iostream>
#include<iomanip>
#include<stdexcept>
using namespace std;

Matrix::Matrix(double *arrays, int rows, int cols) {
	numOfCols = cols;
	numOfRows = rows;
	rank = -1;
	solution = nullptr;
	if (arrays != nullptr) {
		for (int i = 0; i < rows; ++i) 
			for (int j = 0; j < cols; ++j) 
				matrix[i][j] = arrays[i * cols + j];
	}
	memset(pivotColumns, false, sizeof(pivotColumns));
	memset(pivotRows, false, sizeof(pivotRows));

	status = Normal;
}

Matrix::Matrix(const vector<int>& vec, int rows, int cols){
	size_t len = vec.size();
	double *arrays = new double[len];
	for (int i = 0; i < len; ++i) {
		arrays[i] = vec[i];
	}
	Matrix(arrays, rows, cols);
	delete[] arrays;
	arrays = nullptr;
}

Matrix::~Matrix(){
	if (solution != nullptr) {
		delete[] solution;
		solution = nullptr;
	}
}

void Matrix::print() const{
	for (int i = 0; i < numOfRows; ++i) {
		for (int j = 0; j < numOfCols; ++j)
			cout << setw(4) << matrix[i][j] << " ";
		cout << endl;
	}
	cout << endl;

	if (status == RREF || status == REF) {
		cout << "pivot rows: ";
		for (int i = 0; i < numOfRows; ++i)
			if (pivotRows[i])
				cout << i << " ";
		cout << endl;
		cout << "pivot columns: ";
		for (int i = 0; i < numOfCols; ++i)
			if (pivotColumns[i])
				cout << i << " ";
		cout << endl;
	}


}

int Matrix::getRank() const{
	return rank;
}

void Matrix::rowExchange(int firstRow, int secondRow){
	if (firstRow < numOfRows && secondRow < numOfRows) {
		for (int i = 0; i < numOfCols; ++i)
			swap(matrix[firstRow][i], matrix[secondRow][i]);
	}
	else if (firstRow == secondRow)
		return;
	else 
		throw out_of_range("row out of range");
}

void Matrix::rowSubTraction(int pivotCol, int targetRow, int unchangedRow) {
	double multiple = matrix[targetRow][pivotCol];
	if (unchangedRow == -1) {//targetRow minimization
		for (int i = pivotCol; i < numOfCols; ++i) {
			matrix[targetRow][i] = matrix[targetRow][i] * 1.0 / multiple;
		}
		return;
	}

	for (int i = pivotCol; i < numOfCols; ++i) {
		double temp1 = matrix[targetRow][i] * matrix[unchangedRow][pivotCol];
		double temp2 = matrix[unchangedRow][i] * multiple;
		matrix[targetRow][i] = temp1 - temp2;
	}
}

void Matrix::getRREF() {
	int maxRow = numOfRows;
	int curCol = numOfCols;
	while (!pivotRows[--maxRow]);
	for (int i = maxRow; i >= 0; --i) {
		while (!pivotColumns[--curCol]);
		rowSubTraction(curCol, i);
		for (int j = 0; j < i; ++j)
			rowSubTraction(curCol, j, i);
	}
		
}

void Matrix::getRowEchelonForm(bool shouldBeRREF) {
	int pivotCol = 0;
	int i;
	for (i = 0; i < numOfRows; ++i) {
		while (pivotCol < numOfCols && matrix[i][pivotCol] == 0) {
			bool allZeroBelow = true;
			for (int j = numOfRows - 1; j >= i + 1; --j) {
				if (matrix[j][pivotCol] != 0) {//temporary failure
					allZeroBelow = false;
					rowExchange(i, j);
					break;
				}
			}
			if (allZeroBelow) {
				pivotCol++;	//complete failure(no pivot)
			}
			else {
				break;		//has got a pivot
			}
		}
		if (pivotCol >= numOfCols) 
			break;
		
		for (int j = i + 1; j < numOfRows; ++j) {
			if (matrix[j][pivotCol] == 0) 
				continue;
			rowSubTraction(pivotCol, j, i);
		}
		pivotColumns[pivotCol] = true;
		pivotCol++;
	}
	rank = i;
	solution = new double[rank];
	for (int j = 0; j < i; ++j) {
		pivotRows[j] = true;
	}
	if (shouldBeRREF) {
		getRREF();
		status = RREF;
	}
	else
		status = REF;
}

bool Matrix::canGetSolution(){
	if(status != RREF)
		return false;
	int curRow = rank - 1;
	if (rank + 1 < numOfCols)
		return false;//infinite solutions

	for (int i = numOfCols - 1; i >= 0; --i) {
		if (!pivotColumns[i])
			continue;
		if (i == numOfCols - 1)
			return false;//no solution
		solution[curRow] = matrix[curRow][numOfCols - 1] * 1.0
			/ matrix[curRow][i];
		--curRow;
	}
	return true;
}

const double * Matrix::getSolution() const{
	return solution;
}
