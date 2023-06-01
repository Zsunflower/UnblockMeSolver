#include "SolverScene.h"

USING_NS_CC;
using experimental::AudioEngine;

int SlideBlock::_matrix[6][6] = {0};

SolverScene *GameHelper::solver = nullptr;

Scene *SolverScene::createScene()
{
    auto scene = Scene::create();

    auto layer = SolverScene::create();
    scene->addChild(layer);
    return scene;
}

bool SolverScene::init()
{
    if(!Layer::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    std::srand(time(NULL));
    clock.init();
    auto uilayer = CSLoader::createNode("ControlLayer.csb");
    uilayer->setPositionY(0.5f * (visibleSize.height - 960.0f));
    SlideBlock::ORIGIN = uilayer->getPosition() + Vec2(20.0f, 262.5f);
    this->addChild(uilayer);
    numSolved = 0;
    //DrawNode *rec = DrawNode::create();
    //rec->drawRect(SlideBlock::ORIGIN, SlideBlock::ORIGIN + Vec2(500, 500), Color4F::RED);
    //this->addChild(rec, 6);
    newButton = static_cast<ui::Button *>(uilayer->getChildByName("Button_New"));
    newButton->addTouchEventListener([&](Ref *ref, ui::Widget::TouchEventType type)
                                     {
                                         if(type == ui::Widget::TouchEventType::ENDED)
                                         {
                                             initSolver();
                                             GameHelper::solver = this;
                                             GameHelper::openChooseFileDialog();
                                         }
                                     });

    startButton = static_cast<ui::Button *>(uilayer->getChildByName("Button_Start"));
    startButton->addTouchEventListener([&](Ref *ref, ui::Widget::TouchEventType type)
                                       {
                                           if(type == ui::Widget::TouchEventType::ENDED)
                                               startSolver();
                                       });

    pauseButton = static_cast<ui::Button *>(uilayer->getChildByName("Button_Pause"));
    pauseButton->addTouchEventListener([&](Ref *ref, ui::Widget::TouchEventType type)
                                       {
                                           if(type == ui::Widget::TouchEventType::ENDED)
                                               pauseSolver();
                                       });

    nextButton = static_cast<ui::Button *>(uilayer->getChildByName("Button_Next"));
    nextButton->addTouchEventListener([&](Ref *ref, ui::Widget::TouchEventType type)
                                      {
                                          if(type == ui::Widget::TouchEventType::ENDED)
                                              next();
                                      });

    backButton = static_cast<ui::Button *>(uilayer->getChildByName("Button_Back"));
    backButton->addTouchEventListener([&](Ref *ref, ui::Widget::TouchEventType type)
                                      {
                                          if(type == ui::Widget::TouchEventType::ENDED)
                                              back();
                                      });

    clockLabel = static_cast<ui::Text *>(uilayer->getChildByName("Text_Timer"));
    statusLabel = static_cast<ui::Text *>(uilayer->getChildByName("Text_Status"));
    moveLabel = static_cast<ui::Text *>(uilayer->getChildByName("Text_Move"));

    //    auto touchListener = EventListenerTouchOneByOne::create();
    //    touchListener->onTouchBegan = CC_CALLBACK_2(SolverScene::onTouchBegan, this);
    //    touchListener->onTouchMoved = CC_CALLBACK_2(SolverScene::onTouchMoved, this);
    //    touchListener->onTouchEnded = CC_CALLBACK_2(SolverScene::onTouchEnded, this);
    //    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

    auto keyListener = EventListenerKeyboard::create();
    keyListener->onKeyReleased = CC_CALLBACK_2(SolverScene::onKeyReleased, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyListener, this);
    initSolver();

#if(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    this->scheduleOnce(schedule_selector(SolverScene::showAds), 10.0f);
#endif

    return true;
}

void SolverScene::initSolver()
{
    for(auto slideblock : slideblocks)
    {
        if(slideblock != nullptr)
            slideblock->removeFromParentAndCleanup(true);
    }
    slideblocks.clear();
    originSlideBlocks.clear();
    paths.clear();
    stateMatrixString.clear();
    hashMoveString.clear();
    found = false;
    index = -1;
    slideblocks.push_back(nullptr);
    originSlideBlocks.push_back(std::vector<Coor>());
    SlideBlock::resetMatrix();
    clock.init();
    clockLabel->setString(clock.getFormatCurrentTime());
    statusLabel->setString("Pick a image");
    moveLabel->setString("-");
    DISABLE_BUTTON(startButton);
    DISABLE_BUTTON(backButton);
    DISABLE_BUTTON(nextButton);
    status = STATUS::READY;
}

void SolverScene::setPathFile(string path)
{
    _pathFile = path;
    auto delay = DelayTime::create(0.4f);
    this->runAction(Sequence::create(delay, CallFunc::create([&]()
                                                             {
                                                                 parseFromFile();
                                                             }), nullptr));
}

void SolverScene::parseFromFile()
{
    log("[parseFromFile] Screenshot path: %s", _pathFile.c_str());
    vector<vector<cv::Point>> block_position;
    //    parseScreenShot(_pathFile.c_str(), block_position);
    processScreenShot(_pathFile.c_str(), block_position);
    log("[parseFromFile] Parser get %d blocks", block_position.size());
    if(block_position.empty())
    {
        statusLabel->setString("Image invalid");
        return;
    }
    std::vector<Coor> tempBody;
    for(auto block : block_position)
    {
        tempBody.clear();
        for(auto pos : block)
        {
            Coor c = Coor(pos.x, 5 - pos.y);
            tempBody.push_back(c);
            log("(%d, %d)", c.x, c.y);
        }
        if(!isSlideBlockValid(tempBody))
        {
            log("SlideBlock is invalid, so skip this block");
            log("----------------------------------------");
            continue;
        }
        TYPE type;
        sortBodys(tempBody);
        if(tempBody[0].x == tempBody[1].x)
            type = TYPE::VERTICAL;
        else
            type = TYPE::HORIZONTAL;

        auto tempBlock = SlideBlock::create(tempBody, type);
        this->addChild(tempBlock, 1);
        tempBlock->ownBlockParse();
        slideblocks.push_back(tempBlock);
        originSlideBlocks.push_back(tempBody);
        log("-------- Add a block: %d -----------", tempBlock->getSlideBlockID());
    }
    if(isRedBlockValid())
    {
        slideblocks.back()->setFinalSlideBlock();
        statusLabel->setString("Let's start!");
    }
    else
    {
        statusLabel->setString("Image invalid");
    }
}

void SolverScene::startSolver()
{
    DISABLE_BUTTON(backButton);
    DISABLE_BUTTON(nextButton);
    DISABLE_BUTTON(newButton);
    //    slideblocks.back()->setFinalSlideBlock();
    startButton->setVisible(false);
    pauseButton->setVisible(true);
    statusLabel->setString("Solving puzzle...");
    this->schedule(schedule_selector(SolverScene::updateClock), 1.0f);
    status = STATUS::RUNNING;
    std::thread takeMove(&SolverScene::solvePuzzle, this);
    takeMove.detach();
}

void SolverScene::resumeSolver()
{
    pauseButton->setVisible(true);
    startButton->setVisible(false);
    statusLabel->setString("Solving puzzle...");
    this->schedule(schedule_selector(SolverScene::updateClock), 1.0f);
    status = STATUS::RUNNING;
    std::thread takeMove(&SolverScene::solvePuzzle, this);
    takeMove.detach();
}

void SolverScene::pauseSolver()
{
    status = STATUS::PAUSED;
    statusLabel->setString("Solver paused");
    this->unschedule(schedule_selector(SolverScene::updateClock));
    pauseButton->setVisible(false);
    startButton->setVisible(true);
    ENABLE_BUTTON(newButton);
}

void SolverScene::notify(std::string message)
{
    Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]()
                                                                          {
                                                                              statusLabel->setString(
                                                                                      message);
                                                                          });
}

