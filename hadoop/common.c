/**
 * Copyright 2011-2012 the Redfish authors
 *
 * Licensed under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <errno.h>
#include <jni.h>
#include <stdint.h>

#include "hadoop/common.h"
#include "util/compiler.h"

/** Class prefix for Redfish classes */
#define RF_CP "org/apache/hadoop/fs/redfish/"

#define FSPERM_CP "org/apache/hadoop/fs/permission/FsPermission"

#define PATH_CP "org/apache/hadoop/fs/Path"

jfieldID g_fid_m_cli;
jfieldID g_fid_rf_in_stream_m_ofe;
jfieldID g_fid_rf_out_stream_m_ofe;

jclass g_cls_file_status;
jmethodID g_mid_file_status_ctor;

jclass g_cls_fs_perm;
jmethodID g_mid_fs_perm_ctor;

jclass g_cls_path;
jmethodID g_mid_path_ctor;

jclass g_cls_rf_in_stream;
jmethodID g_mid_rf_in_stream_ctor;

jclass g_cls_rf_out_stream;
jmethodID g_mid_rf_out_stream_ctor;

jclass g_cls_block_loc;
jmethodID g_mid_block_loc_ctor;

jclass g_cls_string;

static int cache_class_and_ctor(JNIEnv *jenv, const char *name,
		jclass *out_cls, jmethodID *out_ctor, const char *sig)
{
	jclass cls;
	jmethodID ctor;

	cls = (*jenv)->FindClass(jenv, name);
	if (!cls)
		return -ENOENT;
	ctor = (*jenv)->GetMethodID(jenv, cls, "<init>", sig);
	if (!ctor)
		return -ENOENT;
	*out_cls = (*jenv)->NewWeakGlobalRef(jenv, cls);
	if (!(*out_cls))
		return -ENOENT;
	*out_ctor = ctor;
	return 0;
}

static void uncache_class_and_ctor(JNIEnv *jenv,
		jclass *out_cls, jmethodID *out_ctor)
{
	(*jenv)->DeleteGlobalRef(jenv, *out_cls);
	*out_cls = NULL;
	*out_ctor = NULL;
}

static int cache_redfish_client_fields(JNIEnv *jenv)
{
	jclass cls;

	cls = (*jenv)->FindClass(jenv,  RF_CP "RedfishClient");
	if (!cls)
		return -ENOENT;
	g_fid_m_cli = (*jenv)->GetFieldID(jenv, cls, "m_cli", "J");
	if (!g_fid_m_cli)
		return -ENOENT;
	return 0;
}

void redfish_throw(JNIEnv *jenv, const char *name, const char *msg)
{
	jclass cls = (*jenv)->FindClass(jenv, name);
	if (!cls) {
		/* If !cls, we just raised a NoClassDefFound exception, or
		 * similar. */
		return;
	}
	(*jenv)->ThrowNew(jenv, cls, msg);
	(*jenv)->DeleteLocalRef(jenv, cls);
}

static int cache_redfish_input_stream_fields(JNIEnv *jenv)
{
	int ret;

	ret = cache_class_and_ctor(jenv, RF_CP "RedfishDataInputStream",
		&g_cls_rf_in_stream, &g_mid_rf_in_stream_ctor, "(J)V");
	if (ret)
		return -ENOENT;
	g_fid_rf_in_stream_m_ofe = (*jenv)->GetFieldID(jenv, g_cls_rf_in_stream,
			"m_ofe", "J");
	if (!g_fid_rf_in_stream_m_ofe)
		return -ENOENT;
	return 0;
}

static int cache_redfish_output_stream_fields(JNIEnv *jenv)
{
	int ret;

	ret = cache_class_and_ctor(jenv, RF_CP "RedfishDataOutputStream",
		&g_cls_rf_out_stream, &g_mid_rf_out_stream_ctor, "(J)V");
	if (ret)
		return -ENOENT;
	g_fid_rf_out_stream_m_ofe = (*jenv)->GetFieldID(jenv, g_cls_rf_out_stream,
			"m_ofe", "J");
	if (!g_fid_rf_out_stream_m_ofe)
		return -ENOENT;
	return 0;
}

