/**************************************************************************

Author		: Dongming Sheng

Date		: 2016-08-14

File Name	: SingleAI.cpp

**************************************************************************/

#include<cmath>
#include "SingleAI.h"
#include "GameStarter.h"

SingleAI::SingleAI(GameStarter* gamestarter, int snakeIndex)
	:SnakeAI(gamestarter, snakeIndex) {

}


int SingleAI::decide(){
	//strategy 1: Can eat the food and find the tail at the same time
	if (decisions->empty()) {
		tryToFindDirectPath();
	}
	
	//strategy 2: Find a block around the head where you can find the tail 
		//and away from food
	if (decisions->empty()) {
		beAwayFromFood();
	}

	//in case no way to go
	if (decisions->empty())
		return GameStarter::Up;
	
	int decision = decisions->front();
	decisions->pop();

	return changeMapIndexToDir(decision, snakeIndex);
}

void SingleAI::tryToFindDirectPath(){
	int headIndex = game->getHeadIndex(snakeIndex);
	int foodIndex = game->getFoodIndex();
	int sizeY = game->getSizeY();
	stack<int> directRoadToFood(bfs(game->getMap(), headIndex, foodIndex));

	//situations when there is no need to find the tail(winning)
	if (game->getRemainPathNumber() == 1) {
		pushDecisions(directRoadToFood);
	}

	else if (game->getRemainPathNumber() == 2 
		&& game->getDistanceBetTwoIndex(foodIndex, headIndex) == 1) {
		if (game->isPath(foodIndex + sizeY) || game->isPath(foodIndex - sizeY)
			|| game->isPath(foodIndex + 1) || game->isPath(foodIndex - 1))
			pushDecisions(directRoadToFood);
	}

	else if(canFindTheTail(directRoadToFood)){
		pushDecisions(directRoadToFood);
	}
}

void SingleAI::beAwayFromFood(){
	stack<int> steps;
	int step = 0;
	int headIndex = game->getHeadIndex(snakeIndex);
	int foodIndex = game->getFoodIndex();
	int tailIndex = game->getTailIndex(snakeIndex);
	int sizeY = game->getSizeY();

	int recordDisToFood = -1;
	int recordDisToTail = -1;
	int decidedIndex = -1;
	for (int i = 0; i < 4; ++i) {
		while(!steps.empty())
			steps.pop();
		if (i == 0)
			step = -1;
		else if (i == 1)
			step = 1;
		else if (i == 2)
			step = sizeY;
		else
			step = -sizeY;
		int curIndex = headIndex + step;

		//road checking
		if (!game->isPath(curIndex) 
			&& !game->isSnakeTail(curIndex, snakeIndex))
			continue;

		steps.push(curIndex);

		if (canFindTheTail(steps, false)) {
			int disToFood = game->getDistanceBetTwoIndex(foodIndex, curIndex);
			int disToTail = game->getDistanceBetTwoIndex(tailIndex, curIndex);

			if (disToFood >= recordDisToFood) {
				if (disToFood == recordDisToFood 
					&& disToTail < recordDisToTail)
					continue;
				recordDisToFood = disToFood;
				recordDisToTail = disToTail;
				decidedIndex = curIndex;
			}
			
		}

	}

	if (decidedIndex == -1)
		return;

	while (!steps.empty())
		steps.pop();
	steps.push(decidedIndex);
	pushDecisions(steps);
}

bool SingleAI::canFindTheTail(stack<int> roadInRev, bool getToFood){
	//no road checking
	//simulation:
	const list<int> *curSnake = game->getSnake(snakeIndex);
	const int *curMap = game->getMap();
	int size = game->getSize();
	int *map = new int[size];
	for (int i = 0; i < size; ++i)
		map[i] = curMap[i];
	list<int> *snake = new list<int>;
	list<int>::const_iterator it;
	for (it = curSnake->begin(); it != curSnake->end(); ++it) {
		snake->push_back(*it);
	}

	int len = roadInRev.size();
	for (int i = len; i >= 1; --i) {
		int head = roadInRev.top();
		if (i != 1 || i == 1 && !getToFood) {
			int tail = snake->back();
			map[tail] = game->Path;
			snake->pop_back();
		}	
		snake->push_front(head);
		map[head] = game->SnakeOneBody;
		roadInRev.pop();
	}
	stack<int> roadToTail(bfs(map, snake->front(), snake->back()));
	delete[] map;
	delete snake;
	if (!roadToTail.empty()) 
		return true;
	return false;
}

