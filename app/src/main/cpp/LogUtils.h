//
// Created by 叶亮 on 2019/9/17.
//

#ifndef ANDROID_RTMP_PUSHER_LOGUTILS_H
#define ANDROID_RTMP_PUSHER_LOGUTILS_H

#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "RTMP-PUSHER", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RTMP-PUSHER", __VA_ARGS__)
#endif //ANDROID_RTMP_PUSHER_LOGUTILS_H
