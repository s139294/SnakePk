/**************************************************************************

Author		: Dongming Sheng

Date		: 2016-08-26

File Name	: OADSnakeAI.h

**************************************************************************/

#ifndef OADSNAKEAI_H
#define OADSNAKEAI_H

#include"SnakeAI.h"

class GameStarter;

class OADSnakeAI :public SnakeAI {
public:
	OADSnakeAI(GameStarter*, int);

	virtual int decide();

private:
	int movex[4] = { 1,0,-1,0 };
	int movey[4] = { 0,1,0,-1 };

	void judge();

	void getStepMap(int *,int, const int*)const;
	pair<int,int> getMaxStep(int *, int*)const;//opMax,myMax


	void simpleJudge();
	pair<int, int> explore(int, const int*) const;//totalStepNum, reachableBlockNum
	double compare(pair<int, int>)const;
	double evaluate(pair<int, int>)const;

};



#endif