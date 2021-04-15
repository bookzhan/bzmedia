//
/**
 * Created by bookzhan on 2021-04-07 13:21.
 *description:
 */
//

#ifndef BZMEDIA_PCMDEQUE_H
#define BZMEDIA_PCMDEQUE_H

#include<mutex>
#include<deque>
#include <bean/PcmData.h>

using namespace std;

class PcmDeque {
public:
    void put(PcmData *val);

    PcmData *get();

    bool isEmpty();

    void clear();

    unsigned long count();

private:
    mutex lock;
    deque<PcmData *> mDeque;
    int size;
};


#endif //BZMEDIA_PCMDEQUE_H
