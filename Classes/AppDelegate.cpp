#include "AppDelegate.h"
#include "Menu.h"
#include "SolverScene.h"
#include "Ultils.h"

USING_NS_CC;

bool sound_enabled = false;
string cache_dir = "";

static cocos2d::Size designResolutionSize = cocos2d::Size(540, 960);

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()
{
}

void AppDelegate::initGLContextAttrs()
{
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

static int register_all_packages()
{
    return 0; //flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching()
{
    sound_enabled = UserDefault::getInstance()->getBoolForKey("sound_enabled", true);

    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview)
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        glview = GLViewImpl::createWithRect("UnblockMe Solver", cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
#else
        glview = GLViewImpl::create("SlideBlockSolver");
#endif
        director->setOpenGLView(glview);
    }

    director->setAnimationInterval(1.0f / 60);

    glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height,
                                    ResolutionPolicy::FIXED_WIDTH);

    register_all_packages();
    FileUtils::getInstance()->addSearchPath("res");

    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("wood.plist");
    auto scene = MenuScene::createScene();

    director->runWithScene(scene);

    return true;
}

void AppDelegate::applicationDidEnterBackground()
{
    Director::getInstance()->stopAnimation();

}

void AppDelegate::applicationWillEnterForeground()
{
    Director::getInstance()->startAnimation();

}
