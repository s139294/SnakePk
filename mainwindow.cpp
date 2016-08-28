/**************************************************************************

Author		: Dongming Sheng

Date		: 2016-08-26

File Name	: mainwindow.cpp

**************************************************************************/

#include<QtGui>
#include<qmainwindow.h>
#include<qmenubar.h>
#include<qmessagebox.h>
#include<qaction.h>
#include<qpixmap.h>
#include<qstring.h>
#include<qpalette.h>
#include<QKeyEvent>
#include<qtimer.h>
#include<qbytearray.h>
#include<qpushbutton.h>
#include<qboxlayout.h>
#include<qapplication.h>
#include<qdesktopwidget.h>
#include<qfiledialog.h>
#include<qlabel.h>
#include<qstatusbar.h>
#include<qdir.h>
#include<qgraphicsproxywidget.h>
#include<qfile.h>
#include<qgraphicsscene.h>
#include<qgraphicsview.h>
#include<qgraphicsitem.h>
#include<assert.h>

#include "mainwindow.h"
#include "AIs\SingleAI.h"
#include "AIs\TwoSnakeAI.h"
#include"AIs\OADSnakeAI.h"

MainWindow::MainWindow() {
	//initiate parameters:
	ignoreKeyPress = false;
	shouldRecord = false;
	game = nullptr;

	connect(this, SIGNAL(statusChangedSignal(int)),
		this, SLOT(hideHelpMenuSlot()));
	
	snakeBody[0] = snakeBody[1] = nullptr;
	computerI = nullptr;
	playerI = nullptr;
	playerII = nullptr;
	crash = nullptr;
	roundText = nullptr;

	//generate main window:
	resize(windowWidth, windowHeight);
	setMinimumSize(windowWidth, windowHeight);
	setMaximumSize(windowWidth, windowHeight);
	setWindowIcon(QIcon(":/mainWindow/images/icon.png"));
	setWindowTitle("SnakePK 1.0");
	setWindowFlags(windowFlags()&~Qt::WindowContextHelpButtonHint);
	setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
	createActions();
	createMenus();
	createStatusBar();
	changeStatus(Ready);

	scene = new QGraphicsScene(this);	
	//scene->setPalette(palette);
	view = new QGraphicsView(scene, this);

	//background
	QPixmap pixmap(":/mainWindow/images/bg.png");
	QPalette palette;
	palette.setBrush(view->backgroundRole(), QBrush(pixmap));
	view->setPalette(palette);

	setCentralWidget(view);//default:in the center
						   /*scene->setSceneRect(0, 0, 1500, 1000);
						   scene->setSceneRect(0, 0
						   , static_cast<QWidget *>(scene->parent())->size().width()
						   , static_cast<QWidget *>(scene->parent())->size().height());*/
	view->resize(this->size());
	view->show();
	//view->setFocus();
	
	createHomeScreen();

	//create timer
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timeoutSlot()));
}

MainWindow::~MainWindow() {
	if (game != nullptr) {
		delete game;
		game = nullptr;
	}
	if (computerI != nullptr) {
		delete computerI;
		computerI = nullptr;
	}
	if (playerI != nullptr) {
		delete playerI;
		playerI = nullptr;
	}
	if (playerII != nullptr) {
		delete playerII;
		playerII = nullptr;
	}
	if (crash != nullptr) {
		delete crash;
		crash = nullptr;
	}
	if (roundText != nullptr) {
		delete roundText;
		roundText = nullptr;
	}
	
}

