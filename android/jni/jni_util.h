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
/* C99 macros for getting class/field/method ids from jni */
/* won't work with C++ because the jni api will be different */
/* usage:
   #define my_uses(use, _)\
   use##class(_, Foo, "my/package/Foo")\
   use##field(_, Foo, bar, "mBar", "I")\
   use##method(_, Foo, init, "<init>", "V")
   ... etc ...
   
   somewhere in top level:
   JNI_UTIL_GEN_STATIC(my_uses)

   somewhere in init code:
   JNI_UTIL_GEN_INIT(my_uses, ptr_to_jni_env)
*/

/* name mangling for static vars */

#define JNI_UTIL_CLASS(csym)        _JNI_UTIL_CLASS_##csym
#define JNI_UTIL_FIELD(csym, fsym)  _JNI_UTIL_FIELD_##csym##_##fsym
#define JNI_UTIL_METHOD(csym, msym) _JNI_UTIL_METHOD_##csym##_##msym

/* generate statics */

#define _JNI_UTIL_S_CLASS(env, sym, name)\
	static jclass JNI_UTIL_CLASS(sym);
#define _JNI_UTIL_S_FIELD(env, csym, fsym, name, sig)\
	static jfieldID JNI_UTIL_FIELD(csym, fsym);
#define _JNI_UTIL_S_METHOD(env, csym, msym, name, sig)\
	static jmethodID JNI_UTIL_METHOD(csym, msym);

#define JNI_UTIL_GEN_STATIC(uses) uses(_JNI_UTIL_S_, _jni_util_unused)


/* generate init code */

#define _JNI_UTIL_I_CLASS(env, sym, name)\
	JNI_UTIL_CLASS(sym) = (*(env))->FindClass((env), name);\
	if (!(JNI_UTIL_CLASS(sym)))\
		DIE("cannot FindClass %s\n", #name);\
	JNI_UTIL_CLASS(sym) = (jclass)(*(env))->NewGlobalRef((env), JNI_UTIL_CLASS(sym));
#define _JNI_UTIL_I_FIELD(env, csym, fsym, name, sig)\
	JNI_UTIL_FIELD(csym, fsym) = (*(env))->GetFieldID((env), JNI_UTIL_CLASS(csym), name, sig);\
	if (!(JNI_UTIL_FIELD(csym, fsym)))\
		DIE("cannot GetFieldID %s\n", #csym " " #name " " #sig);
#define _JNI_UTIL_I_METHOD(env, csym, msym, name, sig)\
	JNI_UTIL_METHOD(csym, msym) = (*(env))->GetMethodID((env), JNI_UTIL_CLASS(csym), name, sig);\
	if (!(JNI_UTIL_METHOD(csym, msym)))\
		DIE("cannot GetMethodID %s\n", #msym " " #name " " #sig);

#define JNI_UTIL_GEN_INIT(uses, env) uses(_JNI_UTIL_I_, env)
