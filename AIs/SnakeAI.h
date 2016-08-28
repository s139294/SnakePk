/**************************************************************************

Author		: Dongming Sheng

Date		: 2016-08-15

File Name	: SnakeAI.h

**************************************************************************/

#ifndef SNAKEAI_H
#define SNAKEAI_H

#include<queue>
#include<stack>
using namespace std;

class GameStarter;

class SnakeAI {
public:
	enum DecisionType { MapIndex, Direction };
	SnakeAI(GameStarter*, int);
	~SnakeAI();

	virtual int decide() = 0;	//return directions:Left/Up/Right/Down

protected:
	int snakeIndex;
	GameStarter* game;
	queue<int>* decisions;		//store mapIndexes

	void pushDecisions(stack<int>&);
	void clearMyQueue();
	int changeMapIndexToDir(int, int) const;//contain checking procedure

private:
};

#endif