void MainWindow::keyPressEvent(QKeyEvent * k) {
	if (ignoreKeyPress) 
		return;

	if (gameStatus != Playing) 
		return;

	bool canGoBackward = game->getSnakeNumber() == 1 
		&& game->getSnakeLength(0) == 1;

	if (k->key() == Qt::Key_D || k->key() == Qt::Key_Apostrophe) {
		int snakeIndex = 0;
		if (k->key() == Qt::Key_Apostrophe)//"/'"
			snakeIndex = 1;
		if (canGoBackward || directions[snakeIndex] != GameStarter::Left) {
			directions[snakeIndex] = GameStarter::Right;
			emit turnRightSignal();
		}
	}
	else if (k->key() == Qt::Key_W || k->key() == Qt::Key_P) {
		int snakeIndex = 0;
		if (k->key() == Qt::Key_P)
			snakeIndex = 1;
		if (canGoBackward || directions[snakeIndex] != GameStarter::Down) {
			directions[snakeIndex] = GameStarter::Up;
			emit turnUpSignal();
		}
	}
	else if (k->key() == Qt::Key_A || k->key() == Qt::Key_L) {
		int snakeIndex = 0;
		if (k->key() == Qt::Key_L)
			snakeIndex = 1;
		if (canGoBackward || directions[snakeIndex] != GameStarter::Right) {
			directions[snakeIndex] = GameStarter::Left;
			emit turnLeftSignal();
		}
	}
	else if (k->key() == Qt::Key_S || k->key() == Qt::Key_Semicolon) {
		int snakeIndex = 0;
		if (k->key() == Qt::Key_Semicolon)
			snakeIndex = 1;
		if (canGoBackward || directions[snakeIndex] != GameStarter::Up) {
			directions[snakeIndex] = GameStarter::Down;
			emit turnDownSignal();
		}
	}
}

void MainWindow::changeStatus(int newStatus) {
	gameStatus = newStatus;
	emit statusChangedSignal(newStatus);
}

void MainWindow::gamePause() {
	if (gameStatus == Playing)
		changeStatus(Pause);	
}

void MainWindow::gameContinue() {
	if (gameStatus == Pause) 
		changeStatus(Playing);
}

void MainWindow::startNewGame(int gameMode){
	if (!homeScreen->isHidden()) {
		homeScreen->hide();
		delete homeScreenIcon;
		delete authorText;
	}

	if (game != nullptr) {									//a game has created
		if (gameStatus == Pause || gameStatus == Playing) { //the game is not over
			gamePause();

			QString hint = "restart a new "
				+ QString(gameModeInString[gameMode]) + " game?";
			if (QMessageBox::question(this, tr("restart"), tr(hint.toLatin1()), 
				QMessageBox::Yes | QMessageBox::No)	== QMessageBox::No) {
				gameContinue();
				return;
			}
		}
		timer->stop();
		deleteMap();
		delete game;
		changeStatus(GameOver);
	}

	game = new GameStarter(gameMode);

	if (gameMode == GameStarter::SinglePlayer 
#ifndef TwoPlayersNotAvailable
		|| gameMode == GameStarter::TwoPlayers
#endif
		|| gameMode == GameStarter::Player_vs_AI) 
		ignoreKeyPress = false;
	else {
		ignoreKeyPress = true;
	}

	if (gameMode == GameStarter::SingleAI)
		computerI = new SingleAI(game, GameStarter::Player1);
	else if (gameMode == GameStarter::Player_vs_AI 
		|| gameMode == GameStarter::AI_vs_AI)
		playerII = new Player2AIName(game, GameStarter::Player2);
	if(gameMode == GameStarter::AI_vs_AI)
		playerI = new Player1AIName(game, GameStarter::Player1);

	createMap();

	setRoundTip();
	int snakeNumber = game->getSnakeNumber();
	if (snakeNumber == 1) {
		food = scene->addPixmap(QPixmap(":/game/images/apple.png"));
		setFood();

		timeInteval = singleInterval;
		timer->setInterval(timeInteval);
		updateSpeedTip();
	}
	else{
		timeInteval = doubleInterval;
		timer->setInterval(timeInteval);
		updateSpeedTip();
	}

	if (crash != nullptr) {
		delete crash;
		crash = nullptr;
	}

	for (int i = 0; i < snakeNumber; ++i) {
		snakeBody[i] = new list<pair<QGraphicsPixmapItem *, int> >;
		printSnake(i, false);
	}

	if (snakeNumber != 1)
		shouldRecord = true;
	else
		shouldRecord = false;

	changeStatus(Playing);
	timer->start();
}

void MainWindow::startSinglePlayerSlot() {
	startNewGame(GameStarter::SinglePlayer);
}

void MainWindow::startSingleAISlot() {
	startNewGame(GameStarter::SingleAI);
}

void MainWindow::startPlayerVSAISlot() {
	startNewGame(GameStarter::Player_vs_AI);
}

