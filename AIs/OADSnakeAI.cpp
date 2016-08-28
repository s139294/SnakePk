/**************************************************************************

Author		: Dongming Sheng

Date		: 2016-08-27

File Name	: OADSnakeAI.cpp

**************************************************************************/

#include "OADSnakeAI.h"
#include "GameStarter.h"
#include<memory.h>
using namespace std;

OADSnakeAI::OADSnakeAI(GameStarter *gameStarter, int snakeIndex)
	:SnakeAI(gameStarter, snakeIndex) {
}

int OADSnakeAI::decide() {
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

void OADSnakeAI::judge() {
	//create the map
	int cols = game->getSizeY();
	int rows = game->getSizeX();
	int size = game->getSize();

	int *nextStepMap = new int[size];
	for (int i = 0; i < size; ++i)
		nextStepMap[i] = 0;
	for (int i = 0; i < size; ++i) {
		if (!game->isPath(i))
			nextStepMap[i] = -1;
	}

	//tail position reachable?
	if (!game->isRoundGrow(game->getCurrentRound() + 1)) {
		int tail_op = game->getTailIndex(1 - snakeIndex);
		int tail_self = game->getTailIndex(snakeIndex);
		nextStepMap[tail_op] = nextStepMap[tail_self] = 0;
	}
	
	int *myStepMap = new int[size];
	int *opStepMap = new int[size];

	int myHeadIndex = game->getHeadIndex(snakeIndex);
	int opHeadIndex = game->getHeadIndex(1 - snakeIndex);

	pair<int, int> gameBoard[3][3];
	int myDecisionIndex[3];

	int opDecisionCnt = 0;
	int myDecisionCnt = 0;

#pragma region create GameBoard
	for (int i = 0; i < 4; ++i) {
		int opNextStep;
		if (i == GameStarter::Left)
			opNextStep = opHeadIndex - 1;
		else if (i == GameStarter::Up)
			opNextStep = opHeadIndex - cols;
		else if (i == GameStarter::Right)
			opNextStep = opHeadIndex + 1;
		else
			opNextStep = opHeadIndex + cols;
		if (nextStepMap[opNextStep] == -1)
			continue;
		//nextStepMap[opNextStep] = -1;

		myDecisionCnt = 0;
		for (int j = 0; j < 4; ++j) {
			int myNextStep;
			if (j == GameStarter::Left)
				myNextStep = myHeadIndex - 1;
			else if (j == GameStarter::Up)
				myNextStep = myHeadIndex - cols;
			else if (j == GameStarter::Right)
				myNextStep = myHeadIndex + 1;
			else
				myNextStep = myHeadIndex + cols;
			if (nextStepMap[myNextStep] == -1)
				continue;
			//nextStepMap[myNextStep] = -1;
			getStepMap(myStepMap, myNextStep, nextStepMap);
			getStepMap(opStepMap, opNextStep, nextStepMap);

			gameBoard[opDecisionCnt][myDecisionCnt]
				= getMaxStep(opStepMap, myStepMap);
			myDecisionIndex[myDecisionCnt] = myNextStep;

			myDecisionCnt++;

			//nextStepMap[myNextStep] = 0;
		}
		//nextStepMap[opNextStep] = 0;
		opDecisionCnt++;
	}
#pragma endregion

	delete[] nextStepMap;
	nextStepMap = nullptr;
	delete[] myStepMap;
	myStepMap = nullptr;
	delete[] opStepMap;
	opStepMap = nullptr;

#pragma region simple decisions
	//no way to go
	if (myDecisionCnt == 0) {
		return;
	}

	//I have only one strategy:
	if (myDecisionCnt == 1) {
		decisions->push(myDecisionIndex[0]);
		return;
	}

	//op. have only one strategy:
	if (opDecisionCnt == 1) {
		double bestValue = -1000;
		int myBR = 0;
		for (int i = 0; i < myDecisionCnt; ++i) {
			double value = compare(gameBoard[0][i]);
			if (value > bestValue) {
				myBR = i;
				bestValue = gameBoard[0][i].second;
			}
		}
		decisions->push(myDecisionIndex[myBR]);
		return;
	}
#pragma endregion

	//check for dominated strategy
	bool myNotGoodStrategy[3];
	bool opNotGoodStrategy[3];
	memset(myNotGoodStrategy, 0, sizeof(myNotGoodStrategy));
	memset(opNotGoodStrategy, 0, sizeof(opNotGoodStrategy));

	bool hasChosen = true;

	//while (hasChosen) {
	//	hasChosen = false;
	//	for (int i = 0; i < opDecisionCnt; ++i) {//chose row
	//		if (opNotGoodStrategy[i])
	//			continue;
	//		bool fail = false;
	//		for (int j = 0; !fail && j < opDecisionCnt; ++j) {//compared rows
	//			if (j == i || opNotGoodStrategy[j]) 
	//				continue;
	//			for (int k = 0; k < myDecisionCnt; ++k) {//for every col
	//				if (myNotGoodStrategy[k])
	//					continue;
	//				if (gameBoard[i][k].first >= gameBoard[j][k].first) {
	//					fail = true;
	//					break;
	//				}
	//			}
	//		}
	//		if (!fail) {
	//			hasChosen = true;
	//			opNotGoodStrategy[i] = true;
	//		}
	//	}
	//}

	hasChosen = true;
	while (hasChosen) {
		hasChosen = false;
		for (int i = 0; i < myDecisionCnt; ++i) {//chose col
			if (myNotGoodStrategy[i])
				continue;
			bool fail = false;
			for (int j = 0; !fail && j < myDecisionCnt; ++j) {//compared cols
				if (j == i || myNotGoodStrategy[j])
					continue;
				for (int k = 0; k < opDecisionCnt; ++k) {//for every row
					if (opNotGoodStrategy[k])
						continue;
					if (gameBoard[k][i].second >= gameBoard[k][j].second) {
						fail = true;
						break;
					}
				}
			}
			if (!fail) {
				hasChosen = true;
				myNotGoodStrategy[i] = true;
			}
		}
	}


#pragma region check for pure strategy to hurt op.(take advantage)
	double myHighestAver = -1;
	int myBR = -1;

	for (int i = 0; i < myDecisionCnt; ++i) {//chose col
		if (myNotGoodStrategy[i])
			continue;
		bool fail = false;
		for (int k = 0; k < opDecisionCnt; ++k) {
			//cannot take advantage:
			if (gameBoard[k][i].first >= gameBoard[k][i].second) {
				fail = true;
				break;
			}
		}
		if (fail)
			continue;

		for (int j = 0; !fail && j < myDecisionCnt; ++j) {//compared cols
			if (j == i || myNotGoodStrategy[j]) 
				continue;
			for (int k = 0; k < opDecisionCnt; ++k) {//for every row
				if (opNotGoodStrategy[k])
					continue;
				if (gameBoard[k][i].first > gameBoard[k][j].first) {
					fail = true;
					break;
				}
			}
		}
		if (!fail) {
			double average = 0;
			for (int j = 0; j < myDecisionCnt; ++j)
				average += gameBoard[j][i].second;
			average = average * 1.0 / myDecisionCnt;
			if (average > myHighestAver) {
				myHighestAver = average;
				myBR = i;
			}
		}
	}
	if (myBR != -1) {
		decisions->push(myDecisionIndex[myBR]);//kill enemy
		return;
	}
#pragma endregion

#pragma region check if op. have a dominant strategy
	int opBR = -1;
	int myLowestAver = 1000;
	for (int i = 0; i < opDecisionCnt; ++i) {//chose row
		bool fail = false;
		if (opNotGoodStrategy[i])
			continue;
		for (int j = 0; !fail && j < opDecisionCnt; ++j) {//compared rows
			if (j == i || opNotGoodStrategy[j]) 
				continue;
			for (int k = 0; k < myDecisionCnt; ++k) {//for every col
				if (myNotGoodStrategy[k])
					continue;
				if (gameBoard[i][k].first < gameBoard[j][k].first) {
					fail = true;
					break;
				}
			}
		}
		if (!fail) {
			double average = 0;
			for (int j = 0; j < myDecisionCnt; ++j)
				average += gameBoard[i][j].second;
			average = average * 1.0 / myDecisionCnt;
			if (average < myLowestAver) {
				myLowestAver = average;
				opBR = i;
			}
		}
	}
	if (opBR != -1) {
		//I have to choose the best response to reduce the damage
		int myResponse = 0;
		int myBestValue = gameBoard[opBR][0].second;
		for (int i = 1; i < myDecisionCnt; ++i) {
			if (gameBoard[opBR][i].second > myBestValue) {
				myBestValue = gameBoard[opBR][i].second;
				myResponse = i;
			}
		}
		decisions->push(myDecisionIndex[myResponse]);
		return;
	}
#pragma endregion

#pragma region check for pure strategy(longest path for myself)
	myBR = -1;
	double opLowestAver = 10000;

	for (int i = 0; i < myDecisionCnt; ++i) {//chose col
		if (myNotGoodStrategy[i])
			continue;
		bool fail = false;
		for (int j = 0; !fail && j < myDecisionCnt; ++j) {//compared cols
			if (j == i || myNotGoodStrategy[j])
				continue;
			for (int k = 0; k < opDecisionCnt; ++k) {//for every row
				if (opNotGoodStrategy[k])
					continue;
				if (gameBoard[k][i].second < gameBoard[k][j].second) {
					fail = true;
					break;
				}
			}
		}
		if (!fail) {
			double average = 0;
			for (int j = 0; j < opDecisionCnt; ++j)
				average += gameBoard[j][i].first;
			average = average * 1.0 / opDecisionCnt;
			if (average < opLowestAver) {
				opLowestAver = average;
				myBR = i;
			}
		}
	}
	if (myBR != -1) {
		decisions->push(myDecisionIndex[myBR]);//pick longest path
		return;
	}
#pragma endregion

#pragma region check for pure strategy to hurt op.(get close to op.)
	myHighestAver = -1;
	myBR = -1;

	for (int i = 0; i < myDecisionCnt; ++i) {//chose col
		if (myNotGoodStrategy[i])
			continue;
		bool fail = false;
		for (int j = 0; !fail && j < myDecisionCnt; ++j) {//compared cols
			if (j == i || myNotGoodStrategy[j]) 
				continue;
			for (int k = 0; k < opDecisionCnt; ++k) {//for every row
				if (opNotGoodStrategy[k])
					continue;
				if (gameBoard[k][i].first > gameBoard[k][j].first) {
					fail = true;
					break;
				}
			}
		}
		if (!fail) {
			double average = 0;
			for (int j = 0; j < myDecisionCnt; ++j)
				average += gameBoard[j][i].second;
			average = average * 1.0 / myDecisionCnt;
			if (average > myHighestAver) {
				myHighestAver = average;
				myBR = i;
			}
		}
	}
	if (myBR != -1) {
		decisions->push(myDecisionIndex[myBR]);//kill enemy
		return;
	}
#pragma endregion
	
	//simpleJudge();
	myHighestAver = -100;
	myBR = -1;
	for (int i = 0; i < myDecisionCnt; ++i) {
		double average = 0;
		if (myNotGoodStrategy[i])
			continue;
		for (int j = 0; j < opDecisionCnt; ++j) 
			average += gameBoard[j][i].second;
		
		average = average * 1.0 / opDecisionCnt;
		if (average > myHighestAver) {
			average = myHighestAver;
			myBR = i;
		}

	}
	if (myBR != -1) {
		decisions->push(myDecisionIndex[myBR]);
		return;
	}
}

void OADSnakeAI::getStepMap(int * resStepMap, int nextStepIndex,
	const int *map) const{
	//resStepMap for returning the info
	for (int i = 0; i < game->getSize(); ++i)
		resStepMap[i] = map[i];

	int rows = game->getSizeX();
	int cols = game->getSizeY();
	int size = game->getSize();

	pair<int, int> startPos = game->toPosition(nextStepIndex);

	struct node {
		int x, y, cnt, mapIndex;
		node(int p = 0, int q = 0, int _cnt = 0, int index = 0)
			:x(p), y(q), cnt(_cnt), mapIndex(index) {}
	}cur;

	queue<node> que;
	que.push(node(startPos.first, startPos.second, 1, nextStepIndex));
	resStepMap[nextStepIndex] = 1;

	while (!que.empty()) {
		cur = que.front();
		que.pop();
		
		int p, q, nextIndex;
		for (int i = 0; i < 4; ++i) {
			p = cur.x + movex[i];
			q = cur.y + movey[i];
			nextIndex = game->toMapIndex(p, q);

			if (p <= 0 || q <= 0 || p >= rows || q >= cols ||
				resStepMap[nextIndex] != 0)
				continue;

			que.push(node(p, q, cur.cnt + 1, nextIndex));
			resStepMap[nextIndex] = cur.cnt + 1;
		}
	}
	/*int test[GameStarter::MAXX][GameStarter::MAXY];
	for (int i = 0; i < game->getSize(); ++i) {
		pair<int, int> pos = game->toPosition(i);
		test[pos.first][pos.second] = resStepMap[i];
	}*/
}

pair<int, int> OADSnakeAI::getMaxStep(int *opStepMap, int *myStepMap) const{
	int opMax = 0;
	int myMax = 0;
	int size = game->getSize();
	for (int i = 0; i < size; ++i) {
		if(opStepMap[i] <= 0 && myStepMap[i] > 0)
			myMax = max(myStepMap[i], myMax);

		else if(myStepMap[i] <= 0 && opStepMap[i] > 0)
			opMax = max(opStepMap[i], opMax);

		else if (opStepMap[i] <= myStepMap[i]) //op arrive first
			opMax = max(opStepMap[i], opMax);

		else if (myStepMap[i] <= opStepMap[i]) //I arrive first
			myMax = max(myStepMap[i], myMax);

	}	
	return pair<int, int>(opMax, myMax);
}

void OADSnakeAI::simpleJudge() {
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
		pair<int, int> res = explore(move_self, stepsToGet);
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

pair<int, int> OADSnakeAI::explore(int startIndex, const int* map)const {
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

double OADSnakeAI::compare(pair<int, int> res) const {
	//op: res.first, me: res.second
	if (res.second > res.first)
		return res.second - res.first * 1.2;
	else
		return res.second * 1.2 - res.first;
}

double OADSnakeAI::evaluate(pair<int, int> res) const {
	return res.second * 1000 * 1.0 - res.first;
}
