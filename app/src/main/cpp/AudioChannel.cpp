//
// Created by zhengjun on 2019-11-17.
//

#include "AudioChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
extern "C"{
#include <libswresample/swresample.h>
}

AudioChannel::AudioChannel(int id,AVCodecContext *codecContext,AVRational timeBase) :BaseChannel(id,codecContext,timeBase){

    //根据布局获取声道数
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_samplesize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;

    data = static_cast<uint8_t *>(malloc(out_sample_rate * out_samplesize * out_channels));
    memset(data,0,out_sample_rate * out_samplesize * out_channels);
}

AudioChannel::~AudioChannel() {
    if(data){
        free(data);
        data = 0;
    }

}

void* task_audio_decode(void *args){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->decode();
    return 0;
}

void* task_audio_render(void *args){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->render();
    return 0;

}

void AudioChannel::play() {
    startWork();

    swrContext = swr_alloc_set_opts(0,AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,out_sample_rate,
            codecContext->channel_layout,codecContext->sample_fmt,
            codecContext->sample_rate,0,0);
    swr_init(swrContext);

    isPlaying = 1;


    //1.解码
    pthread_create(&pid_audio_decode,0,task_audio_decode,this);
    //2.播放
    pthread_create(&pid_audio_play,0,task_audio_render,this);


}

void AudioChannel::decode() {
    AVPacket *packet =0;
    while(isPlaying){
        int ret = packets.pop(packet);
        LOGD("解码");
        if(!isPlaying){
            break;
        }
        if (!ret){
            continue;
        }
        //把包丢给解码器
        ret = avcodec_send_packet(codecContext,packet);
        releaseAVPacket(packet);

        if (ret != 0) {
            //失败
            break;
        }

        //代表了一个图像
        AVFrame* avFrame = av_frame_alloc();
        //从解码器中读取解码后的数据包AVFrame;
        ret = avcodec_receive_frame(codecContext,avFrame);
        if(ret == AVERROR(EAGAIN)){
            continue;
        }else if(ret != 0){
            break;
        }

        frames.push(avFrame);
    }
    releaseAVPacket(packet);
}

void AudioChannel::startWork() {
    packets.setWork(1);
    frames.setWork(1);
}

void AudioChannel::stopWork() {
    packets.setWork(0);
    frames.setWork(0);
}

//第一次主动调用在调用线程
//之后在新线程中回调
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);
    int dataSize = audioChannel->getPcm();
    if (dataSize > 0) {
        (*bq)->Enqueue(bq, audioChannel->data, dataSize);
    }
}

int AudioChannel::getPcm() {
    int data_size = 0;

    AVFrame *frame;
    int ret = frames.pop(frame);
    if(!isPlaying){
        if(ret){
            releaseAVFrame(frame);
        }
        return data_size;
    }
    if(ret < 0){
        return data_size;
    }

    //48000HZ 8位 => 44100 16位
    //重采样

    int64_t delays = swr_get_delay(swrContext,frame->sample_rate);

    int64_t  max_samples = av_rescale_rnd(delays+frame->nb_samples,out_sample_rate,frame->sample_rate,AV_ROUND_UP);

    int samples = swr_convert(swrContext, &data, max_samples, (const uint8_t **)(frame->data), frame->nb_samples);

    data_size = samples * out_samplesize * out_channels;

    //获取frame的一个相对播放时间
    //获得相对播放这一段数据的秒数
    clock = frame->pts * av_q2d(timeBase);

    return data_size;
}



void AudioChannel::render() {
    //创建引擎
    SLresult result;
    // 创建引擎engineObject
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // 初始化引擎engineObject
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // 获取引擎接口engineEngine
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE,
                                           &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // 创建混音器outputMixObject
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0,
                                              0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // 初始化混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
    if (SL_RESULT_SUCCESS == result) {
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &settings);
    }

    //创建队列
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};

    //数据源 将上述配置信息放到这个数据源中
    SLDataSource slDataSource = {&android_queue, &pcm};
    //设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    //开启功能
    SLDataSink audioSnk = {&outputMix, NULL};
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    //创建播放器
    (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &slDataSource,
                                       &audioSnk, 1,
                                       ids, req);

    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);


//    得到接口后调用  获取Player接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);

    //    注册回调缓冲区 //获取缓冲队列接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                    &bqPlayerBufferQueue);

    //缓冲接口回调
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    //    获取播放状态接口
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    bqPlayerCallback(bqPlayerBufferQueue, this);
}