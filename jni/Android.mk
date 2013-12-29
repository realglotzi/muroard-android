LOCAL_PATH := $(call my-dir)

TINYALSA_PATH := $(LOCAL_PATH)/tinyalsa
MUROARD_NATIVE_PATH := $(LOCAL_PATH)/muroard
MUROARD_JNI_PATH := $(LOCAL_PATH)/muroard_jni

include $(CLEAR_VARS)

include $(TINYALSA_PATH)/Android.mk
include $(MUROARD_NATIVE_PATH)/Android.mk
include $(MUROARD_JNI_PATH)/Android.mk

LOCAL_MODULE    := muroard
LOCAL_MODULE_FILENAME := libmuroard

LOCAL_C_INCLUDES += $(MUROARD_NATIVE_PATH) $(MUROARD_JNI_PATH) $(TINYALSA_PATH)/include
LOCAL_CFLAGS	:= -Wno-multichar  
 
LOCAL_LDLIBS 	:= -landroid -llog
LOCAL_STATIC_LIBRARIES := muroard_native muroard_jni tinyalsa 
	
include $(BUILD_SHARED_LIBRARY)
