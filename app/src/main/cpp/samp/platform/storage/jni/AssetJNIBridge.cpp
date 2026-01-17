#include "AssetJNIBridge.h"
#include "../../main.h"
#include "../../java/jniutil.h"
#include "../../log.h"
#include <cstring>
#include <memory>

namespace samp {
namespace platform {
namespace storage {

AssetJNIBridge& AssetJNIBridge::Get() {
    static AssetJNIBridge instance;
    static bool initialized = false;
    if (!initialized) {
        instance.InitializeMembers();
        initialized = true;
    }
    return instance;
}

bool AssetJNIBridge::Initialize(JNIEnv* env, jobject activity) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    InitializeMembers();
    
    if (m_initialized) {
        return true;
    }
    
    if (!env || !activity) {
        LOGE("AssetJNIBridge::Initialize: Invalid parameters");
        return false;
    }
    
    jint result = env->GetJavaVM(&m_javaVM);
    if (result != JNI_OK || !m_javaVM) {
        LOGE("AssetJNIBridge::Initialize: GetJavaVM failed");
        return false;
    }
    
    m_activityObject = env->NewGlobalRef(activity);
    if (!m_activityObject) {
        LOGE("AssetJNIBridge::Initialize: NewGlobalRef failed");
        return false;
    }
    
    jclass contextClass = env->GetObjectClass(activity);
    if (!contextClass) {
        LOGE("AssetJNIBridge::Initialize: GetObjectClass failed");
        env->DeleteGlobalRef(m_activityObject);
        m_activityObject = nullptr;
        return false;
    }
    
    jclass storageManagerClass = env->FindClass("com/samp/mobile/platform/storage/AssetStorageManager");
    if (!storageManagerClass) {
        LOGE("AssetJNIBridge::Initialize: AssetStorageManager class not found");
        EXCEPTION_CHECK(env);
        env->DeleteLocalRef(contextClass);
        env->DeleteGlobalRef(m_activityObject);
        m_activityObject = nullptr;
        return false;
    }
    
    jmethodID getInstanceMethod = env->GetStaticMethodID(storageManagerClass, "getInstance", 
                                                         "(Landroid/content/Context;)Lcom/samp/mobile/platform/storage/AssetStorageManager;");
    if (!getInstanceMethod) {
        LOGE("AssetJNIBridge::Initialize: getInstance method not found");
        EXCEPTION_CHECK(env);
        env->DeleteLocalRef(storageManagerClass);
        env->DeleteLocalRef(contextClass);
        env->DeleteGlobalRef(m_activityObject);
        m_activityObject = nullptr;
        return false;
    }
    
    jobject storageManagerObj = env->CallStaticObjectMethod(storageManagerClass, getInstanceMethod, activity);
    EXCEPTION_CHECK(env);
    env->DeleteLocalRef(contextClass);
    
    if (!storageManagerObj) {
        LOGE("AssetJNIBridge::Initialize: getInstance returned null");
        env->DeleteLocalRef(storageManagerClass);
        env->DeleteGlobalRef(m_activityObject);
        m_activityObject = nullptr;
        return false;
    }
    
    m_storageManagerClass = static_cast<jclass>(env->NewGlobalRef(storageManagerClass));
    env->DeleteLocalRef(storageManagerObj);
    env->DeleteLocalRef(storageManagerClass);
    
    if (!m_storageManagerClass) {
        LOGE("AssetJNIBridge::Initialize: Failed to create global ref for storage manager class");
        env->DeleteGlobalRef(m_activityObject);
        m_activityObject = nullptr;
        return false;
    }
    
    m_getBasePathMethod = env->GetMethodID(m_storageManagerClass, "getBasePath", "()Ljava/lang/String;");
    m_fileExistsMethod = env->GetMethodID(m_storageManagerClass, "fileExists", "(Ljava/lang/String;)Z");
    m_readFileMethod = env->GetMethodID(m_storageManagerClass, "readFile", "(Ljava/lang/String;)[B");
    
    if (!m_getBasePathMethod || !m_fileExistsMethod || !m_readFileMethod) {
        LOGE("AssetJNIBridge::Initialize: Failed to get method IDs");
        EXCEPTION_CHECK(env);
        env->DeleteGlobalRef(m_storageManagerClass);
        m_storageManagerClass = nullptr;
        env->DeleteGlobalRef(m_activityObject);
        m_activityObject = nullptr;
        return false;
    }
    
    m_initialized = true;
    return true;
}

void AssetJNIBridge::Shutdown(JNIEnv* env) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    if (env) {
        if (m_storageManagerClass) {
            env->DeleteGlobalRef(m_storageManagerClass);
            m_storageManagerClass = nullptr;
        }
        if (m_activityObject) {
            env->DeleteGlobalRef(m_activityObject);
            m_activityObject = nullptr;
        }
    }
    
    m_javaVM = nullptr;
    m_getBasePathMethod = nullptr;
    m_fileExistsMethod = nullptr;
    m_readFileMethod = nullptr;
    m_initialized = false;
}

JNIEnv* AssetJNIBridge::GetEnv() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized || !m_javaVM) {
        return nullptr;
    }
    
    JNIEnv* env = nullptr;
    bool needsDetach = false;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("AssetJNIBridge::GetEnv: GetJNIEnvSafe failed");
        return nullptr;
    }
    
    return env;
}