void MainWindow::startTwoPlayersSlot(){
	startNewGame(GameStarter::TwoPlayers);
}

void MainWindow::startAIVSAISlot(){
	startNewGame(GameStarter::AI_vs_AI);
}

void MainWindow::screenShotSlot(){
	gamePause();
	QPixmap pixmap = QPixmap::grabWidget(view);
	QString format = "png";
	QString initialPath = QDir::currentPath() + tr("/untitled.") + format;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
		initialPath,tr("%1 Files (*.%2);;All Files (*)").arg(format.toUpper())
		.arg(format));
	if (!fileName.isEmpty())
		pixmap.save(fileName, format.toLatin1());
	gameContinue();
}

void MainWindow::hideHelpMenuSlot() {
	if (gameStatus == Pause || gameStatus == Playing) {
		aboutAction->setEnabled(false);
		aboutQtAction->setEnabled(false);
	}
	else if (gameStatus == GameOver) {
		aboutAction->setEnabled(true);
		aboutQtAction->setEnabled(true);
	}
}

void MainWindow::speedUpSlot() {
	timeInteval -= 50;
	int limits = game->getSnakeNumber() == 1 
		? singleFastestSpeed : doubleFastestSpeed;
	if (timeInteval <= limits) {
		timeInteval = limits;
		updateSpeedTip(true);
	}
	else
		updateSpeedTip();
	timer->setInterval(timeInteval);
}

void MainWindow::slowDownSlot() {
	timeInteval += 100;
	if (timeInteval >= slowestSpeed) {
		timeInteval = slowestSpeed;
		updateSpeedTip(true);
	}
	else
		updateSpeedTip();
	timer->setInterval(timeInteval);
}

void MainWindow::aboutSlot() {
	QMessageBox::about(this, tr("About SnakePK"),
		tr("<h2>SnakePK 1.0</h2>"
			"<p>Copyright &copy; 2016 <b>Hans Sheng</b>"
			"<p>SnakePK is a small application that allows "
			"you to play 5 different kinds of snake modes."
			"<p>Use WASD to control Player1"
			"<p>Use PL;' to control Player2"
		));
}

void MainWindow::exitSlot() {
	if (gameStatus != GameOver)
		gamePause();

	if (QMessageBox::question(this, tr("exit"),
		tr("quit this game?"), QMessageBox::Yes |
		QMessageBox::No) == QMessageBox::Yes) {
		delete game;
		game = nullptr;
		if (computerI != nullptr) {
			delete computerI;
			computerI = nullptr;
		}
		exit(0);
	}
	else {
		if (gameStatus == Pause)
			gameContinue();
	}
}

