#include <jni.h>
#include <pthread.h>
#include <syscall.h>

#include "main.h"
#include "game/game.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "playertags.h"
#include "audiostream.h"
#include "java/jniutil.h"
#include <dlfcn.h>
#include "StackTrace.h"
#include "servers.h"

// voice
#include "voice_new/Plugin.h"

#include "vendor/armhook/patch.h"
#include "vendor/str_obfuscator/str_obfuscator.hpp"

#include "settings.h"

#include "crashlytics.h"

/*
Peerapol Unarak
*/
JavaVM* javaVM;

char* g_pszStorage = nullptr;

// Buffer próprio para armazenar o storage path recebido via JNI
static char s_szStoragePath[512] = {0};

// Flag para indicar se o storage path foi definido via JNI
bool g_bStoragePathSetViaJNI = false;

UI* pUI = nullptr;
CGame *pGame = nullptr;

CNetGame *pNetGame = nullptr;
CPlayerTags* pPlayerTags = nullptr;
CSnapShotHelper* pSnapShotHelper = nullptr;
CAudioStream* pAudioStream = nullptr;
CJavaWrapper* pJavaWrapper = nullptr;
CSettings* pSettings = nullptr;
//CVoice* pVoice = nullptr;

MaterialTextGenerator* pMaterialTextGenerator = nullptr;

bool bDebug = false;
bool bGameInited = false;
bool bNetworkInited = false;

uintptr_t g_libGTASA = 0x00;
uintptr_t g_libSAMP = 0x00;

void ApplyGlobalPatches();
void ApplyPatches_level0();
void ApplyMultiTouchPatches();
void InstallGlobalHooks();
void InstallSpecialHooks();
void InitRenderWareFunctions();
void FLog(const char* fmt, ...);
//void MyLog(const char* fmt, ...);

int work = 0;
bool serverConnect = false;

void ReadSettingFile()
{
	/*char path[255] = { 0 };
	//sprintf(path, "%ssamp.set", g_pszStorage);
	sprintf(path, "%sNickName.ini", g_pszStorage);

	FILE* fp = fopen(path, "r");
	if (fp == NULL) return;

	char buf[1024];

	// nickname
	if (fgets(buf, 1024, fp) != NULL) {
		buf[strcspn(buf, "\n\r")] = 0;
		strcpy(g_nick, buf);
	}

	fclose(fp);*/

	pSettings = new CSettings();

	firebase::crashlytics::SetUserId(pSettings->Get().szNickName);
}
/*#include <GLES2/gl2.h>
void  (*RQ_Command_rqTextureMipMode)(int **a1);
void  RQ_Command_rqTextureMipMode_hook(int **a1)
{
    int v1; // r3
    int v2; // r6
    GLuint v3; // r4
    // LOGBR(__FUNCTION__ );
    v1 = **a1;
    *a1 += 2;
    v2 = g_libGTASA;
    v3 = *(uintptr_t *)(v1 + 0x28);
    if ( *(uintptr_t *)(g_libGTASA + (VER_x32 ? 0x0067A274 : 0x00852338) != 5 ))
    {
        glActiveTexture(0x84C5u);
        *(uintptr_t *)(v2 + VER_x32 ? 0x0067A274 : 0x00852338) = 5; 
    }
    glBindTexture(0xDE1u, v3);
    *(uintptr_t *)(g_libGTASA + (VER_x32 ? 0x1D02E8 : 0x265800)) = v3; 
    glTexParameteri(0xDE1u, 0x2801u, 0x2601);
    glTexParameteri(0xDE1u, 0x2800u, 0x2601);
}*/

int hashing(const char* str) {
	int hashing = 5381;
	int c;
	while (c = *str++) {
		hashing = ((hashing << 5) + hashing) + c; /* hash * 33 + c */
		if (hashing < 0) hashing = 100;
	}
	if (hashing < 0) hashing = 100;
	return hashing;
}

void DoDebugLoop()
{
	// ...
}

