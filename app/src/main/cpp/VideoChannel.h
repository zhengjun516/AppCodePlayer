//
// Created by zhengjun on 2019-11-17.
//

#ifndef APPCODEPLAYER_VIDEOCHANNEL_H
#define APPCODEPLAYER_VIDEOCHANNEL_H


#include "BaseChannel.h"
#include "constant.h"

extern "C"{
#include <libswscale/swscale.h>
};

typedef void(*RenderFrameCallback)(uint8_t *, int,int,int);

class VideoChannel :public BaseChannel {
public:
    VideoChannel(int id,AVCodecContext *codecContext);
    ~VideoChannel();
    void play();
    void decode();
    void render();
    void setRenderFrameCallback(RenderFrameCallback renderFrameCallback);

private:
    pthread_t pid_decode;
    pthread_t pid_render;
    SwsContext* swsContext = 0;
    RenderFrameCallback renderFrameCallback;
};


#endif //APPCODEPLAYER_VIDEOCHANNEL_H
