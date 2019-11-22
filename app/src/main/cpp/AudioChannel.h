//
// Created by zhengjun on 2019-11-17.
//

#ifndef APPCODEPLAYER_AUDIOCHANNEL_H
#define APPCODEPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id,AVCodecContext *codecContext);
    void play();
};


#endif //APPCODEPLAYER_AUDIOCHANNEL_H