bool AssetJNIBridge::FileExists(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("AssetJNIBridge::FileExists: GetJNIEnvSafe failed");
        return false;
    }
    
    if (!m_initialized || !m_storageManagerClass || !m_activityObject || !m_fileExistsMethod) {
        LOGE("AssetJNIBridge::FileExists: Not initialized");
        DETACH_IF_NEEDED(needsDetach);
        return false;
    }
    
    jmethodID getInstanceMethod = env->GetStaticMethodID(m_storageManagerClass, "getInstance", 
                                                         "(Landroid/content/Context;)Lcom/samp/mobile/platform/storage/AssetStorageManager;");
    if (!getInstanceMethod) {
        LOGE("AssetJNIBridge::FileExists: getInstance method not found");
        EXCEPTION_CHECK(env);
        DETACH_IF_NEEDED(needsDetach);
        return false;
    }
    
    jobject storageManagerObj = env->CallStaticObjectMethod(m_storageManagerClass, getInstanceMethod, m_activityObject);
    EXCEPTION_CHECK(env);
    
    if (!storageManagerObj) {
        LOGE("AssetJNIBridge::FileExists: getInstance returned null");
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
    
    jboolean exists = env->CallBooleanMethod(storageManagerObj, m_fileExistsMethod, jPath);
    EXCEPTION_CHECK(env);
    
    env->DeleteLocalRef(jPath);
    env->DeleteLocalRef(storageManagerObj);
    DETACH_IF_NEEDED(needsDetach);
    
    return exists == JNI_TRUE;
}

std::unique_ptr<uint8_t[]> AssetJNIBridge::ReadFile(const std::string& path, size_t& outSize) {
    outSize = 0;
    
    if (path.empty()) {
        return nullptr;
    }
    
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("AssetJNIBridge::ReadFile: GetJNIEnvSafe failed");
        return nullptr;
    }
    
    if (!m_initialized || !m_storageManagerClass || !m_activityObject || !m_readFileMethod) {
        LOGE("AssetJNIBridge::ReadFile: Not initialized");
        DETACH_IF_NEEDED(needsDetach);
        return nullptr;
    }
    
    jmethodID getInstanceMethod = env->GetStaticMethodID(m_storageManagerClass, "getInstance", 
                                                         "(Landroid/content/Context;)Lcom/samp/mobile/platform/storage/AssetStorageManager;");
    if (!getInstanceMethod) {
        LOGE("AssetJNIBridge::ReadFile: getInstance method not found");
        EXCEPTION_CHECK(env);
        DETACH_IF_NEEDED(needsDetach);
        return nullptr;
    }
    
    jobject storageManagerObj = env->CallStaticObjectMethod(m_storageManagerClass, getInstanceMethod, m_activityObject);
    EXCEPTION_CHECK(env);
    
    if (!storageManagerObj) {
        LOGE("AssetJNIBridge::ReadFile: getInstance returned null");
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
    
    jbyteArray jData = static_cast<jbyteArray>(env->CallObjectMethod(storageManagerObj, m_readFileMethod, jPath));
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
    DETACH_IF_NEEDED(needsDetach);
    
    outSize = static_cast<size_t>(dataSize);
    return data;
}

std::string AssetJNIBridge::GetBasePath() {
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("AssetJNIBridge::GetBasePath: GetJNIEnvSafe failed");
        return "";
    }
    
    if (!m_initialized || !m_storageManagerClass || !m_activityObject || !m_getBasePathMethod) {
        LOGE("AssetJNIBridge::GetBasePath: Not initialized");
        DETACH_IF_NEEDED(needsDetach);
        return "";
    }
    
    jmethodID getInstanceMethod = env->GetStaticMethodID(m_storageManagerClass, "getInstance", 
                                                         "(Landroid/content/Context;)Lcom/samp/mobile/platform/storage/AssetStorageManager;");
    if (!getInstanceMethod) {
        LOGE("AssetJNIBridge::GetBasePath: getInstance method not found");
        EXCEPTION_CHECK(env);
        DETACH_IF_NEEDED(needsDetach);
        return "";
    }
    
    jobject storageManagerObj = env->CallStaticObjectMethod(m_storageManagerClass, getInstanceMethod, m_activityObject);
    EXCEPTION_CHECK(env);
    
    if (!storageManagerObj) {
        LOGE("AssetJNIBridge::GetBasePath: getInstance returned null");
        DETACH_IF_NEEDED(needsDetach);
        return "";
    }
    
    jstring jPath = static_cast<jstring>(env->CallObjectMethod(storageManagerObj, m_getBasePathMethod));
    EXCEPTION_CHECK(env);
    
    std::string result;
    if (jPath) {
        const char* utf8 = env->GetStringUTFChars(jPath, nullptr);
        if (utf8) {
            result = utf8;
            env->ReleaseStringUTFChars(jPath, utf8);
        }
        env->DeleteLocalRef(jPath);
    }
    
    env->DeleteLocalRef(storageManagerObj);
    DETACH_IF_NEEDED(needsDetach);
    
    return result;
}

} // namespace storage
} // namespace platform
} // namespace samp
