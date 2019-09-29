package com.yeliang;

import android.Manifest;
import android.hardware.Camera;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.PermissionChecker;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceView;
import android.view.View;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {


    private LivePusher mLivePusher;
    private SurfaceView mSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        requestPermission();
        initView();
        initData();
    }

    private void requestPermission() {
        int CameraPermissionResult = PermissionChecker.checkSelfPermission(this, Manifest.permission.CAMERA);
        if (CameraPermissionResult == -1) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, 1000);
        }

        int AudioPermissionResult = PermissionChecker.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO);
        if (AudioPermissionResult == -1) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.RECORD_AUDIO}, 1000);
        }
    }

    private void initView() {
        findViewById(R.id.btn_start_live).setOnClickListener(this);
        findViewById(R.id.btn_stop_live).setOnClickListener(this);
        findViewById(R.id.btn_switch_camera).setOnClickListener(this);

        mSurfaceView = findViewById(R.id.surfaceView);
    }


    private void initData() {
        mLivePusher = new LivePusher(this, 800, 480, 800_000, 10, Camera.CameraInfo.CAMERA_FACING_BACK);
        mLivePusher.setPreviewDisplay(mSurfaceView.getHolder());
    }


    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_start_live:
                mLivePusher.startLive("rtmp://10.2.1.41:1935/live");
                break;
            case R.id.btn_stop_live:
                mLivePusher.stopLive();
                break;
            case R.id.btn_switch_camera:
                mLivePusher.switchCamera();
                break;
        }
    }
}