bool SolverScene::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event)
{
    if(status != STATUS::READY)
        return false;
    tempBlock = nullptr;
    coor.clear();
    return true;
}

void SolverScene::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event)
{
    if(status != STATUS::READY)
        return;
    bool createNew = false;
    Rect r = Rect(SlideBlock::ORIGIN, Size(500, 500));
    Vec2 touchLocation = touch->getLocation();
    if(r.containsPoint(touchLocation))
    {
        int x, y;
        x = (touchLocation.x - SlideBlock::ORIGIN.x) / 84;
        y = (touchLocation.y - SlideBlock::ORIGIN.y) / 84;
        Coor c = Coor(x, y);
        if(!SlideBlock::isFreePosition(c))
            return;
        auto iter = std::find(coor.begin(), coor.end(), c);
        if(iter == coor.end())
        {
            if(coor.empty())
            {
                coor.push_back(c);
            }
            else
            {
                if(coor.front().x == c.x || coor.front().y == c.y)
                {
                    if(coor.back().x == c.x || coor.back().y == c.y)
                    {
                        if(ABS(coor.back().x - c.x) == 1 || ABS(coor.back().y - c.y) == 1)
                        {
                            coor.push_back(c);
                            createNew = true;
                        }
                        else
                        {
                            if(ABS(coor.front().x - c.x) == 1 || ABS(coor.front().y - c.y) == 1)
                            {
                                coor.erase(coor.begin() + 1, coor.end());
                                coor.push_back(c);
                                createNew = true;
                            }
                        }
                    }
                    else
                    {
                        if(ABS(coor.front().x - c.x) == 1 || ABS(coor.front().y - c.y) == 1)
                        {
                            coor.erase(coor.begin() + 1, coor.end());
                            coor.push_back(c);
                            createNew = true;
                        }
                    }
                }
            }
        }
        else
        {
            if(iter != std::prev(coor.end()))
            {
                coor.erase(iter + 1, coor.end());
                createNew = true;
            }
        }
        if(createNew && coor.size() > 1)
        {
            if(tempBlock)
            {
                tempBlock->removeFromParentAndCleanup(true);
                tempBlock = nullptr;
            }
            TYPE type;
            std::vector<Coor> tempBody;
            if(coor.size() > 3)
                coor.erase(coor.begin() + 3, coor.end());
            if(coor[0].x == coor[1].x)
                type = TYPE::VERTICAL;
            else
                type = TYPE::HORIZONTAL;
            tempBody = coor;
            sortBodys(tempBody);
            tempBlock = SlideBlock::create(tempBody, type);
            tempBlock->releaseBlock();
            this->addChild(tempBlock, 1);
        }
    }
    else
    {
        coor.clear();
        if(tempBlock)
        {
            tempBlock->removeFromParentAndCleanup(true);
            tempBlock = nullptr;
        }
    }
}

