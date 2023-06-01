#include "Ultils.h"

#if(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"
#include <jni.h>

#endif

void DISABLE_BUTTON(cocos2d::ui::Button *button)
{
    button->setEnabled(false);
    button->setBright(false);
}

void ENABLE_BUTTON(cocos2d::ui::Button *button)
{
    button->setEnabled(true);
    button->setBright(true);
}

#if(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
std::string getPackageName()
{
    cocos2d::JniMethodInfo methodInfo;
    if(cocos2d::JniHelper::getStaticMethodInfo(methodInfo, "org/cocos2dx/lib/Cocos2dxActivity",
                                               "myGetPackageName", "()Ljava/lang/String;"))
    {
        jstring str = (jstring)methodInfo.env->CallStaticObjectMethod(methodInfo.classID,
                                                                      methodInfo.methodID);
        std::string pkgName = cocos2d::JniHelper::jstring2string(str);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
        return pkgName;
    }
    return std::string("");
}

#endif