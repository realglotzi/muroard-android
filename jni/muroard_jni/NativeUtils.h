/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com.roaraudio.muroard_NativeUtils */

#ifndef _Included_com.roaraudio.muroard_NativeUtils
#define _Included_com.roaraudio.muroard_NativeUtils
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com.roaraudio.muroard_NativeUtils
 * Method:    getCpuInfo
 * Signature: ()Ljava/lang/String;
 */
//JNIEXPORT jstring JNICALL Java_com.roaraudio.muroard_NativeUtils_getCpuInfo
//  (JNIEnv *, jobject);

/*
 * Class:     com.roaraudio.muroard_NativeUtils
 * Method:    startMuroarD
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_com_roaraudio_muroard_NativeUtils_startMuroarD
  (JNIEnv *, jobject, jstring, jint);

JNIEXPORT jint JNICALL Java_com_roaraudio_muroard_NativeUtils_stopMuroarD
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