void SolverScene::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event)
{
    if(status != STATUS::READY)
        return;
    if(tempBlock)
    {
        sortBodys(coor);
        tempBlock->ownBlock();
        slideblocks.push_back(tempBlock);
        tempBlock = nullptr;
        originSlideBlocks.push_back(coor);
        checkSlideBlockValid();
    }
}

void SolverScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event *event)
{
    if(keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
    {
        if(status == STATUS::PAUSED || status == STATUS::READY || status == STATUS::FINISH)
        {
            Director::getInstance()->replaceScene(
                    TransitionFade::create(0.5f, MenuScene::createScene(), Color3B(10, 20, 30)));
        }
    }
}

void SolverScene::checkSlideBlockValid()
{
    if(slideblocks.back()->type() != TYPE::HORIZONTAL ||
       originSlideBlocks.back().back().y != DESTINATION_COOR.y)
        DISABLE_BUTTON(startButton);
    else
        ENABLE_BUTTON(startButton);
}

bool SolverScene::isRedBlockValid()
{
    if(slideblocks.back()->type() != TYPE::HORIZONTAL ||
       originSlideBlocks.back().back().y != DESTINATION_COOR.y)
    {
        DISABLE_BUTTON(startButton);
        return false;
    }
    ENABLE_BUTTON(startButton);
    return true;
}

bool SolverScene::isSlideBlockValid(vector<Coor> &slideBlock)
{
    for(auto &coor : slideBlock)
    {
        if(!SlideBlock::isFreePosition(coor))
            return false;
    }
    return true;
}


void SolverScene::resetOriginalMatrix()
{
    int i;
    SlideBlock::resetMatrix();
    for(i = 1; i < slideblocks.size(); ++i)
    {
        slideblocks[i]->setBodyCoor(originSlideBlocks[i]);
    }
}

bool SolverScene::checkPathValid()
{
    bool isValid;
    resetOriginalMatrix();
    for(auto path : paths)
    {
        if(slideblocks[path._id]->isValidMove(path._distance))
        {
            slideblocks[path._id]->moveBy(path._distance);
        }
        else
        {
            break;
        }
    }
    if(slideblocks.back()->getFinalMoveDistance() == 0)
        isValid = true;
    else
        isValid = false;
    resetOriginalMatrix();
    return isValid;
}

void SolverScene::sortBodys(std::vector<Coor> &coors)
{
    std::sort(coors.begin(), coors.end(), [&](Coor &first, Coor &second)
    {
        if(first.x == second.x)
            return first.y < second.y;
        return first.x < second.x;
    });
    coors.erase(unique(coors.begin(), coors.end()), coors.end());
}

void SolverScene::getAllMove(std::vector<Move> &moves)
{
    int i;
    for(i = 1; i < slideblocks.size(); ++i)
    {
        std::vector<int> dis;
        slideblocks[i]->getAllMove(dis);
        for(auto d : dis)
        {
            moves.push_back(Move(i, d));
        }
    }
}

void SolverScene::sortMove(std::vector<Move> &moves, std::vector<Move> &his)
{
    int i, j;
    if(his.size() > 0)
    {
        for(i = 0; i < moves.size(); ++i)
        {
            if(moves[i]._id == his.back()._id && moves[i]._distance + his.back()._distance == 0)
            {
                moves.erase(moves.begin() + i);
                break;
            }
        }
    }
    std::vector<unsigned int> numberOfMoves;
    std::vector<Move> temp;
    for(auto move : moves)
    {
        temp.clear();
        slideblocks[move._id]->moveBy(move._distance);
        getAllMove(temp);
        size_t currentHash = SlideBlock::hashCurrentMatrix();
        if(stateMatrixString.find(currentHash) == stateMatrixString.end())
        {
            if(slideblocks.back()->possibleReachTo(DESTINATION_COOR))
                numberOfMoves.push_back(UINT_MAX);
            else
                numberOfMoves.push_back(temp.size());
        }
        else
        {
            numberOfMoves.push_back(0);
        }
        slideblocks[move._id]->reverseMove();
    }
    for(j = moves.size() - 1; j > 0; --j)
    {
        for(i = 0; i < j; ++i)
        {
            if(numberOfMoves[i] > numberOfMoves[j])
            {
                unsigned int temp = numberOfMoves[i];
                numberOfMoves[i] = numberOfMoves[j];
                numberOfMoves[j] = temp;
                Move m = moves[i];
                moves[i] = moves[j];
                moves[j] = m;
            }
        }
    }
}

bool SolverScene::findMove(Move &maxMove)
{
    int i, j;
    std::vector<Move> allMoves, temp;
    std::vector<unsigned int> numberOfMoves;
    getAllMove(allMoves);
    for(auto move : allMoves)
    {
        temp.clear();
        slideblocks[move._id]->moveBy(move._distance);
        getAllMove(temp);
        size_t currentHash = SlideBlock::hashCurrentMatrix();
        if(stateMatrixString.find(currentHash) == stateMatrixString.end())
        {
            if(slideblocks.back()->possibleReachTo(DESTINATION_COOR))
            {
                numberOfMoves.push_back(UINT_MAX);
            }
            else
            {
                numberOfMoves.push_back(temp.size());
            }
        }
        else
        {
            numberOfMoves.push_back(0);
        }
        slideblocks[move._id]->reverseMove();
        slideblocks[move._id]->popReverse();
    }
    for(j = allMoves.size() - 1; j > 0; --j)
    {
        for(i = 0; i < j; ++i)
        {
            if(numberOfMoves[i] > numberOfMoves[j])
            {
                unsigned int temp = numberOfMoves[i];
                numberOfMoves[i] = numberOfMoves[j];
                numberOfMoves[j] = temp;
                Move m = allMoves[i];
                allMoves[i] = allMoves[j];
                allMoves[j] = m;
            }
        }
    }
    maxMove = allMoves.back();
    if(numberOfMoves.back() > 0)
        return true;
    return false;
}

//void SolverScene::takeAMove()
//{
//	std::vector<Move> moves;
//
//	if (slideblocks.back()->possibleReachTo(DESTINATION_COOR))
//	{
//		log("------------- found path -------------- %d moves", paths.size());
//		found = true;
//		optimizePath();
//		return;
//	}
//	size_t hashMatrix = SlideBlock::hashCurrentMatrix();
//	auto state = stateMatrixString.insert(hashMatrix);
//	moves.clear();
//	getAllMove(moves);
//	sortMove(moves, paths);
//	if (moves.size() > 0)
//	{
//		Move m;
//		if (state.second)
//			m = moves[moves.size() - 1];
//		else
//			m = moves[std::rand() % moves.size()];
//		slideblocks[m._id]->moveBy(m._distance);
//		paths.push_back(m);
//		hashMoveString.push_back(hashMatrix);
//		slideblocks[m._id]->refreshPosition(0.0025f, [&]()
//		{
//			takeAMove();
//		});
//	}
//	else
//	{
//		log("=================== out of move ================");
//	}
//}

//void SolverScene::takeAMove2()
//{
//	if (status != STATUS::RUNNING)
//		return;
//	std::vector<Move> moves;
//	if (slideblocks.back()->possibleReachTo(DESTINATION_COOR))
//	{
//		found = true;
//		hashMoveString.push_back(SlideBlock::hashCurrentMatrix());
//		paths.push_back(Move(slideblocks.back()->getSlideBlockID(), slideblocks.back()->getFinalMoveDistance()));
//		if (checkPathValid())
//		{
//			log("------------- found path -------------- %d moves", paths.size());
//			reducePathLenght();
//		}
//		else
//		{
//			log("----------------- found path failed ---------------");
//		}
//		return;
//	}
//	size_t hashMatrix = SlideBlock::hashCurrentMatrix();
//	stateMatrixString.insert(hashMatrix);
//
//	Move nextMove;
//	while (findMove(nextMove) == false)
//	{
//		if (paths.empty())
//		{
//			log("----------- can't find path ---------------");
//			return;
//		}
//		Move preMove = paths.back();
//		paths.pop_back();
//		hashMoveString.pop_back();
//		slideblocks[preMove._id]->reverseMove();
//		slideblocks[preMove._id]->popReverse();
//	}
//	hashMoveString.push_back(SlideBlock::hashCurrentMatrix());
//	paths.push_back(nextMove);
//	slideblocks[nextMove._id]->moveBy(nextMove._distance);
//	slideblocks[nextMove._id]->refreshPosition(0.0f, [&]()
//	{
//		takeAMove2();
//	});
//	//takeAMove2();
//}

void SolverScene::solvePuzzle()
{
    Move nextMove;
    while(status == STATUS::RUNNING)
    {
        if(slideblocks.back()->possibleReachTo(DESTINATION_COOR))
        {
            hashMoveString.push_back(SlideBlock::hashCurrentMatrix());
            paths.push_back(Move(slideblocks.back()->getSlideBlockID(),
                                 slideblocks.back()->getFinalMoveDistance()));
            if(checkPathValid())
            {
                log("------------- found path -------------- %d moves", paths.size());
                reducePathLenght();
            }
            else
            {
                log("----------------- found path failed ---------------");
            }
            status = STATUS::FINISH;
            break;
        }
        size_t hashMatrix = SlideBlock::hashCurrentMatrix();
        stateMatrixString.insert(hashMatrix);
        while(findMove(nextMove) == false)
        {
            if(paths.empty())
            {
                Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]()
                                                                                      {
                                                                                          statusLabel->setString(
                                                                                                  "Can't solve this puzzle");
                                                                                          ENABLE_BUTTON(
                                                                                                  newButton);
                                                                                          DISABLE_BUTTON(
                                                                                                  startButton);
                                                                                          pauseButton->setVisible(
                                                                                                  false);
                                                                                          startButton->setVisible(
                                                                                                  true);
                                                                                          this->unschedule(
                                                                                                  schedule_selector(
                                                                                                          SolverScene::updateClock));
                                                                                      });
                return;
            }
            Move preMove = paths.back();
            paths.pop_back();
            hashMoveString.pop_back();
            slideblocks[preMove._id]->reverseMove();
            slideblocks[preMove._id]->popReverse();
        }
        hashMoveString.push_back(SlideBlock::hashCurrentMatrix());
        paths.push_back(nextMove);
        slideblocks[nextMove._id]->moveBy(nextMove._distance);
    }
}

