#include "Menu.h"
#include "Preprocessing.h"
#include <dirent.h>

USING_NS_CC;

Scene *MenuScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();

    // 'layer' is an autorelease object
    auto layer = MenuScene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool MenuScene::init()
{
    if(!Layer::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto layer = CSLoader::createNode("MenuLayer.csb");

    auto startButton = static_cast<ui::Button *>(layer->getChildByName("Button_Start"));
    startButton->addTouchEventListener([&](Ref *ref, ui::Widget::TouchEventType type)
                                       {
                                           if(type == ui::Widget::TouchEventType::ENDED)
                                           {
                                               Director::getInstance()->replaceScene(
                                                       TransitionFade::create(0.5f,
                                                                              SolverScene::createScene(),
                                                                              Color3B(10, 20, 30)));
                                           }
                                       });

    auto soundButton = static_cast<ui::Button *>(layer->getChildByName("Button_Sound"));
    if(sound_enabled)
        soundButton->loadTextureNormal("btn_sound_on.png");
    else
        soundButton->loadTextureNormal("btn_sound_off.png");
    soundButton->addTouchEventListener([&](Ref *ref, ui::Widget::TouchEventType type)
                                       {
                                           if(type == ui::Widget::TouchEventType::ENDED)
                                           {
                                               sound_enabled = !sound_enabled;
                                               if(sound_enabled)
                                                   static_cast<ui::Button *>(ref)->loadTextureNormal(
                                                           "btn_sound_on.png");
                                               else
                                                   static_cast<ui::Button *>(ref)->loadTextureNormal(
                                                           "btn_sound_off.png");
                                               UserDefault::getInstance()->setBoolForKey(
                                                       "sound_enabled", sound_enabled);
                                           }
                                       });

    auto rateButton = static_cast<ui::Button *>(layer->getChildByName("Button_Rate"));
    rateButton->addTouchEventListener([&](Ref *ref, ui::Widget::TouchEventType type)
                                      {
                                          if(type == ui::Widget::TouchEventType::ENDED)
                                          {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
                                              std::string pkgName = getPackageName();
                                              Application::getInstance()->openURL(
                                                      "market://details?id=" + pkgName);
#endif
                                          }
                                      });

    layer->setPositionY(origin.y + 0.5f * (visibleSize.height - 960));
    this->addChild(layer);

    auto keyListener = EventListenerKeyboard::create();
    keyListener->onKeyReleased = CC_CALLBACK_2(MenuScene::onKeyReleased, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyListener, this);

    return true;
}

void MenuScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event *event)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}