static int cache_string_class(JNIEnv *jenv)
{
	jclass cls;

	cls = (*jenv)->FindClass(jenv, "java/lang/String");
	if (!cls)
		return -ENOENT;
	g_cls_string = (*jenv)->NewWeakGlobalRef(jenv, cls);
	if (!g_cls_string)
		return -ENOENT;
	return 0;
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *jvm, POSSIBLY_UNUSED(void *reserved))
{
	int ret;
	JNIEnv *jenv = NULL;

	HJNI_DEBUG("libhfishc: entering JNI_OnLoad\n");
	if ((*jvm)->GetEnv(jvm, (void **)&jenv, JNI_VERSION_1_2)) {
		return JNI_ERR; /* JNI version not supported */
	}
	HJNI_DEBUG("libhfishc: caching client fields\n");
	ret = cache_redfish_client_fields(jenv);
	if (ret)
		return JNI_ERR;
	HJNI_DEBUG("libhfishc: caching input stream fields\n");
	ret = cache_redfish_input_stream_fields(jenv);
	if (ret)
		return JNI_ERR;
	HJNI_DEBUG("libhfishc: caching output stream fields\n");
	ret = cache_redfish_output_stream_fields(jenv);
	if (ret)
		return JNI_ERR;
	HJNI_DEBUG("libhfishc: caching FilePermission\n");
	ret = cache_class_and_ctor(jenv, FSPERM_CP,
			&g_cls_fs_perm, &g_mid_fs_perm_ctor, "(S)V");
	if (ret)
		return JNI_ERR;
	HJNI_DEBUG("libhfishc: caching FileStatus\n");
	ret = cache_class_and_ctor(jenv, "org/apache/hadoop/fs/FileStatus",
			&g_cls_file_status, &g_mid_file_status_ctor,
			"(JZIJJJL" FSPERM_CP ";Ljava/lang/String;"
			"Ljava/lang/String;L" PATH_CP ";)V");
	if (ret)
		return JNI_ERR;
	HJNI_DEBUG("libhfishc: caching Path\n");
	ret = cache_class_and_ctor(jenv, PATH_CP, &g_cls_path, &g_mid_path_ctor,
			"(Ljava/lang/String;)V");
	if (ret)
		return JNI_ERR;
	HJNI_DEBUG("libhfishc: caching BlockLocation\n");
	ret = cache_class_and_ctor(jenv, "org/apache/hadoop/fs/BlockLocation",
			&g_cls_block_loc, &g_mid_block_loc_ctor,
			"([Ljava/lang/String;[Ljava/lang/String;JJ)V");
	if (ret)
		return JNI_ERR;
	HJNI_DEBUG("libhfishc: caching Java string class\n");
	ret = cache_string_class(jenv);
	if (ret)
		return JNI_ERR;
	HJNI_DEBUG("libhfishc: JNI_OnLoad succeeded\n");
	return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM *jvm, POSSIBLY_UNUSED(void *reserved))
{
	JNIEnv *jenv;

	HJNI_DEBUG("libhfishc: entering JNI_OnUnload\n");
	if ((*jvm)->GetEnv(jvm, (void **)&jenv, JNI_VERSION_1_2)) {
		return;
	}
	g_fid_m_cli = 0;
	uncache_class_and_ctor(jenv, &g_cls_rf_in_stream,
		     &g_mid_rf_in_stream_ctor);
	uncache_class_and_ctor(jenv, &g_cls_rf_out_stream,
		     &g_mid_rf_out_stream_ctor);
	uncache_class_and_ctor(jenv, &g_cls_fs_perm,
			&g_mid_fs_perm_ctor);
	uncache_class_and_ctor(jenv, &g_cls_file_status,
			&g_mid_file_status_ctor);
	uncache_class_and_ctor(jenv, &g_cls_path,
			&g_mid_path_ctor);
	uncache_class_and_ctor(jenv, &g_cls_block_loc,
			&g_mid_block_loc_ctor);
	(*jenv)->DeleteGlobalRef(jenv, g_cls_string);
	HJNI_DEBUG("libhfishc: exiting JNI_OnUnload\n");
}

