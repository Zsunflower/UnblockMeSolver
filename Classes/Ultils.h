#ifndef ULTILS_H
#define ULTILS_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <sstream>

extern bool sound_enabled;
extern std::string cache_dir;

template<typename T>
std::string to_string(T value)
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

void DISABLE_BUTTON(cocos2d::ui::Button *button);

void ENABLE_BUTTON(cocos2d::ui::Button *button);

#if(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
std::string getPackageName();
#endif

#endif