//
// Created by zhengjun on 2019-11-16.
//

#ifndef APPCODEPLAYER_APPCODEFFMPEG_H
#define APPCODEPLAYER_APPCODEFFMPEG_H


#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include <pthread.h>

extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

class AppCodeFFmpeg {
public:
    AppCodeFFmpeg(JavaCallHelper *javaCallHellper,const char* dataSource);
    ~AppCodeFFmpeg();
    void prepare();
    void _prepare();
    void start();
    void _start();

    void setRenderCallback(RenderFrameCallback renderFrameCallback);

private:
    char * dataSource = 0;
    pthread_t pid;
    pthread_t pid_play;
    AVFormatContext *formatContext = 0;
    JavaCallHelper *javaCallHellper = 0;
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    int isPlaying;

    RenderFrameCallback  renderFrameCallback;

};


#endif //APPCODEPLAYER_APPCODEFFMPEG_H
