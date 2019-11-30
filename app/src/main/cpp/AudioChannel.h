//
// Created by zhengjun on 2019-11-17.
//

#ifndef APPCODEPLAYER_AUDIOCHANNEL_H
#define APPCODEPLAYER_AUDIOCHANNEL_H


#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "BaseChannel.h"

extern "C"{
#include <libswresample/swresample.h>
};

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id,AVCodecContext *codecContext,AVRational timeBase);
    ~AudioChannel();
    void play();
    void stop();
    void decode();
    void initOpenSL();
    void releaseOpenSL();
    int getPcm();
    void stopWork();
    void startWork();

private:
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;

    /*
     * OpenSL ES
     */
    SLObjectItf engineObject = 0;
    SLEngineItf engineInterface = 0;

    //混音器
    SLObjectItf outputMixObject = 0;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = 0;

    //
    SLObjectItf bqPlayerObject = 0;
    SLPlayItf bqPlayerPlay = 0;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = 0;

    SwrContext *swrContext = 0;

    int out_channels;
    int out_samplesize;
    int out_sample_rate;

public:
    uint8_t *data;
};


#endif //APPCODEPLAYER_AUDIOCHANNEL_H
