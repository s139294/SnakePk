/**************************************************************************

Author		: Dongming Sheng

Date		: 2016-08-16

File Name	: TwoSnakeAI.cpp

**************************************************************************/

#include "TwoSnakeAI.h"
#include "GameStarter.h"
#include<memory.h>
using namespace std;

TwoSnakeAI::TwoSnakeAI(GameStarter *gameStarter, int snakeIndex)
	:SnakeAI(gameStarter, snakeIndex){
}

int TwoSnakeAI::decide(){
	judge();
	if (decisions->empty()) {
		int curIndex = game->getHeadIndex(snakeIndex);
		if (game->isPath(curIndex + 1))
			return GameStarter::Right;
		else if (game->isPath(curIndex + game->getSizeY()))
			return GameStarter::Down;
		else if (game->isPath(curIndex - game->getSizeY()))
			return GameStarter::Up;
		else
			return GameStarter::Left;
	}
	
	int decision = decisions->front();
	decisions->pop();

	return changeMapIndexToDir(decision, snakeIndex);
}

void TwoSnakeAI::judge(){
	//create the map
	int cols = game->getSizeY();
	int rows = game->getSizeX();
	int size = game->getSize();
	int *stepsToGet = new int[size];
	for (int i = 0; i < size; ++i)
		stepsToGet[i] = 0;
	for (int i = 0; i < size; ++i) {
		if (!game->isPath(i))
			stepsToGet[i] = -1;
	}

	//tail position reachable?
	if (!game->isRoundGrow(game->getCurrentRound() + 1)) {
		int tail_op = game->getTailIndex(1 - snakeIndex);
		int tail_self = game->getTailIndex(snakeIndex);
		stepsToGet[tail_op] = stepsToGet[tail_self] = 0;
	}

	//opponent's possible movement
	int head_op = game->getHeadIndex(1 - snakeIndex);
	int head_self = game->getHeadIndex(snakeIndex);

	stepsToGet[head_op + 1] = stepsToGet[head_op - 1] 
		= stepsToGet[head_op + cols] = stepsToGet[head_op - cols] = -1;

	int chooseIndex = -1;
	double bestEval = -1;
	for (int i = 0; i < 4; ++i) {
		int move_self;
		if (i == 0)
			move_self = head_self + 1;
		else if (i == 1)
			move_self = head_self - 1;
		else if (i == 2)
			move_self = head_self + cols;
		else
			move_self = head_self - cols;
		if (!game->isPath(move_self))
			continue;
		pair<int,int> res = explore(move_self, stepsToGet);
		double eval = evaluate(res);
		if (eval > bestEval) {
			bestEval = eval;
			chooseIndex = move_self;
		}
	}
	if (chooseIndex != -1)
		decisions->push(chooseIndex);

	delete[] stepsToGet;
}

pair<int, int> TwoSnakeAI::explore(int startIndex, const int* map)const{
	bool vis[GameStarter::MAXX][GameStarter::MAXY];
	memset(vis, 0, sizeof(vis));

	int rows = game->getSizeX();
	int cols = game->getSizeY();
	int size = game->getSize();

	int* stepsToGet = new int[size];
	for (int i = 0; i < size; ++i)
		stepsToGet[i] = map[i];

	pair<int, int> startPos = game->toPosition(startIndex);

	struct node {
		int x, y, cnt, mapIndex;
		node(int p = 0, int q = 0, int _cnt = 0, int index = 0)
			:x(p), y(q), cnt(_cnt), mapIndex(index) {}
	}cur;

	queue<node> que;
	que.push(node(startPos.first, startPos.second, 0, startIndex));
	vis[startPos.first][startPos.second] = true;

	while (!que.empty()) {
		cur = que.front();
		que.pop();
		stepsToGet[cur.mapIndex] = cur.cnt;

		int p, q, nextIndex;
		for (int i = 0; i < 4; ++i) {
			p = cur.x + movex[i];
			q = cur.y + movey[i];
			nextIndex = game->toMapIndex(p, q);

			if (p <= 0 || q <= 0 || p >= rows || q >= cols || vis[p][q])
				continue;

			if (stepsToGet[nextIndex] == -1)
				continue;

			que.push(node(p, q, cur.cnt + 1, nextIndex));
			vis[p][q] = true;
		}
	}

	int reachableBlockNum = 0;
	int totalStepNum = 0;
	for (int i = 0; i < size; ++i) {
		if (stepsToGet[i] > 0) {
			reachableBlockNum++;
			totalStepNum += stepsToGet[i];
		}
	}
	delete[] stepsToGet;
	return pair<int, int>(totalStepNum, reachableBlockNum);
}

double TwoSnakeAI::evaluate(pair<int, int> res) const{
	return res.second * 1000 * 1.0 - res.first;
}

