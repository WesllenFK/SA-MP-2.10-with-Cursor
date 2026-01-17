#include "FilesDirProvider.h"
#include "../Asset.h"
#include "../jni/AssetJNIBridge.h"
#include "../../java/jniutil.h"
#include "../../log.h"
#include <algorithm>
#include <cstring>

namespace samp {
namespace platform {
namespace storage {

FilesDirProvider::FilesDirProvider(JavaVM* javaVM, jobject activity)
    : m_javaVM(javaVM)
    , m_activityObject(activity)
{
    if (!m_javaVM || !m_activityObject) {
        LOGE("FilesDirProvider: Invalid parameters");
    }
}

std::unique_ptr<Asset> FilesDirProvider::Load(const std::string& path) {
    if (!ValidatePath(path)) {
        LOGE("FilesDirProvider::Load: Invalid path: %s", path.c_str());
        return nullptr;
    }
    
    AssetJNIBridge& bridge = AssetJNIBridge::Get();
    if (!bridge.FileExists(path)) {
        return nullptr;
    }
    
    size_t size = 0;
    std::unique_ptr<uint8_t[]> data = bridge.ReadFile(path, size);
    if (!data || size == 0) {
        return nullptr;
    }
    
    return std::make_unique<FileAsset>(path, std::move(data), size, AssetProviderType::FilesDir);
}

bool FilesDirProvider::Exists(const std::string& path) const {
    if (!ValidatePath(path)) {
        return false;
    }
    
    AssetJNIBridge& bridge = AssetJNIBridge::Get();
    return bridge.FileExists(path);
}

std::vector<std::string> FilesDirProvider::List(const std::string& directory) const {
    std::vector<std::string> result;
    
    if (!ValidatePath(directory)) {
        return result;
    }
    
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("FilesDirProvider::List: GetJNIEnvSafe failed");
        return result;
    }
    
    try {
        jclass storageManagerClass = env->FindClass("com/samp/mobile/platform/storage/AssetStorageManager");
        if (!storageManagerClass) {
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jmethodID getInstanceMethod = env->GetStaticMethodID(storageManagerClass, "getInstance",
                                                             "(Landroid/content/Context;)Lcom/samp/mobile/platform/storage/AssetStorageManager;");
        if (!getInstanceMethod) {
            EXCEPTION_CHECK(env);
            env->DeleteLocalRef(storageManagerClass);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jobject storageManagerObj = env->CallStaticObjectMethod(storageManagerClass, getInstanceMethod, m_activityObject);
        EXCEPTION_CHECK(env);
        env->DeleteLocalRef(storageManagerClass);
        
        if (!storageManagerObj) {
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jclass fileClass = env->FindClass("java/io/File");
        if (!fileClass) {
            env->DeleteLocalRef(storageManagerObj);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jmethodID fileConstructor = env->GetMethodID(fileClass, "<init>", "(Ljava/lang/String;)V");
        jmethodID listFilesMethod = env->GetMethodID(fileClass, "listFiles", "()[Ljava/io/File;");
        jmethodID getNameMethod = env->GetMethodID(fileClass, "getName", "()Ljava/lang/String;");
        jmethodID isFileMethod = env->GetMethodID(fileClass, "isFile", "()Z");
        
        if (!fileConstructor || !listFilesMethod || !getNameMethod || !isFileMethod) {
            env->DeleteLocalRef(fileClass);
            env->DeleteLocalRef(storageManagerObj);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        AssetJNIBridge& bridge = AssetJNIBridge::Get();
        std::string basePath = bridge.GetBasePath();
        std::string fullPath = basePath + directory;
        
        jstring jPath = env->NewStringUTF(fullPath.c_str());
        if (!jPath) {
            env->DeleteLocalRef(fileClass);
            env->DeleteLocalRef(storageManagerObj);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jobject fileObj = env->NewObject(fileClass, fileConstructor, jPath);
        env->DeleteLocalRef(jPath);
        
        if (!fileObj) {
            env->DeleteLocalRef(fileClass);
            env->DeleteLocalRef(storageManagerObj);
            EXCEPTION_CHECK(env);
            DETACH_IF_NEEDED(needsDetach);
            return result;
        }
        
        jobjectArray filesArray = static_cast<jobjectArray>(env->CallObjectMethod(fileObj, listFilesMethod));
        EXCEPTION_CHECK(env);
        env->DeleteLocalRef(fileObj);
        
        if (filesArray) {
            jsize length = env->GetArrayLength(filesArray);
            for (jsize i = 0; i < length; ++i) {
                jobject file = env->GetObjectArrayElement(filesArray, i);
                if (file) {
                    jboolean isFile = env->CallBooleanMethod(file, isFileMethod);
                    if (isFile == JNI_TRUE) {
                        jstring name = static_cast<jstring>(env->CallObjectMethod(file, getNameMethod));
                        if (name) {
                            const char* utf8 = env->GetStringUTFChars(name, nullptr);
                            if (utf8) {
                                result.push_back(std::string(utf8));
                                env->ReleaseStringUTFChars(name, utf8);
                            }
                            env->DeleteLocalRef(name);
                        }
                    }
                    env->DeleteLocalRef(file);
                }
            }
            env->DeleteLocalRef(filesArray);
        }
        
        env->DeleteLocalRef(fileClass);
        env->DeleteLocalRef(storageManagerObj);
    } catch (...) {
        LOGE("FilesDirProvider::List: Exception occurred");
    }
    
    DETACH_IF_NEEDED(needsDetach);
    return result;
}

bool FilesDirProvider::ValidatePath(const std::string& path) const {
    if (path.empty()) {
        return false;
    }
    
    if (path.find("../") != std::string::npos || path.find("..\\") != std::string::npos) {
        return false;
    }
    
    if (path.find("/data/") == 0 || path.find("/sdcard/") == 0 || path.find("/storage/") == 0) {
        return false;
    }
    
    return true;
}

} // namespace storage
} // namespace platform
} // namespace samp