void SolverScene::finalOptimizePath()
{
    int i, j, t, g;
    resetOriginalMatrix();
    for(i = 0; i < paths.size(); ++i)
    {
        for(j = i + 1; j < paths.size(); ++j)
        {
            if(paths[i]._id == paths[j]._id)
            {
                paths[i]._distance += paths[j]._distance;
                for(t = i; t < j; ++t)
                {
                    if(slideblocks[paths[t]._id]->isValidMove(paths[t]._distance))
                        slideblocks[paths[t]._id]->moveBy(paths[t]._distance);
                    else
                        break;
                }
                g = t;
                while(--g >= i)
                {
                    slideblocks[paths[g]._id]->reverseMove();
                    slideblocks[paths[g]._id]->popReverse();
                }
                if(t == j)
                {
                    paths.erase(paths.begin() + j);
                }
                else
                {
                    paths[i]._distance -= paths[j]._distance;
                    paths[j]._distance += paths[i]._distance;
                    for(t = i + 1; t <= j; ++i)
                    {
                        if(slideblocks[paths[t]._id]->isValidMove(paths[t]._distance))
                            slideblocks[paths[t]._id]->moveBy(paths[t]._distance);
                        else
                            break;
                    }
                    if(t == j + 1)
                    {
                        paths[i]._distance = 0;
                    }
                    else
                    {
                        paths[j]._distance -= paths[i]._distance;
                    }
                    while(--t > i)
                    {
                        slideblocks[paths[t]._id]->reverseMove();
                        slideblocks[paths[t]._id]->popReverse();
                    }
                }
                break;
            }
        }
        slideblocks[paths[i]._id]->moveBy(paths[i]._distance);
    }
    for(i = 0; i < paths.size(); ++i)
    {
        if(paths[i]._distance == 0)
            paths.erase(paths.begin() + i--);
    }

}

