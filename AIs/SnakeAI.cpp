/**************************************************************************

Author		: Dongming Sheng

Date		: 2016-08-14

File Name	: SingleAI.cpp

**************************************************************************/

#include "SnakeAI.h"
#include"GameStarter.h"

SnakeAI::SnakeAI(GameStarter* gamestarter, int index) 
	:game(gamestarter), snakeIndex(index) {
	decisions = new queue<int>;
}

SnakeAI::~SnakeAI() {
	if (decisions != nullptr) {
		delete decisions;
		decisions = nullptr;
	}	
}

int SnakeAI::changeMapIndexToDir(int mapIndex, int snakeIndex) const {
	int head_dir = game->getHeadIndex(snakeIndex);
	int temp = mapIndex - head_dir;

	if (temp == 1)
		return GameStarter::Right;
	else if (temp == -1)
		return GameStarter::Left;
	else if (temp == game->getSizeY())
		return GameStarter::Down;
	else if (temp == -game->getSizeY())
		return GameStarter::Up;
	else
		throw invalid_argument("mapIndex error, decision error");
}

void SnakeAI::pushDecisions(stack<int>& indexInRevOrder) {
	//directions in stack are in the reverse order
	while (!indexInRevOrder.empty()) {
		int top = indexInRevOrder.top();
		indexInRevOrder.pop();
		decisions->push(top);
	}
}

void SnakeAI::clearMyQueue() {
	if (decisions != nullptr) {
		while (!decisions->empty())
			decisions->pop();
	}
}

