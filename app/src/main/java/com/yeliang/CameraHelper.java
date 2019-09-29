package com.yeliang;

import android.app.Activity;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.io.IOException;
import java.util.Iterator;
import java.util.List;

/**
 * Author: yeliang
 * Date: 2019/9/20
 * Time: 4:51 PM
 * Description:
 */

public class CameraHelper implements SurfaceHolder.Callback, Camera.PreviewCallback {

    private Activity mActivity;
    private int mHeight;
    private int mWidth;
    private int mCameraId;
    private Camera mCamera;
    private byte[] buffer;
    private SurfaceHolder mSurfaceHolder;
    private Camera.PreviewCallback mPreviewCallBack;
    private int mRotation;
    private OnSizeChangeListener mOnSizeChangeListener;
    private byte[] bytes;


    public CameraHelper(Activity activity, int cameraId, int width, int height) {
        mActivity = activity;
        mCameraId = cameraId;
        mWidth = width;
        mHeight = height;

        Log.i("CameraHelper", "CameraHelper mWidth = " + mWidth + "mHeight = " + mHeight);
    }

    public void switchCamera() {
        if (mCameraId == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            mCameraId = Camera.CameraInfo.CAMERA_FACING_BACK;
        } else {
            mCameraId = Camera.CameraInfo.CAMERA_FACING_FRONT;
        }
        stopPreview();
        startPreview();
    }

    public void setPreviewCallBack(Camera.PreviewCallback previewCallBack) {
        mPreviewCallBack = previewCallBack;
    }

    public void setPreviewDisPlay(SurfaceHolder surfaceHolder) {
        mSurfaceHolder = surfaceHolder;
        mSurfaceHolder.addCallback(this);
    }

    public void setmOnSizeChangeListener(OnSizeChangeListener onSizeChangeListener) {
        mOnSizeChangeListener = onSizeChangeListener;
    }

    private void stopPreview() {
        if (mCamera != null) {
            //预览数据回调接口
            mCamera.setPreviewCallback(null);
            //停止预览
            mCamera.stopPreview();
            ;
            //释放摄像头
            mCamera.release();
            mCamera = null;
        }
    }

    private void startPreview() {

        try {
            mCamera = Camera.open(mCameraId);
            Camera.Parameters parameters = mCamera.getParameters();
            parameters.setPreviewFormat(ImageFormat.NV21);
            setPreviewSize(parameters);
            setPreviewOrientation();
            mCamera.setParameters(parameters);
            buffer = new byte[mWidth * mHeight * 3 / 2];
            bytes = new byte[buffer.length];
            mCamera.addCallbackBuffer(buffer);
            mCamera.setPreviewCallbackWithBuffer(this);
            mCamera.setPreviewDisplay(mSurfaceHolder);
            mCamera.startPreview();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void setPreviewSize(Camera.Parameters parameters) {
        List<Camera.Size> supportedPreviewSizes = parameters.getSupportedPreviewSizes();
        Camera.Size size = supportedPreviewSizes.get(0);

        int m = Math.abs(size.height * size.width - mWidth * mHeight);
        supportedPreviewSizes.remove(0);
        Iterator<Camera.Size> iterator = supportedPreviewSizes.iterator();

        while (iterator.hasNext()) {
            Camera.Size next = iterator.next();

            int n = Math.abs(next.height * next.width - mWidth * mHeight);
            if (n < m) {
                m = n;
                size = next;
            }
        }

        mWidth = size.width;
        mHeight = size.height;

        Log.i("CameraHelper", "setPreviewSize mWidth = " + mWidth + "mHeight = " + mHeight);
        parameters.setPreviewSize(mWidth, mHeight);
    }

    private void setPreviewOrientation() {
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(mCameraId, info);

        //这里获得的角度是系统默认的角度，手机不通默认的角度会有不通
        mRotation = mActivity.getWindowManager().getDefaultDisplay().getRotation();
        int degress = 0;
        switch (mRotation) {
            case Surface.ROTATION_0:
                degress = 0;
                mOnSizeChangeListener.onChanged(mHeight, mWidth);
                break;
            case Surface.ROTATION_90:
                degress = 90;
                mOnSizeChangeListener.onChanged(mHeight, mWidth);
                break;
            case Surface.ROTATION_180:
                degress = 180;
                mOnSizeChangeListener.onChanged(mHeight, mWidth);
                break;
            case Surface.ROTATION_270:
                degress = 270;
                mOnSizeChangeListener.onChanged(mHeight, mWidth);
                break;
            default:
                break;
        }

        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degress) % 360;
            result = (360 - result) % 360;
        } else {
            result = (info.orientation - degress + 360) % 360;
        }

        mCamera.setDisplayOrientation(result);
    }


    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        switch (mRotation) {
            case Surface.ROTATION_0:
                rotation90(data);
                break;
            case Surface.ROTATION_90:
                break;
            case Surface.ROTATION_180:
                break;
            case Surface.ROTATION_270:
                break;

        }
        mPreviewCallBack.onPreviewFrame(bytes, camera);
        camera.addCallbackBuffer(buffer);
    }

    private void rotation90(byte[] data) {
        int index = 0;
        int ySize = mWidth * mHeight;


        int uvHeight = mHeight / 2;
        if (mCameraId == Camera.CameraInfo.CAMERA_FACING_BACK) {

            //将y的数据旋转之后 放入新的byte数组
            for (int i = 0; i < mWidth; i++) {
                for (int j = mHeight - 1; j >= 0; j--) {
                    bytes[index++] = data[mWidth * j + i];
                }
            }

            for (int i = 0; i < mWidth; i += 2) {
                for (int j = uvHeight - 1; j >= 0; j--) {
                    //u
                    bytes[index++] = data[ySize + mWidth * j + i];

                    //v
                    bytes[index++] = data[ySize + mWidth * j + i + 1];
                }
            }
        } else {
            for (int i = 0; i < mWidth; i++) {
                int nPos = mWidth - 1;
                for (int j = 0; j < mHeight; j++) {
                    bytes[index++] = data[nPos - i];
                    nPos += mWidth;
                }
            }

            for (int i = 0; i < mWidth; i += 2) {
                int nPos = ySize + mWidth - 1;
                for (int j = 0; j < uvHeight; j++) {
                    bytes[index++] = data[nPos - i - 1];
                    bytes[index++] = data[nPos - i];
                    nPos += mWidth;
                }
            }
        }
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        stopPreview();
        startPreview();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        stopPreview();
    }

    public void release() {
        mSurfaceHolder.removeCallback(this);
        stopPreview();
    }

    public interface OnSizeChangeListener {
        void onChanged(int width, int height);
    }
}
