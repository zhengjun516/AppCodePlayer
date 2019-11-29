//
// Created by zhengjun on 2019-11-17.
//


#include "VideoChannel.h"

extern "C"{
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

void dropAvPacket(queue<AVPacket *> &queue){
    while(!queue.empty()){
        AVPacket *packet = queue.front();
        if(packet->flags != AV_PKT_FLAG_KEY){
            BaseChannel::releaseAVPacket(packet);
            queue.pop();
        }else{
            break;
        }
    }
}

void dropAvFrame(queue<AVFrame *> &queue){
    if(!queue.empty()){
       AVFrame* frame = queue.front();
       BaseChannel::releaseAVFrame(frame);
       queue.pop();
    }

}

VideoChannel::VideoChannel(int id,AVCodecContext *codecContext,int fps,AVRational timeBase) :BaseChannel(id,codecContext,timeBase){
    this->fps = fps;

    packets.setSyncHandle(dropAvPacket);

}



VideoChannel::~VideoChannel() {
    frames.clear();
}

void* decode_task(void* args){
    VideoChannel* videoChannel = static_cast<VideoChannel*>(args);
    videoChannel->decode();
    return 0;
}

void* render_task(void* args){
    VideoChannel* videoChannel = static_cast<VideoChannel*>(args);
    videoChannel->render();
    return 0;
}

//解码+播放
void VideoChannel::play() {
    //解码
    startWork();
    isPlaying = true;
    pthread_create(&pid_decode,0,decode_task,this);

    //播放
    pthread_create(&pid_render,0,render_task,this);

}

void VideoChannel::decode() {
    AVPacket* packet =0;
    while(isPlaying){
        int ret = packets.pop(packet);
        //LOGD("解码");
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

void VideoChannel::render() {
    //转换成RGBA
    swsContext = sws_getContext(
            codecContext->width,codecContext->height,codecContext->pix_fmt,
            codecContext->width,codecContext->height,AV_PIX_FMT_RGBA,
            SWS_BILINEAR,0,0,0);
    double frame_delays = 1.0/fps;
    int  microsecond = 1000000;
    //指针数组
    uint8_t *dst_data[4];
    int dst_lineSize[4];
    av_image_alloc(dst_data, dst_lineSize,
                   codecContext->width, codecContext->height, AV_PIX_FMT_RGBA, 1);
    AVFrame* frame = 0;
    while(isPlaying){
        int ret = frames.pop(frame);

        if(!isPlaying){
            break;
        }
        sws_scale(swsContext,reinterpret_cast<const uint8_t *const *>(frame->data),
                frame->linesize,0,
                codecContext->height,
                dst_data,
                dst_lineSize
                );

        //获得当前画面播放的一个相对时间
        double clock = frame->best_effort_timestamp*av_q2d(timeBase);

        int  extra_delay = frame->repeat_pict / (2*fps);

        int delays = frame_delays+extra_delay;
        if(!audioChannel){
            av_usleep(delays*microsecond);
        }else{
            if(clock == 0){
                av_usleep(delays* microsecond);
            }else{
                double audioClock = audioChannel->clock;
                //音视频时间相差时间间隔
                double diff = clock-audioClock;
                //大于0表示视频比较快
                //小于0表示音频比较快
                if(diff > 0){
                    LOGD("视频快了：%lf",diff);
                    av_usleep((delays+diff) * microsecond);
                }else{
                    LOGD("音频快了：%lf",diff);
                    if(fabs(diff) > 0.05 ){
                        releaseAVFrame(frame);
                        //丢包
                        //packets.sync();
                        frames.sync();
                        continue;
                    }
                }
            }
        }

        renderFrameCallback(dst_data[0],dst_lineSize[0],codecContext->width,codecContext->height);
        releaseAVFrame(frame);
    }

    av_freep(&dst_data[0]);
    isPlaying = false;
    releaseAVFrame(frame);
    sws_freeContext(swsContext);


}

void VideoChannel::startWork() {
    packets.setWork(1);
    frames.setWork(1);
}

void VideoChannel::stopWork() {
    packets.setWork(0);
    frames.setWork(0);
}


void VideoChannel::setRenderFrameCallback(RenderFrameCallback renderFrameCallback) {
    this->renderFrameCallback = renderFrameCallback;
}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel = audioChannel;
}