void SolverScene::combineTwoPath()
{
    int i, j, t;
    for(i = 0; i + 1 < paths.size(); ++i)
    {
        for(j = i + 1; j < paths.size(); ++j)
        {
            if(paths[i]._id == paths[j]._id)
            {
                resetOriginalMatrix();
                paths[i]._distance += paths[j]._distance;
                for(t = 0; t < j; ++t)
                {
                    if(slideblocks[paths[t]._id]->isValidMove(paths[t]._distance))
                        slideblocks[paths[t]._id]->moveBy(paths[t]._distance);
                    else
                        break;
                }
                if(t == j)
                {
                    paths.erase(paths.begin() + j);
                    if(paths[i]._distance == 0)
                        paths.erase(paths.begin() + i);
                    return;
                }
                else
                {
                    paths[i]._distance -= paths[j]._distance;
                    paths[j]._distance += paths[i]._distance;
                    resetOriginalMatrix();
                    for(t = 0; t <= j; ++t)
                    {
                        if(t != i)
                        {
                            if(slideblocks[paths[t]._id]->isValidMove(paths[t]._distance))
                                slideblocks[paths[t]._id]->moveBy(paths[t]._distance);
                            else
                                break;

                        }
                    }
                    if(t == j + 1)
                    {
                        if(paths[j]._distance == 0)
                            paths.erase(paths.begin() + j);
                        paths.erase(paths.begin() + i);
                        return;
                    }
                    else
                    {
                        paths[j]._distance -= paths[i]._distance;
                    }
                }
                break;
            }
        }
    }
}

