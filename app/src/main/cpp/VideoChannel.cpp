//
// Created by zhengjun on 2019-11-17.
//

#include "VideoChannel.h"

extern "C"{
#include <libavutil/imgutils.h>
}

VideoChannel::VideoChannel(int id,AVCodecContext *codecContext) :BaseChannel(id,codecContext){

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
    packets.setWork(1);
    frames.setWork(1);
    isPlaying = true;
    pthread_create(&pid_decode,0,decode_task,this);

    //播放
    pthread_create(&pid_render,0,render_task,this);

}

void VideoChannel::decode() {
    AVPacket* packet =0;
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

        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            //失败
            break;
        }

        //代表了一个图像
        AVFrame* avFrame = av_frame_alloc();
       //从解码器中读取解码后的数据包AVFrame;
       ret = avcodec_receive_frame(codecContext,avFrame);
       if(ret == AVERROR(EAGAIN)){
           continue;
       }else if(ret < 0){
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
        renderFrameCallback(dst_data[0],dst_lineSize[0],codecContext->width,codecContext->height);
        releaseAVFrame(frame);
    }

    av_freep(&dst_data[0]);
    isPlaying = false;
    releaseAVFrame(frame);
    sws_freeContext(swsContext);


}

void VideoChannel::setRenderFrameCallback(RenderFrameCallback renderFrameCallback) {
    this->renderFrameCallback = renderFrameCallback;
}