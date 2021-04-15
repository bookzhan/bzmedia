//
/**
 * Created by bookzhan on 2018-04-10 11:17.
 * 说明:
 */
//

#ifndef BZFFMPEG_GETJNIENV_H
#define BZFFMPEG_GETJNIENV_H

#include <jni.h>
#include <cstdint>


class JvmManager {
public:
    static JavaVM *getJavaVM();

    static bool getJNIEnv(JNIEnv **pJNIEnv);

    static int32_t JNI_VERSION;
};

#endif //BZFFMPEG_GETJNIENV_H
