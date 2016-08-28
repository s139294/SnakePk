/**************************************************************************

Author		: Dongming Sheng

Date		: 2016-08-16

File Name	: TwoSnakeAI.h

**************************************************************************/

#ifndef TWOSNAKEAI_H
#define TWOSNAKEAI_H

#include"SnakeAI.h"

class GameStarter;

class TwoSnakeAI :public SnakeAI{//can get ans within 2ms
public:
	TwoSnakeAI(GameStarter*, int);

	virtual int decide();

private:
	int movex[4] = { 1,0,-1,0 };
	int movey[4] = { 0,1,0,-1 };

	void judge();
	pair<int, int> explore(int, const int*) const;//totalStepNum, reachableBlockNum
	double evaluate(pair<int, int>)const;
};



#endif