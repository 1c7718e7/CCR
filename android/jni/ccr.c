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
#include "ccr.h"
#include <stdio.h>
#include <assert.h>
#include "shirokov_martin_ccr_Worker_CCR.h"

#ifndef NDEBUG
#include <android/log.h>
#define DEBUG(...) __android_log_print(ANDROID_LOG_INFO, "ccr.native", __VA_ARGS__)
#else
#define DEBUG(...) (void)0
#endif

#define DIE(...) {DEBUG(__VA_ARGS__); abort();}

#define JNI_USES(USE, _)\
	USE##CLASS(_, Kanji, "shirokov/martin/ccr/Kanji")\
	USE##FIELD(_, Kanji, pointv, "pointv", "[D")\
	USE##FIELD(_, Kanji, pointc, "pointc", "I")\
	USE##FIELD(_, Kanji, sizev,  "sizev",  "[I")\
	USE##FIELD(_, Kanji, sizec,  "sizec",  "I")\
	USE##METHOD(_, Kanji, init,  "<init>",  "([D[I)V")\
	\
	USE##CLASS(_, Match, "shirokov/martin/ccr/Match")\
	USE##FIELD(_, Match, score,  "score",  "D")\
	USE##FIELD(_, Match, code,   "code",   "I")\
	USE##FIELD(_, Match, cookie, "cookie", "Lshirokov/martin/ccr/Match$Cookie;")\
	USE##METHOD(_, Match, init, "<init>", "(DILshirokov/martin/ccr/Match$Cookie;)V")\
	\
	USE##CLASS(_, Cookie, "shirokov/martin/ccr/Match$Cookie")\
	USE##FIELD(_, Cookie, ptr, "ptr", "J")\
	USE##METHOD(_, Cookie, init, "<init>", "(J)V")\
	\
	USE##CLASS(_, Feedback, "shirokov/martin/ccr/Worker$Feedback")\
	USE##METHOD(_, Feedback, init, "<init>", "([I[ILshirokov/martin/ccr/Kanji;)V")

#include "jni_util.h"

#define JC(x) JNI_UTIL_CLASS(x)
#define JF(x, y) JNI_UTIL_FIELD(x, y)
#define JM(x, y) JNI_UTIL_METHOD(x, y)

JNI_UTIL_GEN_STATIC(JNI_USES)

JNIEXPORT void JNICALL Java_shirokov_martin_ccr_Worker_00024CCR_initJNI
  (JNIEnv *env, jclass CCR)
{
	JNI_UTIL_GEN_INIT(JNI_USES, env)
}

/*
 * Class:     shirokov_martin_ccr_Worker_CCR
 * Method:    open
 * Signature: ([BJ)J
 */
JNIEXPORT jlong JNICALL Java_shirokov_martin_ccr_Worker_00024CCR_open
  (JNIEnv *env, jclass CCR, jbyteArray _data, jlong size)
{
	jboolean is_copy;
	size_t maxsize = (*env)->GetArrayLength(env, (jarray)_data);
	assert(size <= maxsize);
	void *data = (*env)->GetByteArrayElements(env, _data, &is_copy);
	KanjiDB *db = kanji_db_open(data, size);
	assert(db);
	(*env)->ReleaseByteArrayElements(env, _data, data, 0);
	jlong r = (jlong)db;
	assert((KanjiDB*)r == db);
	return r;
}

/*
 * Class:     shirokov_martin_ccr_Worker_CCR
 * Method:    close
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_shirokov_martin_ccr_Worker_00024CCR_close
  (JNIEnv *env, jclass CCR, jlong _db)
{
	kanji_db_close((KanjiDB*)_db);
}

static void *load_kanji(JNIEnv *env, Kanji *k, jobject _k)
{
	jboolean is_copy;
	char *mem;

	k->n = (*env)->GetIntField(env, _k, JF(Kanji, sizec));
	DEBUG("got sizec\n");

	jintArray _sizev = (jintArray*)(*env)->GetObjectField(env, _k, JF(Kanji, sizev));
	jint *sizev = (*env)->GetIntArrayElements(env, _sizev, &is_copy);
	DEBUG("got sizev\n");

	jdoubleArray _pointv = (jdoubleArray*)(*env)->GetObjectField(env, _k, JF(Kanji, pointv));
	jdouble *pointv = (*env)->GetDoubleArrayElements(env, _pointv, &is_copy);
	size_t pointv_len = (*env)->GetArrayLength(env, (jarray)_pointv);
	DEBUG("got pointv\n");

	int pc = 0;
	for (int i = 0, ofs = 0; i < k->n; i++)
		pc += sizev[i];
	mem = malloc(pc * sizeof(Vec2) + k->n * sizeof *k->p);
	k->p = (void*)mem;
	Vec2 *vv = (void*)(mem + k->n * sizeof *k->p);
	for (int i = 0, ofs = 0; i < k->n; i++) {
		DEBUG("STROKE %i\n", sizev[i]);
		assert(ofs < pointv_len);
		k->p[i].n = sizev[i];
		k->p[i].p = vv;
		for (int j = 0; j < sizev[i]; j++) {
			vv->x = pointv[ofs++];
			vv->y = pointv[ofs++];
			DEBUG("POINT %lf,%lf\n", vv->x, vv->y);
			vv++;
		}
	}

	(*env)->ReleaseDoubleArrayElements(env, _pointv, pointv, 0);
	DEBUG("released _pointv\n");
	(*env)->ReleaseIntArrayElements(env, _sizev, sizev, 0);
	DEBUG("released _sizev\n");

	return mem;
}

/*
 * Class:     shirokov_martin_ccr_Worker_CCR
 * Method:    lookup
 * Signature: (JLshirokov/martin/ccr/Kanji;I)[Lshirokov/martin/ccr/Match;
 */