void MainWindow::createActions() {
	startSinglePlayerAction = new QAction(tr("Single Player"), this);
	startSinglePlayerAction->setIcon(
		QIcon(":/mainWindow/images/single_player.png"));
	startSinglePlayerAction->setShortcut(tr("Ctrl+1"));
	startSinglePlayerAction->setStatusTip(tr("Start a single player game"));
	connect(startSinglePlayerAction, SIGNAL(triggered()),
		this, SLOT(startSinglePlayerSlot()));

	startSingleAIAction = new QAction(tr("Single AI"), this);
	startSingleAIAction->setIcon(
		QIcon(":/mainWindow/images/single_AI.png"));
	startSingleAIAction->setShortcut(tr("Ctrl+2"));
	startSingleAIAction->setStatusTip(tr("Start a single computer game"));
	connect(startSingleAIAction, SIGNAL(triggered()),
		this, SLOT(startSingleAISlot()));

	startPlayerVSAIAction = new QAction(tr("Player vs AI"), this);
	startPlayerVSAIAction->setIcon(
		QIcon(":/mainWindow/images/player_AI.png"));
	startPlayerVSAIAction->setShortcut(tr("Ctrl+3"));
	startPlayerVSAIAction->setStatusTip(tr("Start a player vs computer game"));
	connect(startPlayerVSAIAction, SIGNAL(triggered()),
		this, SLOT(startPlayerVSAISlot()));

	startTwoPlayersAction = new QAction(tr("Two Players"), this);
	startTwoPlayersAction->setIcon(
		QIcon(":/mainWindow/images/two_players.png"));
	startTwoPlayersAction->setShortcut(tr("Ctrl+4"));
	startTwoPlayersAction->setStatusTip(tr("Start a player vs player game"));
	connect(startTwoPlayersAction, SIGNAL(triggered()),
		this, SLOT(startTwoPlayersSlot()));

	startAIVSAIAction = new QAction(tr("AI vs AI"), this);
	startAIVSAIAction->setIcon(
		QIcon(":/mainWindow/images/two_AIs.png"));
	startAIVSAIAction->setShortcut(tr("Ctrl+5"));
	startAIVSAIAction->setStatusTip(tr("Start a computer vs computer game"));
	connect(startAIVSAIAction, SIGNAL(triggered()),
		this, SLOT(startAIVSAISlot()));

	exitAction = new QAction(tr("E&xit"), this);
	exitAction->setShortcut(tr("Ctrl+Q"));
	exitAction->setIcon(QIcon(":/mainWindow/images/exit.png"));
	exitAction->setStatusTip(tr("Exit SnakePk application"));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(exitSlot()));

	QString speedUpTip = "Snake will move faster(current interval: "
		+ QString::number(singleInterval) + ")";
	speedUpAction = new QAction(tr("Speed &Up"), this);
	speedUpAction->setIcon(QIcon(":/mainWindow/images/speed_up.png"));
	speedUpAction->setShortcut(tr("Ctrl+="));
	speedUpAction->setStatusTip(tr(speedUpTip.toLatin1()));
	connect(speedUpAction, SIGNAL(triggered()),
		this, SLOT(speedUpSlot()));

	QString slowDownTip = "Snake will move slower(current interval: "
		+ QString::number(singleInterval) + ")";
	slowDownAction = new QAction(tr("Slow &Down"), this);
	slowDownAction->setShortcut(tr("Ctrl+-"));
	slowDownAction->setIcon(QIcon(":/mainWindow/images/slow_down.png"));
	slowDownAction->setStatusTip(tr(slowDownTip.toLatin1()));
	connect(slowDownAction, SIGNAL(triggered()),
		this, SLOT(slowDownSlot()));

	screenShotAction = new QAction(tr("&Screenshot"), this);
	screenShotAction->setShortcut(tr("Ctrl+S"));
	screenShotAction->setIcon(
		QIcon(":/mainWindow/images/screenshot.png"));
	screenShotAction->setStatusTip(tr("Record the current screen"));
	connect(screenShotAction, SIGNAL(triggered()), 
		this, SLOT(screenShotSlot()));

	aboutAction = new QAction(tr("&About"), this);
	aboutAction->setShortcut(tr("Ctrl+/"));
	aboutAction->setIcon(QIcon(":/mainWindow/images/pk.png"));
	aboutAction->setStatusTip(tr("Infomation of this game"));
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutSlot()));

	aboutQtAction = new QAction(tr("About &Qt"), this);
	aboutQtAction->setIcon(QIcon(":/mainWindow/images/qt.png"));
	aboutQtAction->setStatusTip(tr("Infomation of the UI of this game"));
	connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus() {
	gameMenu = menuBar()->addMenu(tr("&Game"));
	gameMenu->addAction(startSinglePlayerAction);
	gameMenu->addAction(startSingleAIAction);
	gameMenu->addAction(startPlayerVSAIAction);

#ifndef TwoPlayersNotAvailable
	gameMenu->addAction(startTwoPlayersAction);
#endif // !TwoPlayersNotAvailable

	gameMenu->addAction(startAIVSAIAction);
	gameMenu->addSeparator();
	gameMenu->addAction(exitAction);

	toolsMenu = menuBar()->addMenu(tr("&Tools"));
	toolsMenu->addAction(speedUpAction);
	toolsMenu->addAction(slowDownAction);
	toolsMenu->addSeparator();
#ifndef NoCamera
	toolsMenu->addAction(screenShotAction);
#endif // !NoCamera

	menuBar()->addSeparator();
	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAction);
	helpMenu->addAction(aboutQtAction);

	connect(gameMenu, SIGNAL(aboutToShow()),
		this, SLOT(gamePause()));
	connect(toolsMenu, SIGNAL(aboutToShow()),
		this, SLOT(gamePause()));
	connect(helpMenu, SIGNAL(aboutToShow()),
		this, SLOT(gamePause()));

	connect(gameMenu, SIGNAL(aboutToHide()),
		this, SLOT(gameContinue()));
	connect(toolsMenu, SIGNAL(aboutToHide()),
		this, SLOT(gameContinue()));
	connect(helpMenu, SIGNAL(aboutToHide()),
		this, SLOT(gameContinue()));

}

