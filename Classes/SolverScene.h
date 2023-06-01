#ifndef __SOLVER_SCENE_H__
#define __SOLVER_SCENE_H__

#include "cocos2d.h"
#include "Preprocessing.h"
#include "ui/CocosGUI.h"
#include "cocostudio\CocoStudio.h"
#include "AudioEngine.h"
#include "SlideBlock.h"
#include "Clock.h"
#include "Menu.h"
#include "AdmobHelper.h"
#include "GameHelper.h"
#include <stack>
#include <set>
#include <iostream>
#include <thread>


const Coor DESTINATION_COOR = Coor(5, 3);

using std::string;

struct Move
{
    Move()
    {

    }
    Move(int id, int distance)
    {
        _id = id;
        _distance = distance;
    }
    Move &operator=(const Move &other)
    {
        _id = other._id;
        _distance = other._distance;
        return *this;
    }
    bool operator==(const Move &other)
    {
        return (_id == other._id) && (_distance == other._distance);
    }

    int _id, _distance;
};

enum class STATUS
{
    READY, RUNNING, PAUSED, FINISH
};

class SolverScene : public cocos2d::Layer
{
private:
    std::vector<Coor> coor;

    std::vector<SlideBlock *> slideblocks;

    SlideBlock *tempBlock;

    std::vector<std::vector<Coor>> originSlideBlocks;

    std::vector<Move> paths;

    std::set<size_t> stateMatrixString;

    std::vector<size_t> hashMoveString;

    bool found;

    int index;
    int numSolved;
    STATUS status;

    cocos2d::Layer *controlLayer;

    cocos2d::ui::Button *newButton, *startButton, *backButton, *nextButton, *pauseButton;

    Clock clock;

    cocos2d::ui::Text *clockLabel, *statusLabel, *moveLabel;
    string _pathFile;


public:
    static cocos2d::Scene *createScene();
    void initSolver();
    void parseFromFile();
    void setPathFile(std::string path);
    void startSolver();
    void resumeSolver();
    void pauseSolver();
    void notify(std::string message);
    virtual bool init();

    void checkSlideBlockValid();
    bool isRedBlockValid();
    bool isSlideBlockValid(std::vector<Coor> &slideBlock);
    void resetOriginalMatrix();
    bool checkPathValid();
    void getAllMove(std::vector<Move> &moves);
    void sortBodys(std::vector<Coor> &coors);
    void sortMove(std::vector<Move> &moves, std::vector<Move> &his);
    bool findMove(Move &move);

    //void takeAMove();
    //void takeAMove2();
    void solvePuzzle();
    void finalOptimizePath();
    bool optimizePath();

    void combineTwoPath();
    void combineTwoPathAdvance(int i, int j, int c, bool &combined, std::vector<int> &mark);

    bool combinationPath();
    bool combinationPathAdvance();
    bool minimizePath();
    bool truncatePath();
    void reducePathLenght();

    void next();
    void back();
    void showResult();
    void updateClock(float dt);
    void showAds(float dt);
    CREATE_FUNC(SolverScene);
    bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event);
    void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event);
    void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event *event);
};

#endif // __SOLVER_SCENE_H__
