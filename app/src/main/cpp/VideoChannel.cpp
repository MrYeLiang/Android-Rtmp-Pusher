//
// Created by 叶亮 on 2019/9/17.
//

#include "VideoChannel.h"
#include "LogUtils.h"
#include <pthread.h>
#include <x264.h>

VideoChannel::VideoChannel() {
    pthread_mutex_init(&mutex, 0);
}

VideoChannel::~VideoChannel() {
    pthread_mutex_destroy(&mutex);
    if (videoCodec) {
        x264_encoder_close(videoCodec);
        videoCodec = 0;
    }

    if (pic_in) {
        x264_picture_clean(pic_in);
        delete pic_in;
        pic_in = 0;
    }
}

void VideoChannel::setVideoEncodeInfo(int width, int height, int fps, int bitrate) {
    pthread_mutex_lock(&mutex);

    mWidth = width;
    mHeight = height;

    mFps = fps;
    mBitrate = bitrate;

    ySize = width * height;
    uvSize = ySize / 4;

    if (videoCodec) {
        x264_encoder_close(videoCodec);
        videoCodec = 0;
    }

    if (pic_in) {
        x264_picture_clean(pic_in);
        delete pic_in;
        pic_in = 0;
    }

    //打开x264解码器
    //x264解码器的属性
    x264_param_t param;

    //ultrafast 最快
    //zerolatency 无延迟解码
    x264_param_default_preset(&param, "ultrafast", "zerolatency");

    param.i_level_idc = 32;

    //输入数据格式
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = height;

    //无b帧
    param.i_bframe = 0;
    //参数i_rc_method表示码率控制， CQP(恒定质量) CRF(恒定码率) ABR(平均码率)
    param.rc.i_rc_method = X264_RC_ABR;
    //码率(比特率 单位Kbps)
    param.rc.i_bitrate = bitrate / 1000;

    //瞬时最大码率
    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2;
    //设置了i_vbv_max_bitrate必须设置此参数， 码率控制区大小 单位kbps
    param.rc.i_vbv_buffer_size = bitrate / 1000;

    //帧率
    param.i_fps_num = fps;
    param.i_fps_den = 1;
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;

    //用fps而不是时间戳来计算帧间距离
    param.b_vfr_input = 0;
    //帧距离(关键帧) 2s一个关键帧
    param.i_keyint_max = fps * 2;

    //是否赋值sps和pps放在每个关键帧的前面 该参数设置是让每个关键帧(I帧)都附带sps/pps
    param.b_repeat_headers = 1;
    //多线程
    param.i_threads = 1;

    x264_param_apply_profile(&param, "baseline");
    //打开解码器
    videoCodec = x264_encoder_open(&param);
    pic_in = new x264_picture_t;

    x264_picture_alloc(pic_in, X264_CSP_I420, width, height);
    pthread_mutex_unlock(&mutex);
}

void VideoChannel::setVideoCallback(VideoCallback callback) {
    this->callback = callback;
}

void VideoChannel::encodeData(int8_t *data) {
    //编码
    pthread_mutex_lock(&mutex);

    memcpy(pic_in->img.plane[0], data, ySize);

    for (int i = 0; i < uvSize; ++i) {
        //间隔1个字节取一个数据
        //u数据
        *(pic_in->img.plane[1] + i) = *(data + ySize + i * 2 + 1);

        //v数据
        *(pic_in->img.plane[2] + i) = *(data + ySize + i * 2);
    }

    //编码出的数据
    x264_nal_t *pp_nal;

    //编码除了几个nalu
    int pi_nal;
    x264_picture_t pic_out;
    //编码
    int ret = x264_encoder_encode(videoCodec, &pp_nal, &pi_nal, pic_in, &pic_out);
    if (ret < 0) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    int sps_len, pps_len;
    uint8_t sps[100];
    uint8_t pps[100];

    for (int i = 0; i < pi_nal; ++i) {
        //数据类型
        if (pp_nal[i].i_type == NAL_SPS) {

            //去掉00 00 00 01
            sps_len = pp_nal[i].i_payload - 4;
            memcpy(sps, pp_nal[i].p_payload + 4, sps_len);

        } else if(pp_nal[i].i_type == NAL_PPS){

            pps_len = pp_nal[i].i_payload - 4;
            memcpy(pps, pp_nal[i].p_payload + 4, pps_len);
            sendSpsPps(sps, pps, sps_len, pps_len);

        }else {
            sendFrame(pp_nal[i].i_type, pp_nal[i].p_payload, pp_nal[i].i_payload);
        }
    }

    pthread_mutex_unlock(&mutex);
}

void VideoChannel::sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len) {
    RTMPPacket *packet = new RTMPPacket;
    int bodysize = 13 + sps_len + 3 + pps_len;
    RTMPPacket_Alloc(packet, bodysize);

    int i = 0;

    //固定头
    packet->m_body[i++] = 0x17;

    //类型
    packet->m_body[i++] = 0x00;

    //composition time 0x000000
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;

    //版本
    packet->m_body[i++] = 0x01;
    //编码规格
    packet->m_body[i++] = sps[1];
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];
    packet->m_body[i++] = 0xFF;

    //整个sps
    packet->m_body[i++] = 0xE1;
    //sps长度
    packet->m_body[i++] = (sps_len >> 8) & 0xff;
    packet->m_body[i++] = sps_len & 0xff;
    memcpy(&packet->m_body[i], sps, sps_len);
    i += sps_len;

    //pps
    packet->m_body[i++] = 0x01;
    packet->m_body[i++] = (pps_len >> 8) & 0xff;
    packet->m_body[i++] = (pps_len) & 0xff;
    memcpy(&packet->m_body[i], pps, pps_len);

    //视频
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = bodysize;

    //随意分配一个管道(尽量避开rtmp.c中使用的)
    packet->m_nChannel = 10;
    //sps pps没有时间戳
    packet->m_nTimeStamp = 0;
    //不使用绝对时间
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    callback(packet);
}

void VideoChannel::sendFrame(int type, uint8_t *p_payload, int payload) {
    //去掉 00 00 00 01/ 00 00 01
    if (p_payload[2] == 0x00) {
        payload -= 4;
        p_payload += 4;
    } else {
        payload -= 3;
        p_payload += 3;
    }

    int bodysize = 9 + payload;

    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, bodysize);
    RTMPPacket_Reset(packet);

    packet->m_body[0] = 0x27;
    //关键帧
    if (type == NAL_SLICE_IDR) {
        LOGE("关键帧");
        packet->m_body[0] = 0x17;
    }

    //类型
    packet->m_body[1] = 0x01;

    packet->m_body[2] = 0x00;
    packet->m_body[3] = 0x00;
    packet->m_body[4] = 0x00;

    //数据长度 int 4个字节 相当于把int转成4个字节的byte数组
    packet->m_body[5] = (payload >> 24) & 0xff;
    packet->m_body[6] = (payload >> 16) & 0xff;
    packet->m_body[7] = (payload >> 8) & 0xff;
    packet->m_body[8] = (payload) & 0xff;

    //图片数据
    memcpy(&packet->m_body[9], p_payload, payload);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodysize;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nChannel = 0x10;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    callback(packet);
}

