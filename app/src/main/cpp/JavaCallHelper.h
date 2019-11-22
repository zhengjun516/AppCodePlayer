//
// Created by zhengjun on 2019-11-16.
//

#ifndef APPCODEPLAYER_JAVACALLHELPER_H
#define APPCODEPLAYER_JAVACALLHELPER_H

#include <jni.h>


class JavaCallHelper {
public:
    JavaCallHelper(JavaVM *vm,JNIEnv* env,jobject instance);
    ~JavaCallHelper();
    //回调java
    void onError(int thread,int errorCode);
    void onPrepared(int thread);
private:
    JavaVM *vm;
    JNIEnv *env;
    jobject instance;
    jmethodID onErrorId;
    jmethodID onPreparedId;
};


#endif //APPCODEPLAYER_JAVACALLHELPER_H