void DoDebugStuff()
{
	// ...

	RwMatrix mat = pGame->FindPlayerPed()->m_pPed->GetMatrix().ToRwMatrix();
	
	for (int i = 0; i < 100; i++)
	{
		CPlayerPed* ped = pGame->NewPlayer(i, mat.pos.x + i, mat.pos.y, mat.pos.z, 0.0f, false, false);
		//ped->SetCollisionChecking(false);
		//ped->SetGravityProcessing(false);
	}
}
struct sigaction act_old;
struct sigaction act1_old;
struct sigaction act2_old;
struct sigaction act3_old;

extern int g_iLastProcessedSkinCollision, g_iLastProcessedEntityCollision, g_iLastRenderedObject;
extern uintptr_t g_dwLastRetAddrCrash;
void handler(int signum, siginfo_t *info, void* contextPtr)
{
	ucontext* context = (ucontext_t*)contextPtr;

	if (act_old.sa_sigaction)
	{
		act_old.sa_sigaction(signum, info, contextPtr);
	}

	if(info->si_signo == SIGSEGV)
	{
		FLog("SIGSEGV | Fault address: 0x%x", info->si_addr);

		PRINT_CRASH_STATES(context);

		CStackTrace::printBacktrace();
	}

	return;
}

void handler1(int signum, siginfo_t *info, void* contextPtr)
{
	ucontext* context = (ucontext_t*)contextPtr;

	if (act1_old.sa_sigaction)
	{
		act1_old.sa_sigaction(signum, info, contextPtr);
	}

	if(info->si_signo == SIGABRT)
	{
		FLog("SIGABRT | Fault address: 0x%x", info->si_addr);

		PRINT_CRASH_STATES(context);

		CStackTrace::printBacktrace();
	}

	return;
}

void handler2(int signum, siginfo_t *info, void* contextPtr)
{
	ucontext* context = (ucontext_t*)contextPtr;

	if (act2_old.sa_sigaction)
	{
		act2_old.sa_sigaction(signum, info, contextPtr);
	}

	if(info->si_signo == SIGFPE)
	{
		FLog("SIGFPE | Fault address: 0x%x", info->si_addr);

		PRINT_CRASH_STATES(context);

		CStackTrace::printBacktrace();
	}

	return;
}

void handler3(int signum, siginfo_t *info, void* contextPtr)
{
	ucontext* context = (ucontext_t*)contextPtr;

	if (act3_old.sa_sigaction)
	{
		act3_old.sa_sigaction(signum, info, contextPtr);
	}

	if(info->si_signo == SIGBUS)
	{
		FLog("SIGBUS | Fault address: 0x%x", info->si_addr);

		PRINT_CRASH_STATES(context);

		CStackTrace::printBacktrace();
	}

	return;
}

void DoInitStuff() {
    if (bGameInited == false) {
        pPlayerTags = new CPlayerTags();
        pSnapShotHelper = new CSnapShotHelper();
        pMaterialTextGenerator = new MaterialTextGenerator();
        pAudioStream = new CAudioStream();
        pAudioStream->Initialize();

        pUI->splashscreen()->setVisible(false);
        pUI->chat()->setVisible(true);
        //pUI->buttonpanel()->setVisible(true);

        pGame->Initialize();
        pGame->SetMaxStats();
        pGame->ToggleThePassingOfTime(false);

        // voice
        /*LogVoice("[dbg:samp:load] : module loading...");

        for (const auto& loadCallback : Samp::loadCallbacks) {
            if (loadCallback != nullptr) {
                loadCallback();
            }
        }

        Samp::loadStatus = true;*/

        LogVoice("[dbg:samp:load] : module loaded");

        if (bDebug) {
            CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                              (VER_x32 ? 0x00951FA8 : 0xBBA8D0));
            //TheCamera.Restore();
            CCamera::SetBehindPlayer();
            pGame->DisplayHUD(true);
            pGame->EnableClock(false);

            DoDebugStuff();
        }

        bGameInited = true;
    }

    if (!bNetworkInited && !bDebug  && !serverConnect) {

        int serverid = pSettings->GetReadOnly().iServerID;

        if (serverid == 0)
        {
            pNetGame = new CNetGame(SERVER_HOST_TEST, SERVER_PORT_TEST, pSettings->Get().szNickName, pSettings->Get().szPassword);
        }
        else if (serverid == 1)
        {
            pNetGame = new CNetGame(SERVER_HOST_1, SERVER_PORT_1, pSettings->Get().szNickName, pSettings->Get().szPassword);
        }
        else if (serverid == 2)
        {
            pNetGame = new CNetGame(SERVER_HOST_2, SERVER_PORT_2, pSettings->Get().szNickName, pSettings->Get().szPassword);
        }
        else if (serverid == 3)
        {
            pNetGame = new CNetGame(SERVER_HOST_3, SERVER_PORT_3, pSettings->Get().szNickName, pSettings->Get().szPassword);
        }
        else if (serverid == 4)
        {
            pNetGame = new CNetGame(SERVER_HOST_4, SERVER_PORT_4, pSettings->Get().szNickName, pSettings->Get().szPassword);
        }

        bNetworkInited = true;
        pUI->chat()->addDebugMessage("Connected to server... {622cf5}ID: %d", serverid);


        FLog("DoInitStuff end");
    }
}

