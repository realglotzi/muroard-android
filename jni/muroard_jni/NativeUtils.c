/*
 * NativeUtils.cpp
 *
 *  Created on: 04.04.2013
 *      Author: jenkinsci
 */


#include <jni.h>
#include <android/log.h>
#include <math.h>
#include <stdio.h>
//#include <cpu-features.h>

#include <muroard.h>
#include <muroar.h>

/*
JNIEXPORT jstring JNICALL Java_com_roaraudio_muroard_NativeUtils_getCpuInfo(JNIEnv* env,
                jobject obj) {

        string a;
        uint64_t cpu_features;

        if (android_getCpuFamily() != ANDROID_CPU_FAMILY_ARM) {
                return env->NewStringUTF("Not ARM");
        }

        cpu_features = android_getCpuFeatures();
        if (cpu_features & ANDROID_CPU_ARM_FEATURE_ARMv7) {
                a.append(" ARMc7 \n");
                LOGI("Arm7");
        }
        if (cpu_features & ANDROID_CPU_ARM_FEATURE_VFPv3) {
                a.append(" ARM w VFPv3 support \n");
                LOGI("VFP3");
        }
        if ((cpu_features & ANDROID_CPU_ARM_FEATURE_NEON)) {
                a.append(" ARM w NEON support \n");
                LOGI("NEON");
        }

        if (cpu_features & ANDROID_CPU_ARM_FEATURE_LDREX_STREX) {
                a.append(" LDREX_STREX ");
                LOGI("LDREX_STREX");
        }

        if (a == "") {
                return env->NewStringUTF("Unknown");
        }

        return env->NewStringUTF(a.c_str());
}
*/

JNIEXPORT jint JNICALL Java_com_roaraudio_muroard_NativeUtils_startMuroarD (JNIEnv *env, jobject obj, jstring args, jint argc)
{
	 __android_log_print(ANDROID_LOG_DEBUG, "NativeUtils_startMuroarD", "start");

	struct muroard_state state;
	int ret;

	muroard_g_state = &state;

	ret = muroard_main(0, NULL);

	 __android_log_print(ANDROID_LOG_DEBUG, "NativeUtils_startMuroarD", "stop");

	return ret;
}

JNIEXPORT void JNICALL Java_com_roaraudio_muroard_NativeUtils_stopMuroarD ()
{
	__android_log_print(ANDROID_LOG_DEBUG, "NativeUtils_stopMuroarD", "start");

	muroard_state_member(alive)  = 0;
}