void MainWindow::createStatusBar() {
	statusLabel = new QLabel("     ready     ");
	statusLabel->setAlignment(Qt::AlignHCenter);
	statusLabel->setMinimumSize(statusLabel->sizeHint());
	statusBar()->addWidget(statusLabel);
	connect(this, SIGNAL(statusChangedSignal(int)), 
		this, SLOT(updateStatusBarSlot(int)));
}

void MainWindow::createHomeScreen() {
	homeScreen = new QWidget();

	QPushButton *singlePlayerButton = new QPushButton(
		"Single player", homeScreen);
	QPushButton *singleComputerButton = new QPushButton(
		"Single AI", homeScreen);
	QHBoxLayout *firstRowLayout = new QHBoxLayout();
	firstRowLayout->addStretch();
	firstRowLayout->addWidget(singlePlayerButton);
	firstRowLayout->addWidget(singleComputerButton);
	firstRowLayout->addStretch();

	QPushButton *PlayerVSAIButton = new QPushButton(
		"Player VS AI", homeScreen);
	QPushButton *TwoComputersButton = new QPushButton(
		"AI VS AI", homeScreen);
	QPushButton *TwoPlayersButton = new QPushButton(
		"Player VS Player", homeScreen);
	QHBoxLayout *secondRowLayout = new QHBoxLayout();
	secondRowLayout->addWidget(TwoPlayersButton);
	secondRowLayout->addWidget(PlayerVSAIButton);
	secondRowLayout->addWidget(TwoComputersButton);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->addLayout(firstRowLayout);
	mainLayout->addStretch();
	mainLayout->addLayout(secondRowLayout);

	homeScreen->setLayout(mainLayout);
	homeScreen->resize(windowWidth - 1000, windowHeight - 980);
	//transparent:
	homeScreen->setAttribute(Qt::WA_TranslucentBackground, true);

	QPixmap headPic(":/mainWindow/images/icon.png");
	homeScreenIcon = scene->addPixmap(headPic);
	homeScreenIcon->setPos(200, 250);

	//add QButton stylesheet
	QFile file(":/mainWindow/QPushButton.qss");
	file.open(QFile::ReadOnly);
	QString styleString = QLatin1String(file.readAll());

	homeScreen->setStyleSheet(styleString);
	homeScreen->setPalette(QPalette(Qt::white));
	file.close();

	homeScreen->setStyleSheet(QString::fromUtf8("border:1px solid red"));
	QGraphicsProxyWidget *item = scene->addWidget(homeScreen);
	item->setPos(0, 700);

	QFont font("Footlight MT Light", 15, 2);
	QString tip = "Author:  Hans Sheng from SCUT";
	authorText = scene->addText(tip, font);
	authorText->setPos(180, 950);

	connect(singlePlayerButton, SIGNAL(clicked()), 
		this, SLOT(startSinglePlayerSlot()));
	connect(singleComputerButton, SIGNAL(clicked()),
		this, SLOT(startSingleAISlot()));
	connect(PlayerVSAIButton, SIGNAL(clicked()),
		this, SLOT(startPlayerVSAISlot()));
	connect(TwoPlayersButton, SIGNAL(clicked()),
		this, SLOT(startTwoPlayersSlot()));
	connect(TwoComputersButton, SIGNAL(clicked()),
		this, SLOT(startAIVSAISlot()));
}

void MainWindow::updateSpeedTip(bool haveWarning){
	QString isOk = haveWarning ? "can't" : "will";
	QString slowDownTip = "Snake " + isOk + 
		" move slower(current interval: " + 
		QString::number(timeInteval) + ")";
	QString speedUpTip = "Snake " + isOk +
		" move faster(current interval: " +
		QString::number(timeInteval) + ")";
	speedUpAction->setStatusTip(tr(speedUpTip.toLatin1()));
	slowDownAction->setStatusTip(tr(slowDownTip.toLatin1()));
}

