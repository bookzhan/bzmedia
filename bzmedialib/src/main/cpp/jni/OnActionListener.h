//
/**
 * Created by zhandalin on 2019-02-18 10:16.
 * 说明:回调上层代码
 */
//

#ifndef BZMEDIA_ONACTIONLISTENER_H
#define BZMEDIA_ONACTIONLISTENER_H


#include <jni.h>
#include <cstdint>

class OnActionListener {
public:
    explicit OnActionListener(jobject obj);

    //传入C层的回调
    static void progressCallBack(int64_t handle, int what, float progress);

    void progress(float progress);

    void fail();

    void success();

    ~OnActionListener();

private:
    jobject actionObj = nullptr;
    jmethodID progressMID = nullptr;
    jmethodID failMID = nullptr;
    jmethodID successMID = nullptr;
};


#endif //BZMEDIA_ONACTIONLISTENER_H
