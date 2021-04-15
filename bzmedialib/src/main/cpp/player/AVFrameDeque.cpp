//
/**
 * Created by bookzhan on 2019-05-09 16:09.
 * 说明:
 */
//

#include <common/BZLogUtil.h>
#include "AVFrameDeque.h"

void AVFrameDeque::pushBack(AVFrame *avFrame) {
    if (nullptr == avFrame
        || nullptr == avFrame->data[0]
        || nullptr == avFrame->data[1]) {
        BZLogUtil::logE("AVFrameDeque::pushBack AVFrame data Error");
        return;
    }
    avFrameLock.lock();
    videoFrameDeque.push_back(avFrame);
    avFrameLock.unlock();
}

AVFrame *AVFrameDeque::getFirst() {
    avFrameLock.lock();
    bool isEmpty = videoFrameDeque.empty();
    AVFrame *avFrame = nullptr;
    if (!isEmpty) {
        avFrame = videoFrameDeque.front();
        videoFrameDeque.pop_front();
    }
    avFrameLock.unlock();
    return avFrame;
}

void AVFrameDeque::clear() {
    avFrameLock.lock();
    auto itor = videoFrameDeque.begin();
    while (itor != videoFrameDeque.end()) {
        AVFrame *avFrame = *itor;
        av_frame_free(&avFrame);
        itor++;
    }
    videoFrameDeque.clear();
    avFrameLock.unlock();
}

bool AVFrameDeque::isEmpty() {
    avFrameLock.lock();
    bool isEmpty = videoFrameDeque.empty();
    avFrameLock.unlock();
    return isEmpty;
}

long AVFrameDeque::getSize() {
    avFrameLock.lock();
    long size = videoFrameDeque.size();
    avFrameLock.unlock();
    return size;
}

AVFrame *AVFrameDeque::front() {
    avFrameLock.lock();
    bool isEmpty = videoFrameDeque.empty();
    AVFrame *avFrame = nullptr;
    if (!isEmpty) {
        avFrame = videoFrameDeque.front();
    }
    avFrameLock.unlock();
    return avFrame;
}
