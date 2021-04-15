//
/**
 * Created by bookzhan on 2021-04-07 13:21.
 *description:
 */
//
#include "PcmDeque.h"


void PcmDeque::put(PcmData *val) {
    lock.lock();
    mDeque.push_back(val);
    lock.unlock();
}

PcmData *PcmDeque::get() {
    if (isEmpty()) {
        return nullptr;
    }
    lock.lock();
    PcmData *pcmData = mDeque.front();
    mDeque.pop_front();
    lock.unlock();
    return pcmData;
}

bool PcmDeque::isEmpty() {
    lock.lock();
    bool empty = mDeque.empty();
    lock.unlock();
    return empty;
}

void PcmDeque::clear() {
    lock.lock();

    lock.unlock();
}

unsigned long PcmDeque::count() {
    lock.lock();
    unsigned long size = mDeque.size();
    lock.unlock();
    return size;
}