void MainWindow::updateStatusBarSlot(int status){
	QString tip;
	if (status != Playing)
		tip = QString(gameStatusDescription[status]);
	else {
		tip = "playing " + QString(gameModeInString[game->getGameMode()]);
		if (shouldRecord)
			tip = tip + "(recording...)";
	}
	statusLabel->setText(tr(tip.toLatin1()));
}

void MainWindow::setFood() {
	food->setPos(images[game->getFoodIndex()]->pos());
}

void MainWindow::setRoundTip(){
	if (roundText != nullptr) {
		delete roundText;
	}
	QFont font("Cambria", 20, 5);
	QString tip = "Round: " + QString::number(game->getCurrentRound());
	roundText = scene->addText(tip, font);
	roundText->setPos(-250, 550);
}

void MainWindow::timeoutSlot() {
	if (gameStatus == Pause) {
		return;
	}
	else if (gameStatus == GameOver) {
		timer->stop();
		return;
	}
	if(shouldRecord)
		recordGame();

	//get directions from AIs:
	if (game->getGameMode() == GameStarter::AI_vs_AI)
		directions[GameStarter::Player1] = playerI->decide();
	else if (game->getGameMode() == GameStarter::SingleAI)
		directions[GameStarter::Player1] = computerI->decide();
	if (game->getGameMode() == GameStarter::Player_vs_AI
		|| game->getGameMode() == GameStarter::AI_vs_AI)
		directions[GameStarter::Player2] = playerII->decide();
	
	int snakeNumber = game->getSnakeNumber();

	int respond = game->move(directions);
	for(int i = 0; i < snakeNumber; ++i)
		directions[i] = game->getDirection(i);	//actual moving direction
	
	setRoundTip();

	if (respond == GameStarter::Grow) {
		if (snakeNumber == 1)
			setFood();
		for (int i = 0; i < snakeNumber; ++i)
			printSnake(i, false);
	}

	else if (respond == GameStarter::KeepTheLen) {
		for(int i = 0; i < snakeNumber; ++i)
			printSnake(i);
	}
	else {
		changeStatus(SavingData);
		timer->stop();

		if (snakeNumber >= 2) {
			//painting order
			if (respond == GameStarter::Player1Lose) {
				printSnake(GameStarter::Player2, 
					!game->isRoundGrow(game->getCurrentRound()));
				printSnake(GameStarter::Player1, 
					!game->isRoundGrow(game->getCurrentRound()));
			}
			else {
				printSnake(GameStarter::Player1, 
					!game->isRoundGrow(game->getCurrentRound()));
				printSnake(GameStarter::Player2, 
					!game->isRoundGrow(game->getCurrentRound()));
			}

			if (respond == GameStarter::Draw)
				for (int i = 0; i < snakeNumber; ++i)
					printCrash(i);
			else if (respond == GameStarter::Player2Lose)
				printCrash(GameStarter::Player2);
			else
				printCrash(GameStarter::Player1);

			if (respond == GameStarter::Draw) 
				QMessageBox::information(this, tr("oh"),
					tr("It's a draw!"));
			else if (respond == GameStarter::Player1Lose) 
				QMessageBox::information(this, tr("game over"),
					tr("Player 2 wins!"));
			else
				QMessageBox::information(this, tr("game over"),
					tr("Player 1 wins!"));
		}

		else {//singleMode
			if (respond == GameStarter::IllegalPosition) {
				printSnake(GameStarter::Player1);
				printCrash(GameStarter::Player1);
				QMessageBox::information(this, tr("oops"),
					tr("Game over!"));
			}

			else if (respond == GameStarter::SingleModeWon) {
				printSnake(GameStarter::Player1, false);
				QMessageBox::information(this, tr("great"),
					tr("You win!"));
			}
		}
		if (shouldRecord)
			recordGame(true);

		changeStatus(Ready);
	}
}

