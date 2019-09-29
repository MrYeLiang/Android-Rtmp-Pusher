package com.yeliang;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Author: yeliang
 * Date: 2019/9/20
 * Time: 4:38 PM
 * Description:
 */

public class AudioChannel {

    private int inputSamples;
    private ExecutorService executor;
    private AudioRecord audioRecord;
    private LivePusher mLivePusher;
    private int channels = 1;
    private boolean isLiving;

    public AudioChannel(LivePusher livePusher) {
        mLivePusher = livePusher;
        executor = Executors.newSingleThreadExecutor();
        int channelConfig;
        if (channels == 2) {
            channelConfig = AudioFormat.CHANNEL_IN_STEREO;
        } else {
            channelConfig = AudioFormat.CHANNEL_IN_MONO;
        }

        mLivePusher.native_setAudioInfo(44100, channels);
        inputSamples = mLivePusher.getInputSamples() * 2;

        int minBufferSize = AudioRecord.getMinBufferSize(44100, channelConfig, AudioFormat.ENCODING_PCM_16BIT) * 2;

        minBufferSize = minBufferSize > inputSamples ? minBufferSize : inputSamples;
        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, 44100, channelConfig, AudioFormat.ENCODING_PCM_16BIT, minBufferSize);
    }

    public void stopLive() {
        isLiving = false;
    }

    public void startLive() {
        isLiving = true;
        executor.submit(new AudioTask());
    }

    public void release() {
        audioRecord.release();
    }

    class AudioTask implements Runnable {

        @Override
        public void run() {

            audioRecord.startRecording();
            byte[] bytes = new byte[inputSamples];
            while (isLiving) {
                int len = audioRecord.read(bytes, 0, bytes.length);
                if (len > 0) {
                    mLivePusher.native_pushAudio(bytes);
                }

            }
            audioRecord.stop();
        }
    }
}
