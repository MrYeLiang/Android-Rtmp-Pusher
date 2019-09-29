#include <jni.h>
#include <string>
#include "VideoChannel.h"
#include "safe_queue.h"
#include "LogUtils.h"
#include "AudioChannel.h"

VideoChannel *videoChannel = 0;

uint32_t start_time;
SafeQueue<RTMPPacket *> packets;
int isStart = 0;
pthread_t pid;
int readyPushing = 0;
AudioChannel *audioChannel = 0;

void releasePackets(RTMPPacket *&packet) {
    if (packet) {
        RTMPPacket_Free(packet);
        delete packet;
        packet = 0;
    }
}

void callback(RTMPPacket *packet) {
    if (packet) {
        packet->m_nTimeStamp = RTMP_GetTime() - start_time;
        packets.push(packet);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yeliang_LivePusher_native_1init(JNIEnv *env, jobject instance) {
    videoChannel = new VideoChannel;
    videoChannel->setVideoCallback(callback);

    audioChannel = new AudioChannel;
    audioChannel->setAudioCallback(callback);
    packets.setReleaseCallback(releasePackets);
}

void *start(void *args) {
    char *url = static_cast<char *>(args);
    RTMP *rtmp = 0;
    do {
        rtmp = RTMP_Alloc();
        if (!rtmp) {
            LOGE("rtmp创建失败");
            break;
        }

        RTMP_Init(rtmp);
        rtmp->Link.timeout = 5;
        int ret = RTMP_SetupURL(rtmp, url);
        if (!ret) {
            LOGE("rtmp设置地址失败： %s", url);
            break;
        }

        RTMP_EnableWrite(rtmp);
        ret = RTMP_Connect(rtmp, 0);
        if (!ret) {
            LOGE("rtmp连接地址失败：%s", url);
        }

        ret = RTMP_ConnectStream(rtmp, 0);
        if (!ret) {
            LOGE("rtmp连接流失败：%s", url);
            break;
        }
        readyPushing = 1;
        start_time = RTMP_GetTime();
        packets.setWork(1);
        RTMPPacket *packet = 0;
        while (readyPushing) {
            packets.pop(packet);
            if (!readyPushing) {
                break;
            }

            if (!packet) {
                continue;
            }

            packet->m_nInfoField2 = rtmp->m_stream_id;
            ret = RTMP_SendPacket(rtmp, packet, 1);
            releasePackets(packet);
            if (!ret) {
                LOGE("发送流数据失败");
                break;
            }

        }
        releasePackets(packet);
    } while (0);

    isStart = 0;
    readyPushing = 0;
    packets.setWork(0);
    packets.clear();
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }

    delete url;
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yeliang_LivePusher_native_1start(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    if (isStart) {
        return;
    }

    char *url = new char[strlen(path) + 1];
    strcpy(url, path);
    isStart = 1;
    pthread_create(&pid, 0, start, url);
    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yeliang_LivePusher_native_1stop(JNIEnv *env, jobject instance) {
    LOGI("enter stop");
    readyPushing = 0;
    //关闭队列工作
    packets.setWork(0);
    LOGI("enter stop 1");
    pthread_join(pid, 0);
    LOGI("leave stop");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yeliang_LivePusher_native_1push_1video(JNIEnv *env, jobject instance, jbyteArray data_) {

    if (!videoChannel || !readyPushing) {
        return;
    }
    jbyte *data = env->GetByteArrayElements(data_, NULL);
    videoChannel->encodeData(data);
    env->ReleaseByteArrayElements(data_, data, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yeliang_LivePusher_native_1setVideoEncInfo(JNIEnv *env, jobject instance, jint width,
                                                    jint height, jint fps, jint mBitrate) {
    if (videoChannel) {
        videoChannel->setVideoEncodeInfo(width, height, fps, mBitrate);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yeliang_LivePusher_native_1setAudioInfo(JNIEnv *env, jobject instance, jint sampleRate,
                                                 jint channels) {
    if (audioChannel) {
        audioChannel->setAudioEncInfo(sampleRate, channels);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_yeliang_LivePusher_getInputSamples(JNIEnv *env, jobject instance) {


    if (audioChannel) {
        return audioChannel->getInputSamples();
    }
    return -1;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yeliang_LivePusher_native_1pushAudio(JNIEnv *env, jobject instance, jbyteArray bytes_) {

    if (!audioChannel || !readyPushing) {
        return;
    }

    jbyte *bytes = env->GetByteArrayElements(bytes_, NULL);
    LOGI("pushAudioBegin");
    audioChannel->encodeData(bytes);
    env->ReleaseByteArrayElements(bytes_, bytes, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yeliang_LivePusher_native_1release(JNIEnv *env, jobject instance) {
    if (audioChannel) {
        delete audioChannel;
    }
    if (videoChannel) {
        delete videoChannel;
    }

}