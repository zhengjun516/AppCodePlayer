//
// Created by zhengjun on 2019-11-16.
//

#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include "AppCodeFFmpeg.h"
#include "constant.h"

AppCodeFFmpeg *appCodeFFmpeg = 0;
ANativeWindow* window;
JavaVM *javaVm = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm,void *reserved){
    javaVm = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_appcode_player_AppCodePlayer_getUrlProtocolInfo(JNIEnv *env, jobject thiz) {
    char info[40000] = {0};
   // av_register_all();

    struct URLProtocol *pup = NULL;

    struct URLProtocol **p_temp = (struct URLProtocol **) &pup;
    avio_enum_protocols((void **) p_temp, 0);

    while ((*p_temp) != NULL) {
        sprintf(info, "%sInput: %s\n", info, avio_enum_protocols((void **) p_temp, 0));
    }
    pup = NULL;
    avio_enum_protocols((void **) p_temp, 1);
    while ((*p_temp) != NULL) {
        sprintf(info, "%sInput: %s\n", info, avio_enum_protocols((void **) p_temp, 1));
    }
    LOGE("%s", info);
    return env->NewStringUTF(info);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_appcode_player_AppCodePlayer_getAVFormatInfo(JNIEnv *env, jobject thiz) {
    char info[40000] = {0};

    //av_register_all();

    AVInputFormat *if_temp =  (NULL);
    AVOutputFormat *of_temp = av_oformat_next(NULL);
    while (if_temp != NULL) {
        sprintf(info, "%sInput: %s\n", info, if_temp->name);
        if_temp = if_temp->next;
    }
    while (of_temp != NULL) {
        sprintf(info, "%sOutput: %s\n", info, of_temp->name);
        of_temp = of_temp->next;
    }
    LOGE("%s", info);
    return env->NewStringUTF(info);
}extern "C"
JNIEXPORT jstring JNICALL
Java_com_appcode_player_AppCodePlayer_getAVCodecInfo(JNIEnv *env, jobject thiz) {
    char info[40000] = {0};

   // av_register_all();

    AVCodec *c_temp = av_codec_next(NULL);

    while (c_temp != NULL) {
        if (c_temp->decode != NULL) {
            sprintf(info, "%sdecode:", info);
        } else {
            sprintf(info, "%sencode:", info);
        }
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s(video):", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s(audio):", info);
                break;
            case AVMEDIA_TYPE_SUBTITLE:
                sprintf(info, "%s(subtitle):", info);
                break;
            default:
                sprintf(info, "%s(other):", info);
                break;
        }
        sprintf(info, "%s[%10s]\n", info, c_temp->name);
        c_temp = c_temp->next;
    }
    LOGE("%s", info);
    return env->NewStringUTF(info);
}

//画画
void render(uint8_t *data, int lineSize,int w,int h){
    pthread_mutex_lock(&mutex);
    //LOGD("渲染");
    if(!window){
        pthread_mutex_unlock(&mutex);
        return;
    }
    ANativeWindow_setBuffersGeometry(window, w,
                                     h,
                                     WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }
    //填充rgb数据给dst_data
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    //stride:一个多少个数据
    int dst_lineSize = window_buffer.stride*4;
    //一行一行的拷贝
    for(int i =0;i<window_buffer.height;++i){
        memcpy(dst_data+i*dst_lineSize,data+i*lineSize,dst_lineSize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_appcode_player_AppCodePlayer_native_1prepare(JNIEnv *env, jobject thiz,
                                                      jstring data_source) {
    const char *dataSource = env->GetStringUTFChars(data_source,0);

    JavaCallHelper *javaCallHelper = new JavaCallHelper(javaVm,env,thiz);

    appCodeFFmpeg = new AppCodeFFmpeg(javaCallHelper,dataSource);
    appCodeFFmpeg->setRenderCallback(render);
    appCodeFFmpeg->prepare();

    env->ReleaseStringUTFChars(data_source,dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_appcode_player_AppCodePlayer_native_1setSurface(JNIEnv *env, jobject thiz,
                                                         jobject surface) {
    pthread_mutex_lock(&mutex);
    if(window){
        ANativeWindow_release(window);
        window = 0;
    }

    window = ANativeWindow_fromSurface(env,surface);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_appcode_player_AppCodePlayer_native_1start(JNIEnv *env, jobject thiz) {
    if(appCodeFFmpeg){
        appCodeFFmpeg->start();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_appcode_player_AppCodePlayer_native_1pause(JNIEnv *env, jobject thiz) {
    // TODO: implement native_pause()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_appcode_player_AppCodePlayer_native_1stop(JNIEnv *env, jobject thiz) {
    if(appCodeFFmpeg){
        appCodeFFmpeg->stop();
        DELETE(appCodeFFmpeg)
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_appcode_player_AppCodePlayer_native_1release(JNIEnv *env, jobject thiz) {
    pthread_mutex_lock(&mutex);
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_appcode_player_AppCodePlayer_native_1reset(JNIEnv *env, jobject thiz) {
    // TODO: implement native_reset()
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_appcode_player_AppCodePlayer_getDuration(JNIEnv *env, jobject thiz) {
    // TODO: implement getDuration()
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_appcode_player_AppCodePlayer_getCurrentPosition(JNIEnv *env, jobject thiz) {
    // TODO: implement getCurrentPosition()
}extern "C"
JNIEXPORT void JNICALL
Java_com_appcode_player_AppCodePlayer_seekTo(JNIEnv *env, jobject thiz, jlong mesc) {
    // TODO: implement seekTo()
}