void SolverScene::combineTwoPathAdvance(int i, int j, int c, bool &combined, std::vector<int> &mark)
{
    int t;
    if(c == j - i)
    {
        combined = true;
        return;
    }
    for(t = 0; t < j - i; ++t)
    {
        if(combined == false && mark[t] == -1 &&
           slideblocks[paths[i + t]._id]->isValidMove(paths[i + t]._distance))
        {
            slideblocks[paths[i + t]._id]->moveBy(paths[i + t]._distance);
            mark[t] = c;
            combineTwoPathAdvance(i, j, c + 1, combined, mark);
            if(combined == false)
                mark[t] = -1;
            slideblocks[paths[i + t]._id]->reverseMove();
            slideblocks[paths[i + t]._id]->popReverse();
        }
    }
}

bool SolverScene::combinationPath()
{
    while(true)
    {
        auto length = paths.size();
        combineTwoPath();
        auto new_length = paths.size();
        if(length == new_length)
            break;
    }
    if(checkPathValid())
    {
        return true;
    }
    log("============= combinationPath failed %d =============", paths.size());
    return false;
}

bool SolverScene::combinationPathAdvance()
{
    int i, j, t;
    bool combined;
    resetOriginalMatrix();
    for(i = 0; i + 2 < paths.size(); ++i)
    {
        for(j = i + 2; j < paths.size(); ++j)
        {
            if(paths[i]._id == paths[j]._id)
            {
                if(j - i < 10)
                {
                    paths[i]._distance += paths[j]._distance;
                    std::vector<int> mark(j - i, -1);
                    combined = false;
                    combineTwoPathAdvance(i, j, 0, combined, mark);
                    if(combined)
                    {
                        std::vector<Move> temp(paths.begin() + i, paths.begin() + j);
                        for(t = 0; t < mark.size(); ++t)
                        {
                            paths[i + mark[t]] = temp[t];
                        }
                        paths.erase(paths.begin() + j);

                    }
                    else
                    {
                        paths[i]._distance -= paths[j]._distance;
                    }
                }
                break;
            }
        }
        if(paths[i]._distance == 0)
        {
            paths.erase(paths.begin() + i--);

        }
        else
            slideblocks[paths[i]._id]->moveBy(paths[i]._distance);
    }
    if(checkPathValid())
        return true;
    log("------------------------------------ combination failed ----------------------------------------------");
    return false;
}

