#include "APKAssetProvider.h"
#include "../Asset.h"
#include "../jni/AssetJNIBridge.h"
#include "../../java/jniutil.h"
#include "../../log.h"
#include <algorithm>
#include <cstring>

namespace samp {
namespace platform {
namespace storage {

APKAssetProvider::APKAssetProvider(JavaVM* javaVM, jobject activity)
    : m_javaVM(javaVM)
    , m_activityObject(activity)
{
    if (!m_javaVM || !m_activityObject) {
        LOGE("APKAssetProvider: Invalid parameters");
    }
}

std::unique_ptr<Asset> APKAssetProvider::Load(const std::string& path) {
    if (!ValidatePath(path)) {
        LOGE("APKAssetProvider::Load: Invalid path: %s", path.c_str());
        return nullptr;
    }
    
    if (!Exists(path)) {
        return nullptr;
    }
    
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("APKAssetProvider::Load: GetJNIEnvSafe failed");
        return nullptr;
    }
    
    std::unique_ptr<Asset> result;
    
    try {
        jclass storageManagerClass = env->FindClass("com/samp/mobile/platform/storage/AssetStorageManager");
        if (!storageManagerClass) {
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return nullptr;
        }
        
        jmethodID getInstanceMethod = env->GetStaticMethodID(storageManagerClass, "getInstance",
                                                             "(Landroid/content/Context;)Lcom/samp/mobile/platform/storage/AssetStorageManager;");
        if (!getInstanceMethod) {
            EXCEPTION_CHECK(env);
            env->DeleteLocalRef(storageManagerClass);
            DETACH_IF_NEEDED(needsDetach);
            return nullptr;
        }
        
        jobject storageManagerObj = env->CallStaticObjectMethod(storageManagerClass, getInstanceMethod, m_activityObject);
        EXCEPTION_CHECK(env);
        env->DeleteLocalRef(storageManagerClass);
        
        if (!storageManagerObj) {
            DETACH_IF_NEEDED(needsDetach);
            return nullptr;
        }
        
        jmethodID readAssetMethod = env->GetMethodID(env->GetObjectClass(storageManagerObj), "readAsset", "(Ljava/lang/String;)[B");
        if (!readAssetMethod) {
            env->DeleteLocalRef(storageManagerObj);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return nullptr;
        }
        
        jstring jPath = env->NewStringUTF(path.c_str());
        if (!jPath) {
            env->DeleteLocalRef(storageManagerObj);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return nullptr;
        }
        
        jbyteArray jData = static_cast<jbyteArray>(env->CallObjectMethod(storageManagerObj, readAssetMethod, jPath));
        EXCEPTION_CHECK(env);
        env->DeleteLocalRef(jPath);
        
        if (!jData) {
            env->DeleteLocalRef(storageManagerObj);
            DETACH_IF_NEEDED(needsDetach);
            return nullptr;
        }
        
        jsize dataSize = env->GetArrayLength(jData);
        if (dataSize <= 0) {
            env->DeleteLocalRef(jData);
            env->DeleteLocalRef(storageManagerObj);
            DETACH_IF_NEEDED(needsDetach);
            return nullptr;
        }
        
        std::unique_ptr<uint8_t[]> data(new uint8_t[dataSize]);
        env->GetByteArrayRegion(jData, 0, dataSize, reinterpret_cast<jbyte*>(data.get()));
        EXCEPTION_CHECK(env);
        
        env->DeleteLocalRef(jData);
        env->DeleteLocalRef(storageManagerObj);
        
        result = std::make_unique<FileAsset>(path, std::move(data), static_cast<size_t>(dataSize), AssetProviderType::APK);
    } catch (...) {
        LOGE("APKAssetProvider::Load: Exception occurred");
    }
    
    DETACH_IF_NEEDED(needsDetach);
    return result;
}

bool APKAssetProvider::Exists(const std::string& path) const {
    if (!ValidatePath(path)) {
        return false;
    }
    
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("APKAssetProvider::Exists: GetJNIEnvSafe failed");
        return false;
    }
    
    bool result = false;
    
