//Hooking.cpp
#include "stdafx.h"

using namespace Memory;
HMODULE _hmoduleDLL;
HANDLE mainFiber;
DWORD wakeAt;

static eGameState* 									m_gameState;
static uint64_t										m_worldPtr;
static BlipList*									m_blipList;
static Hooking::NativeRegistration**				m_registrationTable;
static __int64**                                            m_globalPtr;

/* Start Hooking */

void Hooking::Start(HMODULE hmoduleDLL)
{
	_hmoduleDLL = hmoduleDLL;
	Log::Init(hmoduleDLL);
	FindPatterns();
	if (!InitializeHooks()) Cleanup();
}

/* Hooks */

// Originals
extern "C"
{
	static UINT(WINAPI *orig_ResetWriteWatch)(LPVOID lpBaseAddress, SIZE_T dwRegionSize) = ResetWriteWatch;
}

// Detoured
UINT WINAPI my_ResetWriteWatch(LPVOID lpBaseAddress, SIZE_T dwRegionSize)
{
	Hooking::InitNativeHooK();
	return orig_ResetWriteWatch(lpBaseAddress, dwRegionSize);
}
class Ini
{
private:
	std::string inifile;
public:
	Ini(std::string file)
	{
		this->inifile = file;
	}

	void WriteString(std::string string, std::string app, std::string key)
	{
		WritePrivateProfileStringA(app.c_str(), key.c_str(), string.c_str(), this->inifile.c_str());
	}
	std::string GetString(std::string app, std::string key)
	{
		char buf[100];
		GetPrivateProfileStringA(app.c_str(), key.c_str(), "NULL", buf, 100, this->inifile.c_str());
		return (std::string)buf;
	}

	void WriteInt(int value, std::string app, std::string key)
	{
		WriteString(std::to_string(value), app, key);
	}
	int GetInt(std::string app, std::string key)
	{
		return std::stoi(GetString(app, key));
	}

	void WriteFloat(float value, std::string app, std::string key)
	{
		WriteString(std::to_string(value), app, key);
	}
	float GetFloat(std::string app, std::string key)
	{
		return std::stof(GetString(app, key));
	}
};
// Initialization
BOOL Hooking::InitializeHooks()
{
	BOOL returnVal = TRUE;
	/*Ini *settingsIni = new Ini(".\\AnoymousFiles\\settings.ini");
	string name = settingsIni->GetString("Login", "User");
	string pw = settingsIni->GetString("Login", "Pass");*/
	// Input hook
	/*if (!is_user_authed(name, pw))
	{
		Log::Fatal("Buy The Menu First");
		exit(0);
	}*/
	if (!iHook.Initialize()) {

		Log::Error("Failed to initialize InputHook");
		returnVal = FALSE;
	}

	// init minhook
	if (MH_Initialize() != MH_OK) {
		Log::Error("MinHook failed to initialize");
		returnVal = FALSE;
	}

	// init reset write watch
	if (MH_CreateHook(&ResetWriteWatch, &my_ResetWriteWatch, reinterpret_cast<void**>(&orig_ResetWriteWatch)) != MH_OK || (MH_EnableHook(&ResetWriteWatch) != MH_OK)) {
		Log::Error("Failed to hook ResetWriteWatch");
		returnVal = FALSE;
	}

	return returnVal;
}

/* Native Hook Function  */
template <typename T>
bool Native(DWORD64 hash, LPVOID hookFunction, T** trampoline)
{
	if (*reinterpret_cast<LPVOID*>(trampoline) != NULL)
		return true;
	auto originalFunction = Hooking::GetNativeHandler(hash);
	if (originalFunction != 0) {
		MH_STATUS createHookStatus = MH_CreateHook(originalFunction, hookFunction, reinterpret_cast<LPVOID*>(trampoline));
		if (((createHookStatus == MH_OK) || (createHookStatus == MH_ERROR_ALREADY_CREATED)) && (MH_EnableHook(originalFunction) == MH_OK))
		{
			DEBUGMSG("Hooked Native 0x%#p", hash);
			return true;
		}
	}

	return false;
}

void Hooking::InitNativeHooK()
{
	if (!GetGameState() == GameStatePlaying) return;

	static bool initialized = false;
	if (!initialized)
	{
		HookNatives() ? initialized = true : initialized = false;
	}
}

Hooking::NativeHandler ORIG_GET_FRAME_COUNT = NULL;
void* __cdecl MY_GET_FRAME_COUNT(NativeContext *cxt)
{
	Hooking::onTickInit();
	return cxt;
}

bool Hooking::HookNatives()
{
	return true
	// native hooks
		&& Native(0x812595A0644CE1DE, &MY_GET_FRAME_COUNT, &ORIG_GET_FRAME_COUNT)
;
}

void __stdcall ScriptFunction(LPVOID lpParameter)
{
	if (Script::isInit())
	{
		try
		{
			while (1)
			{
				Script::onTick();
				SwitchToFiber(mainFiber);
			}
		}
		catch (...)
		{
			Log::Fatal("Failed scriptFiber");
		}
	}
	else
	{
		Log::Fatal("Script::isInit");
	}
}