bool SolverScene::optimizePath()
{
    if(paths.size() != hashMoveString.size())
    {
        log("------------------------ optimizePath failed! ---------------------");
        return false;
    }
    int i, j;
    for(i = 0; i < hashMoveString.size() - 2; ++i)
    {
        for(j = hashMoveString.size() - 1; j > i + 1; --j)
        {
            if(hashMoveString[i] == hashMoveString[j])
            {
                hashMoveString.erase(hashMoveString.begin() + i, hashMoveString.begin() + j);
                paths.erase(paths.begin() + i, paths.begin() + j);
                break;
            }
        }
    }
    if(checkPathValid())
    {
        return true;
    }
    log("====================== optimizePath failed! %d ============================",
        paths.size());
    return false;
}

bool SolverScene::minimizePath()
{
    int i, index_state;
    size_t currentHash;
    std::vector<Move> moves;
    Move nextMove;
    resetOriginalMatrix();
    for(index_state = 0; index_state + 2 < hashMoveString.size(); ++index_state)
    {
        nextMove = paths[index_state];
        moves.clear();
        getAllMove(moves);
        for(auto move : moves)
        {
            slideblocks[move._id]->moveBy(move._distance);
            currentHash = SlideBlock::hashCurrentMatrix();
            for(i = hashMoveString.size() - 1; i > index_state + 1; --i)
            {
                if(hashMoveString[i] == currentHash)
                {
                    hashMoveString.erase(hashMoveString.begin() + index_state + 1,
                                         hashMoveString.begin() + i);
                    paths.erase(paths.begin() + index_state, paths.begin() + i);
                    paths.insert(paths.begin() + index_state, move);
                    nextMove = move;
                    break;
                }
            }
            slideblocks[move._id]->reverseMove();
            slideblocks[move._id]->popReverse();
        }
        slideblocks[nextMove._id]->moveBy(nextMove._distance);
    }
    if(checkPathValid())
        return true;
    log("----------------------- minimizePath failed ---------------------");
    return false;
}

