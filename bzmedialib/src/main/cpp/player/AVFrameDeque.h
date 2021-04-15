//
/**
 * Created by bookzhan on 2019-05-09 16:09.
 * 说明:
 */
//

#ifndef BZMEDIA_AVFRAMEDEQUE_H
#define BZMEDIA_AVFRAMEDEQUE_H

extern "C" {
#include <libavformat/avformat.h>
};

#include <mutex>
#include <deque>

using namespace std;


class AVFrameDeque {
public:
    void pushBack(AVFrame *avFrame);

    AVFrame *getFirst();

    AVFrame *front();

    void clear();

    bool isEmpty();

    long getSize();

private:
    mutex avFrameLock;
    deque<AVFrame *> videoFrameDeque;
};


#endif //BZMEDIA_AVFRAMEDEQUE_H
