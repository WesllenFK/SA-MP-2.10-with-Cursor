#include "CFileMgr.h"
#include "main.h"
#include "../vendor/armhook/patch.h"
#include "../platform/storage/AssetManager.h"

void CFileMgr::SetDir(const char *path) {
    ( ( void(*)(const char *path) )(g_libGTASA + (VER_x32 ? 0x003F0C54 + 1 : 0x4D293C)) )(path);
}

FILE* CFileMgr::OpenFile(const char *path, const char *mode) {
    using namespace samp::platform::storage;
    
    AssetManager& assetMgr = AssetManager::Get();
    const std::string& basePath = assetMgr.GetBasePath();
    
    if (basePath.empty()) {
        FLog("OpenFile failed: storage path not set");
        return nullptr;
    }

    sprintf(ms_path, "%s%s", basePath.c_str(), path);

    auto file = fopen(ms_path, mode);

    if(!file) {
        FLog("Fail open file %s", ms_path);
    }
    return file;
}

int32_t CFileMgr::CloseFile(FILE* file) {
    return fclose(file);
}

void CFileMgr::Initialise() {
    CHook::CallFunction<void>("_ZN8CFileMgr10InitialiseEv"); // ������� ����� �����
}