void Hooking::onTickInit()
{
	if (mainFiber == nullptr)
		mainFiber = ConvertThreadToFiber(nullptr);

	if (timeGetTime() < wakeAt)
		return;

	static HANDLE scriptFiber;
	if (scriptFiber)
		SwitchToFiber(scriptFiber);
	else
		scriptFiber = CreateFiber(NULL, ScriptFunction, nullptr);
}

/* Pattern Scanning */

void Hooking::FailPatterns(const char* name, pattern ptn)
{
	Log::Error("finding %s", name);
	Cleanup();
}

void Hooking::FindPatterns()
{
	auto p_gameState =		pattern("83 3D ? ? ? ? ? 8A D9 74 0A");
	auto p_worldPtr =		pattern("48 8B 05 ? ? ? ? 45 ? ? ? ? 48 8B 48 08 48 85 C9 74 07");
	auto p_blipList =		pattern("4C 8D 05 ? ? ? ? 0F B7 C1");
	auto p_nativeTable =	pattern("76 61 49 8B 7A 40 48 8D 0D");
	auto p_gameLogos =		pattern("70 6C 61 74 66 6F 72 6D 3A");
	auto p_gameLegals =		pattern("72 1F E8 ? ? ? ? 8B 0D");
	auto p_modelCheck =		pattern("48 85 C0 0F 84 ? ? ? ? 8B 48 50");
	auto p_modelSpawn =		pattern("48 8B C8 FF 52 30 84 C0 74 05 48");

	char * c_location = nullptr;

	// Executable Base Address
	DEBUGMSG("baseAddr\t\t 0x%p", get_base());

	// Executable End Address
	DEBUGMSG("endAddr\t\t 0x%p", get_base() + get_size());

	// Get game state
	c_location = p_gameState.count(1).get(0).get<char>(2);
	c_location == nullptr ? FailPatterns("gameState", p_gameState) : m_gameState = reinterpret_cast<decltype(m_gameState)>(c_location + *(int32_t*)c_location + 5);

	// Skip game logos
	Memory::putVP<uint8_t>(p_gameLogos.count(1).get(0).get<void>(0), 0xC3);

	// Wait for legals
	DWORD ticks = GetTickCount();
	while (*m_gameState != GameStateLicenseShit || GetTickCount() < ticks + 5000) Sleep(50);

	// Skip game legals
	Memory::nop(p_gameLegals.count(1).get(0).get<void>(0), 2);

	// Get native registration table
	c_location = p_nativeTable.count(1).get(0).get<char>(9);
	c_location == nullptr ? FailPatterns("native registration Table", p_nativeTable) : m_registrationTable = reinterpret_cast<decltype(m_registrationTable)>(c_location + *(int32_t*)c_location + 4);

	// Get world pointer
	c_location = p_worldPtr.count(1).get(0).get<char>(0);
	c_location == nullptr ? FailPatterns("world Pointer", p_worldPtr) : m_worldPtr = reinterpret_cast<uint64_t>(c_location) + *reinterpret_cast<int*>(reinterpret_cast<uint64_t>(c_location) + 3) + 7;

	// Get blip list
	c_location = p_blipList.count(1).get(0).get<char>(0);
	c_location == nullptr ? FailPatterns("blip List", p_blipList) : m_blipList = (BlipList*)(c_location + *reinterpret_cast<int*>(c_location + 3) + 7);

	// Bypass online model requests block
	Memory::nop(p_modelCheck.count(1).get(0).get<void>(0), 24);

	// Bypass is player model allowed to spawn checks
	Memory::nop(p_modelSpawn.count(1).get(0).get<void>(8), 2);

	DEBUGMSG("Initializing natives");
	CrossMapping::initNativeMap();

	// Check if game is ready
	Log::Msg("Checking if game is ready...");
	while (!*m_gameState == GameStatePlaying) {
		Sleep(100);
	}
	Log::Msg("Game ready");
}

Hooking::NativeHandler Hooking::GetNativeHandler(uint64_t origHash) {

	uint64_t newHash = CrossMapping::MapNative(origHash);
	if (newHash == 0) {
		return nullptr;
	}

	NativeRegistration * table = m_registrationTable[newHash & 0xFF];

	for (; table; table = table->nextRegistration)
	{
		for (uint32_t i = 0; i < table->numEntries; i++)
		{
			if (newHash == table->hashes[i])
			{
				return table->handlers[i];
			}

		}
	}

	return nullptr;
}

eGameState Hooking::GetGameState()
{
	return *m_gameState;
}

BlipList* Hooking::GetBlipList()
{
	return m_blipList;
}
void WAIT(DWORD ms)
{
	wakeAt = timeGetTime() + ms;
	SwitchToFiber(mainFiber);
}

/* Clean Up */
void Hooking::Cleanup()
{
	Log::Msg("CleanUp: SudoMod");

	iHook.keyboardHandlerUnregister(OnKeyboardMessage);
	iHook.Remove();
	MH_DisableHook(&ResetWriteWatch);
	MH_Uninitialize();
	FreeLibraryAndExitThread(_hmoduleDLL, 0);
}
__int64** Hooking::getGlobalPtr()
{
	return m_globalPtr;
}