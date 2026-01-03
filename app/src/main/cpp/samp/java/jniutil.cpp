#include "jniutil.h"
#include "game/game.h"
extern CGame *pGame;

CJavaWrapper::CJavaWrapper(JNIEnv *env, jobject activity)
{
    this->activity = env->NewGlobalRef(activity);

    jclass clas = env->GetObjectClass(activity);
    if(!clas)
    {
        LOGE("CJavaWrapper: GetObjectClass failed");
        return;
    }

    s_showTab = env->GetMethodID(clas, "showTab", "()V");
    s_hideTab = env->GetMethodID(clas, "hideTab", "()V");

    s_clearTab = env->GetMethodID(clas, "clearTab", "()V");
    s_setTab = env->GetMethodID(clas, "setTab", "(ILjava/lang/String;II)V");

    s_showLoadingScreen = env->GetMethodID(clas, "showLoadingScreen", "()V");
    s_hideLoadingScreen = env->GetMethodID(clas, "hideLoadingScreen", "()V");

    s_setPauseState = env->GetMethodID(clas, "setPauseState", "(Z)V");
    
    s_ShowDialog = env->GetMethodID(clas, "showDialog", "(II[B[B[B[B)V");

	s_showInputLayout = env->GetMethodID(clas, "showKeyboard", "()V");
    s_hideInputLayout = env->GetMethodID(clas, "hideKeyboard", "()V");

    s_exitGame = env->GetMethodID(clas, "exitGame", "()V");

    s_showEditObject = env->GetMethodID(clas, "showEditObject", "()V");
    s_hideEditObject = env->GetMethodID(clas, "hideEditObject", "()V");

    s_updateHudInfo = env->GetMethodID(clas, "updateHudInfo", "(IIIIIII)V");
    s_showHud = env->GetMethodID(clas, "showHud", "()V");
    s_hideHud = env->GetMethodID(clas, "hideHud", "()V");

    env->DeleteLocalRef(clas);
}

void CJavaWrapper::ShowKeyboard()
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("ShowKeyboard: Failed to get JNIEnv");
        return;
    }
    
    env->CallVoidMethod(activity, s_showInputLayout);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::HideKeyboard()
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("HideKeyboard: Failed to get JNIEnv");
        return;
    }
    
    env->CallVoidMethod(activity, s_hideInputLayout);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::ShowLoadingScreen()
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("ShowLoadingScreen: Failed to get JNIEnv");
        return;
    }
    
    env->CallVoidMethod(activity, s_showLoadingScreen);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::HideLoadingScreen()
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("HideLoadingScreen: Failed to get JNIEnv");
        return;
    }
    
    env->CallVoidMethod(activity, s_hideLoadingScreen);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::SetPauseState(bool pause)
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("SetPauseState: Failed to get JNIEnv");
        return;
    }
    
    env->CallVoidMethod(activity, s_setPauseState, pause);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::SetTab(int id, char* names, int score, int pings)
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("SetTab: Failed to get JNIEnv");
        return;
    }

	jclass strClass = env->FindClass("java/lang/String"); 
	if (!strClass) {
		LOGE("SetTab: FindClass failed");
		DETACH_IF_NEEDED(needsDetach);
		return;
	}
	
	jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V"); 
	jstring encoding = env->NewStringUTF("UTF-8"); 

	jbyteArray bytes = env->NewByteArray(strlen(names)); 
	env->SetByteArrayRegion(bytes, 0, strlen(names), (jbyte*)names); 
	jstring str1 = (jstring)env->NewObject(strClass, ctorID, bytes, encoding);

    env->CallVoidMethod(activity, s_setTab, id, str1, score, pings);
    EXCEPTION_CHECK(env);
    
    // Cleanup local refs
    env->DeleteLocalRef(bytes);
    env->DeleteLocalRef(str1);
    env->DeleteLocalRef(encoding);
    env->DeleteLocalRef(strClass);
    
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::ShowTab()
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("ShowTab: Failed to get JNIEnv");
        return;
    }
    
    env->CallVoidMethod(activity, s_showTab);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::HideTab()
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("HideTab: Failed to get JNIEnv");
        return;
    }
    
    env->CallVoidMethod(activity, s_hideTab);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::ClearTab()
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("ClearTab: Failed to get JNIEnv");
        return;
    }
    
    env->CallVoidMethod(activity, s_clearTab);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::ShowDialog(int dialogStyle, int dialogID, char* title, char* text, char* button1, char* button2)
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("ShowDialog: Failed to get JNIEnv");
        return;
    }

	std::string sTitle(title);
	std::string sText(text);
	std::string sButton1(button1);
	std::string sButton2(button2);

	jbyteArray jstrTitle = as_byte_array(env, (unsigned char*)sTitle.c_str(), sTitle.length());
	jbyteArray jstrText = as_byte_array(env, (unsigned char*)sText.c_str(), sText.length());
	jbyteArray jstrButton1 = as_byte_array(env, (unsigned char*)sButton1.c_str(), sButton1.length());
	jbyteArray jstrButton2 = as_byte_array(env, (unsigned char*)sButton2.c_str(), sButton2.length());

	env->CallVoidMethod(activity, s_ShowDialog, dialogID, dialogStyle, jstrTitle, jstrText, jstrButton1, jstrButton2);
	EXCEPTION_CHECK(env);
	
	// Cleanup local refs
	if (jstrTitle) env->DeleteLocalRef(jstrTitle);
	if (jstrText) env->DeleteLocalRef(jstrText);
	if (jstrButton1) env->DeleteLocalRef(jstrButton1);
	if (jstrButton2) env->DeleteLocalRef(jstrButton2);
	
	DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::UpdateHudInfo(int health, int armour, int weaponid, int ammo, int ammoinclip, int money, int wanted)
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("UpdateHudInfo: Failed to get JNIEnv");
        return;
    }

    env->CallVoidMethod(this->activity, this->s_updateHudInfo, health, armour, weaponid, ammo, ammoinclip, money, wanted);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::ShowHud()
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("ShowHud: Failed to get JNIEnv");
        return;
    }
    
    env->CallVoidMethod(this->activity, this->s_showHud);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::HideHud()
{
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("HideHud: Failed to get JNIEnv");
        return;
    }
    
    env->CallVoidMethod(this->activity, this->s_hideHud);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::exitGame() {
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("exitGame: Failed to get JNIEnv");
        return;
    }

    env->CallVoidMethod(this->activity, this->s_exitGame);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::ShowEditObject() {
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("ShowEditObject: Failed to get JNIEnv");
        return;
    }

    env->CallVoidMethod(this->activity, this->s_showEditObject);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}

void CJavaWrapper::HideEditObject() {
    JNIEnv* env;
    bool needsDetach;
    
    if (!GetJNIEnvSafe(&env, &needsDetach)) {
        LOGE("HideEditObject: Failed to get JNIEnv");
        return;
    }

    env->CallVoidMethod(this->activity, this->s_hideEditObject);
    EXCEPTION_CHECK(env);
    DETACH_IF_NEEDED(needsDetach);
}
