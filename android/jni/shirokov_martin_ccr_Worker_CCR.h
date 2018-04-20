/* This file is part of CCR.
 * Copyright (C) 2018  Martin Shirokov
 * 
 * CCR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * CCR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CCR.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <jni.h>
/* Header for class shirokov_martin_ccr_Worker_CCR */

#ifndef _Included_shirokov_martin_ccr_Worker_CCR
#define _Included_shirokov_martin_ccr_Worker_CCR
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     shirokov_martin_ccr_Worker_CCR
 * Method:    initJNI
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_shirokov_martin_ccr_Worker_00024CCR_initJNI
  (JNIEnv *, jclass);

/*
 * Class:     shirokov_martin_ccr_Worker_CCR
 * Method:    open
 * Signature: ([BJ)J
 */
JNIEXPORT jlong JNICALL Java_shirokov_martin_ccr_Worker_00024CCR_open
  (JNIEnv *, jclass, jbyteArray, jlong);

/*
 * Class:     shirokov_martin_ccr_Worker_CCR
 * Method:    close
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_shirokov_martin_ccr_Worker_00024CCR_close
  (JNIEnv *, jclass, jlong);

/*
 * Class:     shirokov_martin_ccr_Worker_CCR
 * Method:    lookup
 * Signature: (JLshirokov/martin/ccr/Kanji;I)[Lshirokov/martin/ccr/Match;
 */
JNIEXPORT jobjectArray JNICALL Java_shirokov_martin_ccr_Worker_00024CCR_lookup
  (JNIEnv *, jclass, jlong, jobject, jint);

/*
 * Class:     shirokov_martin_ccr_Worker_CCR
 * Method:    feedback
 * Signature: (JLshirokov/martin/ccr/Kanji;Lshirokov/martin/ccr/Match/Cookie;)Lshirokov/martin/ccr/Worker/Feedback;
 */
JNIEXPORT jobject JNICALL Java_shirokov_martin_ccr_Worker_00024CCR_feedback
  (JNIEnv *, jclass, jlong, jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif
