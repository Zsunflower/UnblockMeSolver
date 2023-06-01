#ifndef MENU_H
#define MENU_H

#include "cocos2d.h"
#include "cocostudio\CocoStudio.h"
#include "ui\CocosGUI.h"
#include "Ultils.h"
#include "SolverScene.h"


class MenuScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene *createScene();

    virtual bool init();

    CREATE_FUNC(MenuScene);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event *event);
};

#endif