/*
void GameBackground()
{
    CPlayerPed* pPlayerPed = pGame->FindPlayerPed();

    if (pPlayerPed->IsInVehicle())
    {
        pPlayerPed->RemoveFromVehicleAndPutAt(1093.4, -2036.5, 82.710602);
    }
    else
    {
        pPlayerPed->m_pPed->SetPosn(-391.4141, 73.0535, 13.6677);
    }

    CCamera::SetPosition(-292.2141, 73.0535, 13.5891, 0.0, 0.0, 0.0);
    CCamera::LookAtPoint(-291.4141, 73.0535, 13.6677, 2);

    pGame->SetWorldWeather(1);
    pGame->DisplayHUD(false);
    pPlayerPed->TogglePlayerControllable(false);

    CPlayerPed* fufy = new CPlayerPed(35, 122, -288.00f, 72.0f, 13.6965f, 80.0f);
    pJavaWrapper->HideLoadingScreen();
    bNetworkInited = true;
    FLog("No env");
    pJavaWrapper->ShowMenu();
}
*/

extern "C" {
	// Função JNI para definir o storage path a partir do Java
	JNIEXPORT void JNICALL Java_com_samp_mobile_game_SAMP_setStoragePath(
		JNIEnv *pEnv, jobject thiz, jstring path)
	{
		if (path == nullptr) {
			LOGE("setStoragePath: path is null!");
			return;
		}

		const char* pathStr = pEnv->GetStringUTFChars(path, nullptr);
		if (pathStr) {
			// Copia o path para o buffer estático
			strncpy(s_szStoragePath, pathStr, sizeof(s_szStoragePath) - 1);
			s_szStoragePath[sizeof(s_szStoragePath) - 1] = '\0';

			// Atualiza o ponteiro global
			g_pszStorage = s_szStoragePath;
			g_bStoragePathSetViaJNI = true;

			LOGI("Storage path set via JNI: %s", g_pszStorage);

			// Verifica se o diretório existe
			if (access(g_pszStorage, F_OK) != 0) {
				LOGE("WARNING: Storage path does not exist: %s", g_pszStorage);
			} else {
				LOGI("Storage path verified: exists and accessible");
			}

			pEnv->ReleaseStringUTFChars(path, pathStr);
		} else {
			LOGE("setStoragePath: failed to get string from Java!");
		}
	}

	JNIEXPORT void JNICALL Java_com_samp_mobile_game_SAMP_initializeSAMP(JNIEnv *pEnv, jobject thiz)
	{
		pJavaWrapper = new CJavaWrapper(pEnv, thiz);

	}
	JNIEXPORT void JNICALL Java_com_samp_mobile_game_SAMP_onInputEnd(JNIEnv *pEnv, jobject thiz, jbyteArray str)
	{
		if(pUI)
		{
			pUI->keyboard()->sendForGB(pEnv, thiz, str);
		}
	}
	JNIEXPORT void JNICALL Java_com_samp_mobile_game_SAMP_onEventBackPressed(JNIEnv *pEnv, jobject thiz)
	{
		if(pSettings)
		{
			if(pSettings->Get().iAndroidKeyboard)
				pJavaWrapper->HideKeyboard();
		}
	}
	JNIEXPORT void JNICALL Java_com_samp_mobile_game_ui_dialog_DialogManager_sendDialogResponse(JNIEnv* pEnv, jobject thiz, jint i3, jint i, jint i2, jbyteArray str)
	{
		jboolean isCopy = true;

		jbyte* pMsg = pEnv->GetByteArrayElements(str, &isCopy);
		jsize length = pEnv->GetArrayLength(str);

		std::string szStr((char*)pMsg, length);
		
		if(pNetGame) {
			pNetGame->SendDialogResponse(i, i3, i2, (char*)szStr.c_str());
			//pGame->FindPlayerPed()->TogglePlayerControllableWithoutLock(true);
		}

		pEnv->ReleaseByteArrayElements(str, pMsg, JNI_ABORT);
	}
}

