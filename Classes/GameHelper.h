//
// Created by CUONG on 12/3/2018.
//

#ifndef PROJ_ANDROID_STUDIO_GAMEHELPER_H
#define PROJ_ANDROID_STUDIO_GAMEHELPER_H

#include "cocos2d.h"
#include "SolverScene.h"
#include "Preprocessing.h"

class SolverScene;

class GameHelper
{
public:
     static SolverScene *solver;
public:
    static void openChooseFileDialog();
};

# ifdef __cplusplus
extern "C"
{
# endif

JNIEXPORT void JNICALL Java_org_game_cpp_AppActivity_solve(JNIEnv * env, jclass obj, jstring path);
JNIEXPORT void JNICALL Java_org_game_cpp_AppActivity_setCachePath(JNIEnv * env, jclass obj, jstring path);


# ifdef __cplusplus
}
# endif

#endif //PROJ_ANDROID_STUDIO_GAMEHELPER_H