JNIEXPORT jobjectArray JNICALL Java_shirokov_martin_ccr_Worker_00024CCR_lookup
  (JNIEnv *env, jclass CCR, jlong _db, jobject _k, jint maxres)
{

	Kanji __k, *k = &__k;

	void *kmem = load_kanji(env, k, _k);
	kanji_normalize(k);

	KanjiMatch *res = malloc(maxres * sizeof *res);
	uint resc = kanji_db_lookup((KanjiDB*)_db, k, res, maxres);
	DEBUG("lookup done\n");
	
	jobjectArray _res = (*env)->NewObjectArray(env, resc, JC(Match), NULL);
	DEBUG("created _res\n");
	for (int i = 0; i < resc; i++) {
		jobject _cookie = (*env)->NewObject(env,
			JC(Cookie), JM(Cookie, init), (jlong)res[i].cookie);
		DEBUG("created a _cookie\n");
		jobject _match = (*env)->NewObject(env,
			JC(Match), JM(Match, init),
				(jdouble)res[i].score,
				(jint)res[i].code,
				_cookie);
		DEBUG("created a _match\n");
		(*env)->SetObjectArrayElement(env, _res, i, _match);
		DEBUG("_res[%i] <-\n", i);
	}

	free(kmem);
	free(res);
	return _res;
}

static jobject new_jkanji(JNIEnv *env, const Kanji *k)
{
	jboolean is_copy;

	size_t pc = 0;
	for (int i = 0; i < k->n; i++)
		pc += k->p[i].n;

	jdoubleArray _pv = (*env)->NewDoubleArray(env, pc*2);
	double *pv = (*env)->GetDoubleArrayElements(env, _pv, &is_copy);
	jintArray _sv = (*env)->NewIntArray(env, k->n);
	int *sv = (*env)->GetIntArrayElements(env, _sv, &is_copy);

	DEBUG("new_jkanji\n");
	int ofs = 0;
	for (int i = 0; i < k->n; i++) {
		sv[i] = k->p[i].n;
		DEBUG("STROKE LEN %i\n", sv[i]);
		for (int j = 0; j < k->p[i].n; j++) {
			assert(0 <= ofs && ofs < pc*2);
			pv[ofs++] = k->p[i].p[j].x;
			pv[ofs++] = k->p[i].p[j].y;
		}
	}
	assert(ofs == 2*pc);
	(*env)->ReleaseDoubleArrayElements(env, _pv, pv, 0);
	(*env)->ReleaseIntArrayElements(env, _sv, sv, 0);
	return (*env)->NewObject(env, JC(Kanji), JM(Kanji, init), _pv, _sv);
}

/*
 * Class:     shirokov_martin_ccr_Worker_CCR
 * Method:    feedback
 * Signature: (JLshirokov/martin/ccr/Kanji;J)Lshirokov/martin/ccr/Worker/Feedback;
 */
JNIEXPORT jobject JNICALL Java_shirokov_martin_ccr_Worker_00024CCR_feedback
  (JNIEnv *env, jclass CCR, jlong _db, jobject _user, jobject _cookie)
{
	Kanji luser, *user = &luser;
	jboolean is_copy;

	void *kmem = load_kanji(env, user, _user);
	kanji_normalize(user);

	void *cookie = (*env)->GetLongField(env, _cookie, JF(Cookie, ptr));
	DEBUG("got cookie\n");
	const Kanji *model = kanji_db_data((KanjiDB*)_db, cookie);

	jintArray _ui = (*env)->NewIntArray(env, user->n);
	int *ui = (*env)->GetIntArrayElements(env, _ui, &is_copy);

	jintArray _mi = (*env)->NewIntArray(env, model->n);
	int *mi = (*env)->GetIntArrayElements(env, _mi, &is_copy);

	DEBUG("created arrays\n");

	kanji_feedback(user, model, ui, mi);
	DEBUG("got feedback\n");
	free(kmem);

	(*env)->ReleaseIntArrayElements(env, _ui, ui, 0);
	(*env)->ReleaseIntArrayElements(env, _mi, mi, 0);
	DEBUG("released arrays\n");
	ui = NULL; mi = NULL;

	jobject _fb = (*env)->NewObject(env,
		JC(Feedback), JM(Feedback, init),
		_ui, _mi, new_jkanji(env, model));
	DEBUG("created Feedback\n");
	kanji_db_data_release((KanjiDB*)_db, cookie, model);
	return _fb;
}
