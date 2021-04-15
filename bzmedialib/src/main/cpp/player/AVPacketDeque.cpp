//
/**
 * Created by bookzhan on 2019-05-09 15:54.
 * 说明:
 */
//

#include "AVPacketDeque.h"

void AVPacketDeque::pushBack(AVPacket *avPacket) {
    if (nullptr == avPacket) {
        return;
    }
    avPacketLock.lock();
    videoPacketDeque.push_back(avPacket);
    avPacketLock.unlock();
}

AVPacket *AVPacketDeque::getFirst() {
    avPacketLock.lock();
    bool isEmpty = videoPacketDeque.empty();
    AVPacket *avPacket = nullptr;
    if (!isEmpty) {
        avPacket = videoPacketDeque.front();
        videoPacketDeque.pop_front();
    }
    avPacketLock.unlock();
    return avPacket;
}

void AVPacketDeque::clear() {
    avPacketLock.lock();
    auto itor = videoPacketDeque.begin();
    while (itor != videoPacketDeque.end()) {
        AVPacket *avPacket = *itor;
        av_packet_free(&avPacket);
        itor++;
    }
    videoPacketDeque.clear();
    avPacketLock.unlock();
}

bool AVPacketDeque::isEmpty() {
    avPacketLock.lock();
    bool isEmpty = videoPacketDeque.empty();
    avPacketLock.unlock();
    return isEmpty;
}

long AVPacketDeque::getSize() {
    avPacketLock.lock();
    long size = videoPacketDeque.size();
    avPacketLock.unlock();
    return size;
}

AVPacket *AVPacketDeque::front() {
    avPacketLock.lock();
    bool isEmpty = videoPacketDeque.empty();
    AVPacket *avPacket = nullptr;
    if (!isEmpty) {
        avPacket = videoPacketDeque.front();
    }
    avPacketLock.unlock();
    return avPacket;
}