bool SolverScene::truncatePath()
{
    int i;
    for(i = 0; i < paths.size(); ++i)
    {
        auto move = paths[i];
        paths.erase(paths.begin() + i);
        if(checkPathValid())
        {
            hashMoveString.erase(hashMoveString.begin() + i);
            --i;
        }
        else
        {
            paths.insert(paths.begin() + i, move);
        }
    }
    if(checkPathValid())
        return true;
    log("--------------------- truncatePath failed %d ------------------------", paths.size());
    return false;
}

void SolverScene::reducePathLenght()
{
    if(minimizePath() && combinationPath() && truncatePath() && combinationPathAdvance())
    {
        showResult();
    }
    else
    {
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]()
                                                                              {
                                                                                  statusLabel->setString(
                                                                                          "Can't solve this puzzle");
                                                                                  ENABLE_BUTTON(
                                                                                          newButton);
                                                                                  DISABLE_BUTTON(
                                                                                          startButton);
                                                                                  pauseButton->setVisible(
                                                                                          false);
                                                                                  startButton->setVisible(
                                                                                          true);
                                                                              });
    }
    this->unschedule(schedule_selector(SolverScene::updateClock));
}

void SolverScene::next()
{
    if(index < paths.size())
    {
        slideblocks[paths[index]._id]->moveBy(paths[index]._distance);
        slideblocks[paths[index]._id]->refreshPosition(0.3f, [&]()
        {
        });
        ++index;
        if(index == 1)
            ENABLE_BUTTON(backButton);
        if(index == paths.size())
            DISABLE_BUTTON(nextButton);

        char status[100];
        sprintf(status, "%d/%d", index, paths.size());
        moveLabel->setString(status);
        if(sound_enabled)
            AudioEngine::play2d("move.mp3");
    }
}

void SolverScene::back()
{
    if(index <= paths.size())
    {
        --index;
        if(index == 0)
            DISABLE_BUTTON(backButton);
        if(index == paths.size() - 1)
            ENABLE_BUTTON(nextButton);

        slideblocks[paths[index]._id]->reverseMove();
        slideblocks[paths[index]._id]->refreshPosition(0.3f, [&]()
        {
        });
        slideblocks[paths[index]._id]->popReverse();
        char status[100];
        sprintf(status, "%d/%d", index, paths.size());
        moveLabel->setString(status);
        if(sound_enabled)
            AudioEngine::play2d("move.mp3");
    }
}

void SolverScene::showResult()
{
    resetOriginalMatrix();
    index = 0;
    Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]()
                                                                          {
                                                                              char status[100];
                                                                              sprintf(status,
                                                                                      "%d/%d",
                                                                                      index,
                                                                                      paths.size());
                                                                              statusLabel->setString(
                                                                                      "Moves");
                                                                              moveLabel->setString(
                                                                                      status);
                                                                              ENABLE_BUTTON(
                                                                                      nextButton);
                                                                              ENABLE_BUTTON(
                                                                                      newButton);
                                                                              DISABLE_BUTTON(
                                                                                      startButton);
                                                                              pauseButton->setVisible(
                                                                                      false);
                                                                              startButton->setVisible(
                                                                                      true);
                                                                          });
    if(sound_enabled)
        AudioEngine::play2d("success.mp3");
    ++numSolved;
    if(numSolved % 3 == 0)
        AdmobHelper::showInterstitialAd();
}

void SolverScene::updateClock(float dt)
{
    clock.tick();
    clockLabel->setString(clock.getFormatCurrentTime());
}

void SolverScene::showAds(float dt)
{
#if(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    AdmobHelper::showAd();
#endif
}
