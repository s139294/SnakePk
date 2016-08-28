/**************************************************************************

Author		: Dongming Sheng

Date		: 2016-08-14

File Name	: SingleAI.h

**************************************************************************/

#ifndef SINGLEAI_H
#define SINGLEAI_H

#include"SnakeAI.h"
class GameStarter;

class SingleAI :public SnakeAI {//can get ans within 2ms
public:
	SingleAI(GameStarter*,int);

	virtual int decide();

private:
	int movex[4] = { 1,0,-1,0 };
	int movey[4] = { 0,1,0,-1 };

	void tryToFindDirectPath();	//strategy 1
	void beAwayFromFood();		//strategy 2
	void followTheTail();		//strategy 3(abandoned: not effecient)

	bool canFindTheTail(stack<int>, bool = true);

	stack<int> bfs(const int *, int, int) const;
	void dfs(int, int) const;//(abandoned: too slow)
};

#endif
