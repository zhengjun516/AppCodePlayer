//
// Created by zhengjun on 2019-11-21.
//

#include "BaseChannel.h"

BaseChannel::BaseChannel(int id,AVCodecContext *codecContext):id(id),codecContext(codecContext) {

    packets.setReleaseCallback(BaseChannel::releaseAVPacket);
    frames.setReleaseCallback(BaseChannel::releaseAVFrame);

}

BaseChannel::~BaseChannel() {
    packets.clear();
    frames.clear();
}

//释放AVPacket
void BaseChannel::releaseAVPacket(AVPacket *&packet){
    if (packet){
        av_packet_free(&packet);
        packet = 0;
    }
}

void BaseChannel::releaseAVFrame(AVFrame *&frame) {
    if(frame){
        av_frame_free(&frame);
        frame = 0;
    }

}