void MainLoop()
{
	if (pGame->bIsGameExiting) return;

	DoInitStuff();

	if (bDebug) {
		DoDebugLoop();
	}

	if (pNetGame) {
		pNetGame->Process();
	}

	if (pAudioStream) {
		pAudioStream->Process();
	}

}

void InitGui()
{
	// new voice
	Plugin::OnPluginLoad();
	Plugin::OnSampLoad();

	std::string font_path = string_format("%sSAMP/fonts/%s", g_pszStorage, FONT_NAME);
	pUI = new UI(ImVec2(RsGlobal->maximumWidth, RsGlobal->maximumHeight), font_path.c_str());
	pUI->initialize();
	pUI->performLayout();
}

#include "game/multitouch.h"
#include "armhook/patch.h"
#include "util/CUtil.h"
#include "obfusheader.h"



void secret_function()
{
    WATERMARK("SECRET OF FUCKING MIMIC: I HAVE PENIS", // Message for crackers ;)
              "hpdev daun",
              "go touch some grass", 0);

}

void SetUpGLHooks();
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	javaVM = vm;
	LOGI(OBF("SA-MP library loaded! Build time: " __DATE__ " " __TIME__));

    WATERMARK("                   by github.com/kuzia15                   ",
              "                   by github.com/kuzia15                   ",
              "                   by github.com/kuzia15                   ",
              "                   by github.com/kuzia15                   ",
              "                   by github.com/kuzia15                   ",
              "                   by github.com/kuzia15                   ",
              "                   by github.com/kuzia15                   ",
              "                   by github.com/kuzia15                   ",
              "                                                           ", 0);

	g_libGTASA = CUtil::FindLib(MAKEOBF("libGTASA.so"));
	if (g_libGTASA == 0x00) {
		LOGE(OBF("libGTASA.so address was not found! "));
		return JNI_VERSION_1_6;
	}

	g_libSAMP = CUtil::FindLib(MAKEOBF("libsamp.so"));
	if (g_libSAMP == 0x00) {
		LOGE(OBF("libsamp.so address was not found! "));
		return JNI_VERSION_1_6;
	}

	firebase::crashlytics::Initialize();

	uintptr_t libgtasa = CUtil::FindLib(MAKEOBF("libGTASA.so"));
	uintptr_t libsamp = CUtil::FindLib(MAKEOBF("libsamp.so"));
	uintptr_t libc = CUtil::FindLib(MAKEOBF("libc.so"));

	FLog(OBF("libGTASA.so: 0x%x"), libgtasa);
	FLog(OBF("libsamp.so: 0x%x"), libsamp);
	FLog(OBF("libc.so: 0x%x"), libc);

	char str[100];

	sprintf(str, "0x%x", libgtasa);
	firebase::crashlytics::SetCustomKey("libGTASA.so", str);
	
	sprintf(str, "0x%x", libsamp);
	firebase::crashlytics::SetCustomKey("libsamp.so", str);

	sprintf(str, "0x%x", libc);
	firebase::crashlytics::SetCustomKey("libc.so", str);

	CHook::InitHookStuff();
	InstallSpecialHooks();
	ApplyPatches_level0();
    //CHook::InstallPLT(g_libGTASA + (VER_x32 ? 0x1D0350 : 0x265688), &RQ_Command_rqTextureMipMode_hook, &RQ_Command_rqTextureMipMode);
    //SetUpGLHooks();
    InitRenderWareFunctions();
    MultiTouch::initialize();

	pGame = new CGame();

	//pVoice = new CVoice();
	//pVoice->Initialize(VOICE_FREQUENCY, CODEC_FREQUENCY, VOICE_SENDRRATE);

	//pthread_t thread;
	//pthread_create(&thread, 0, Init, 0);

	struct sigaction act;
	act.sa_sigaction = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &act, &act_old);

	struct sigaction act1;
	act1.sa_sigaction = handler1;
	sigemptyset(&act1.sa_mask);
	act1.sa_flags = SA_SIGINFO;
	sigaction(SIGABRT, &act1, &act1_old);

	struct sigaction act2;
	act2.sa_sigaction = handler2;
	sigemptyset(&act2.sa_mask);
	act2.sa_flags = SA_SIGINFO;
	sigaction(SIGFPE, &act2, &act2_old);

	struct sigaction act3;
	act3.sa_sigaction = handler3;
	sigemptyset(&act3.sa_mask);
	act3.sa_flags = SA_SIGINFO;
	sigaction(SIGBUS, &act3, &act3_old);

    secret_function();

	return JNI_VERSION_1_6;
}