    try {
        jclass storageManagerClass = env->FindClass("com/samp/mobile/platform/storage/AssetStorageManager");
        if (!storageManagerClass) {
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return false;
        }
        
        jmethodID getInstanceMethod = env->GetStaticMethodID(storageManagerClass, "getInstance",
                                                             "(Landroid/content/Context;)Lcom/samp/mobile/platform/storage/AssetStorageManager;");
        if (!getInstanceMethod) {
            EXCEPTION_CHECK(env);
            env->DeleteLocalRef(storageManagerClass);
            DETACH_IF_NEEDED(needsDetach);
            return false;
        }
        
        jobject storageManagerObj = env->CallStaticObjectMethod(storageManagerClass, getInstanceMethod, m_activityObject);
        EXCEPTION_CHECK(env);
        env->DeleteLocalRef(storageManagerClass);
        
        if (!storageManagerObj) {
            DETACH_IF_NEEDED(needsDetach);
            return false;
        }
        
        jmethodID assetExistsMethod = env->GetMethodID(env->GetObjectClass(storageManagerObj), "assetExists", "(Ljava/lang/String;)Z");
        if (!assetExistsMethod) {
            env->DeleteLocalRef(storageManagerObj);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return false;
        }
        
        jstring jPath = env->NewStringUTF(path.c_str());
        if (!jPath) {
            env->DeleteLocalRef(storageManagerObj);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return false;
        }
        
        jboolean exists = env->CallBooleanMethod(storageManagerObj, assetExistsMethod, jPath);
        EXCEPTION_CHECK(env);
        
        env->DeleteLocalRef(jPath);
        env->DeleteLocalRef(storageManagerObj);
        
        result = (exists == JNI_TRUE);
    } catch (...) {
        LOGE("APKAssetProvider::Exists: Exception occurred");
    }
    
    DETACH_IF_NEEDED(needsDetach);
    return result;
}

std::vector<std::string> APKAssetProvider::List(const std::string& directory) const {
    std::vector<std::string> result;
    
    if (!ValidatePath(directory)) {
        return result;
    }
    
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("APKAssetProvider::List: GetJNIEnvSafe failed");
        return result;
    }
    
    try {
        jclass assetManagerClass = env->FindClass("android/content/res/AssetManager");
        if (!assetManagerClass) {
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jmethodID listMethod = env->GetMethodID(assetManagerClass, "list", "(Ljava/lang/String;)[Ljava/lang/String;");
        if (!listMethod) {
            env->DeleteLocalRef(assetManagerClass);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jclass contextClass = env->FindClass("android/content/Context");
        if (!contextClass) {
            env->DeleteLocalRef(assetManagerClass);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jmethodID getAssetsMethod = env->GetMethodID(contextClass, "getAssets", "()Landroid/content/res/AssetManager;");
        if (!getAssetsMethod) {
            env->DeleteLocalRef(assetManagerClass);
            env->DeleteLocalRef(contextClass);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jobject assetManager = env->CallObjectMethod(m_activityObject, getAssetsMethod);
        EXCEPTION_CHECK(env);
        env->DeleteLocalRef(contextClass);
        
        if (!assetManager) {
            env->DeleteLocalRef(assetManagerClass);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jstring jDir = env->NewStringUTF(directory.c_str());
        if (!jDir) {
            env->DeleteLocalRef(assetManager);
            env->DeleteLocalRef(assetManagerClass);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jobjectArray filesArray = static_cast<jobjectArray>(env->CallObjectMethod(assetManager, listMethod, jDir));
        EXCEPTION_CHECK(env);
        env->DeleteLocalRef(jDir);
        env->DeleteLocalRef(assetManager);
        env->DeleteLocalRef(assetManagerClass);
        
        if (filesArray) {
            jsize length = env->GetArrayLength(filesArray);
            for (jsize i = 0; i < length; ++i) {
                jstring name = static_cast<jstring>(env->GetObjectArrayElement(filesArray, i));
                if (name) {
                    const char* utf8 = env->GetStringUTFChars(name, nullptr);
                    if (utf8) {
                        result.push_back(std::string(utf8));
                        env->ReleaseStringUTFChars(name, utf8);
                    }
                    env->DeleteLocalRef(name);
                }
            }
            env->DeleteLocalRef(filesArray);
        }
    } catch (...) {
        LOGE("APKAssetProvider::List: Exception occurred");
    }
    
    DETACH_IF_NEEDED(needsDetach);
    return result;
}

bool APKAssetProvider::ValidatePath(const std::string& path) const {
    if (path.empty()) {
        return false;
    }
    
    if (path.find("../") != std::string::npos || path.find("..\\") != std::string::npos) {
        return false;
    }
    
    return true;
}

} // namespace storage
} // namespace platform
} // namespace samp
