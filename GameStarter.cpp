/**************************************************************************

	Author		: Dongming Sheng

	Date		: 2016-08-15

	File Name	: GameStarter.cpp

**************************************************************************/

#include<cstdlib>
#include<ctime>
#include<memory.h>
#include<iostream>
#include<iomanip>
#include<queue>
#include<list>
#include<assert.h>
using namespace std;
#include "GameStarter.h"

GameStarter::GameStarter(int m) {
	//initialize parameters:
	srand((unsigned)time(NULL));
	gameMode = m;

	round = 0;
	snakeNumber = 1;
	if (gameMode == Player_vs_AI || gameMode == TwoPlayers
		|| gameMode == AI_vs_AI)
		snakeNumber = 2;

	createMap();

	//generate snakes:
	directions[Player1] = Down;			//initiate direction
	directions[Player2] = Up;
	for (int i = 0; i < snakeNumber; ++i) {
		snakes[i] = new list<int>;
		moveHead(getStartPoint(i), i);
	}
	//generate food:
	if (snakeNumber == 1)
		updateNewFood();
}

GameStarter::~GameStarter() {
	delete[] map;
	map = nullptr;
	for (int i = 0; i < snakeNumber; ++i) {
		snakes[i]->clear();
		delete snakes[i];
		snakes[i] = nullptr;
	}
}

int GameStarter::getGameMode() const {
	return gameMode;
}

int GameStarter::getSizeX() const {
	return sizeX;
}

int GameStarter::getSizeY() const {
	return sizeY;
}

int GameStarter::getSize() const{
	return size;
}

const int* GameStarter::getMap() const {
	return map;
}

int GameStarter::getCurrentRound() const{
	return round;
}

const list<int> * GameStarter::getSnake(int snakeIndex) const {
	return snakes[snakeIndex];
}

int GameStarter::getRandomPath() const {
	int temp = 0;

	do {
		temp = rand() % size;
	} while (!isPath(temp));
	return temp;
}

int GameStarter::getStartPoint(int snakeIndex) const {
	if (snakeIndex == 0)
		return toMapIndex(1, 1);
	else
		return toMapIndex(getSizeX() - 2, getSizeY() - 2);
}

int GameStarter::getHeadIndex(int snakeIndex) const {
	return headIndexes[snakeIndex];
}

int GameStarter::getTailIndex(int snakeIndex) const {
	return snakes[snakeIndex]->back();
}

int GameStarter::getSnakeLength(int snakeIndex) const {
	return snakes[snakeIndex]->size();
}

int GameStarter::getSnakeNumber() const{
	return snakeNumber;
}

int GameStarter::getDirection(int snakeIndex) const {
	return directions[snakeIndex];
}

int GameStarter::getNeckIndex(int snakeIndex) const {
	assert(getSnakeLength(snakeIndex) >= 2);
	
	list<int>::iterator it = snakes[snakeIndex]->begin();
	return *(++it);
}

int GameStarter::getFootIndex(int snakeIndex) const {
	assert(getSnakeLength(snakeIndex) > 2);

	list<int>::iterator it = snakes[snakeIndex]->end();
	--it;
	--it;
	return *it;
}

int GameStarter::getRemainPathNumber() const{
	return getTotalPathNumber() - getSnakeLength(Player1);
}

int GameStarter::getTotalPathNumber() const{
	return size - ((sizeX + sizeY) * 2 - 4) - stoneNumber;
}

int GameStarter::getDistanceBetTwoIndex(int mapIndex1, int mapIndex2) const{
	pair<int, int> pos1 = toPosition(mapIndex1);
	pair<int, int> pos2 = toPosition(mapIndex2);
	return abs(pos1.first - pos2.first) + abs(pos1.second - pos2.second);
}

int GameStarter::getFoodIndex() const {
	return foodIndex;
}

int GameStarter::getGroundCondition(int targetIndex) const{
	if (targetIndex < 0 || targetIndex >= getSize())
		return -1;
	else
		return map[targetIndex];
}

pair<int, int> GameStarter::toPosition(int targetIndex) const {
	int y = getSizeY();
	int col = targetIndex % y;
	int row = targetIndex / y;
	return pair<int, int>(row, col);
}

int GameStarter::toMapIndex(int x0, int y0) const {
	return getSizeY() * x0 + y0;
}