uint32_t GetTickCount()
{
    return CTimer::m_snTimeInMillisecondsNonClipped;
}

void ShowToastAndExit(const char* message)
{
	JNIEnv* env = nullptr;
	if (javaVM == nullptr) {
		LOGE("ShowToastAndExit: javaVM is null, exiting without toast");
		exit(1);
		return;
	}

	bool needsDetach = false;
	int getEnvResult = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
	
	if (getEnvResult == JNI_EDETACHED) {
		if (javaVM->AttachCurrentThread(&env, nullptr) != 0) {
			LOGE("ShowToastAndExit: Failed to attach thread");
			exit(1);
			return;
		}
		needsDetach = true;
	} else if (getEnvResult != JNI_OK) {
		LOGE("ShowToastAndExit: Failed to get JNI env");
		exit(1);
		return;
	}

	// Mostra Toast via JNI
	jclass toastClass = env->FindClass("android/widget/Toast");
	if (toastClass != nullptr) {
		jclass activityThread = env->FindClass("android/app/ActivityThread");
		if (activityThread != nullptr) {
			jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
			jmethodID getApplication = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
			
			jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
			if (at != nullptr) {
				jobject context = env->CallObjectMethod(at, getApplication);
				if (context != nullptr) {
					jmethodID makeText = env->GetStaticMethodID(toastClass, "makeText", 
						"(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
					jmethodID show = env->GetMethodID(toastClass, "show", "()V");
					
					jstring jMessage = env->NewStringUTF(message);
					jobject toast = env->CallStaticObjectMethod(toastClass, makeText, context, jMessage, 1); // 1 = LENGTH_LONG
					if (toast != nullptr) {
						env->CallVoidMethod(toast, show);
					}
					env->DeleteLocalRef(jMessage);
				}
			}
		}
	}

	// Limpa exceções se houver
	if (env->ExceptionCheck()) {
		env->ExceptionClear();
	}

	if (needsDetach) {
		javaVM->DetachCurrentThread();
	}

	LOGE("ShowToastAndExit: %s", message);
	
	// Aguarda um pouco para o Toast aparecer e depois fecha
	usleep(500000); // 500ms
	exit(1);
}

void FLog(const char* fmt, ...)
{
	char buffer[0xFF];
	static FILE* flLog = nullptr;
	const char* pszStorage = g_pszStorage;


	// Tenta abrir arquivo apenas se o path for válido (não NULL e não vazio)
	if (flLog == nullptr && pszStorage != nullptr && pszStorage[0] != '\0')
	{
		sprintf(buffer, "%s/samp_log.txt", pszStorage);
		flLog = fopen(buffer, "a");
	}

	memset(buffer, 0, sizeof(buffer));

	va_list arg;
	va_start(arg, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arg);
	va_end(arg);

	// Sempre envia para logcat e crashlytics (fallback garantido)
	LOGI("%s", buffer);
	firebase::crashlytics::Log(buffer);

	// Se arquivo não está disponível, apenas retorna (log já foi para logcat)
	if (flLog == nullptr) return;
	fprintf(flLog, "%s\n", buffer);
	fflush(flLog);

	return;
}

void ChatLog(const char* fmt, ...)
{
	char buffer[0xFF];
	static FILE* flLog = nullptr;
	const char* pszStorage = g_pszStorage;

	// Tenta abrir arquivo apenas se o path for válido
	if (flLog == nullptr && pszStorage != nullptr && pszStorage[0] != '\0')
	{
		sprintf(buffer, "%s/chat_log.txt", pszStorage);
		flLog = fopen(buffer, "a");
	}

	memset(buffer, 0, sizeof(buffer));

	va_list arg;
	va_start(arg, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arg);
	va_end(arg);

	// Se arquivo não está disponível, apenas retorna
	if (flLog == nullptr) return;
	fprintf(flLog, "%s\n", buffer);
	fflush(flLog);
}

void MyLog(const char* fmt, ...)
{
	char buffer[0xFF];
	static FILE* flLog = nullptr;
	const char* pszStorage = g_pszStorage;

	// Tenta abrir arquivo apenas se o path for válido
	if (flLog == nullptr && pszStorage != nullptr && pszStorage[0] != '\0')
	{
		sprintf(buffer, "%s/samp_log.txt", pszStorage);
		flLog = fopen(buffer, "a");
	}

	memset(buffer, 0, sizeof(buffer));

	va_list arg;
	va_start(arg, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arg);
	va_end(arg);

	// Se arquivo não está disponível, apenas retorna
	if (flLog == nullptr) return;
	fprintf(flLog, "%s\n", buffer);
	fflush(flLog);
}

void MyLog2(const char* fmt, ...)
{
	char buffer[0xFF];
	static FILE* flLog = nullptr;
	const char* pszStorage = g_pszStorage;

	// Tenta abrir arquivo apenas se o path for válido
	if (flLog == nullptr && pszStorage != nullptr && pszStorage[0] != '\0')
	{
		sprintf(buffer, "%s/samp_log.txt", pszStorage);
		flLog = fopen(buffer, "a");
	}

	memset(buffer, 0, sizeof(buffer));

	va_list arg;
	va_start(arg, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arg);
	va_end(arg);

	if (pUI) pUI->chat()->addDebugMessage(buffer);

	// Se arquivo não está disponível, apenas retorna
	if (flLog == nullptr) return;
	fprintf(flLog, "%s\n", buffer);
	fflush(flLog);
}

void LogVoice(const char* fmt, ...)
{
	char buffer[0xFF];
	static FILE* flLog = nullptr;
	const char* pszStorage = g_pszStorage;

	// Tenta abrir arquivo apenas se o path for válido
	if (flLog == nullptr && pszStorage != nullptr && pszStorage[0] != '\0')
	{
		sprintf(buffer, "%sSAMP/%s", pszStorage, SV::kLogFileName);
		flLog = fopen(buffer, "w");
	}

	memset(buffer, 0, sizeof(buffer));

	va_list arg;
	va_start(arg, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arg);
	va_end(arg);

	// Sempre envia para logcat (fallback garantido)
	__android_log_write(ANDROID_LOG_INFO, "AXL", buffer);

	// Se arquivo não está disponível, apenas retorna
	if (flLog == nullptr) return;
	fprintf(flLog, "%s\n", buffer);
	fflush(flLog);
}