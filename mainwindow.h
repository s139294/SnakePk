/**************************************************************************

Author		: Dongming Sheng

Date		: 2016-08-20

File Name	: mainwindow.h

**************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "GameStarter.h"
using namespace std;

class QAction;
class QMenu;
class QGraphicsScene;
class QGraphicsView;
class QGraphicsPixmapItem;
class QGraphicsTextItem;
class QKeyEvent;
class QTimer;
class QPixmap;
class QLabel;

class GameStarter;

class SnakeAI;

#define Player1AIName TwoSnakeAI
#define Player2AIName OADSnakeAI

class MainWindow : public QMainWindow{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

signals:
	void turnUpSignal();
	void turnDownSignal();
	void turnLeftSignal();
	void turnRightSignal();
	void statusChangedSignal(int);

private slots:
	void startSinglePlayerSlot();
	void startSingleAISlot();
	void startPlayerVSAISlot();
	void startTwoPlayersSlot();
	void startAIVSAISlot();

	void gamePause();
	void gameContinue();

	void updateStatusBarSlot(int);
	void screenShotSlot();
	void hideHelpMenuSlot();
	void speedUpSlot();
	void slowDownSlot();
	void aboutSlot();
	void exitSlot();

	void timeoutSlot();


protected:
	void keyPressEvent(QKeyEvent * event);

private:
	enum GameStatus { Ready, GameOver, Playing, Pause , SavingData};
	char *gameStatusDescription[5] = {
		"ready","game over", "playing", 
		"pause", "saving pics..." 
	};

	enum MoveDirection { horizontal, vertical };//for drawing graphs only
	enum Constants { 
		picSize = 80, 
		windowWidth = 1800, windowHeight = 1200,
		singleInterval = 300, doubleInterval = 500,
		singleFastestSpeed = 25, doubleFastestSpeed = 100,
		slowestSpeed = 600,
		startX = 200 , startY = 0
	};
	
	/*enum Directions {
	Left, Up, Right, Down,
	top_left, bottom_left,
	top_right, bottom_right
	};*/
	char *directionsInString[8] = { 
		"left", "up", "right", "down",
		"top_left",  "bottom_left",
		"top_right", "bottom_right",
	};	
	/*enum GameModes {
		SinglePlayer, SingleAI, Player_vs_AI, AI_vs_AI, TwoPlayers
	};*/
	char *gameModeInString[5] = {
		"SinglePlayer", "SingleAI",
		"Player_vs_AI", "AI_vs_AI", "TwoPlayers"
	};


	int directions[GameStarter::MAXSNAKE];	//change when pressing keys/making decisions
	int gameStatus;		
	bool ignoreKeyPress;					//AI-only modes
	bool shouldRecord;						//record the game
	GameStarter *game;						//game starter:generate map, control snakes

	QTimer *timer;
	int timeInteval;						//inteval of the timer

	QGraphicsPixmapItem *food;				//control the position of the food
	QGraphicsPixmapItem *crash;
	QGraphicsPixmapItem *images[20 * 20];
	QWidget *homeScreen;
	QGraphicsPixmapItem *homeScreenIcon;

	list< pair< QGraphicsPixmapItem *, int > > *snakeBody[2];

	QGraphicsTextItem *roundText;
	QGraphicsTextItem *authorText;
	QPixmap gameRecordings[300];

	SnakeAI* computerI;
	//TwoSnakeAI* playerI;
	SnakeAI* playerII;
	SnakeAI* playerI;

	//MainWindow components:
	QAction *startSinglePlayerAction;
	QAction *startSingleAIAction;
	QAction *startPlayerVSAIAction;
	QAction *startTwoPlayersAction;
	QAction *startAIVSAIAction;
	QAction *exitAction;
	QAction *speedUpAction;
	QAction *slowDownAction;
	QAction *screenShotAction;
	QAction *aboutAction;
	QAction *aboutQtAction;

	QMenu *gameMenu;
	QMenu *toolsMenu;
	QMenu *helpMenu;

	QLabel *statusLabel;

	QGraphicsScene *scene;
	QGraphicsView  *view;

	void startNewGame(int);
	void changeStatus(int);		//change game status

	void createActions();
	void createMenus();
	void createStatusBar();
	void createHomeScreen();
	void updateSpeedTip(bool = false);
	void createMap();
	void deleteMap();
	void recordGame(bool = false);

	void setFood();
	void setRoundTip();
	void printSnake(int, bool = true);

	void cutTail(int);
	void moveHead(int, bool = true);
	int footChangeGuide(int, int) const;
	int neckChangeGuide(int, int) const;
	bool shouldFootChangeShape(int) const;
	bool shouldNeckChangeShape(int, int) const;

	void printCrash(int);	//print crash image in the head position
};

#endif // MAINWINDOW_H