bool GameStarter::isPath(int targetIndex) const{
	return map[targetIndex] == Path;
}

bool GameStarter::isFood(int targetIndex) const {
	return foodIndex == targetIndex;
}

bool GameStarter::isStone(int mapIndex) const {
	return map[mapIndex] == Stone;
}

bool GameStarter::isWall(int targetIndex) const {
	return map[targetIndex] == Wall;
}

bool GameStarter::isSnakeHead(int targetIndex, int snakeIndex) const{
	return targetIndex == headIndexes[snakeIndex];
}

bool GameStarter::isSnakeTail(int targetIndex, int snakeIndex) const{
	return targetIndex == getTailIndex(snakeIndex);
}

bool GameStarter::isSnakeBody(int targetIndex) const {
	return map[targetIndex] == SnakeOneBody 
		|| map[targetIndex] == SnakeTwoBody;
}

bool GameStarter::isStartPoint(int targetIndex) const {
	if (targetIndex == getStartPoint(Player1))
		return true;
	else if (snakeNumber == 2 && targetIndex == getStartPoint(Player2))
		return true;
	else
		return false;
}

bool GameStarter::isRoundGrow(int roundNum) const{
	return !(roundNum >= 10 && roundNum % 3 != 0);
}

bool GameStarter::isSingleModeWin() const {
	return getTotalPathNumber() == getSnakeLength(Player1);
}

void GameStarter::updateHeadIndex(int snakeIndex) {
	headIndexes[snakeIndex] = snakes[snakeIndex]->front();
}

void GameStarter::updateNewFood(){
	int availablePathNumber = getTotalPathNumber() - getSnakeLength(Player1);
	int random = rand() % availablePathNumber + 1;
	int cnt = 0;
	for (int index = 0; ;++index) {
		if (isPath(index))
			cnt++;
		if (cnt == random) {
			foodIndex = index;
			return;
		}
	}
}

void GameStarter::moveHead(int targetIndex, int snakeIndex) {
	//SnakeOneBody + 1 = SnakeTwoBody,snakeIndex = 0 or 1
	map[targetIndex] = SnakeOneBody + snakeIndex;
	snakes[snakeIndex]->push_front(targetIndex);
	
	updateHeadIndex(snakeIndex);
}

void GameStarter::cutTail(int snakeIndex) {
	map[getTailIndex(snakeIndex)] = Path;
	snakes[snakeIndex]->pop_back();
}

int GameStarter::move(int *moveDirs) {
	round++;
	int nextIndexes[MAXSNAKE];

	for (int i = 0; i < snakeNumber; ++i) {
		bool canGoBackward = getSnakeLength(i) == 1 && snakeNumber == 1;
		nextIndexes[i] = headIndexes[i];

		switch (moveDirs[i]) {
		case Up:
			if (directions[i] == Down && !canGoBackward)
				nextIndexes[i] += getSizeY();
			else {
				directions[i] = Up;
				nextIndexes[i] -= getSizeY();
			}
			break;

		case Down:
			if (directions[i] == Up && !canGoBackward)
				nextIndexes[i] -= getSizeY();
			else {
				directions[i] = Down;
				nextIndexes[i] += getSizeY();
			}
			break;

		case Right:
			if (directions[i] == Left && !canGoBackward)
				--nextIndexes[i];
			else {
				directions[i] = Right;
				++nextIndexes[i];
			}
			break;

		case Left:
			if (directions[i] == Right && !canGoBackward)
				++nextIndexes[i];
			else {
				directions[i] = Left;
				--nextIndexes[i];
			}
		}
	}

	if (snakeNumber == 1) {
		if (round != 1 && !isFood(nextIndexes[Player1]))
			cutTail(Player1);
		if (isPath(nextIndexes[Player1])) {
			if (isFood(nextIndexes[Player1])) {
				moveHead(nextIndexes[Player1], Player1);//will update the headIndex
				if (isSingleModeWin())
					return SingleModeWon;	//single player wins
				else {
					updateNewFood();
					return Grow;			//no need to cut the tail
				}
			}
			else if (round == 1) {
				moveHead(nextIndexes[Player1], Player1);
				return Grow;
			}
			else {
				moveHead(nextIndexes[Player1], Player1);
				return KeepTheLen;		//need to cut the tail
			}
		}
		else {
			moveHead(nextIndexes[Player1], Player1);
			return IllegalPosition;
		}
	}

	else {			
		if (!isRoundGrow(round))
			for (int i = 0; i < snakeNumber; ++i)
				cutTail(i);

		if (nextIndexes[Player1] == nextIndexes[Player2]
			|| !isPath(nextIndexes[Player1])
			&& !isPath(nextIndexes[Player2])) {
			for (int i = 0; i < snakeNumber; ++i)
				moveHead(nextIndexes[i], i);
			return Draw;
		}
		else if (isPath(nextIndexes[Player1]) && isPath(nextIndexes[Player2])) {
			for (int i = 0; i < snakeNumber; ++i)
				moveHead(nextIndexes[i], i);
			if (round >= 10 && round % 3 != 0)
				return KeepTheLen;
			else
				return Grow;
		}

		
		else if (!isPath(nextIndexes[Player1])) {
			for (int i = 0; i < snakeNumber; ++i)
				moveHead(nextIndexes[i], i);
			return Player1Lose;
		}

		else{
			for (int i = 0; i < snakeNumber; ++i)
				moveHead(nextIndexes[i], i);
			return Player2Lose;
		}

	}
}

