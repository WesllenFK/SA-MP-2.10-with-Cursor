#pragma once

#include "main.h"

// Macro para verificar exceções JNI (não retorna, apenas limpa)
#define EXCEPTION_CHECK(env) \
	if ((env)->ExceptionCheck()) { \
		(env)->ExceptionDescribe(); \
		(env)->ExceptionClear(); \
	}

// Helper para obter JNIEnv de forma segura, anexando a thread se necessário
// Retorna true se obteve env válido
// needsDetach será true se a thread foi anexada e precisa ser desanexada depois
static inline bool GetJNIEnvSafe(JNIEnv** env, bool* needsDetach) {
	*needsDetach = false;
	*env = nullptr;
	
	if (!javaVM) {
		LOGE("GetJNIEnvSafe: javaVM is null");
		return false;
	}
	
	jint result = javaVM->GetEnv((void**)env, JNI_VERSION_1_6);
	
	if (result == JNI_OK) {
		return true;  // Thread já estava anexada
	}
	
	if (result == JNI_EDETACHED) {
		// Thread nativa não anexada - anexar agora
		if (javaVM->AttachCurrentThread(env, nullptr) == JNI_OK) {
			*needsDetach = true;
			return true;
		}
		LOGE("GetJNIEnvSafe: AttachCurrentThread failed");
	} else {
		LOGE("GetJNIEnvSafe: GetEnv failed with %d", result);
	}
	
	return false;
}

// Macro para desanexar thread se necessário
#define DETACH_IF_NEEDED(needsDetach) \
	if (needsDetach) { \
		javaVM->DetachCurrentThread(); \
	}

class CJavaWrapper
{
public:
    CJavaWrapper(JNIEnv *env, jobject activity);
    ~CJavaWrapper() {};
    
    const char* GetClipboardString();
    void SetPauseState(bool pause);
    void ShowLoadingScreen();
    void HideLoadingScreen();
    void ShowTab();
    void HideTab();

    void UpdateHudInfo(int health, int armour, int weaponid, int ammo, int ammoinclip, int money, int wanted);
    void ShowHud();
    void HideHud();

    void SetTab(int id, char* names, int score, int pings);
    void ClearTab();
    
    void ShowKeyboard();
    void HideKeyboard();

	void exitGame();

    jbyteArray as_byte_array(JNIEnv* env, unsigned char* buf, int len) {
		if (!env || !buf || len <= 0) return nullptr;
		jbyteArray array = env->NewByteArray(len);
		if (array) {
			env->SetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte*>(buf));
		}
		return array;
	}

    void ShowDialog(int dialogStyle, int dialogID, char* title, char* text, char* button1, char* button2);

	void ShowEditObject();
	void HideEditObject();

    jobject activity;
    jmethodID s_setPauseState;
    jmethodID s_showLoadingScreen;
    jmethodID s_hideLoadingScreen;
    jmethodID s_ShowDialog;
    jmethodID s_showTab;
    jmethodID s_setTab;
    jmethodID s_clearTab;
    jmethodID s_hideTab;
    jmethodID s_showInputLayout;
    jmethodID s_hideInputLayout;
	jmethodID s_exitGame;
	jmethodID s_showEditObject;
	jmethodID s_hideEditObject;

    jmethodID s_showHud;
    jmethodID s_hideHud;
    jmethodID s_updateHudInfo;
};