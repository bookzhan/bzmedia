//
/**
 * Created by bookzhan on 2018-07-21 10:24.
 * 说明:
 */
//

#ifndef APKENCRYPT_PERMISSIONUTIL_H
#define APKENCRYPT_PERMISSIONUTIL_H

#include <string>

class PermissionUtil {
public:

    static const int SUPPORT_PACKAGE_NUMBERS;
    static const char *PACKAGE_NAMES[];

    static bool checkPermission(void *env, void *context);

    static int encode_str(const char *srcStr, char **outStr);

    static int decode_str(const char *srcStr, char **outStr);

private:
    static void exitProgram();
    static bool endWith(std::string *srcStr, std::string *subStr);
};


#endif //APKENCRYPT_PERMISSIONUTIL_H
