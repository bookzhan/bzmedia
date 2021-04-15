//
/**
 * Created by bookzhan on 2018-09-03 10:10.
 * 说明:
 */
//

#ifndef BZMEDIA_AUDIOFEATUREINFOUTIL_H
#define BZMEDIA_AUDIOFEATUREINFOUTIL_H


#include <cstdint>

class AudioFeatureInfoUtil {
public:
    int getAudioFeatureInfo(const char *audioPath, int samples,
                            void (*featureCallBack)(int64_t, int64_t, float) = nullptr);

    void setCallBackHandle(int64_t callBackHandle);

    int64_t getCallBackHandle();

private:
    int64_t callBackHandle = 0;
};


#endif //BZMEDIA_AUDIOFEATUREINFOUTIL_H
