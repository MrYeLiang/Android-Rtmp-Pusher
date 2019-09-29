//
// Created by 叶亮 on 2019/9/29.
//

#ifndef ANDROID_RTMP_PUSHER_AUDIOCHANNEL_H
#define ANDROID_RTMP_PUSHER_AUDIOCHANNEL_H

#include <faac.h>
#include <sys/types.h>
#include "librtmp/rtmp.h"

class AudioChannel{
    typedef void (*AudioCallback)(RTMPPacket *packet);

public:
    AudioChannel();
    ~AudioChannel();

    void setAudioEncInfo(int samplesInHZ, int channels);

    void setAudioCallback(AudioCallback audioCallback);

    int getInputSamples();

    void encodeData(int8_t *data);

    RTMPPacket* getAudioTag();

private :
    AudioCallback  audioCallback;
    int mChannel;
    faacEncHandle  audioCodec = 0;
    u_long inputSamples;
    u_long maxOutputBytes;
    u_char *buffer = 0;
};
#endif //ANDROID_RTMP_PUSHER_AUDIOCHANNEL_H
