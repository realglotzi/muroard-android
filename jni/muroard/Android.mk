LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -DPACKAGE_VERSION="\"0.1.13\"" -DDEVICE_VENDOR_STRING="\"Android\"" -DDISTRIBUTION_VERSION_STRING="\"\""

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(TINYALSA_PATH)/include
       
LOCAL_SRC_FILES := muroard.c cf_riff_wave.c cf_au.c \
	proto.c para.c driver_tinyalsa.c clients.c dsp.c waveform.c \
	network.c streams.c muroarstream.c muroar.c muroario.c muroar_stream.c \
	muroar_setvolume.c muroar_quit.c muroar_noop.c muroar_close.c muroar_beep.c     

LOCAL_MODULE := muroard_native

include $(BUILD_STATIC_LIBRARY)