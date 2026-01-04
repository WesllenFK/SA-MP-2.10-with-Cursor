#include "main.h"
#include "settings.h"
#include "vendor/inih/cpp/INIReader.h"

#include "vendor/SimpleIni/SimpleIni.h"
#include "game/game.h"
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>

extern CGame *pGame;

CSettings::CSettings()
{
	FLog("Loading settings..");	

	// 1. Verificar g_pszStorage ANTES de usar (igual patches.cpp)
	if (g_pszStorage == nullptr || g_pszStorage[0] == '\0') {
		FLog("CSettings ERROR: storage path not set!");
		std::terminate();
		return;
	}

	// 2. Usar buffer maior (igual patches.cpp e outros sistemas)
	char buff[0xFF];  // 255 bytes em vez de 127
	sprintf(buff, "%sSAMP/settings.ini", g_pszStorage);
	
	// 3. Log do caminho para debug
	FLog("CSettings: Attempting to load: [%s]", buff);
	FLog("CSettings: Storage path length: %zu", strlen(g_pszStorage));
	FLog("CSettings: Full path length: %zu", strlen(buff));

	// 4. Ajustar ownership e permissões do arquivo (para arquivos criados externamente)
	// Isso resolve problemas de ownership no Android 11+ onde arquivos criados via adb
	// podem ter ownership diferente do processo do app
	uid_t current_uid = getuid();
	gid_t current_gid = getgid();
	FLog("CSettings: Current UID=%d, GID=%d", current_uid, current_gid);
	
	// Tentar ajustar ownership (pode falhar se não tiver permissão, mas não é crítico)
	if (chown(buff, current_uid, current_gid) != 0) {
		FLog("CSettings WARNING: chown failed for [%s], errno = %d (%s)", buff, errno, strerror(errno));
		// Continuar mesmo se chown falhar - pode funcionar se permissões estiverem corretas
	} else {
		FLog("CSettings: chown successful for [%s]", buff);
	}
	
	// Ajustar permissões para garantir (rw-rw-r--)
	if (chmod(buff, 0664) != 0) {
		FLog("CSettings WARNING: chmod failed for [%s], errno = %d (%s)", buff, errno, strerror(errno));
		// Continuar mesmo se chmod falhar
	} else {
		FLog("CSettings: chmod successful for [%s]", buff);
	}

	// 5. Verificar se arquivo pode ser aberto antes (igual patches.cpp faz indiretamente)
	FILE* test_file = fopen(buff, "r");
	if (!test_file) {
		FLog("CSettings ERROR: fopen failed for [%s]", buff);
		FLog("CSettings ERROR: errno = %d", errno);
		FLog("CSettings ERROR: %s", strerror(errno));
		std::terminate();
		return;
	}
	fclose(test_file);
	FLog("CSettings: File exists and is readable, proceeding with INIReader");

	INIReader reader(buff);
	int parse_error = reader.ParseError();
	FLog("CSettings: ParseError = %d", parse_error);

	if(parse_error < 0)
	{
		FLog("CSettings ERROR: can't load %s (ParseError: %d)", buff, parse_error);
		std::terminate();
		return;
	}

	// client
	size_t length = 0;
	sprintf(buff, "__android_%d%d", rand() % 1000, rand() % 1000);
	length = reader.Get("client", "name", buff).copy(m_Settings.szNickName, 24);
	m_Settings.szNickName[length] = '\0';
	length = reader.Get("client", "password", "").copy(m_Settings.szPassword, MAX_SETTINGS_STRING);
	m_Settings.szPassword[length] = '\0';
	m_Settings.iServerID = reader.GetInteger("client", "serverid", 1);

	// debug
	m_Settings.bDebug = reader.GetBoolean("debug", "debug", false);
	m_Settings.bOnline = reader.GetBoolean("debug", "online", true);

	// gui
	length = reader.Get("gui", "Font", "arial.ttf").copy(m_Settings.szFont, MAX_SETTINGS_STRING);
	m_Settings.szFont[length] = '\0';
	m_Settings.fFontSize = reader.GetReal("gui", "FontSize", 30.0f);
	m_Settings.iFontOutline = reader.GetInteger("gui", "FontOutline", 2);

	// chat
	m_Settings.fChatPosX = reader.GetReal("gui", "ChatPosX", 325.0f);
	m_Settings.fChatPosY = reader.GetReal("gui", "ChatPosY", 25.0f);
	m_Settings.fChatSizeX = reader.GetReal("gui", "ChatSizeX", 1150.0f);
	m_Settings.fChatSizeY = reader.GetReal("gui", "ChatSizeY", 220.0f);
	m_Settings.iChatMaxMessages = reader.GetInteger("gui", "ChatMaxMessages", 6);

	// spawnscreen
	m_Settings.fSpawnScreenPosX = reader.GetReal("gui", "SpawnScreenPosX", 660.0f);
	m_Settings.fSpawnScreenPosY = reader.GetReal("gui", "SpawnScreenPosY", 950.0f);
	m_Settings.fSpawnScreenSizeX = reader.GetReal("gui", "SpawnScreenSizeX", 600.0f);
	m_Settings.fSpawnScreenSizeY = reader.GetReal("gui", "SpawnScreenSizeY", 100.0f);

	// nametags
	m_Settings.fHealthBarWidth = reader.GetReal("gui", "HealthBarWidth", 100.0f);
	m_Settings.fHealthBarHeight = reader.GetReal("gui", "HealthBarHeight", 10.0f);

	// scoreboard
	m_Settings.fScoreBoardSizeX = reader.GetReal("gui", "ScoreBoardSizeX", 846.0f);
	m_Settings.fScoreBoardSizeY = reader.GetReal("gui", "ScoreBoardSizeY", 614.0f);

	// passenger
	m_Settings.bPassengerUseTexture = reader.GetBoolean("gui", "PassengerUseTexture", true);
	m_Settings.fPassengerTextureSize = reader.GetReal("gui", "PassengerTextureSize", 30.0f);
	m_Settings.fPassengerTextureX = reader.GetReal("gui", "PassengerTexturePosX", 120.0f);
	m_Settings.fPassengerTextureY = reader.GetReal("gui", "PassengerTexturePosY", 430.0f);

	m_Settings.iDialog = reader.GetBoolean("gui", "Dialog", true);

	m_Settings.bVoiceChatEnable = reader.GetBoolean("gui", "VoiceChatEnable", true);
	m_Settings.iVoiceChatKey = reader.GetInteger("gui", "VoiceChatKey", 66);
	m_Settings.fVoiceChatSize = reader.GetReal("gui", "VoiceChatSize", 30.0f);
	m_Settings.fVoiceChatPosX = reader.GetReal("gui", "VoiceChatPosX", 1520.0f);
	m_Settings.fVoiceChatPosY = reader.GetReal("gui", "VoiceChatPosY", 480.0f);

	m_Settings.iAndroidKeyboard = reader.GetBoolean("gui", "androidkeyboard", false);
	m_Settings.iFirstPerson = reader.GetBoolean("gui", "firstperson", true);
	m_Settings.iCutout = reader.GetBoolean("gui", "cutout", false);
	m_Settings.iFPSCounter = reader.GetBoolean("gui", "fps", true);
	m_Settings.iFPSCount = reader.GetInteger("gui", "FPSLimit", 60);
	m_Settings.iHPArmourText = reader.GetBoolean("gui", "hparmourtext", false);
	m_Settings.iOutfitGuns = reader.GetBoolean("gui", "outfitguns", false);
	m_Settings.iPCMoney = reader.GetBoolean("gui", "pcmoney", false);
	m_Settings.iRadarRect = reader.GetBoolean("gui", "radarrect", false);
	m_Settings.iSkyBox = reader.GetBoolean("gui", "skybox", false);
	m_Settings.iSnow = reader.GetBoolean("gui", "snow", false);
    m_Settings.iHud = reader.GetBoolean("gui", "hud", false);
	FLog("Settings loaded.");
}

const stSettings& CSettings::GetReadOnly()
{
    return m_Settings;
}

stSettings& CSettings::GetWrite()
{
    return m_Settings;
}