jint validate_rw_params(JNIEnv *jenv, jbyteArray jarr,
		jint boff, jint blen)
{
	/* TODO: we could probably skip at least some of this verification,
	 * since a lot of the JNI functions, like SetByteArrayRegion, seem to
	 * validate paramaters for us.  It would have to be done carefully,
	 * though, to avoid exposing us to mishaps elsewhere in the code or
	 * implementation details of the various JNI implementations.
	 */
	int32_t alen;
	uint32_t end;

	if (boff < 0) {
		redfish_throw(jenv, "java/lang/IndexOutOfBoundsException",
				"boff < 0");
		return -1;
	}
	if (blen < 0) {
		redfish_throw(jenv, "java/lang/IndexOutOfBoundsException",
				"blen < 0");
		return -1;
	}
	if (jarr == NULL) {
		redfish_throw(jenv, "java/lang/NullPointerException",
				"buf == NULL");
		return -1;
	}
	/* It's important to do the addition of boff and blen as an unsigned
	 * operation, so that we don't get undefined behavior on integer
	 * overflow.  We do the comparison as a signed comparison, so that if
	 * overflow did take place, we're comparing a positive number with a
	 * negative one.
	 *
	 * Unlike C, Java defines integers as 4 bytes, no matter what the
	 * underlying machine architecture may be.  That's why we can ignore the
	 * jint, etc typedefs and just use uint32_t and friends.
	 */
	alen = (*jenv)->GetArrayLength(jenv, jarr);
	end = ((uint32_t)boff) + ((uint32_t)blen);
	if (((int32_t)end) > alen) {
		redfish_throw(jenv, "java/lang/IndexOutOfBoundsException",
				"boff + blen > buf.length");
		return -1;
	}
	return 0;
}

int jstr_to_cstr(JNIEnv *jenv, jstring jstr, char *cstr, size_t cstr_len)
{
	char err[128];
	size_t err_len = sizeof(err);
	int32_t jlen, clen;

	/* GetStringUTFLength is kind of a bizarre API.
	 *
	 * First, we have to supply the length of the region we want to get--
	 * but not in bytes, in UTF-8 characters!  If you ask for too many, you
	 * get an exception.  How about having a simple API that just gets the
	 * whole string; isn't that what 9999 people out of 10000 want?
	 * But no, we have to make the extra function call every time.
	 *
	 * Secondly, it does no bounds checking.  So we have to make another
	 * extra function call here to find out what the length would be as a
	 * UTF-8 string.  The example in the JNI programmer's guide even omits
	 * bounds checking-- they comment on ther omission, but don't fix it!
	 * Somebody was not thinking clearly here.
	 *
	 * Despite all the warts, this API at least lets us avoid doing tons of
	 * malloc()s, and that is a good thing.
	 */
	clen = (*jenv)->GetStringUTFLength(jenv, jstr);
	if (clen > (int32_t)cstr_len) {
		snprintf(err, err_len, "jstr_to_cstr: tried to load %d byte "
			 "java string into %Zd byte C string\n", clen, cstr_len);
		redfish_throw(jenv,
			"java/lang/StringIndexOutOfBoundsException", err);
		return -ENAMETOOLONG;
	}
	jlen = (*jenv)->GetStringLength(jenv, jstr);
	(*jenv)->GetStringUTFRegion(jenv, jstr, 0, jlen, cstr);
	if ((*jenv)->ExceptionCheck(jenv))
		return -EIO;
	return 0;
}
