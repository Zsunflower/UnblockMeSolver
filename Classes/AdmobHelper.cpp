/*
Copyright (c) 2014 Mudafar
GPLv3
*/


#include "AdmobHelper.h"
#include "cocos2d.h"


bool AdmobHelper::isAdShowing = true;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"
#include <jni.h>

const char *AppActivityClassName = "org/cocos2dx/cpp/AppActivity";

void AdmobHelper::hideAd()
{
    cocos2d::JniMethodInfo t;
    if(cocos2d::JniHelper::getStaticMethodInfo(t, AppActivityClassName, "hideAd", "()V"))
    {

        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
        isAdShowing = false;
    }
}


void AdmobHelper::showAd()
{
    cocos2d::log("=============== Admob display ads ====================");
    cocos2d::JniMethodInfo t;
    if(cocos2d::JniHelper::getStaticMethodInfo(t, AppActivityClassName, "showAd", "()V"))
    {

        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
        isAdShowing = true;
    }
}


void AdmobHelper::showInterstitialAd()
{
    cocos2d::log("=============== Admob showInterstitialAd ====================");
    cocos2d::JniMethodInfo t;
    if (cocos2d::JniHelper::getStaticMethodInfo(t, AppActivityClassName, "showInterstitialAd", "()V"))
    {
        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
}



#endif


