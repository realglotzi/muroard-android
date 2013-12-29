LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := muroar_jni

LOCAL_SRC_FILES :=  NativeUtils.c

include $(BUILD_STATIC_LIBRARY)
