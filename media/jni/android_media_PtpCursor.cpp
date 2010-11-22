/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "PtpCursorJNI"
#include "utils/Log.h"

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include "binder/CursorWindow.h"

#include "MtpClient.h"
#include "PtpCursor.h"

using namespace android;

// ----------------------------------------------------------------------------

static jfieldID field_context;

// From android_media_PtpClient.cpp
MtpClient * get_client_from_object(JNIEnv * env, jobject javaClient);

// ----------------------------------------------------------------------------

static bool ExceptionCheck(void* env)
{
    return ((JNIEnv *)env)->ExceptionCheck();
}

static void
android_media_PtpCursor_setup(JNIEnv *env, jobject thiz, jobject javaClient,
        jint queryType, jint deviceID, jlong storageID, jlong objectID, jintArray javaColumns)
{
#ifdef HAVE_ANDROID_OS
    LOGD("android_media_PtpCursor_setup queryType: %d deviceID: %d storageID: %lld objectID: %lld\n",
                queryType, deviceID, storageID, objectID);

    int* columns = NULL;
    int columnCount = 0;
    if (javaColumns) {
        columns = env->GetIntArrayElements(javaColumns, 0);
        columnCount = env->GetArrayLength(javaColumns);
    }

    MtpClient* client = get_client_from_object(env, javaClient);
    PtpCursor* cursor = new PtpCursor(client, queryType,
            deviceID, storageID, objectID, columnCount, columns);

    if (columns)
        env->ReleaseIntArrayElements(javaColumns, columns, 0);
    env->SetIntField(thiz, field_context, (int)cursor);
#endif
}

static void
android_media_PtpCursor_finalize(JNIEnv *env, jobject thiz)
{
#ifdef HAVE_ANDROID_OS
    LOGD("finalize\n");
    PtpCursor *cursor = (PtpCursor *)env->GetIntField(thiz, field_context);
    delete cursor;
#endif
}

static jint
android_media_PtpCursor_fill_window(JNIEnv *env, jobject thiz, jobject javaWindow, jint startPos)
{
#ifdef HAVE_ANDROID_OS
    CursorWindow* window = get_window_from_object(env, javaWindow);
    if (!window) {
        LOGE("Invalid CursorWindow");
        jniThrowException(env, "java/lang/IllegalArgumentException",
                          "Bad CursorWindow");
        return 0;
    }
    PtpCursor *cursor = (PtpCursor *)env->GetIntField(thiz, field_context);

    return cursor->fillWindow(window, startPos);
#else
    return 0;
#endif
}

// ----------------------------------------------------------------------------

static JNINativeMethod gMethods[] = {
    {"native_setup",            "(Landroid/media/PtpClient;IIJJ[I)V",
                                        (void *)android_media_PtpCursor_setup},
    {"native_finalize",         "()V",  (void *)android_media_PtpCursor_finalize},
    {"native_fill_window",      "(Landroid/database/CursorWindow;I)I",
                                        (void *)android_media_PtpCursor_fill_window},

};

static const char* const kClassPathName = "android/media/PtpCursor";

int register_android_media_PtpCursor(JNIEnv *env)
{
    jclass clazz;

    LOGD("register_android_media_PtpCursor\n");

    clazz = env->FindClass("android/media/PtpCursor");
    if (clazz == NULL) {
        LOGE("Can't find android/media/PtpCursor");
        return -1;
    }
    field_context = env->GetFieldID(clazz, "mNativeContext", "I");
    if (field_context == NULL) {
        LOGE("Can't find PtpCursor.mNativeContext");
        return -1;
    }

    return AndroidRuntime::registerNativeMethods(env,
                "android/media/PtpCursor", gMethods, NELEM(gMethods));
}