void MainWindow::createMap(){
	const int *map = game->getMap();
	directions[0] = GameStarter::Down;
	directions[1] = GameStarter::Up;

	for (int i = 0; i < game->getSize(); ++i) {
		QString path(":/game/images/");
		int index;
		if (map[i] < 2)
			index = map[i];
		else
			index = 2;
		path = path + QString::number(index) + QString(".png");
		QPixmap pixmap;
		if (!pixmap.load(path)) {
			QMessageBox::information(this, tr("error"),
				tr("fail to load the pictures"));
		}
		images[i] = scene->addPixmap(pixmap);
		images[i]->setPos(startY + picSize * (i % game->getSizeY()),
			startX + picSize * (i / game->getSizeY()));
	}

}

void MainWindow::deleteMap() {
	for (int i = 0; i < game->getSize(); ++i) {
		//scene->removeItem(images[i]);
		delete images[i];
		images[i] = nullptr;
	}

	for (int i = 0; i < game->getSnakeNumber(); ++i) {
		while (!snakeBody[i]->empty()) {
			QGraphicsPixmapItem* item;
			item = snakeBody[i]->back().first;
			delete item;
			snakeBody[i]->pop_back();
		}
		delete snakeBody[i];
		snakeBody[i] = nullptr;
	}
	
	if (game->getSnakeNumber() == 1) {
		delete food;
		food = nullptr;
	}

	if (roundText != nullptr) {
		delete roundText;
		roundText = nullptr;
	}

	if (computerI != nullptr) {
		delete computerI;
		computerI = nullptr;
	}
	if (playerI != nullptr) {
		delete playerI;
		playerI = nullptr;
	}
	if (playerII != nullptr) {
		delete playerII;
		playerII = nullptr;
	}
}

