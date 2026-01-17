#include "AssetInitialization.h"
#include "jni/AssetJNIBridge.h"
#include "../../java/jniutil.h"
#include "../../log.h"

namespace samp {
namespace platform {
namespace storage {

AssetInitialization& AssetInitialization::Get() {
    static AssetInitialization instance;
    return instance;
}

bool AssetInitialization::InitializeAssets(JNIEnv* env, jobject activity) {
    if (!env || !activity) {
        LOGE("AssetInitialization::InitializeAssets: Invalid parameters");
        return false;
    }
    
    if (IsInitialized(env, activity)) {
        FLog("AssetInitialization::InitializeAssets: Assets already initialized");
        return true;
    }
    
    try {
        jclass initializationServiceClass = env->FindClass("com/samp/mobile/platform/storage/AssetInitializationService");
        if (!initializationServiceClass) {
            EXCEPTION_CHECK(env);
            return false;
        }
        
        jmethodID initializeMethod = env->GetStaticMethodID(initializationServiceClass, "initializeAssets", 
                                                           "(Landroid/content/Context;)Z");
        if (!initializeMethod) {
            EXCEPTION_CHECK(env);
            env->DeleteLocalRef(initializationServiceClass);
            return false;
        }
        
        jboolean result = env->CallStaticBooleanMethod(initializationServiceClass, initializeMethod, activity);
        EXCEPTION_CHECK(env);
        env->DeleteLocalRef(initializationServiceClass);
        
        return result == JNI_TRUE;
    } catch (...) {
        LOGE("AssetInitialization::InitializeAssets: Exception occurred");
        return false;
    }
}

bool AssetInitialization::IsInitialized(JNIEnv* env, jobject activity) const {
    if (!env || !activity) {
        return false;
    }
    
    try {
        jclass initializationServiceClass = env->FindClass("com/samp/mobile/platform/storage/AssetInitializationService");
        if (!initializationServiceClass) {
            EXCEPTION_CHECK(env);
            return false;
        }
        
        jmethodID isInitializedMethod = env->GetStaticMethodID(initializationServiceClass, "isInitialized", 
                                                               "(Landroid/content/Context;)Z");
        if (!isInitializedMethod) {
            EXCEPTION_CHECK(env);
            env->DeleteLocalRef(initializationServiceClass);
            return false;
        }
        
        jboolean result = env->CallStaticBooleanMethod(initializationServiceClass, isInitializedMethod, activity);
        EXCEPTION_CHECK(env);
        env->DeleteLocalRef(initializationServiceClass);
        
        return result == JNI_TRUE;
    } catch (...) {
        LOGE("AssetInitialization::IsInitialized: Exception occurred");
        return false;
    }
}

} // namespace storage
} // namespace platform
} // namespace samp
