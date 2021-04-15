//
/**
 * Created by bookzhan on 2019-05-09 15:54.
 * 说明:线程安全的AVPacketDeque
 */
//

#ifndef BZMEDIA_AVPACKETDEQUE_H
#define BZMEDIA_AVPACKETDEQUE_H

extern "C" {
#include <include/libavcodec/avcodec.h>
};

#include <mutex>
#include <deque>

using namespace std;

class AVPacketDeque {
public:
    void pushBack(AVPacket *avPacket);

    AVPacket *getFirst();

    AVPacket *front();

    void clear();

    bool isEmpty();

    long getSize();

private:
    mutex avPacketLock;
    deque<AVPacket *> videoPacketDeque;
};


#endif //BZMEDIA_AVPACKETDEQUE_H
