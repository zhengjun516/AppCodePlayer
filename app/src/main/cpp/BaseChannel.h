//
// Created by zhengjun on 2019-11-21.
//

#ifndef APPCODEPLAYER_BASECHANNEL_H
#define APPCODEPLAYER_BASECHANNEL_H

extern "C"{
#include <libavcodec/avcodec.h>
};



#include "SafeQueue.h"

class BaseChannel {
public:
    BaseChannel(int i,AVCodecContext *codecContext);
    virtual ~BaseChannel();
    static void releaseAVPacket(AVPacket*& packet);
    static void releaseAVFrame(AVFrame*& frame);
    //纯虚方法，相当于抽象方法
    virtual void play() = 0;

    int id;
    SafeQueue<AVPacket*> packets;
    SafeQueue<AVFrame*> frames;
    bool  isPlaying;
    AVCodecContext* codecContext = 0;
};


#endif //APPCODEPLAYER_BASECHANNEL_H
