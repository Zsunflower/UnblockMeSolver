//
// Created by CUONG on 12/3/2018.
//

#include <jni.h>
#include "GameHelper.h"
#include "platform/android/jni/JniHelper.h"

using namespace cocos2d;

void GameHelper::openChooseFileDialog()
{
    JniMethodInfo t;

    if(JniHelper::getStaticMethodInfo(t, "org/game/cpp/AppActivity", "chooseFile", "()V"))
    {
        log("Calling chooseFile from JNI");
        t.env->CallStaticVoidMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
    }
}

JNIEXPORT void JNICALL Java_org_game_cpp_AppActivity_solve(JNIEnv * env, jclass obj, jstring path)
{
    const char *_path;
    _path = env->GetStringUTFChars(path, NULL);
    log("JNI recieved %s", _path);
    if(GameHelper::solver)
        GameHelper::solver->setPathFile(_path);

    env->ReleaseStringUTFChars(path, _path);
}

JNIEXPORT void JNICALL Java_org_game_cpp_AppActivity_setCachePath(JNIEnv * env, jclass obj, jstring path)
{
    const char *_path;
    _path = env->GetStringUTFChars(path, NULL);
    log("UBMSolver JNI set cache path: %s", _path);
    cache_dir = _path;
    env->ReleaseStringUTFChars(path, _path);
}