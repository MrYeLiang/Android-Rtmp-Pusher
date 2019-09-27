//
// Created by 叶亮 on 2019/9/17.
//

#ifndef ANDROID_RTMP_PUSHER_VIDEOCHANNEL_H
#define ANDROID_RTMP_PUSHER_VIDEOCHANNEL_H

#include <inttypes.h>
#include <sys/types.h>
#include <x264.h>
#include "librtmp/rtmp.h"

class VideoChannel {

    typedef void (*VideoCallback)(RTMPPacket* packet);

public:
    VideoChannel();
    ~VideoChannel();

    //创建x264编码器
    void setVideoEncodeInfo(int width, int height, int fps, int bitrate);

    void encodeData(int8_t *data);

    void setVideoCallback(VideoCallback callback);

private:
    pthread_mutex_t mutex;
    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    x264_t *videoCodec = 0;

    x264_picture_t *pic_in = 0;
    int ySize;
    int uvSize;
    int index = 0;
    VideoCallback callback;

    void sendSpsPps(uint8_t *sps, uint8_t *pps, int len, int pps_len);

    void sendFrame(int type, uint8_t *p_payload, int payload);
};


#endif //ANDROID_RTMP_PUSHER_VIDEOCHANNEL_H
