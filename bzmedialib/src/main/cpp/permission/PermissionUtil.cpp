//
/**
 * Created by bookzhan on 2018-07-21 10:24.
 * 说明:
 */
//

#include <cwchar>
#include <malloc.h>
#include "PermissionUtil.h"
#include <sys/ptrace.h>
#include <string>
#include <jni.h>
#include <mutex>
#include <common/BZLogUtil.h>
//#include <android/log.h>
extern "C" {
#include "base64.h"
}

using namespace std;
static std::mutex *permissionUtilLock = new mutex();
const int  PermissionUtil::SUPPORT_PACKAGE_NUMBERS = 4;
const char *PermissionUtil::PACKAGE_NAMES[PermissionUtil::SUPPORT_PACKAGE_NUMBERS] = {
        "nYXRk4qQhprRjJ6Sj5Oa",
        "nJCS0ZGdndGdmoyLiZabmpCam5aLkI0=",
        "nJCS0ZmTnpiMl5CL0ZyKi4+TnoY=",
        "nJCS0ZGdndGamZmanIvRjJ6Sj5Oa"
};

bool PermissionUtil::checkPermission(void *_env, void *_context) {
    if (nullptr == _env || nullptr == _context) {
        exitProgram();
        return false;
    }
    auto *env = static_cast<JNIEnv *>(_env);
    auto context = static_cast<jobject>(_context);

    permissionUtilLock->lock();
    //反调试
#ifndef DEBUG_TYPE
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    BZLogUtil::logD("checkPermission release");
#endif
    jclass context_class = env->GetObjectClass(context);
    jmethodID methodId = env->GetMethodID(context_class, "getFilesDir",
                                          "()Ljava/io/File;");
    jobject fileObj = env->CallObjectMethod(context, methodId);

    if (nullptr == fileObj) {
        exitProgram();
        return false;
    }
    jclass fileClass = env->GetObjectClass(fileObj);
    methodId = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jstring absolute_path = (jstring) env->CallObjectMethod(fileObj, methodId);

    if (nullptr == absolute_path) {
        exitProgram();
        return false;
    }
    env->DeleteLocalRef(context_class);
    env->DeleteLocalRef(fileClass);
    const char *filePath = env->GetStringUTFChars(absolute_path, 0);

    string strFilePath;
    strFilePath.append(filePath);

    bool hasPermission = false;
    for (int i = 0; i < SUPPORT_PACKAGE_NUMBERS; ++i) {
        char *decodeStr = NULL;
        PermissionUtil::decode_str(PermissionUtil::PACKAGE_NAMES[i], &decodeStr);
        if (NULL == decodeStr) {
            continue;
        }
//        BZLogUtil::logD("decodeStr=%s", decodeStr);
        std::string subStr;
        subStr.append(decodeStr);
        subStr.append("/files");
        bool endStr = endWith(&strFilePath, &subStr);
        free(decodeStr);
        if (!endStr) {
            continue;
        }
        hasPermission = true;
        break;
    }
    env->DeleteLocalRef(fileObj);
    env->ReleaseStringUTFChars(absolute_path, filePath);
    if (!hasPermission) {
        exitProgram();
    }
    permissionUtilLock->unlock();
    return hasPermission;
}

int PermissionUtil::encode_str(const char *srcStr, char **outStr) {
    if (NULL == srcStr) {
        return -1;
    }
    size_t length = strlen(srcStr) + 1;
    char *buffer = static_cast<char *>(malloc(length));
    memset(buffer, 0, length);

    for (int i = 0; i < length - 1; ++i) {
        *(buffer + i) = ~(*(srcStr + i));
    }
    //base64编码
    size_t len = 0;
    unsigned char *result = base64_encode(reinterpret_cast<const unsigned char *>(buffer),
                                          strlen(buffer), &len);
    if (nullptr == result) {
        return -1;
    }
    char *buffer_encode = static_cast<char *>(malloc(len + 1));
    memset(buffer_encode, 0, len + 1);
    memcpy(buffer_encode, result, len);

    free(buffer);
    free(result);
    *outStr = buffer_encode;
    return 0;
}

int PermissionUtil::decode_str(const char *srcStr, char **outStr) {
    if (NULL == srcStr) {
        return -1;
    }
    //base64解码
    size_t len = 0;
    unsigned char *baRetStr = base64_decode(
            reinterpret_cast<const unsigned char *>(srcStr), strlen(srcStr), &len);
    if (nullptr == baRetStr) {
        return -1;
    }

    char *buffer_decode = static_cast<char *>(malloc(len + 1));
    memset(buffer_decode, 0, len + 1);
    memcpy(buffer_decode, baRetStr, len);

    size_t length = strlen(buffer_decode) + 1;
    char *buffer = static_cast<char *>(malloc(length));
    memset(buffer, 0, length);
    //简单解混淆
    for (int i = 0; i < length - 1; ++i) {
        *(buffer + i) = ~(*(buffer_decode + i));
    }
    free(buffer_decode);
    free(baRetStr);

    *outStr = buffer;
    return 0;
}

void PermissionUtil::exitProgram() {
    //制造奔溃
    const char *str = "hello";
    char *strP = const_cast<char *>(str);
    *(strP + 1) = 'g';
}

bool PermissionUtil::endWith(std::string *srcStr, std::string *subStr) {
    if (nullptr == srcStr || nullptr == subStr) {
        return false;
    }
    if (subStr->length() > srcStr->length()) {
        return false;
    }
    int index = srcStr->rfind(*subStr);
    unsigned long index_2 = srcStr->length() - subStr->length();
    return !(index < 0 || index != index_2);
}
