/**************************************************************************

	Author		: Dongming Sheng

	Date		: 2016-08-15

	File Name	: GameStarter.h

**************************************************************************/

#ifndef GAMESTARTER_H
#define GAMESTARTER_H

//#define TwoPlayersNotAvailable
//#define NoCamera

#include<list>
using namespace std;
//template class list<int>;

class GameStarter {
	friend class MainWindow;
public:
	enum Limits {
		MAXX = 13, MAXY = 15, MINN = 11,
		MAXSTONE = 12, MINSTONE = 7, MAXSNAKE = 2
	};
	enum Directions {
		Left, Up, Right, Down,
		top_left, bottom_left,
		top_right, bottom_right
	};
	enum GameResponds {
		KeepTheLen, Grow, IllegalPosition, SingleModeWon,	//singleMode
		Draw, Player1Lose, Player2Lose						//doubleMode
	};
	enum Players{Player1, Player2, Player3};
	enum GameModes {
		SinglePlayer, SingleAI, Player_vs_AI, AI_vs_AI, TwoPlayers
	};
	enum GroundConditions {
		Wall, Stone, Path, SnakeOneBody, SnakeTwoBody, SnakeThreeBody
	};

	GameStarter(int);
	~GameStarter();

#pragma region get-funcitons
	int getGameMode() const;
	const int* getMap() const;				 //pass map
	int getCurrentRound() const;
	int getSizeX() const;
	int getSizeY() const;
	int getSize() const;				 //number of blocks
	int getStartPoint(int) const;		 //start point of the snake
	int getFoodIndex() const;
	int getGroundCondition(int) const;

	const list<int>* getSnake(int) const;
	int getSnakeLength(int) const;
	int getSnakeNumber() const;
	int getHeadIndex(int) const;
	int getTailIndex(int) const;
	int getDirection(int) const;
	int getNeckIndex(int) const;		 //Head-Neck-Body-Foot-Tail
	int getFootIndex(int) const;
	int getRemainPathNumber() const;	 //for single-mode only
	int getTotalPathNumber() const;		 //number of path in the map
	int getDistanceBetTwoIndex(int, int) const;
#pragma endregion
	pair<int, int> toPosition(int) const;//mapIndex to position(x,y)
	int toMapIndex(int, int) const;		 //position(x,y) to mapIndex

	bool isPath(int) const;
	bool isStone(int) const;
	bool isFood(int) const;
	bool isWall(int) const;
	bool isSnakeHead(int, int) const;
	bool isSnakeTail(int, int) const;
	bool isSnakeBody(int) const;
	bool isStartPoint(int) const;
	bool isRoundGrow(int) const;		 //for double-mode only
										 //for AIs: parameter should be 
										 //			the next round number
	void printMap() const;				 //test only

private:
	int gameMode;
	int snakeNumber;
	int stoneNumber;
	int round;

	int sizeX;							 //number of rows
	int sizeY;							 //number of columns
	int size;							 //number of blocks
	int* map;
	int foodIndex;

	list<int> *snakes[MAXSNAKE];		 //snakes
	int directions[MAXSNAKE];			 //snakes' last move
	int headIndexes[MAXSNAKE];			 //snakes' head index

	int move(int *);					 //snake moves: receive arguments 
										 //				from AI or keyboard

	//Module 1: initialization module
	void createMap();					 //include placing stones
	bool isConnected() const;			 //connectivity check(bfs)
	int getRandomPath() const;			 //get path to place stones

	//Module 2: moving module
	void moveHead(int, int);			 //move snakes' heads
										 //(update headIndex simultaneously)
	void cutTail(int);					 //cut snakes' tails
	void updateHeadIndex(int);
	void updateNewFood();

	bool isSingleModeWin() const;		 //judge whether a single snake wins

};
#endif

