//
// Created by zhengjun on 2019-11-16.
//

#include "JavaCallHelper.h"
#include "constant.h"

JavaCallHelper::JavaCallHelper(JavaVM *vm, JNIEnv *env, jobject instance) {
    this->vm  = vm;
    this->env = env;
    //一旦涉及到jobject跨方法和线程，就需要创建全局引用
    this->instance = env->NewGlobalRef(instance);

    jclass clazz = env->GetObjectClass(instance);
    onErrorId = env->GetMethodID(clazz,"onError","(I)V");
    onPreparedId = env->GetMethodID(clazz,"onPrepared","()V");
}
JavaCallHelper::~JavaCallHelper() {
    env->DeleteGlobalRef(instance);
    LOGD("调用JavaCallHelper析构");
}

void JavaCallHelper::onError(int thread, int errorCode) {
    //主线程
    if(thread == THREAD_MAIN){
        env->CallVoidMethod(instance,onErrorId,errorCode);
    }else{
        JNIEnv *env;
        vm->AttachCurrentThread(&env,0);
        env->CallVoidMethod(instance,onErrorId,errorCode);
        vm->DetachCurrentThread();
    }
}

void JavaCallHelper::onPrepared(int thread) {
    if(thread == THREAD_MAIN){
        env->CallVoidMethod(instance,onPreparedId);
    }else{
        JNIEnv *env;
        vm->AttachCurrentThread(&env,0);
        env->CallVoidMethod(instance,onPreparedId);
        vm->DetachCurrentThread();
    }
}