void GameStarter::printMap() const {
	cout << endl;
	cout << "--------------------generated map--------------------" << endl;
	int size = getSize();
	int y = getSizeY();
	for (int i = 0; i < size; ++i) {
		if (i != 0 && i % y == 0)
			cout << endl;
		cout << setw(3) << map[i] << " ";
	}
	cout << endl;
}

void GameStarter::createMap() {
	bool connectivityCheck = false;

	while (!connectivityCheck) {
		int r1 = rand() * rand() % (MAXX - MINN + 1);
		sizeX = r1 + MINN;
		int r2 = rand() * rand() % (MAXY - MINN + 1);
		sizeY = r2 + MINN;
		size = sizeX * sizeY;		//initiate size

		map = new int[size];		//uninitialized values
		for (int i = 0; i < size; ++i)
			map[i] = 0;
		for (int i = 0; i < size; ++i)
			map[i] = Path;
		for (int i = 0; i < sizeY; ++i) {
			map[i] = Wall;
			map[(sizeX - 1) * sizeY + i] = Wall;
		}
		for (int i = 1; i <= sizeX - 2; ++i) {
			map[i * sizeY] = Wall;
			map[i * sizeY + sizeY - 1] = Wall;
		}

		stoneNumber = 0;

		if (snakeNumber == 1)	//skip the steps of placing stones and 
			return;				//connectivity check

		int stoneNumber = rand() * rand()
			% (MAXSTONE - MINSTONE + 1) + MINSTONE;
		int temp;
		for (int i = 0; i < stoneNumber; i++) {
			temp = getRandomPath();
			while (temp == getStartPoint(Player1)
				|| temp == getStartPoint(Player2)
				|| temp == getStartPoint(Player1) + sizeY
				|| temp == getStartPoint(Player2) - sizeY) {
				temp = getRandomPath();
			}
			map[temp] = Stone;
		}
		
		connectivityCheck = isConnected();
	}
}

bool GameStarter::isConnected() const {
	bool vis[MAXX][MAXY];
	int movex[4] = { 1,-1,0,0 };
	int movey[4] = { 0,0,1,-1 };
	memset(vis, false, sizeof(vis));
	int rows = getSizeX();
	int cols = getSizeY();
	int targetIndex = getStartPoint(Player2);

	struct node {
		int x, y;
		node(int p = 0, int q = 0) :x(p), y(q) {}
	}cur;

	queue<node> que;
	que.push(node(1, 1));
	vis[1][1] = true;
	while (!que.empty()) {
		cur = que.front();
		que.pop();
		int p, q, nextIndex;
		for (int i = 0; i < 4; ++i) {
			p = cur.x + movex[i];
			q = cur.y + movey[i];

			if (p <= 0 || q <= 0 || p >= rows || q >= cols
				|| vis[p][q])
				continue;
			nextIndex = toMapIndex(p, q);
			if (nextIndex == targetIndex)
				return true;
			if (!isPath(nextIndex))
				continue;
			que.push(node(p, q));
			vis[p][q] = true;
		}
	}
	return false;
}