stack<int> SingleAI::bfs(const int *map, int startIndex, int targetIndex) const {
	int size = game->getSize();
	int *lastStepMap = new int[size];
	for (int i = 0; i < size; ++i)
		lastStepMap[i] = 0;
	bool vis[GameStarter::MAXX][GameStarter::MAXY];
	memset(vis, false, sizeof(vis));

	pair<int, int> startPos = game->toPosition(startIndex);
	stack<int> roadToTarget;

	int rows = game->getSizeX();
	int cols = game->getSizeY();

	struct node {
		int x, y, mapIndex;
		node(int p = 0, int q = 0, int index = 0)
			:x(p), y(q), mapIndex(index) {}
	}cur;

	queue<node> que;
	que.push(node(startPos.first, startPos.second, startIndex));
	vis[startPos.first][startPos.second] = true;
	lastStepMap[startIndex] = -1;

	while (!que.empty()) {
		cur = que.front();
		int curIndex = cur.mapIndex;
		que.pop();
		int p, q, nextIndex;
		for (int i = 0; i < 4; ++i) {
			p = cur.x + movex[i];
			q = cur.y + movey[i];

			if (p <= 0 || q <= 0 || p >= rows || q >= cols
				|| vis[p][q])
				continue;
			nextIndex = game->toMapIndex(p, q);

			if (nextIndex == targetIndex) {
				lastStepMap[nextIndex] = curIndex;

				//in reverse order
				int curStepIndex, lastStepIndex;
				curStepIndex = targetIndex;
				do {
					lastStepIndex = lastStepMap[curStepIndex];
					roadToTarget.push(curStepIndex);
					curStepIndex = lastStepIndex;
				} while (lastStepIndex != startIndex);
				delete[] lastStepMap;
				return roadToTarget;
			}

			if (map[nextIndex] != GameStarter::Path)
				continue;

			lastStepMap[nextIndex] = curIndex;
			que.push(node(p, q, nextIndex));
			vis[p][q] = true;
		}
	}
	delete[] lastStepMap;
	return roadToTarget;
}

void SingleAI::followTheTail(){
	//change moving order
	int r1 = rand() % 4;
	int r2 = r1;
	while (r2 == r1) {
		r2 = rand() % 4;
	}
	swap(movex[r1], movex[r2]);
	swap(movey[r1], movey[r2]);

	int curIndex = game->getHeadIndex(GameStarter::Player1);
	int tailIndex = game->getTailIndex(GameStarter::Player1);

	stack<int> roadToTail(bfs(game->getMap(),curIndex, tailIndex));
	pushDecisions(roadToTail);
}

void SingleAI::dfs(int startIndex, int targetIndex) const{
	int map[GameStarter::MAXX][GameStarter::MAXY];
	bool vis[GameStarter::MAXX][GameStarter::MAXY];
	memset(map, 0, sizeof(map));
	memset(vis, 0, sizeof(vis));
	int rows = game->getSizeX();
	int cols = game->getSizeY();
	
	//int bestMap[GameStarter::MAXX][GameStarter::MAXY];
	int longestDis = 0;
	int tot = 0;

	struct node {
		int x, y, cnt, p;
		bool isnew;
		node(int _x = 0, int _y = 0, int _cnt = 0, int _p = 0)
			:x(_x), y(_y), cnt(_cnt), p(_p) {
			isnew = true;
		}
	}*cur;

	stack<node> st;
	pair<int, int> startPoint = game->toPosition(startIndex);
	pair<int, int> foodPoint = game->toPosition(game->getFoodIndex());
	st.push(node(startPoint.first, startPoint.second, 1, 0));
	while (!st.empty()) {
		cur = &st.top();
		vis[cur->x][cur->y] = true;
		map[cur->x][cur->y] = cur->cnt;

		if (cur->isnew) {
			if (game->toMapIndex(cur->x, cur->y) == targetIndex) {
				tot = cur->cnt;
				break;
			}
			/*int dis = abs(foodPoint.first - cur->x)
				+ abs(foodPoint.second - cur->y);
			if (dis > longestDis) {
				longestDis = dis;
				tot = cur->cnt;
				memcpy(bestMap, map, sizeof(map));
				if(tot >= 5)
					break;
			}*/
		}

		bool ok = false;
		for (int i = cur->p; i < 4; ++i) {
			int p = cur->x + movex[i];
			int q = cur->y + movey[i];
			if (p < 0 || p >= rows || q < 0 || q >= cols || vis[p][q]) {
				continue;
			}
			if (!game->isPath(game->toMapIndex(p, q)))
				continue;
			cur->isnew = false;
			cur->p = i + 1;
			st.push(node(p, q, cur->cnt + 1, 0));
			ok = true;
			break;
		}
		if (!ok) {
			vis[cur->x][cur->y] = false;
			st.pop();
		}
	}

	int curx = startPoint.first;
	int cury = startPoint.second;
	for (int i = 2; i <= tot; ++i) {
		int p, q;
		for (int j = 0; j < 4; ++j) {
			p = movex[j] + curx;
			q = movey[j] + cury;
			if (p < 0 || q < 0 || p >= rows || q >= cols)
				continue;
			if (map[p][q] == i)
				break;
		}
		int index = game->toMapIndex(p, q);
		decisions->push(index);
		curx = p;
		cury = q;
	}
}
