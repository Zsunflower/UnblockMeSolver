LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

CVROOT := E:\OpenCV-android-sdk\sdk\native\jni
OPENCV_INSTALL_MODULES:=on
OPENCV_CAMERA_MODULES:=off
OPENCV_LIB_TYPE:=STATIC
include $(CVROOT)/OpenCV.mk


$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d)
$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d/external)
$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d/cocos)
$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d/cocos/audio/include)

LOCAL_MODULE := MyGame_shared

LOCAL_MODULE_FILENAME := libMyGame

LOCAL_SRC_FILES := hellocpp/main.cpp \
../../../Classes/AppDelegate.cpp \
../../../Classes/AdmobHelper.cpp \
../../../Classes/GameHelper.cpp \
../../../Classes/Ultils.cpp \
../../../Classes/Clock.cpp \
../../../Classes/Menu.cpp \
../../../Classes/SlideBlock.cpp \
../../../Classes/Preprocessing.cpp \
../../../Classes/SolverScene.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../Classes
LOCAL_C_INCLUDES += $(CVROOT)/include

# _COCOS_HEADER_ANDROID_BEGIN
# _COCOS_HEADER_ANDROID_END

LOCAL_STATIC_LIBRARIES += cocos2dx_static

# _COCOS_LIB_ANDROID_BEGIN
# _COCOS_LIB_ANDROID_END

include $(BUILD_SHARED_LIBRARY)

$(call import-module,.)

# _COCOS_LIB_IMPORT_ANDROID_BEGIN
# _COCOS_LIB_IMPORT_ANDROID_END
