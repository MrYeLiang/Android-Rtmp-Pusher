package com.yeliang;

import android.app.Activity;
import android.hardware.Camera;
import android.view.SurfaceHolder;

/**
 * Author: yeliang
 * Date: 2019/9/20
 * Time: 4:39 PM
 * Description:
 */

public class VideoChannel implements Camera.PreviewCallback, CameraHelper.OnSizeChangeListener {
    private LivePusher mLivePusher;
    private CameraHelper mCameraHelper;
    private int mBitrate;
    private int mFps;
    private boolean isLiving;

    public VideoChannel(LivePusher livePusher, Activity activity, int width, int height, int bitrate, int fps, int cameraId) {
        mLivePusher = livePusher;
        mBitrate = bitrate;
        mFps = fps;
        mCameraHelper = new CameraHelper(activity, cameraId, width, height);
        mCameraHelper.setPreviewCallBack(this);
        mCameraHelper.setmOnSizeChangeListener(this);
    }


    public void startLive() {
        isLiving = true;
    }

    public void stopLive() {
        isLiving = false;
    }

    public void setPreviewDisplay(SurfaceHolder surfaceHolder){
        mCameraHelper.setPreviewDisPlay(surfaceHolder);
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        if (isLiving) {
            mLivePusher.native_push_video(data);
        }
    }

    @Override
    public void onChanged(int width, int height) {
        mLivePusher.native_setVideoEncInfo(width, height, mFps, mBitrate);
    }

    public void switchCamera() {
       mCameraHelper.switchCamera();
    }
}
