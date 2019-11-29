//
// Created by zhengjun on 2019-11-16.
//

#include <cstring>

#include "AppCodeFFmpeg.h"
#include "constant.h"


AppCodeFFmpeg::AppCodeFFmpeg(JavaCallHelper *javaCallHellper,const char *dataSource) {
    this->javaCallHellper = javaCallHellper;
    //strlen获取的是字符串的长度，不包括\0;
    this->dataSource = new char[strlen(dataSource)+1];
    isPlaying = 0;
    strcpy(this->dataSource,dataSource);
}

AppCodeFFmpeg::~AppCodeFFmpeg() {

    delete dataSource;
    dataSource = 0;

}

void* task_prepare(void *args){
    AppCodeFFmpeg *appCodeFFmpeg = static_cast<AppCodeFFmpeg *>(args);
    appCodeFFmpeg->_prepare();
    return 0;
}

void AppCodeFFmpeg::prepare() {
  pthread_create(&pid,0,task_prepare,this);
}

void AppCodeFFmpeg::_prepare() {

    LOGE("视频地址：%s",dataSource);

    //初始化网络
    avformat_network_init();

    //1.打开媒体文件
    // 代表一个 视频/音频 包含了视频、音频的各种信息
    formatContext = avformat_alloc_context();

    AVDictionary* options = 0;

    av_dict_set(&options,"timeout","5000000",0);

    int ret = avformat_open_input(&formatContext,dataSource,0,&options);
    //ret不为0表示打开失败
    if(ret != 0){
        LOGE("打开媒体失败 %s",av_err2str(ret));
        javaCallHellper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }

    //2.查找媒体中的音视频流
    ret = avformat_find_stream_info(formatContext,0);
    if(ret < 0){
        LOGE("查找流失败",av_err2str(ret));
        javaCallHellper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }

    //
    for (int i=0;i<formatContext->nb_streams;++i){
        //可能是视频也可能是音频
        AVStream *stream = formatContext->streams[i];

        //包含了解码这段流的各种参数
        AVCodecParameters *codecParameters = stream->codecpar;

        //无论视频还是音频都需要干的一些事情（获得解码器）
        //1.通过当前流的使用的编码方式，查找解码器
        AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if(codec == NULL){
            LOGE("查找解码器失败",av_err2str(ret));
            javaCallHellper->onError(THREAD_CHILD,FFMPEG_FIND_DECODER_FAIL);
            return;
        }

        //2,获得解码器上下文
        AVCodecContext *codecContext  = avcodec_alloc_context3(codec);
        if(codecContext == NULL){
            LOGE("创建解码器上下文失败");
            javaCallHellper->onError(THREAD_CHILD,FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }
        //3,设置上下文内的一些参数
        ret = avcodec_parameters_to_context(codecContext,codecParameters);
        if(ret < 0){
            LOGE("设置解码器上下文参数失败",av_err2str(ret));
            javaCallHellper->onError(THREAD_CHILD,FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }

        //4,打开解码器
        ret = avcodec_open2(codecContext,codec,0);

        if(ret < 0){
            LOGE("打开解码器失败",av_err2str(ret));
            javaCallHellper->onError(THREAD_CHILD,FFMPEG_OPEN_DECODER_FAIL);
            return;
        }

       AVRational timeBase = stream->time_base;

        if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO){
             stream->time_base;
             audioChannel = new AudioChannel(i,codecContext,timeBase);
        }else if(codecParameters->codec_type == AVMEDIA_TYPE_VIDEO){
             //帧率:单位时间内，需要显示多少张图像
             AVRational avRational = stream->avg_frame_rate;
             int fps = av_q2d(avRational);

             videoChannel = new VideoChannel(i,codecContext,fps,timeBase);
             videoChannel->setRenderFrameCallback(renderFrameCallback);
        }

        if(!audioChannel && !videoChannel){
            LOGE("没有媒体文件");
            javaCallHellper->onError(THREAD_CHILD,FFMPEG_NOMEDIA);
            return;
        }
    }
    //准备完了，通知java随时可以
    javaCallHellper->onPrepared(THREAD_CHILD);

}

void* task_start(void* args){
    AppCodeFFmpeg *appCodeFFmpeg = static_cast<AppCodeFFmpeg *>(args);
    appCodeFFmpeg->_start();
    return  0;
}

void AppCodeFFmpeg::start() {
    //标记正在播放
    isPlaying = 1;

    if(audioChannel){
        audioChannel->play();
    }

    if(videoChannel){
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->play();
    }


    pthread_create(&pid_play,0,task_start,this);

}

//只负责读取数据包
void AppCodeFFmpeg::_start() {
    //1.读取媒体数据包(音视频数据包)
    int ret;
    while(isPlaying){
        //生成的内存在堆当中
        AVPacket *packet = av_packet_alloc();
        ret = av_read_frame(formatContext,packet);
        if(ret == 0){
            if(packet->stream_index == audioChannel->id){
                audioChannel->packets.push(packet);
            }else if(packet->stream_index == videoChannel->id){
                //LOGD("将Packet放入队列中");
                videoChannel->packets.push(packet);
            }

        }else if(ret == AVERROR_EOF){
            //读取完成，但可能还没播放完成

            while (isPlaying) {
                if (videoChannel->packets.empty()) {
                    break;
                }
                //等待10毫秒再次检查
                av_usleep(10 * 1000);
            }
            break;
        }else{
            break;
        }
    }
    isPlaying = 0;
}

void AppCodeFFmpeg::stop() {

}

void AppCodeFFmpeg::setRenderCallback(RenderFrameCallback renderFrameCallback) {
    this->renderFrameCallback = renderFrameCallback;
}

