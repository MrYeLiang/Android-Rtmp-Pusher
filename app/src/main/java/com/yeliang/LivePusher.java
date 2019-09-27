package com.yeliang;

import android.app.Activity;
import android.view.SurfaceHolder;

/**
 * Author: yeliang
 * Date: 2019/9/20
 * Time: 4:38 PM
 * Description:
 */

public class LivePusher {

    static {
        System.loadLibrary("native-lib");
    }

    private AudioChannel audioChannel;
    private VideoChannel videoChannel;

    public LivePusher(Activity activity, int width, int height, int bitrate,
                      int fps, int cameraId) {
        native_init();

        videoChannel = new VideoChannel(this, activity, width, height, bitrate, fps, cameraId);
        audioChannel = new AudioChannel();
    }

    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        videoChannel.setPreviewDisplay(surfaceHolder);
    }

    public void switchCamera() {
        videoChannel.switchCamera();
    }

    public void startLive(String path) {
        native_start(path);
        videoChannel.startLive();
        audioChannel.startLive();
    }



    public void stopLive() {
        videoChannel.stopLive();
        audioChannel.stopLive();
        native_stop();
    }

    private native void native_init();

    public native void native_setVideoEncInfo(int width, int height, int mFps, int mBitrate);

    private native void native_start(String path);

    public native void native_push_video(byte[] data);

    private native void native_stop();
}