void MainWindow::recordGame(bool end){
	gameRecordings[game->getCurrentRound()] = QPixmap::grabWidget(view);
	if (!end) 
		return;
	if (game->getCurrentRound() > 10) {
		if (QMessageBox::question(this, tr("record"),
			tr("Do you want to record this game?"),
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

			QString path = QFileDialog::getExistingDirectory(
				this, tr("Open Directory"), QString(),
				QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
			);
			path = path + "/";
			if (QDir(path).exists()) {
				QString format = "png";
				QString fileName;
				int totRound = game->getCurrentRound();
				for (int i = 0; i <= totRound; ++i) {
					fileName = path + QString::number(i) + "." + format;
					gameRecordings[i].save(fileName, format.toLatin1(), -1);
				}
			}
		}
	}
}

void MainWindow::printSnake(int snakeIndex, bool shouldCutTail) {
	//drawing pictures:cut tail->add head
	if (shouldCutTail) {
		cutTail(snakeIndex);
	}
	moveHead(snakeIndex, shouldCutTail);
}

void MainWindow::cutTail(int snakeIndex) {
	pair<QGraphicsPixmapItem *, int> tail = snakeBody[snakeIndex]->back();
	QGraphicsPixmapItem *tail_item = tail.first;
	int tail_dir = tail.second;

	//delete tail:
	snakeBody[snakeIndex]->pop_back();
	delete tail_item;

	int snakeLen = snakeBody[snakeIndex]->size();//after cutting tail
	assert(snakeLen > 0);//snakeLen == 1: foot - tail
						 //snakeLen >= 2: neck(moveHead) - foot - tail

	pair<QGraphicsPixmapItem *, int> foot = snakeBody[snakeIndex]->back();
	QGraphicsPixmapItem *foot_item = foot.first;
	int foot_dir = foot.second;
	int footIndex = game->getTailIndex(snakeIndex);

	snakeBody[snakeIndex]->pop_back();
	delete foot_item;

	QGraphicsPixmapItem *additem;	//the handle for replacement
	QString path;					//path of pixmap
	int md;							//moving direction

	if (snakeLen == 1)
		md = game->getDirection(snakeIndex);
	else if (shouldFootChangeShape(foot_dir))
		md = footChangeGuide(tail_dir, foot_dir);
	else
		md = foot_dir;

	path = ":/game/images/tail_" + QString(directionsInString[md])
		+ "_" + QString::number(snakeIndex) + ".png";
	additem = scene->addPixmap(QPixmap(path));
	additem->setPos(images[footIndex]->pos());

	snakeBody[snakeIndex]->push_back(
		pair<QGraphicsPixmapItem *, int>(additem, md));
}

void MainWindow::moveHead(int snakeIndex, bool shouldCutTail){
	QGraphicsPixmapItem *additem;
	QString path;
	int md;
	int headIndex = game->getHeadIndex(snakeIndex);
	int head_dir = game->getDirection(snakeIndex);
	//after cutting tail, before moving head
	int snakeLen = snakeBody[snakeIndex]->size();

	if (snakeLen == 1 && !shouldCutTail || snakeLen >= 2) {
		pair<QGraphicsPixmapItem *, int> neck = snakeBody[snakeIndex]->front();
		QGraphicsPixmapItem *neck_item = neck.first;
		int neck_dir = neck.second;
		int neckIndex = game->getNeckIndex(snakeIndex);

		snakeBody[snakeIndex]->pop_front();
		delete neck_item;

		if (shouldNeckChangeShape(neck_dir, head_dir))
			md = neckChangeGuide(neck_dir, head_dir);
		else
			md = neck_dir;

		path = ":/game/images/" + QString(directionsInString[md])
			+ "_" + QString::number(snakeIndex) + ".png";
		additem = scene->addPixmap(QPixmap(path));
		additem->setPos(images[neckIndex]->pos());

		snakeBody[snakeIndex]->push_front(
			pair<QGraphicsPixmapItem *, int>(additem, md));
	}

	QGraphicsPixmapItem *head;
	md = directions[snakeIndex];
	path = ":/game/images/head_" + QString(directionsInString[md]) 
		+ "_" + QString::number(snakeIndex) + ".png";
	head = scene->addPixmap(QPixmap(path));
	head->setPos(images[headIndex]->pos());

	snakeBody[snakeIndex]->push_front(
		pair<QGraphicsPixmapItem *, int>(head,head_dir));
}

int MainWindow::footChangeGuide(
	int tail_dir, int foot_dir) const{
	if (foot_dir == GameStarter::top_left)
		if (tail_dir == GameStarter::Up)
			return GameStarter::Right;
		else
			return GameStarter::Down;

	if (foot_dir == GameStarter::top_right)
		if (tail_dir == GameStarter::Up)
			return GameStarter::Left;
		else
			return GameStarter::Down;

	if (foot_dir == GameStarter::bottom_left)
		if (tail_dir == GameStarter::Down)
			return GameStarter::Right;
		else
			return GameStarter::Up;

	else
		if (tail_dir == GameStarter::Down)
			return GameStarter::Left;
		else
			return GameStarter::Up;
}

int MainWindow::neckChangeGuide(
	int neck_dir, int head_dir) const{
	if (neck_dir == GameStarter::Up && head_dir == GameStarter::Right
		|| neck_dir == GameStarter::Left && head_dir == GameStarter::Down)
		return GameStarter::top_left;

	else if (neck_dir == GameStarter::Up && head_dir == GameStarter::Left
		|| neck_dir == GameStarter::Right && head_dir == GameStarter::Down)
		return GameStarter::top_right;

	else if (neck_dir == GameStarter::Down && head_dir == GameStarter::Right
		|| neck_dir == GameStarter::Left && head_dir == GameStarter::Up)
		return GameStarter::bottom_left;

	else
		return GameStarter::bottom_right;
}

bool MainWindow::shouldFootChangeShape(int foot_dir) const{

	if (foot_dir == GameStarter::Left
		|| foot_dir == GameStarter::Down
		|| foot_dir == GameStarter::Right
		|| foot_dir == GameStarter::Up)
		return false;

	return true;
}

bool MainWindow::shouldNeckChangeShape(
	int neck_dir, int head_dir) const{

	if (neck_dir == GameStarter::Up
		|| neck_dir == GameStarter::Down)
		if (head_dir == GameStarter::Right
			|| head_dir == GameStarter::Left)
			return true;

	if (neck_dir == GameStarter::Right
		|| neck_dir == GameStarter::Left)
		if (head_dir == GameStarter::Down
			|| head_dir == GameStarter::Up)
			return true;	

	return false;
}

void MainWindow::printCrash(int snakeIndex){
	QPixmap pixmap(":/game/images/crash.png");
	crash = scene->addPixmap(pixmap);
	int headIndex = game->getHeadIndex(snakeIndex);
	crash->setPos(images[headIndex]->pos());
}
