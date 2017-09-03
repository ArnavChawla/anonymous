#include "stdafx.h"
#include "keyboard.h"
#include<xstring>
#include<algorithm>
#include <alg.h>

#include <Psapi.h>
#include <vector>


HINSTANCE _hinstDLL;

struct HashPair
{
	int modelHash;
	int DispHash;
};
std::vector<HashPair> CarHashes;



ScriptTable* scriptTable;
GlobalTable globalTable;
ScriptHeader* cheatController;
ScriptHeader* shopController;


__int64(*GetModelInfo)(int, __int64);//used to extract display names without actually calling natives
int displayNameOffset;//the offset where the display name is stored relative to the address received above changes per game build, so its safer to read the offset from a signature
int(*GetHashKey)(char*, unsigned int);//just use the in game hashing function to save writing my own

									  /*dont even bother trying to understand this function code
									  Its low level ysc assembley code hacked to work with the cheat_controller.ysc script with doesnt significatly change per game update(few globals shifted is about it)
									  void SpawnCarCheck(Hash modelHash, Hash displayHash)
									  {
									  Vehicle Temp;
									  if (GAMEPLAY::_HAS_CHEAT_STRING_JUST_BEEN_ENTERED(displayHash))
									  {
									  if (STREAMING::IS_MODEL_IN_CD_IMAGE(modelHash)) //this isnt strictly required as we check models for validity earlier, but it's best to be triply safe.
									  {
									  while(!STREAMING::HAS_MODEL_LOADED(modelHash))
									  {
									  STREAMING::REQUEST_MODEL(modelHash);
									  SYSTEM::WAIT(0);
									  }
									  Temp = PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID());
									  if (ENTITY::DOES_ENTITY_EXIST(Temp))
									  {
									  VEHICLE::DELETE_VEHICLE(&Temp);
									  }
									  Temp = VEHICLE::CREATE_VEHICLE(modelHash, ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PED::PLAYER_PED_ID(), 0.0f, 4.0f, 1.0f), ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()) + 90f, 0, 1);
									  VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(Temp);
									  STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(modelHash);
									  VEHICLE::SET_VEHICLE_AS_NO_LONGER_NEEDED(&Temp);
									  }
									  }
									  }
									  */
const char *Function = "\x2D\x02\x03\x00\x00\x38\x01\x2C\x05\x00\x1C\x56\x70\x00\x38\x00\x2C\x05\x00\x44\x56\x67\x00\x38\x00\x2C\x05\x00\x25\x06\x56\x0E\x00\x38\x00\x2C\x04\x00\x3D\x6E\x2C\x04\x00\x81\x55\xE8\xFF\x2C\x01\x00\x8F\x2C\x05\x00\x4B\x39\x04\x38\x04\x2C\x05\x00\x83\x56\x06\x00\x37\x04\x2C\x04\x00\x70\x38\x00\x2C\x01\x00\x8F\x77\x7B\x78\x2C\x13\x00\x2A\x2C\x01\x00\x8F\x2C\x05\x00\x4C\x29\x00\x00\xB4\x42\x0E\x6E\x6F\x2C\x1D\x00\x37\x39\x04\x38\x04\x2C\x05\x00\x65\x2B\x38\x00\x2C\x04\x00\x13\x37\x04\x2C\x04\x00\x0A\x2E\x02\x00";

uintptr_t FindPattern(const char *pattern, const char *mask, const char* startAddress, size_t size)
{
	const char* address_end = startAddress + size;
	const auto mask_length = static_cast<size_t>(strlen(mask) - 1);

	for (size_t i = 0; startAddress < address_end; startAddress++)
	{
		if (*startAddress == pattern[i] || mask[i] == '?')
		{
			if (mask[i + 1] == '\0')
			{
				return reinterpret_cast<uintptr_t>(startAddress) - mask_length;
			}

			i++;
		}
		else
		{
			i = 0;
		}
	}

	return 0;
}

uintptr_t FindPattern(const char *pattern, const char *mask)
{
	MODULEINFO module = {};
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &module, sizeof(MODULEINFO));

	return FindPattern(pattern, mask, reinterpret_cast<const char *>(module.lpBaseOfDll), module.SizeOfImage);
}

void EnableCars::FindScriptAddresses()
{

	while (*(__int64*)scriptTable == 0)
	{
		Sleep(100);
	}
	DEBUGMSG("Found script base pointer %llX", (__int64)scriptTable);
	ScriptTableItem* Item = scriptTable->FindScript(0x39DA738B);
	if (Item == NULL)
	{
		FreeLibraryAndExitThread(_hinstDLL, 0);
	}
	while (!Item->IsLoaded())
	{
		Sleep(100);
	}
	shopController = Item->Header;

	Item = scriptTable->FindScript(0xAFD9916D);
	if (Item == NULL)
	{
	
		FreeLibraryAndExitThread(_hinstDLL, 0);
	}
	while (!Item->IsLoaded())
	{
		Sleep(100);
	}
	cheatController = Item->Header;
}

void EnableCars::FindPatterns()
{
	__int64 patternAddr = FindPattern("\x4C\x8D\x05\x00\x00\x00\x00\x4D\x8B\x08\x4D\x85\xC9\x74\x11", "xxx????xxxxxxxx");
	if (!patternAddr)
	{

		FreeLibraryAndExitThread(_hinstDLL, 0);
	}
	globalTable.GlobalBasePtr = (__int64**)(patternAddr + *(int*)(patternAddr + 3) + 7);


	patternAddr = FindPattern("\x48\x03\x15\x00\x00\x00\x00\x4C\x23\xC2\x49\x8B\x08", "xxx????xxxxxx");
	if (!patternAddr)
	{
		
		FreeLibraryAndExitThread(_hinstDLL, 0);
	}
	scriptTable = (ScriptTable*)(patternAddr + *(int*)(patternAddr + 3) + 7);


	patternAddr = FindPattern("\x80\xF9\x05\x75\x08\x48\x05\x00\x00\x00\x00", "xxxxxxx????");
	GetModelInfo = (__int64(*)(int, __int64))(*(int*)(patternAddr - 0x12) + patternAddr - 0x12 + 0x4);
	displayNameOffset = *(int*)(patternAddr + 0x7);
	__int64 getHashKeyPattern = FindPattern("\x48\x8B\x0B\x33\xD2\xE8\x00\x00\x00\x00\x89\x03", "xxxxxx????xx");
	GetHashKey = reinterpret_cast<int(*)(char*, unsigned int)>(*reinterpret_cast<int*>(getHashKeyPattern + 6) + getHashKeyPattern + 10);

	while (!globalTable.IsInitialised())Sleep(100);//Wait for GlobalInitialisation before continuing
	DEBUGMSG("Found global base pointer %llX", (__int64)globalTable.GlobalBasePtr);
}


void EnableCars::Run(HINSTANCE hinstDLL)
{
	_hinstDLL = hinstDLL;
	Log::Init(hinstDLL);
	FindPatterns();
	FindScriptAddresses();
	EnableCarsGlobal();
	FreeLibraryAndExitThread(_hinstDLL, 0);
}


void EnableCars::EnableCarsGlobal()
{
	for (int i = 0; i < shopController->CodePageCount(); i++)
	{
		__int64 sigAddress = FindPattern("\x28\x26\xCE\x6B\x86\x39\x03", "xxxxxxx", (const char*)shopController->GetCodePageAddress(i), shopController->GetCodePageSize(i));
		if (!sigAddress)
		{
			continue;
		}
		DEBUGMSG("Pattern Found in codepage %d at memory address %llX", i, sigAddress);
		int RealCodeOff = (int)(sigAddress - (__int64)shopController->GetCodePageAddress(i) + (i << 14));
		for (int j = 0; j < 2000; j++)
		{
			if (*(int*)shopController->GetCodePositionAddress(RealCodeOff - j) == 0x0008012D)
			{
				int funcOff = *(int*)shopController->GetCodePositionAddress(RealCodeOff - j + 6) & 0xFFFFFF;
				DEBUGMSG("found Function codepage address at %x", funcOff);
				for (int k = 0x5; k < 0x40; k++)
				{
					if ((*(int*)shopController->GetCodePositionAddress(funcOff + k) & 0xFFFFFF) == 0x01002E)
					{
						for (k = k + 1; k < 30; k++)
						{
							if (*(unsigned char*)shopController->GetCodePositionAddress(funcOff + k) == 0x5F)
							{
								int globalindex = *(int*)shopController->GetCodePositionAddress(funcOff + k + 1) & 0xFFFFFF;
								DEBUGMSG("Found Global Variable %d, address = %llX", globalindex, (__int64)globalTable.AddressOf(globalindex));
								
								*globalTable.AddressOf(globalindex) = 1;
								
								FindSwitch(RealCodeOff - j);
								PatchCheatController();
								return;
							}
						}
						break;
					}
				}
				break;
			}
		}
		break;
	}
	
}

void EnableCars::FindSwitch(int funcCallOffset)
{
	for (int i = 14; i<400; i++)
	{
		if (*(unsigned char*)shopController->GetCodePositionAddress(funcCallOffset + i) == 0x62)
		{
			FindAffectedCars(funcCallOffset + i);
			return;
		}
	}
	//Log::Msg("Couldnt find affected cars");
}

void EnableCars::FindAffectedCars(int scriptSwitchOffset)
{
	__int64 curAddress = (__int64)shopController->GetCodePositionAddress(scriptSwitchOffset + 2);
	int startOff = scriptSwitchOffset + 2;
	int cases = *(unsigned char*)(curAddress - 1);
	for (int i = 0; i<cases; i++)
	{
		curAddress += 6;
		startOff += 6;
		int jumpoff = *(short*)(curAddress - 2);
		__int64 caseOff = (__int64)shopController->GetCodePositionAddress(startOff + jumpoff);
		unsigned char opCode = *(unsigned char*)caseOff;
		int hash;
		if (opCode == 0x28)//push int
		{
			hash = *(int*)(caseOff + 1);
		}
		else if (opCode == 0x61)//push int 24
		{
			hash = *(int*)(caseOff + 1) & 0xFFFFFF;
		}
		else
		{
			//Log::Msg("Error with car hash %X", *(int*)(caseOff + 1));// in reality this should never be executed
			continue;
		}
		char* displayName = GetDisplayName(hash);
	//	Log::Msg("Found affected hash: 0x%08X - DisplayName = %s", hash, displayName);
		HashPair h;
		h.DispHash = GetHashKey(displayName, 0);
		h.modelHash = hash;
		CarHashes.push_back(h);
	}
}
char* EnableCars::GetDisplayName(int modelHash)
{
	int data = 0xFFFF;
	__int64 addr = GetModelInfo(modelHash, (__int64)&data);
	if (addr && (*(unsigned char*)(addr + 157) & 0x1F) == 5)//make sure model is valid and is a car
	{
		return (char*)(addr + displayNameOffset);
	}
	return "CARNOTFOUND";
}

void EnableCars::PatchCheatController()
{
	if (CarHashes.size() == 0)
	{
		return;//no car hashes found
	}
	//Log::Msg("Applying patch to enable car spawning");
	__int64 StartOff = ((__int64)(cheatController->localOffset + cheatController->localCount) + 15) & 0xFFFFFFFFFFFFFFF0;
	DEBUGMSG("Cheat controller Loaded, CodeLength = %d", cheatController->codeLength);

	int *setOffset = NULL;
	for (int i = 0; i < cheatController->codeLength; i++)
	{
		if (*(__int64*)cheatController->GetCodePositionAddress(i) == 0x3C6E5C3C6E00002E)//searching for the address of code in the ysc where we will hook our new function
		{
			setOffset = (int*)cheatController->GetCodePositionAddress(i + 3);//the address points to the code for  {iLocal_92 = 0; iLocal_93 = 0;} this code will get overwritten to hook into our new function that checks the new cheats we will add in
			DEBUGMSG("Found cheat controller hook offset: %X", setOffset);
			break;
		}
	}
	if (!setOffset)
	{
	//	Log::Msg("Error applying patch, car spawning disabled"); //this will be thrown if this asi gets injected twice for whatever reason
		return;
	}

	//this is probably very dangerous code as were extending the code table by just adding a new page, meaning other data in the ysc will be able to be though of as being part of the code table
	//however control flow will never reach that undefined part of the code table which is why this doesnt cause any error
	DEBUGMSG("Extending code table");
	cheatController->codeBlocksOffset[1] = (unsigned char*)StartOff;
	cheatController->codeLength = 0x408E + 14 * (int)CarHashes.size();
	DEBUGMSG("CodeTable Length = %X", cheatController->codeLength);
	DEBUGMSG("Starting offset = %llX", StartOff);
	memcpy((void*)StartOff, (void*)Function, 129);//function takes model hash and display hash, returns void
	DEBUGMSG("Copied new function");
	unsigned char* cur = (unsigned char*)((__int64)StartOff + 129);
	memcpy((void*)cur, (void*)"\x2D\x00\x02\x00\x00", 5);//function for adding the new cheats and fixing patch, no params, no returns
	cur += 5;
	//our hook overrides the {iLocal_92 = 0; iLocal_93 = 0;} code from above, so we need to copy that code into the start of our new hooked function to ensure the cheat controller behaves normally
	memcpy((void*)cur, setOffset, 6);
	cur += 6;
	for (size_t i = 0, max = CarHashes.size(); i<max; i++)
	{
		*cur++ = 0x28;
		*(int*)cur = CarHashes[i].modelHash;//push model hash
		cur += 4;
		*cur++ = 0x28;
		*(int*)cur = CarHashes[i].DispHash;//push display hash
		cur += 4;
		*(int*)cur = 0x0040005D;//call our new function for checking if cheat entered, then spawning car
		cur += 4;
	}

	*(int*)cur = 0x0000002E;//return statement, strinctly only needs to be 2E 00 00, but the extra makes the copy faster and that last 00 is just a nop so nothing will change.

	DEBUGMSG("Patching new function");
	//This is where we patch in the hook, its safest to do this last, just on the off chance the cheat controller tries to execute the new function before we have finished writing it (due to running on a different thread).
	memcpy((void*)setOffset, (void*)"\x5D\x80\x40\x00\x00\x00", 6);
//	Log::Msg("Success, spawn the cars using their display name as a cheatcode (press ` in game)");

}

typedef struct { int R, G, B, A; } RGBA;

/* Models */
uint RequestedModel = 0ul;
RequestState model_state = loaded;
void(*modelFunction)() = nullptr;
typedef struct {
	Player p;
	bool hasCameraFlash;
	bool hasBlood;
	bool hasPinkBurst;
	bool hasElectric;
	bool hasCoolLights;
	bool hasMoneyRain;
	bool hasBigSmoke;
}ParticleFXSet;
ParticleFXSet MainPFX;
void modelCheck()
{
	switch (model_state)
	{
	case loaded:
		break;
	case requesting:
		if (STREAMING::HAS_MODEL_LOADED(RequestedModel) == TRUE)
		{
			model_state = loaded;

			modelFunction();

			break;
		}
		else
		{
			STREAMING::REQUEST_MODEL(RequestedModel);
			break;
		}
	}
}

bool Script::isInit()
{
	// Initialize some stuff here
	bool returnVal = true;

	return returnVal;
}

#pragma warning(disable : 4244 4305) // double <-> float conversions

namespace SUB
{
	enum
	{
		// Define submenu (enum) indices here.
		CLOSED,
		MAINMENU,
		SETTINGS,
		SETTINGS_COLOURS,
		SETTINGS_COLOURS2,
		SETTINGS_FONTS,
		SETTINGS_FONTS2,
		// Others:
		XYAYSDKKHAJDK,
		SUMG,
		VEHICLEWEAPS,
		BULLET,
		ANIMAL,
		SPEEDLIMIT,
		OUTFIT,
		PEDCOMPGLASSES,
		PEDCOMP1,
		PEDCOMP2,
		PEDCOMP3,
		PEDCOMP4,
		PEDCOMP11,
		PEDCOMP6,
		PEDCOMP5,
		PEDCOMP7,
		PEDCOMP8,
		PEDCOMP9,
		ESP,	
		GAMEPAINT,
		METALLIC,
		CLASSIC,
		FUNNYVEHICLES,
		NPCSPAWNER,
		BODYGUARDSPAWNER,
		OPEDSPAWNER,
		OBJECTSPAWNERM,
		LISTPRESETS,
		LISTCUSTOMMAPMODS,
		PEARLESCENT,
		MAPMOD_MAZEDEMO,
		MAPMOD_MAZEROOFRAMP,
		MAPMOD_BEACHFERRISRAMP,
		MAPMOD_MOUNTCHILLIADRAMP,
		MAPMOD_AIRPORTMINIRAMP,
		MAPMOD_AIRPORTGATERAMP,
		MATTE,
		METALS,
		MAPMOD_UFOTOWER,
		MAPMOD_MAZEBANKRAMPS,
		MAPMOD_FREESTYLEMOTOCROSS,
		SMETALLIC,
		MAPMOD_HALFPIPEFUNTRACK,
		MAPMOD_AIRPORTLOOP,
		MAPMOD_MAZEBANKMEGARAMP,
		SCLASSIC,
		SPEARLESCENT,
		SMATTE,
		SMETALS,
		SPAINT,
		BENNYS,
		FRAME,
		HORNS,
		PPAINT,
		
		WOFFROAD,
		WSPORT,
		WMUSCLE,
		WLOWRIDER,
		WSUV,
		WHEEL,
		WHIGHEND,
		WTUNER,
		WBIKE,
		WBENNY,
		CHANGEWHEELS,
		ARMOR,
		WHEELCOLOR,
		VEHICLE_PAINT,
		ENGINE,
		ROOF,
		SPOILER,
		WTINT,
		RIGHTFENDER,
		REARBUMPER,
		SIDESKIRT,
		EXHAUST,
		FRMAE,
		GRILLE,
		FENDER,
		NEON,
		VEHICLEEXTRA,
		VEHMULTIPLIERS,
		WHEELSMOKE,
		FRONTBUMPER,
		HOOD,
		BRAKES,
		SUSPENSION,
		TRANSMISSION,
		VEHICLE_LSC2,
		OTHER,//done
		HIGH,//done
		SINGLE,
		LANDMARKS,
		SEA,//done
		MODDED,//dpne
		STORES,//done
		OPLAVEHOPTIONS,
		OATTACHMENTOPTIONS,
		OPLAYEROPTIONS,
		BLAMESELP,
		OPLAYERATTACKERS,
		SELPLAYERPTFX,
		OPLAYERDROPOPTIONS,
		GUNRUNNING,
		SAMPLE,
		YOURSUB,
		SELFMODOPTIONS,
		NAMECHANGER,
		TELEPORTLOCATIONS,
		ONLINEPLAYERS,
		ALLPLAYEROPTIONS,
		WEAPONSMENU,
		VEHICLE_SPAWNER,
		VEHICLEMODSA,
		FORCESPAWNER,
		PARTICLEFX,
		FORCEGUN,
		ROBBERYSPREED,
		FAVORITE_VEV,
		SUPERCAR_VEV,
		LOWRIDER_VEV,
		MUSCLE_VEV,
		OFF_ROADS_VEV,
		SPORTS_VEV,
		SPORTS_CLASSICS_VEV,
		PLANES_VEV,
		SUVS_VEV,
		SEDANS_VEV,
		COMPACT_VEV,
		COUPES_VEV,
		EMERGENCY_VEV,
		MILITARY_VEV,
		VANS_VEV,
		MOTORCYCLES_VEV,
		HELICOPTERS_VEV,
		SERVICES_VEV,
		INDUSTRIAL_VEV,
		BOATS_VEV,
		CYCLES_VEV,
		UTILITY_VEV,
		SAVEDVEHICLES,
		CUSTOMCHARAC,
		SKINCHANGER,
		PANIMATIONMENU,
		GTASCENIC,
		CLIPSETS,
		PEDCOMP10,
		CLEARAERA,
		MAPMOD,
		CUTSCENES,
		RECOVERYOPTIONS,
		VISIONFX,
		SCREENFX,
		IPLOCATIONS,
		ALPHA,
		SELECTEDPLAYER,
			TRUNK,
			CUSTOMPLATE,
			PLATEHOLDER,
			DECALS,
			INTERIORCOATING,
			DECORATIVESHIT,
			SPEEDOMETERS,
			STEERING,
			SHIFTER,
			DECORATIVEPLATES,
			HYDRAULICPUMP,
			MOTORBLOCK,
			AIRFILTER,
			TANK,
			DASHLIGHT,
			DASHCOLOR,
			WHEELSTRIPE,
	};
};

// Declare/Initialise global variables here.
namespace
{
	char str[69];
	bool controllerinput_bool = 1, menujustopened = 1, null;
	enum ControllerInputs
	{
		INPUT_NEXT_CAMERA = 0,
		INPUT_LOOK_LR = 1,
		INPUT_LOOK_UD = 2,
		INPUT_LOOK_UP_ONLY = 3,
		INPUT_LOOK_DOWN_ONLY = 4,
		INPUT_LOOK_LEFT_ONLY = 5,
		INPUT_LOOK_RIGHT_ONLY = 6,
		INPUT_CINEMATIC_SLOWMO = 7,
		INPUT_SCRIPTED_FLY_UD = 8,
		INPUT_SCRIPTED_FLY_LR = 9,
		INPUT_SCRIPTED_FLY_ZUP = 10,
		INPUT_SCRIPTED_FLY_ZDOWN = 11,
		INPUT_WEAPON_WHEEL_UD = 12,
		INPUT_WEAPON_WHEEL_LR = 13,
		INPUT_WEAPON_WHEEL_NEXT = 14,
		INPUT_WEAPON_WHEEL_PREV = 15,
		INPUT_SELECT_NEXT_WEAPON = 16,
		INPUT_SELECT_PREV_WEAPON = 17,
		INPUT_SKIP_CUTSCENE = 18,
		INPUT_CHARACTER_WHEEL = 19,
		INPUT_MULTIPLAYER_INFO = 20,
		INPUT_SPRINT = 21,
		INPUT_JUMP = 22,
		INPUT_ENTER = 23,
		INPUT_ATTACK = 24,
		INPUT_AIM = 25,
		INPUT_LOOK_BEHIND = 26,
		INPUT_PHONE = 27,
		INPUT_SPECIAL_ABILITY = 28,
		INPUT_SPECIAL_ABILITY_SECONDARY = 29,
		INPUT_MOVE_LR = 30,
		INPUT_MOVE_UD = 31,
		INPUT_MOVE_UP_ONLY = 32,
		INPUT_MOVE_DOWN_ONLY = 33,
		INPUT_MOVE_LEFT_ONLY = 34,
		INPUT_MOVE_RIGHT_ONLY = 35,
		INPUT_DUCK = 36,
		INPUT_SELECT_WEAPON = 37,
		INPUT_PICKUP = 38,
		INPUT_SNIPER_ZOOM = 39,
		INPUT_SNIPER_ZOOM_IN_ONLY = 40,
		INPUT_SNIPER_ZOOM_OUT_ONLY = 41,
		INPUT_SNIPER_ZOOM_IN_SECONDARY = 42,
		INPUT_SNIPER_ZOOM_OUT_SECONDARY = 43,
		INPUT_COVER = 44,
		INPUT_RELOAD = 45,
		INPUT_TALK = 46,
		INPUT_DETONATE = 47,
		INPUT_HUD_SPECIAL = 48,
		INPUT_ARREST = 49,
		INPUT_ACCURATE_AIM = 50,
		INPUT_CONTEXT = 51,
		INPUT_CONTEXT_SECONDARY = 52,
		INPUT_WEAPON_SPECIAL = 53,
		INPUT_WEAPON_SPECIAL_TWO = 54,
		INPUT_DIVE = 55,
		INPUT_DROP_WEAPON = 56,
		INPUT_DROP_AMMO = 57,
		INPUT_THROW_GRENADE = 58,
		INPUT_VEH_MOVE_LR = 59,
		INPUT_VEH_MOVE_UD = 60,
		INPUT_VEH_MOVE_UP_ONLY = 61,
		INPUT_VEH_MOVE_DOWN_ONLY = 62,
		INPUT_VEH_MOVE_LEFT_ONLY = 63,
		INPUT_VEH_MOVE_RIGHT_ONLY = 64,
		INPUT_VEH_SPECIAL = 65,
		INPUT_VEH_GUN_LR = 66,
		INPUT_VEH_GUN_UD = 67,
		INPUT_VEH_AIM = 68,
		INPUT_VEH_ATTACK = 69,
		INPUT_VEH_ATTACK2 = 70,
		INPUT_VEH_ACCELERATE = 71,
		INPUT_VEH_BRAKE = 72,
		INPUT_VEH_DUCK = 73,
		INPUT_VEH_HEADLIGHT = 74,
		INPUT_VEH_EXIT = 75,
		INPUT_VEH_HANDBRAKE = 76,
		INPUT_VEH_HOTWIRE_LEFT = 77,
		INPUT_VEH_HOTWIRE_RIGHT = 78,
		INPUT_VEH_LOOK_BEHIND = 79,
		INPUT_VEH_CIN_CAM = 80,
		INPUT_VEH_NEXT_RADIO = 81,
		INPUT_VEH_PREV_RADIO = 82,
		INPUT_VEH_NEXT_RADIO_TRACK = 83,
		INPUT_VEH_PREV_RADIO_TRACK = 84,
		INPUT_VEH_RADIO_WHEEL = 85,
		INPUT_VEH_HORN = 86,
		INPUT_VEH_FLY_THROTTLE_UP = 87,
		INPUT_VEH_FLY_THROTTLE_DOWN = 88,
		INPUT_VEH_FLY_YAW_LEFT = 89,
		INPUT_VEH_FLY_YAW_RIGHT = 90,
		INPUT_VEH_PASSENGER_AIM = 91,
		INPUT_VEH_PASSENGER_ATTACK = 92,
		INPUT_VEH_SPECIAL_ABILITY_FRANKLIN = 93,
		INPUT_VEH_STUNT_UD = 94,
		INPUT_VEH_CINEMATIC_UD = 95,
		INPUT_VEH_CINEMATIC_UP_ONLY = 96,
		INPUT_VEH_CINEMATIC_DOWN_ONLY = 97,
		INPUT_VEH_CINEMATIC_LR = 98,
		INPUT_VEH_SELECT_NEXT_WEAPON = 99,
		INPUT_VEH_SELECT_PREV_WEAPON = 100,
		INPUT_VEH_ROOF = 101,
		INPUT_VEH_JUMP = 102,
		INPUT_VEH_GRAPPLING_HOOK = 103,
		INPUT_VEH_SHUFFLE = 104,
		INPUT_VEH_DROP_PROJECTILE = 105,
		INPUT_VEH_MOUSE_CONTROL_OVERRIDE = 106,
		INPUT_VEH_FLY_ROLL_LR = 107,
		INPUT_VEH_FLY_ROLL_LEFT_ONLY = 108,
		INPUT_VEH_FLY_ROLL_RIGHT_ONLY = 109,
		INPUT_VEH_FLY_PITCH_UD = 110,
		INPUT_VEH_FLY_PITCH_UP_ONLY = 111,
		INPUT_VEH_FLY_PITCH_DOWN_ONLY = 112,
		INPUT_VEH_FLY_UNDERCARRIAGE = 113,
		INPUT_VEH_FLY_ATTACK = 114,
		INPUT_VEH_FLY_SELECT_NEXT_WEAPON = 115,
		INPUT_VEH_FLY_SELECT_PREV_WEAPON = 116,
		INPUT_VEH_FLY_SELECT_TARGET_LEFT = 117,
		INPUT_VEH_FLY_SELECT_TARGET_RIGHT = 118,
		INPUT_VEH_FLY_VERTICAL_FLIGHT_MODE = 119,
		INPUT_VEH_FLY_DUCK = 120,
		INPUT_VEH_FLY_ATTACK_CAMERA = 121,
		INPUT_VEH_FLY_MOUSE_CONTROL_OVERRIDE = 122,
		INPUT_VEH_SUB_TURN_LR = 123,
		INPUT_VEH_SUB_TURN_LEFT_ONLY = 124,
		INPUT_VEH_SUB_TURN_RIGHT_ONLY = 125,
		INPUT_VEH_SUB_PITCH_UD = 126,
		INPUT_VEH_SUB_PITCH_UP_ONLY = 127,
		INPUT_VEH_SUB_PITCH_DOWN_ONLY = 128,
		INPUT_VEH_SUB_THROTTLE_UP = 129,
		INPUT_VEH_SUB_THROTTLE_DOWN = 130,
		INPUT_VEH_SUB_ASCEND = 131,
		INPUT_VEH_SUB_DESCEND = 132,
		INPUT_VEH_SUB_TURN_HARD_LEFT = 133,
		INPUT_VEH_SUB_TURN_HARD_RIGHT = 134,
		INPUT_VEH_SUB_MOUSE_CONTROL_OVERRIDE = 135,
		INPUT_VEH_PUSHBIKE_PEDAL = 136,
		INPUT_VEH_PUSHBIKE_SPRINT = 137,
		INPUT_VEH_PUSHBIKE_FRONT_BRAKE = 138,
		INPUT_VEH_PUSHBIKE_REAR_BRAKE = 139,
		INPUT_MELEE_ATTACK_LIGHT = 140,
		INPUT_MELEE_ATTACK_HEAVY = 141,
		INPUT_MELEE_ATTACK_ALTERNATE = 142,
		INPUT_MELEE_BLOCK = 143,
		INPUT_PARACHUTE_DEPLOY = 144,
		INPUT_PARACHUTE_DETACH = 145,
		INPUT_PARACHUTE_TURN_LR = 146,
		INPUT_PARACHUTE_TURN_LEFT_ONLY = 147,
		INPUT_PARACHUTE_TURN_RIGHT_ONLY = 148,
		INPUT_PARACHUTE_PITCH_UD = 149,
		INPUT_PARACHUTE_PITCH_UP_ONLY = 150,
		INPUT_PARACHUTE_PITCH_DOWN_ONLY = 151,
		INPUT_PARACHUTE_BRAKE_LEFT = 152,
		INPUT_PARACHUTE_BRAKE_RIGHT = 153,
		INPUT_PARACHUTE_SMOKE = 154,
		INPUT_PARACHUTE_PRECISION_LANDING = 155,
		INPUT_MAP = 156,
		INPUT_SELECT_WEAPON_UNARMED = 157,
		INPUT_SELECT_WEAPON_MELEE = 158,
		INPUT_SELECT_WEAPON_HANDGUN = 159,
		INPUT_SELECT_WEAPON_SHOTGUN = 160,
		INPUT_SELECT_WEAPON_SMG = 161,
		INPUT_SELECT_WEAPON_AUTO_RIFLE = 162,
		INPUT_SELECT_WEAPON_SNIPER = 163,
		INPUT_SELECT_WEAPON_HEAVY = 164,
		INPUT_SELECT_WEAPON_SPECIAL = 165,
		INPUT_SELECT_CHARACTER_MICHAEL = 166,
		INPUT_SELECT_CHARACTER_FRANKLIN = 167,
		INPUT_SELECT_CHARACTER_TREVOR = 168,
		INPUT_SELECT_CHARACTER_MULTIPLAYER = 169,
		INPUT_SAVE_REPLAY_CLIP = 170,
		INPUT_SPECIAL_ABILITY_PC = 171,
		INPUT_CELLPHONE_UP = 172,
		INPUT_CELLPHONE_DOWN = 173,
		INPUT_CELLPHONE_LEFT = 174,
		INPUT_CELLPHONE_RIGHT = 175,
		INPUT_CELLPHONE_SELECT = 176,
		INPUT_CELLPHONE_CANCEL = 177,
		INPUT_CELLPHONE_OPTION = 178,
		INPUT_CELLPHONE_EXTRA_OPTION = 179,
		INPUT_CELLPHONE_SCROLL_FORWARD = 180,
		INPUT_CELLPHONE_SCROLL_BACKWARD = 181,
		INPUT_CELLPHONE_CAMERA_FOCUS_LOCK = 182,
		INPUT_CELLPHONE_CAMERA_GRID = 183,
		INPUT_CELLPHONE_CAMERA_SELFIE = 184,
		INPUT_CELLPHONE_CAMERA_DOF = 185,
		INPUT_CELLPHONE_CAMERA_EXPRESSION = 186,
		INPUT_FRONTEND_DOWN = 187,
		INPUT_FRONTEND_UP = 188,
		INPUT_FRONTEND_LEFT = 189,
		INPUT_FRONTEND_RIGHT = 190,
		INPUT_FRONTEND_RDOWN = 191,
		INPUT_FRONTEND_RUP = 192,
		INPUT_FRONTEND_RLEFT = 193,
		INPUT_FRONTEND_RRIGHT = 194,
		INPUT_FRONTEND_AXIS_X = 195,
		INPUT_FRONTEND_AXIS_Y = 196,
		INPUT_FRONTEND_RIGHT_AXIS_X = 197,
		INPUT_FRONTEND_RIGHT_AXIS_Y = 198,
		INPUT_FRONTEND_PAUSE = 199,
		INPUT_FRONTEND_PAUSE_ALTERNATE = 200,
		INPUT_FRONTEND_ACCEPT = 201,
		INPUT_FRONTEND_CANCEL = 202,
		INPUT_FRONTEND_X = 203,
		INPUT_FRONTEND_Y = 204,
		INPUT_FRONTEND_LB = 205,
		INPUT_FRONTEND_RB = 206,
		INPUT_FRONTEND_LT = 207,
		INPUT_FRONTEND_RT = 208,
		INPUT_FRONTEND_LS = 209,
		INPUT_FRONTEND_RS = 210,
		INPUT_FRONTEND_LEADERBOARD = 211,
		INPUT_FRONTEND_SOCIAL_CLUB = 212,
		INPUT_FRONTEND_SOCIAL_CLUB_SECONDARY = 213,
		INPUT_FRONTEND_DELETE = 214,
		INPUT_FRONTEND_ENDSCREEN_ACCEPT = 215,
		INPUT_FRONTEND_ENDSCREEN_EXPAND = 216,
		INPUT_FRONTEND_SELECT = 217,
		INPUT_SCRIPT_LEFT_AXIS_X = 218,
		INPUT_SCRIPT_LEFT_AXIS_Y = 219,
		INPUT_SCRIPT_RIGHT_AXIS_X = 220,
		INPUT_SCRIPT_RIGHT_AXIS_Y = 221,
		INPUT_SCRIPT_RUP = 222,
		INPUT_SCRIPT_RDOWN = 223,
		INPUT_SCRIPT_RLEFT = 224,
		INPUT_SCRIPT_RRIGHT = 225,
		INPUT_SCRIPT_LB = 226,
		INPUT_SCRIPT_RB = 227,
		INPUT_SCRIPT_LT = 228,
		INPUT_SCRIPT_RT = 229,
		INPUT_SCRIPT_LS = 230,
		INPUT_SCRIPT_RS = 231,
		INPUT_SCRIPT_PAD_UP = 232,
		INPUT_SCRIPT_PAD_DOWN = 233,
		INPUT_SCRIPT_PAD_LEFT = 234,
		INPUT_SCRIPT_PAD_RIGHT = 235,
		INPUT_SCRIPT_SELECT = 236,
		INPUT_CURSOR_ACCEPT = 237,
		INPUT_CURSOR_CANCEL = 238,
		INPUT_CURSOR_X = 239,
		INPUT_CURSOR_Y = 240,
		INPUT_CURSOR_SCROLL_UP = 241,
		INPUT_CURSOR_SCROLL_DOWN = 242,
		INPUT_ENTER_CHEAT_CODE = 243,
		INPUT_INTERACTION_MENU = 244,
		INPUT_MP_TEXT_CHAT_ALL = 245,
		INPUT_MP_TEXT_CHAT_TEAM = 246,
		INPUT_MP_TEXT_CHAT_FRIENDS = 247,
		INPUT_MP_TEXT_CHAT_CREW = 248,
		INPUT_PUSH_TO_TALK = 249,
		INPUT_CREATOR_LS = 250,
		INPUT_CREATOR_RS = 251,
		INPUT_CREATOR_LT = 252,
		INPUT_CREATOR_RT = 253,
		INPUT_CREATOR_MENU_TOGGLE = 254,
		INPUT_CREATOR_ACCEPT = 255,
		INPUT_CREATOR_DELETE = 256,
		INPUT_ATTACK2 = 257,
		INPUT_RAPPEL_JUMP = 258,
		INPUT_RAPPEL_LONG_JUMP = 259,
		INPUT_RAPPEL_SMASH_WINDOW = 260,
		INPUT_PREV_WEAPON = 261,
		INPUT_NEXT_WEAPON = 262,
		INPUT_MELEE_ATTACK1 = 263,
		INPUT_MELEE_ATTACK2 = 264,
		INPUT_WHISTLE = 265,
		INPUT_MOVE_LEFT = 266,
		INPUT_MOVE_RIGHT = 267,
		INPUT_MOVE_UP = 268,
		INPUT_MOVE_DOWN = 269,
		INPUT_LOOK_LEFT = 270,
		INPUT_LOOK_RIGHT = 271,
		INPUT_LOOK_UP = 272,
		INPUT_LOOK_DOWN = 273,
		INPUT_SNIPER_ZOOM_IN = 274,
		INPUT_SNIPER_ZOOM_OUT = 275,
		INPUT_SNIPER_ZOOM_IN_ALTERNATE = 276,
		INPUT_SNIPER_ZOOM_OUT_ALTERNATE = 277,
		INPUT_VEH_MOVE_LEFT = 278,
		INPUT_VEH_MOVE_RIGHT = 279,
		INPUT_VEH_MOVE_UP = 280,
		INPUT_VEH_MOVE_DOWN = 281,
		INPUT_VEH_GUN_LEFT = 282,
		INPUT_VEH_GUN_RIGHT = 283,
		INPUT_VEH_GUN_UP = 284,
		INPUT_VEH_GUN_DOWN = 285,
		INPUT_VEH_LOOK_LEFT = 286,
		INPUT_VEH_LOOK_RIGHT = 287,
		INPUT_REPLAY_START_STOP_RECORDING = 288,
		INPUT_REPLAY_START_STOP_RECORDING_SECONDARY = 289,
		INPUT_SCALED_LOOK_LR = 290,
		INPUT_SCALED_LOOK_UD = 291,
		INPUT_SCALED_LOOK_UP_ONLY = 292,
		INPUT_SCALED_LOOK_DOWN_ONLY = 293,
		INPUT_SCALED_LOOK_LEFT_ONLY = 294,
		INPUT_SCALED_LOOK_RIGHT_ONLY = 295,
		INPUT_REPLAY_MARKER_DELETE = 296,
		INPUT_REPLAY_CLIP_DELETE = 297,
		INPUT_REPLAY_PAUSE = 298,
		INPUT_REPLAY_REWIND = 299,
		INPUT_REPLAY_FFWD = 300,
		INPUT_REPLAY_NEWMARKER = 301,
		INPUT_REPLAY_RECORD = 302,
		INPUT_REPLAY_SCREENSHOT = 303,
		INPUT_REPLAY_HIDEHUD = 304,
		INPUT_REPLAY_STARTPOINT = 305,
		INPUT_REPLAY_ENDPOINT = 306,
		INPUT_REPLAY_ADVANCE = 307,
		INPUT_REPLAY_BACK = 308,
		INPUT_REPLAY_TOOLS = 309,
		INPUT_REPLAY_RESTART = 310,
		INPUT_REPLAY_SHOWHOTKEY = 311,
		INPUT_REPLAY_CYCLEMARKERLEFT = 312,
		INPUT_REPLAY_CYCLEMARKERRIGHT = 313,
		INPUT_REPLAY_FOVINCREASE = 314,
		INPUT_REPLAY_FOVDECREASE = 315,
		INPUT_REPLAY_CAMERAUP = 316,
		INPUT_REPLAY_CAMERADOWN = 317,
		INPUT_REPLAY_SAVE = 318,
		INPUT_REPLAY_TOGGLETIME = 319,
		INPUT_REPLAY_TOGGLETIPS = 320,
		INPUT_REPLAY_PREVIEW = 321,
		INPUT_REPLAY_TOGGLE_TIMELINE = 322,
		INPUT_REPLAY_TIMELINE_PICKUP_CLIP = 323,
		INPUT_REPLAY_TIMELINE_DUPLICATE_CLIP = 324,
		INPUT_REPLAY_TIMELINE_PLACE_CLIP = 325,
		INPUT_REPLAY_CTRL = 326,
		INPUT_REPLAY_TIMELINE_SAVE = 327,
		INPUT_REPLAY_PREVIEW_AUDIO = 328,
		INPUT_VEH_DRIVE_LOOK = 329,
		INPUT_VEH_DRIVE_LOOK2 = 330,
		INPUT_VEH_FLY_ATTACK2 = 331,
		INPUT_RADIO_WHEEL_UD = 332,
		INPUT_RADIO_WHEEL_LR = 333,
		INPUT_VEH_SLOWMO_UD = 334,
		INPUT_VEH_SLOWMO_UP_ONLY = 335,
		INPUT_VEH_SLOWMO_DOWN_ONLY = 336,
		INPUT_MAP_POI = 337
	};
}
void set_xmasgs()
{
	GAMEPLAY::SET_OVERRIDE_WEATHER("XMAS");
	STREAMING::REQUEST_NAMED_PTFX_ASSET("core_snow"); //0xCFEA19A9
	if (!STREAMING::HAS_NAMED_PTFX_ASSET_LOADED("core_snow")) //0x9ACC6446
		STREAMING::REQUEST_NAMED_PTFX_ASSET("core_snow"); //0xCFEA19A9

}
void set_msinvisible()
{
	Player player = PLAYER::PLAYER_ID();
	ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), false, 0);

}
void set_msNeverWanted()
{
	Player player = PLAYER::PLAYER_ID();
	PLAYER::CLEAR_PLAYER_WANTED_LEVEL(player);

}
int *settings_font, inull;
RGBA *settings_rgba;
RGBA titlebox = { 0, 0, 0, 255 };
RGBA BG = { 20, 20, 20, 200 };
RGBA titletext = { 255, 255, 255, 255 };
RGBA optiontext = { 255,255,255, 255 };
RGBA optioncount = { 255, 255, 255, 255 };
RGBA selectedtext = { 255, 255, 255, 255 };
RGBA optionbreaks = { 255, 255, 255, 240 };
RGBA selectionhi = { 255, 255, 255, 140 };
int font_title = 7, font_options = 4, font_selection = 4, font_breaks = 1;
float menuPos = 0, OptionY;
int screen_res_x, screen_res_y;
DWORD myVeh, cam_gta2;
float current_timescale = 1.0f;
bool DrawnWarningOnce = 1;
int WakeAt1 = 0;
void drawNoBankingWarning() {


	if (DrawnWarningOnce) {
		if (WakeAt1 == 0) WakeAt1 = timeGetTime() + 2000;
		/*if (timeGetTime() <= WakeAt1) {
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.50, 0.50);
		UI::SET_TEXT_COLOUR(255, 255, 255, 255);
		UI::SET_TEXT_WRAP(0.0, 1.0);
		UI::SET_TEXT_CENTRE(1);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
		UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
		UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
		UI::_ADD_TEXT_COMPONENT_STRING("~r~IF YOU BANK THE CASH GET FUCKED BY TRUMP :)");
		UI::END_TEXT_COMMAND_DISPLAY_TEXT(0.5, 0.15);
		}*/
		else DrawnWarningOnce = 0;
	}
}



// Booleans for loops go here:
DWORD VehicleShootLastTime = 0;
bool loop_massacre_mode = 0, showfps = 0, loop_gravity_gun = 0, xmasgs = 0, coollights01 = 0, moneyrain01 = 0, bigsmoke01 = 0, cameraflash01 = 0,
automax = 0, noclip = 0, laghimout = 0, theleecher = 0, rapidfire = 0, speedLimiter = 0, analog_loop = 0, loop_SuperGrip = 0, msCAR_invisible = 0,
theForceA = 0, awhostalking = 1, amiHOSTl = 0, anticrash3 = 0, vehrpm = 0, crashLobby = 0, loop_RainbowBoxes = 0, oneshotkillv2 = 0, invincible = 0, gModDisabled = 0,
customComponentA = 0, invisible = 0, opmult = 0, espectateP = 0, setWlevelf = 0, featureVehtaser = 0, mobileRadio = 0, featureVehLight = 0, pinkburstfx01 = 0,
ffield = 0, fastrun = 0, loop_fuckCam = 0, fastswim = 0, electricfx01 = 0, lightentity = 0, forcefield = 0, bypassduke = 0, bloodfx01 = 0, featureVehballs = 0,
loop_annoyBomb = 0, infiniteAmmo = 0, loop_gta2cam = 0, earthquake = 0, test2 = 0, tpinspawned = 0, featureVehsnowball = 0, loop_safemoneydrop = 0, loop_safemoneydropv2 = 0, loop_moneydrop = 0,
loop_ClearpTasks = 0, ms_invisible = 0, featureVehprocked = 0, anticrash2 = 0, anticrash = 0, featureMiscHeatVision = 0, runspeed = 0,
featureVehtrounds = 0, flyingUp = 0, featureMiscHideHud = 0, showWhosTalking = 0, featureVehlaser1 = 0, spritetest2 = 0, featureVehlaser2 = 0,
VehSpeedBoost = 0, featureMoneyDropSelfLoop = 0, featureVehFireworks = 0, spodercarmode = 0, lowerVehMID_ms = 0, lowerVehMAX_ms = 0, featureRagdoll = 0,
lowerVeh_ms = 0, featureVehRockets = 0, RainbowP = 0, customsecondary = 0, customprimary = 0, loop_rainbowcar = 0, shootrhino = 0, ExplosiveAmmo = 0,
disableinvincible = 0, shootdummp = 0, nearbypeds = 0, shootcutter = 0, shootstrippers = 0, shootlbrtr = 0, PlayerSuperJump = 0, shootbzrd = 0, shoothydras = 0,
shootbmxs = 0, ExplosiveMelee = 0, ms_rpIncreaser = 0, ms_neverWanted = 0, vehTorque = 0;
bool selltime = 0;
bool planesmoke = 0;
bool speedometer = 0;
bool explodetalker = 0, loop_noreload = 0, onehit = 0, loop_fireammo = 0, loop_explosiveammo = 0,
loop_explosiveMelee = 0, loop_vehicleRockets = 0, loop_rainbowneon = 0, loop_locktime = 0, custombullet = 0, fixAndWashLoop = 0, riskMode = 0,
carspawnlimit = 0, antiParticleFXCrash = 0, aimbot = 0;
Player selPlayer;
char* selpName;
Object mapMods[250];
int mapModsIndex = 0;
bool ExtremeRun = 0;
bool stealth = 0;
RGBA boxColor = { 255, 0, 0, 255 };
float optionYBonus = 0.123f;
float optionXBonus = 0.264f;
float backXBonus = 0.331f;
float titleYBonus = -0.019f;
float backgroundYBonus = -0.135f;
float backgroundLengthBonus = -0.193f;
float titleBoxBonus = 0.045f;
float bwidth = 0.135f;
float titleXBonus = 0.171f;
float titleTextYBonus = -0.016f;
float titleScale = 0.590f;
float titleScaleSmall = 0.472f;
bool useMPH = 1;
bool bulletBlame = 0;
bool controllerMode = 0;
Ped shootAs = -1;
float damageMultiplier = 1;


// Declare subroutines here.
namespace
{

}

// Define subroutines here.
namespace
{

	void VectorToFloat(Vector3 unk, float *Out)
	{
		Out[0] = unk.x;
		Out[1] = unk.y;
		Out[2] = unk.z;
	}
	int RandomRGB()
	{
		srand(GetTickCount());
		return (GAMEPLAY::GET_RANDOM_INT_IN_RANGE(0, 255));
	}
	bool get_key_pressed(int nVirtKey)
	{
		return (GetAsyncKeyState(nVirtKey) & 0x8000) != 0;
	}
	bool Check_self_in_vehicle()
	{
		if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 0)) return true; else return false;
	}
	void offset_from_entity(int entity, float X, float Y, float Z, float * Out)
	{
		VectorToFloat(ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(entity, X, Y, Z), Out);
	}
	void RequestModel(DWORD Hash)
	{
		STREAMING::REQUEST_MODEL(Hash);
		while (!(STREAMING::HAS_MODEL_LOADED(Hash)))
		{
			WAIT(5);
		}
	}
	////bypass
	//DWORD64 GetModuleBase(HANDLE hProc, string &sModuleName)//For 64bit process
	//{
	//	HMODULE *hModules;
	//	hModules = 0;
	//	char szBuf[50];
	//	DWORD cModules = 0;
	//	DWORD64 dwBase = 0;

	//	EnumProcessModules(hProc, hModules, 0, &cModules);
	//	hModules = new HMODULE[cModules / sizeof(HMODULE)];

	//	if (EnumProcessModules(hProc, hModules, cModules / sizeof(HMODULE), &cModules)) {
	//		for (int i = 0; i < cModules / sizeof(HMODULE); i++) {
	//			if (GetModuleBaseNameA(hProc, hModules[i], szBuf, sizeof(szBuf))) {
	//				if (sModuleName.compare(szBuf) == 0) {
	//					dwBase = (DWORD64)hModules[i];
	//					break;
	//				}
	//			}
	//		}
	//	}
	//	void bypass_online()
	//	{
	//		__int64 Address = GetModuleBase(GetCurrentProcess(), string("GTA5.exe"));

	//		CHAR *MemoryBuff = new CHAR[4096];
	//		HANDLE hProcess = GetCurrentProcess();
	//		BYTE bytes[10] = { 0x48, 0x8B, 0x88, 0x10, 0x01, 0x00, 0x00, 0x48, 0x8B, 0xC1 };
	//		BYTE nop[2] = { 0x90, 0x90 };

	//		for (;;)
	//		{
	//			ReadProcessMemory(hProcess, (LPVOID)Address, (LPVOID)MemoryBuff, 4096, NULL);
	//			for (INT p = 0; p < 4096; p++)
	//			{
	//				Address++;
	//				MemoryBuff++;
	//				if (memcmp(MemoryBuff, bytes, 10) == 0)
	//				{
	//					WriteProcessMemory(hProcess, (LPVOID)(Address + 0x20), nop, 2, NULL);
	//					WriteProcessMemory(hProcess, (LPVOID)(Address + 0x2D), nop, 2, NULL);

	//					goto endfunc;
	//				}
	//			}
	//			MemoryBuff = MemoryBuff - 4096;
	//		}



	const int PED_FLAG_CAN_FLY_THRU_WINDSCREEN = 32;

	/*int PlaceObject(DWORD Hash, float X, float Y, float Z, float Pitch, float Roll, float Yaw)
	{
	RequestModel(Hash);
	int object = OBJECT::CREATE_OBJECT(Hash, X, Y, Z, 1, 1, 0);
	ENTITY::SET_ENTITY_ROTATION(object, Pitch, Roll, Yaw, 2, 1);
	ENTITY::FREEZE_ENTITY_POSITION(object, 1);
	ENTITY::SET_ENTITY_LOD_DIST(object, 696969);
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(Hash);
	ENTITY::SET_OBJECT_AS_NO_LONGER_NEEDED(&object);

	return object;
	}*/
	void setupdraw()
	{
		UI::SET_TEXT_FONT(1);
		UI::SET_TEXT_SCALE(0.4f, 0.4f);
		UI::SET_TEXT_COLOUR(255, 255, 255, 255);
		UI::SET_TEXT_WRAP(0.0f, 1.0f);
		UI::SET_TEXT_CENTRE(0);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
		UI::SET_TEXT_EDGE(0, 0, 0, 0, 0);
		UI::SET_TEXT_OUTLINE();
	}			
	void drawstring(char* text, float X, float Y)
	{
		UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
		UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text);
		UI::END_TEXT_COMMAND_DISPLAY_TEXT(X, Y);
	}

	void drawinteger(int text, float X, float Y)
	{
		UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("NUMBER");
		
			UI::ADD_TEXT_COMPONENT_INTEGER(text);
		UI::END_TEXT_COMMAND_DISPLAY_TEXT(X, Y);
	}
	void drawfloat(float text, DWORD decimal_places, float X, float Y)
	{
		UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("NUMBER");
		
			UI::ADD_TEXT_COMPONENT_FLOAT(text, decimal_places);
		UI::END_TEXT_COMMAND_DISPLAY_TEXT(X, Y);
	}
	void PlaySoundFrontend(char* sound_dict, char* sound_name)
	{
		AUDIO::PLAY_SOUND_FRONTEND(-1, sound_name, sound_dict, 0);
	}
	void PlaySoundFrontend_default(char* sound_name)
	{
		AUDIO::PLAY_SOUND_FRONTEND(-1, sound_name, "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
	}
	bool Check_compare_string_length(char* unk1, size_t max_length)
	{
		if (strlen(unk1) <= max_length) return true; else return false;
	}
	enum ICONTYPE {
		ICON_NOTHING = 4,
		ICON_CHAT_BOX = 1,
		ICON_EMAIL = 2,
		ICON_ADD_FRIEND_REQUEST = 3,
		ICON_RIGHT_JUMPING_ARROW = 7,
		ICON_RP_ICON = 8,
		ICON_CASH_ICON = 9
	};
	//void DrawNotif(char* message, char* Icon, ICONTYPE icoType, char* sender = "DANK HAX", char* Subject = "Information", ICONTYPE secondIcon = ICON_NOTHING, char* ClanTag = "___DANKKKKK") {
	//	UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");//202709F4C58A0424
	////	UI::_ADD_TEXT_COMPONENT_STRING(message); //6C188BE134E074AA
	//	UI::_SET_NOTIFICATION_MESSAGE_CLAN_TAG_2(Icon, Icon, 1, icoType, sender, Subject, 1, ClanTag, ICON_NOTHING, 0);//531B84E7DA981FB6
	//	UI::_DRAW_NOTIFICATION(FALSE, FALSE);//2ED7843F8F801023
	//}

	char* AddStrings(char* string1, char* string2)
	{
		memset(str, 0, sizeof(str));
		strcpy_s(str, "");
		strcpy_s(str, string1);
		strcat_s(str, string2);
		return str;
	}
	char* AddInt_S(char* string1, int int2)
	{
		char* Return;
		printf_s(Return, "%i", int2);
		Return = AddStrings(string1, Return);
		return Return;

		/*std::string string2 = string1;
		string2 += std::to_string(int2);

		char * Char = new char[string2.size() + 1];
		std::copy(string2.begin(), string2.end(), Char);
		Char[string2.size()] = '\0';

		char* Return = Char;
		delete[] Char;
		return Return;*/
	}
	char* keyboard()
	{
		float i = 2.0000f;
		while (i > 0.4750f)
		{
			WAIT(6);
			GRAPHICS::DRAW_RECT(i, 0.9389f, 0.3875f, 0.0736f, titlebox.R, titlebox.G, titlebox.B, 120);
			i -= 0.0095f;
		}

		std::string(tempStr);

		while (!IsKeyDown(VK_RETURN))
		{
			WAIT(2);
			GRAPHICS::DRAW_RECT(0.5f, 0.5f, 1.0f, 1.0f, 20, 20, 20, 160);
			GRAPHICS::DRAW_RECT(i, 0.9389f, 0.3875f, 0.0736f, titlebox.R, titlebox.G, titlebox.B, 120);
			setupdraw();
			UI::SET_TEXT_FONT(1);
			drawstring("INPUT:", 0.2836f, 0.9080f);

			CONTROLS::DISABLE_ALL_CONTROL_ACTIONS(0);
			if (!IsKeyDown(VK_SHIFT))
			{
				if (IsKeyJustUp(VK_ESCAPE)) return "";
				else if (IsKeyJustUp(VK_SPACE)) tempStr += " ";
				else if (IsKeyJustUp(VK_BACK) && tempStr.length() > 0) tempStr.pop_back();
				else if (IsKeyJustUp('1')) tempStr += "1";
				else if (IsKeyJustUp('2')) tempStr += "2";
				else if (IsKeyJustUp('3')) tempStr += "3";
				else if (IsKeyJustUp('4')) tempStr += "4";
				else if (IsKeyJustUp('5')) tempStr += "5";
				else if (IsKeyJustUp('6')) tempStr += "6";
				else if (IsKeyJustUp('7')) tempStr += "7";
				else if (IsKeyJustUp('8')) tempStr += "8";
				else if (IsKeyJustUp('9')) tempStr += "9";
				else if (IsKeyJustUp('0')) tempStr += "0";
				else if (IsKeyJustUp('A')) tempStr += "a";
				else if (IsKeyJustUp('B')) tempStr += "b";
				else if (IsKeyJustUp('C')) tempStr += "c";
				else if (IsKeyJustUp('D')) tempStr += "d";
				else if (IsKeyJustUp('E')) tempStr += "e";
				else if (IsKeyJustUp('F')) tempStr += "f";
				else if (IsKeyJustUp('G')) tempStr += "g";
				else if (IsKeyJustUp('H')) tempStr += "h";
				else if (IsKeyJustUp('I')) tempStr += "i";
				else if (IsKeyJustUp('J')) tempStr += "j";
				else if (IsKeyJustUp('K')) tempStr += "k";
				else if (IsKeyJustUp('L')) tempStr += "l";
				else if (IsKeyJustUp('M')) tempStr += "m";
				else if (IsKeyJustUp('N')) tempStr += "n";
				else if (IsKeyJustUp('O')) tempStr += "o";
				else if (IsKeyJustUp('P')) tempStr += "p";
				else if (IsKeyJustUp('Q')) tempStr += "q";
				else if (IsKeyJustUp('R')) tempStr += "r";
				else if (IsKeyJustUp('S')) tempStr += "s";
				else if (IsKeyJustUp('T')) tempStr += "t";
				else if (IsKeyJustUp('U')) tempStr += "u";
				else if (IsKeyJustUp('V')) tempStr += "v";
				else if (IsKeyJustUp('W')) tempStr += "w";
				else if (IsKeyJustUp('X')) tempStr += "x";
				else if (IsKeyJustUp('Y')) tempStr += "y";
				else if (IsKeyJustUp('Z')) tempStr += "z";
				else if (IsKeyJustUp(VK_OEM_7)) tempStr += "'";
				else if (IsKeyJustUp(VK_OEM_MINUS)) tempStr += "-";
				else if (IsKeyJustUp(VK_OEM_PLUS)) tempStr += "=";
				else if (IsKeyJustUp(VK_OEM_1)) tempStr += ";";
				else if (IsKeyJustUp(VK_OEM_2)) tempStr += "/";
				else if (IsKeyJustUp(VK_OEM_3)) tempStr += "`";
				else if (IsKeyJustUp(VK_OEM_4)) tempStr += "[";
				else if (IsKeyJustUp(VK_OEM_6)) tempStr += "]";
				else if (IsKeyJustUp(VK_OEM_COMMA)) tempStr += ",";
				else if (IsKeyJustUp(VK_OEM_PERIOD)) tempStr += ".";

			}
			else
			{
				if (IsKeyJustUp('A')) tempStr += "A";
				else if (IsKeyJustUp('B')) tempStr += "B";
				else if (IsKeyJustUp('C')) tempStr += "C";
				else if (IsKeyJustUp('D')) tempStr += "D";
				else if (IsKeyJustUp('E')) tempStr += "E";
				else if (IsKeyJustUp('F')) tempStr += "F";
				else if (IsKeyJustUp('G')) tempStr += "G";
				else if (IsKeyJustUp('H')) tempStr += "H";
				else if (IsKeyJustUp('I')) tempStr += "I";
				else if (IsKeyJustUp('J')) tempStr += "J";
				else if (IsKeyJustUp('K')) tempStr += "K";
				else if (IsKeyJustUp('L')) tempStr += "L";
				else if (IsKeyJustUp('M')) tempStr += "M";
				else if (IsKeyJustUp('N')) tempStr += "N";
				else if (IsKeyJustUp('O')) tempStr += "O";
				else if (IsKeyJustUp('P')) tempStr += "P";
				else if (IsKeyJustUp('Q')) tempStr += "Q";
				else if (IsKeyJustUp('R')) tempStr += "R";
				else if (IsKeyJustUp('S')) tempStr += "S";
				else if (IsKeyJustUp('T')) tempStr += "T";
				else if (IsKeyJustUp('U')) tempStr += "U";
				else if (IsKeyJustUp('V')) tempStr += "V";
				else if (IsKeyJustUp('W')) tempStr += "W";
				else if (IsKeyJustUp('X')) tempStr += "X";
				else if (IsKeyJustUp('Y')) tempStr += "Y";
				else if (IsKeyJustUp('Z')) tempStr += "Z";
				else if (IsKeyJustUp('1')) tempStr += "!";
				else if (IsKeyJustUp('2')) tempStr += "@";
				else if (IsKeyJustUp('3')) tempStr += "#";
				else if (IsKeyJustUp('4')) tempStr += "$";
				else if (IsKeyJustUp('6')) tempStr += "^";
				else if (IsKeyJustUp('7')) tempStr += "&";
				else if (IsKeyJustUp('8')) tempStr += "*";
				else if (IsKeyJustUp('9')) tempStr += "(";
				else if (IsKeyJustUp('0')) tempStr += ")";
				else if (IsKeyJustUp(VK_OEM_7)) tempStr += "@";
				else if (IsKeyJustUp(VK_OEM_MINUS)) tempStr += "_";
				else if (IsKeyJustUp(VK_OEM_PLUS)) tempStr += "+";
				else if (IsKeyJustUp(VK_OEM_1)) tempStr += ":";
				else if (IsKeyJustUp(VK_OEM_2)) tempStr += "?";
				else if (IsKeyJustUp(VK_OEM_3)) tempStr += "~";
				else if (IsKeyJustUp(VK_OEM_4)) tempStr += "{";
				else if (IsKeyJustUp(VK_OEM_5)) tempStr += "|";
				else if (IsKeyJustUp(VK_OEM_6)) tempStr += "}";
				else if (IsKeyJustUp(VK_OEM_COMMA)) tempStr += "<";
				else if (IsKeyJustUp(VK_OEM_PERIOD)) tempStr += ">";

			}

			setupdraw();
			UI::SET_TEXT_FONT(14);
			UI::SET_TEXT_CENTRE(1);
			memset(str, 0, sizeof(str));
			strcpy_s(str, tempStr.c_str());
			drawstring(str, 0.46, 0.9);

		}

		CONTROLS::ENABLE_ALL_CONTROL_ACTIONS(2);
		return str;
	}
	int StringToInt(char* text)
	{
		int tempp;
		if (text == "") return NULL;
		if (GAMEPLAY::STRING_TO_INT(text, &tempp)) return NULL;

		return tempp;
	}
	void PrintStringBottomCentre(char* text)
	{
		//UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT_2("STRING");
		//UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text);
		//UI::_DRAW_SUBTITLE_TIMED(2000, 1);
	}
	void PrintFloatBottomCentre(float text, __int8 decimal_places)
	{/*
	 UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT_2("NUMBER");
	 UI::ADD_TEXT_COMPONENT_FLOAT(text, (DWORD)decimal_places);
	 UI::_DRAW_SUBTITLE_TIMED(2000, 1);*/
	}
	//void PrintBottomLeft(char* text)
	//{
	//	UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
	//	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text);
	//	UI::_DRAW_NOTIFICATION_2(0, 1); // Not sure if right native
	//}
	void PrintError_no_longer_in_mem()
	{
		PrintStringBottomCentre("~r~Error:~s~ No longer in memory.");
	}
	void PrintError_Waypoint()
	{
		PrintStringBottomCentre("~r~Error:~s~ No Waypoint Set.");
	}
	void PrintError_InvalidInput()
	{
		PrintStringBottomCentre("~r~Error:~s~ Invalid Input.");
	}
	void PrintError_InvalidModel()
	{
		PrintStringBottomCentre("~r~Error:~s~ Invalid Model.");
	}
	/*Vector3 RotationToDirection(Vector3 rot) {
	double num = DegreeToRadian(rot.z);
	double num2 = DegreeToRadian(rot.x);
	double val = cos(num2);
	double num3 = abs(val);
	rot.x = (float)(-(float)sin(num) * num3);
	rot.y = (float)(cos(num) * num3);
	rot.z = (float)sin(num2);
	return rot;

	}
	Vector3 multiplyVector(Vector3 vector, float inc) {
	vector.x *= inc;
	vector.y *= inc;
	vector.z *= inc;
	vector._paddingx *= inc;
	vector._paddingy *= inc;
	vector._paddingz *= inc;
	return vector;
	}
	Vector3 addVector(Vector3 vector, Vector3 vector2) {
	vector.x += vector2.x;
	vector.y += vector2.y;
	vector.z += vector2.z;
	vector._paddingx += vector2._paddingx;
	vector._paddingy += vector2._paddingy;
	vector._paddingz += vector2._paddingz;
	return vector;
	}*/

	class menu
	{
	public:
		static unsigned __int16 currentsub;
		static unsigned __int16 currentop;
		static unsigned __int16 currentop_w_breaks;
		static unsigned __int16 totalop;
		static unsigned __int16 printingop;
		static unsigned __int16 breakcount;
		static unsigned __int16 totalbreaks;
		static unsigned __int16 currentopb;
		static unsigned __int16 printingopb;
		static unsigned __int8 breakscroll;
		static __int16 currentsub_ar_index;
		static int currentsub_ar[20];
		static int currentop_ar[20];
		static int SetSub_delayed;
		static unsigned long int livetimer;
		static bool bit_centre_title, bit_centre_options, bit_centre_breaks;

		static void loops();
		static void sub_handler();
		static void submenu_switch();
		static void DisableControls()
		{
			UI::HIDE_HELP_TEXT_THIS_FRAME();
			CAM::SET_CINEMATIC_BUTTON_ACTIVE(1);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_NEXT_CAMERA, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_VEH_SELECT_NEXT_WEAPON, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_VEH_CIN_CAM, controllerinput_bool);
			CONTROLS::SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_X);
			CONTROLS::SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_ACCEPT);
			CONTROLS::SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_CANCEL);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_HUD_SPECIAL, controllerinput_bool);
			CONTROLS::SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_DOWN);
			CONTROLS::SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_UP);
			CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_ACCEPT, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_CANCEL, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_LEFT, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_RIGHT, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_DOWN, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_UP, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_RDOWN, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_ACCEPT, controllerinput_bool);
			UI::HIDE_HUD_COMPONENT_THIS_FRAME(10);
			UI::HIDE_HUD_COMPONENT_THIS_FRAME(6);
			UI::HIDE_HUD_COMPONENT_THIS_FRAME(7);
			UI::HIDE_HUD_COMPONENT_THIS_FRAME(9);
			UI::HIDE_HUD_COMPONENT_THIS_FRAME(8);
			CONTROLS::SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_LEFT);
			CONTROLS::SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_RIGHT);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_WEAPON, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_WEAPON_UNARMED, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_WEAPON_MELEE, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_WEAPON_HANDGUN, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_WEAPON_SHOTGUN, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_WEAPON_SMG, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_WEAPON_AUTO_RIFLE, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_WEAPON_SNIPER, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_WEAPON_HEAVY, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_WEAPON_SPECIAL, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_WEAPON_WHEEL_NEXT, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_WEAPON_WHEEL_PREV, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_WEAPON_SPECIAL_TWO, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_DIVE, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_MELEE_ATTACK_LIGHT, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_MELEE_ATTACK_HEAVY, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_MELEE_BLOCK, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_ARREST, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_VEH_HEADLIGHT, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_VEH_RADIO_WHEEL, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_CONTEXT, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_RELOAD, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_DIVE, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_VEH_CIN_CAM, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_JUMP, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_VEH_SELECT_NEXT_WEAPON, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_VEH_FLY_SELECT_NEXT_WEAPON, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_CHARACTER_FRANKLIN, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_CHARACTER_MICHAEL, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_CHARACTER_TREVOR, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_SELECT_CHARACTER_MULTIPLAYER, controllerinput_bool);
			CONTROLS::DISABLE_CONTROL_ACTION(0, INPUT_CHARACTER_WHEEL, controllerinput_bool);
		}
		static void background2()
		{
			float temp;
			//	if (totalop > 14) temp = 14; else temp = (float)totalop; // Calculate last option number to draw rect

			if (totalop > 14) temp = 14; else temp = (float)totalop; // Calculate last option number to draw rect
			temp = 14;

			// Calculate Y Coord
			float bg_Y = ((temp * 0.035f) / 2.0f) + 0.159f;
			float bg_length = temp * 0.035f + backgroundLengthBonus;
			if (menuPos < 0.32f) {
				// Draw titlebox
				GRAPHICS::DRAW_RECT(backXBonus + menuPos, 0.1175f + titleYBonus, bwidth, titleBoxBonus, titlebox.R, titlebox.G, titlebox.B, titlebox.A);

				// Draw background
				GRAPHICS::DRAW_RECT(backXBonus + menuPos, bg_Y + backgroundYBonus, bwidth, bg_length, BG.R, BG.G, BG.B, BG.A);
			}
			else {
				// Draw titlebox
				GRAPHICS::DRAW_RECT(-0.015 + menuPos, 0.1175f + titleYBonus, bwidth, titleBoxBonus, titlebox.R, titlebox.G, titlebox.B, titlebox.A);

				// Draw background
				GRAPHICS::DRAW_RECT(-0.015f + menuPos, bg_Y + backgroundYBonus, bwidth, bg_length, BG.R, BG.G, BG.B, BG.A);
			}

		}

		static void base()
		{
			GRAPHICS::GET_SCREEN_RESOLUTION(&screen_res_x, &screen_res_y); // Get screen res
			if (menu::currentsub != SUB::CLOSED)
			{
				DWORD dword = (DWORD)("CommonMenu");
				if (!GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED("CommonMenu")) { GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT("CommonMenu", 0); }
				background();
				optionhi();
			}
		}

		static void background()
		{
			float temp;
			if (totalop > 14) temp = 14; else temp = (float)totalop; // Calculate last option number to draw rect

																	 // Calculate Y Coord
			float bg_Y = ((temp * 0.035f) / 2.0f) + 0.159f;
			float bg_length = temp * 0.035f;

			// Draw titlebox

			GRAPHICS::DRAW_RECT(0.16f + menuPos, 0.1175f, 0.20f, 0.083f, titlebox.R, titlebox.G, titlebox.B, titlebox.A);



			// Draw background
			GRAPHICS::DRAW_RECT(0.16f + menuPos, bg_Y, 0.20f, bg_length, BG.R, BG.G, BG.B, BG.A);
			//		REQUEST_STREAMED_TEXTURE_DICT("CommonMenu", 0);
			//		HAS_STREAMED_TEXTURE_DICT_LOADED("CommonMenu");
			//		DRAW_SPRITE("CommonMenu", "gradient_bgd", 0.16f + menuPos, bg_Y, 0.20f, bg_length, 0.0f, 255, 255, 255, 225);

			// Draw scroller indicator rect


			if (totalop > 14) temp = 14.0f; else temp = (float)totalop;
			float scr_rect_Y = ((temp + 1.0f) * 0.035f) + 0.1415f;
			GRAPHICS::DRAW_RECT(0.16f + menuPos, scr_rect_Y, 0.20f, 0.0345f, BG.R, BG.G, BG.B, BG.A);

			//	REQUEST_STREAMED_TEXTURE_DICT("commonmenu", 0);
			//	HAS_STREAMED_TEXTURE_DICT_LOADED("commonmenu");
			//	DRAW_SPRITE("commonmenu", "interaction_bgd", 0.16f + menuPos, scr_rect_Y, 0.20f, 0.0345f, 0.0f, 255, 255, 255, 225);
			//GRAPHICS::DRAW_SPRITE(")
			// Draw thin line over scroller indicator rect
			if (totalop < 14) GRAPHICS::DRAW_RECT(0.16f + menuPos, (float)(totalop)* 0.035f + 0.16f, 0.20f, 0.0022f, 255, 255, 255, 255);
			else GRAPHICS::DRAW_RECT(0.16f + menuPos, 14.0f * 0.035f + 0.16f, 0.20f, 0.0009f, 255, 255, 255, 255);

			// Draw scroller indicator

			if ((totalop > 14) && GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED("CommonMenu"))
			{
				Vector3 texture_res = GRAPHICS::GET_TEXTURE_RESOLUTION("CommonMenu", "shop_arrows_upANDdown");

				temp = ((14.0f + 1.0f) * 0.035f) + 0.1259f;

				if (currentop == 1)	GRAPHICS::DRAW_SPRITE("CommonMenu", "arrowright", 0.16f + menuPos, temp, (texture_res.x / (float)screen_res_x) / 3, (texture_res.y / (float)screen_res_y) / 3, 270.0f, 255, 255, 255, 255);
				else if (currentop == totalop) GRAPHICS::DRAW_SPRITE("CommonMenu", "arrowright", 0.16f + menuPos, temp, (texture_res.x / (float)screen_res_x) / 3, (texture_res.y / (float)screen_res_y) / 3, 90.0f, 255, 255, 255, 255);
				else GRAPHICS::DRAW_SPRITE("CommonMenu", "shop_arrows_upanddown", 0.16f + menuPos, temp, (texture_res.x / (float)screen_res_x) / 2, (texture_res.y / (float)screen_res_y) / 2, 0.0f, 255, 255, 255, 255);

			}

			// Draw option count

			temp = scr_rect_Y - 0.0124f;		
			setupdraw();
			UI::SET_TEXT_FONT(0);
			UI::SET_TEXT_SCALE(0.0f, 0.26f);
			UI::SET_TEXT_COLOUR(optioncount.R, optioncount.G, optioncount.B, optioncount.A);

			UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("CM_ITEM_COUNT");
			UI::ADD_TEXT_COMPONENT_INTEGER(currentop); // ! currentop_w_breaks
			UI::ADD_TEXT_COMPONENT_INTEGER(totalop); // ! totalop - totalbreaks
			UI::END_TEXT_COMMAND_DISPLAY_TEXT(0.2205f + menuPos, temp);
		}

		static void optionhi()
		{
			float Y_coord;
			if (currentop > 14) Y_coord = 14.0f; else Y_coord = (float)currentop;

			Y_coord = (Y_coord * 0.035f) + 0.1415f;
			GRAPHICS::DRAW_RECT(0.16f + menuPos, Y_coord, 0.20f, 0.035f, selectionhi.R, selectionhi.G, selectionhi.B, selectionhi.A);
		}
		static bool isBinds()
		{
			// Open menu - RB + LB / F8
			return ((CONTROLS::IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_RB) && CONTROLS::IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_LB)) || IsKeyDown(VK_F8));
		}
		static void while_closed()
		{
			if (isBinds())
			{
				PlaySoundFrontend("FocusIn", "HintCamSounds");
				currentsub = 1;
				currentsub_ar_index = 0;
				currentop = 1;
			}
		}
		static void while_opened()
		{
			totalop = printingop; printingop = 0;
			totalbreaks = breakcount; breakcount = 0; breakscroll = 0;

			if (UI::IS_PAUSE_MENU_ACTIVE()) SetSub_closed();

			UI::DISPLAY_AMMO_THIS_FRAME(0);
			UI::DISPLAY_CASH(0);
			UI::SET_RADAR_ZOOM(0);
			MOBILE::SET_MOBILE_PHONE_POSITION(0, 0, 0);

			DisableControls();

			// Scroll up
			if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_PAD_UP) || IsKeyJustUp(VK_UP))
			{
				if (currentop == 1) Bottom(); else Up();
			}

			// Scroll down
			if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_PAD_DOWN) || IsKeyJustUp(VK_DOWN))
			{
				if (currentop == totalop) Top(); else Down();
			}

			// B press
			if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RRIGHT) || IsKeyJustUp(VK_BACK))
			{
				if (currentsub == SUB::MAINMENU) SetSub_closed(); else SetSub_previous();
			}

			// Binds press
			if (currentsub != SUB::MAINMENU && isBinds())
			{
				SetSub_closed();
			}
		}
		static void Up()
		{
			currentop--; currentop_w_breaks--;
			PlaySoundFrontend_default("NAV_UP_DOWN");
			breakscroll = 1;
		}
		static void Down()
		{
			currentop++; currentop_w_breaks++;
			PlaySoundFrontend_default("NAV_UP_DOWN");
			breakscroll = 2;
		}
		static void Bottom()
		{
			currentop = totalop; currentop_w_breaks = totalop;
			PlaySoundFrontend_default("NAV_UP_DOWN");
			breakscroll = 3;
		}
		static void Top()
		{
			currentop = 1; currentop_w_breaks = 1;
			PlaySoundFrontend_default("NAV_UP_DOWN");
			breakscroll = 4;
		}
		static void SetSub_previous()
		{
			currentsub = currentsub_ar[currentsub_ar_index]; // Get previous submenu from array and set as current submenu
			currentop = currentop_ar[currentsub_ar_index]; // Get last selected option from array and set as current selected option
			currentsub_ar_index--; // Decrement array index by 1
			printingop = 0; // Reset option print variable
			PlaySoundFrontend_default("BACK"); // Play sound
		}
		static void SetSub_new(int sub_index)
		{
			currentsub_ar_index++; // Increment array index
			currentsub_ar[currentsub_ar_index] = currentsub; // Store current submenu index in array
			currentsub = sub_index; // Set new submenu as current submenu (Static_1)

			currentop_ar[currentsub_ar_index] = currentop; // Store currently selected option in array
			currentop = 1; currentop_w_breaks = 1; // Set new selected option as first option in submenu

			printingop = 0; // Reset currently printing option var
		}
		static void SetSub_closed()
		{
			CONTROLS::ENABLE_ALL_CONTROL_ACTIONS(2);
			PlaySoundFrontend_default("BACK");
			currentsub = SUB::CLOSED;
		}

	}; unsigned __int16 menu::currentopb = 0;  unsigned __int16 menu::printingopb = 0; unsigned __int16 menu::currentsub = 0; unsigned __int16 menu::currentop = 0; unsigned __int16 menu::currentop_w_breaks = 0; unsigned __int16 menu::totalop = 0; unsigned __int16 menu::printingop = 0; unsigned __int16 menu::breakcount = 0; unsigned __int16 menu::totalbreaks = 0; unsigned __int8 menu::breakscroll = 0; __int16 menu::currentsub_ar_index = 0; int menu::currentsub_ar[20] = {}; int menu::currentop_ar[20] = {}; int menu::SetSub_delayed = 0; unsigned long int menu::livetimer; bool menu::bit_centre_title = 1, menu::bit_centre_options = 0, menu::bit_centre_breaks = 1;
	bool CheckAJPressed()
	{
		if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RDOWN) || IsKeyJustUp(VK_RETURN)) return true; else return false;
	}
	bool CheckRPressed()
	{
		if (CONTROLS::IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_RIGHT) || IsKeyDown(VK_RIGHT)) return true; else return false;
	}
	bool CheckRJPressed()
	{
		if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_RIGHT) || IsKeyJustUp(VK_RIGHT)) return true; else return false;
	}
	bool CheckLPressed()
	{
		if (CONTROLS::IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_LEFT) || IsKeyDown(VK_LEFT)) return true; else return false;
	}
	bool CheckLJPressed()
	{
		if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_LEFT) || IsKeyJustUp(VK_LEFT)) return true; else return false;
	}
	bool IsOptionPressed()
	{
		if (CheckAJPressed())
		{
			PlaySoundFrontend_default("SELECT");
			return true;
		}
		else return false;
	}
	bool IsOptionRPressed()
	{
		if (CheckRPressed())
		{
			PlaySoundFrontend_default("NAV_LEFT_RIGHT");
			return true;
		}
		else return false;
	}
	bool IsOptionRJPressed()
	{
		if (CheckRJPressed())
		{
			PlaySoundFrontend_default("NAV_LEFT_RIGHT");
			return true;
		}
		else return false;
	}
	bool IsOptionLPressed()
	{
		if (CheckLPressed())
		{
			PlaySoundFrontend_default("NAV_LEFT_RIGHT");
			return true;
		}
		else return false;
	}
	bool IsOptionLJPressed()
	{
		if (CheckLJPressed())
		{
			PlaySoundFrontend_default("NAV_LEFT_RIGHT");
			return true;
		}
		else return false;
	}
	void AddTitle(char* text)
	{
		setupdraw();
		UI::SET_TEXT_FONT(font_title);

		UI::SET_TEXT_COLOUR(titletext.R, titletext.G, titletext.B, titletext.A);

		if (menu::bit_centre_title)
		{
			UI::SET_TEXT_CENTRE(1);
			OptionY = 0.16f + menuPos; // X coord
		}
		else OptionY = 0.066f + menuPos; // X coord

		if (Check_compare_string_length(text, 14))
		{
			UI::SET_TEXT_SCALE(0.75f, 0.75f);
			drawstring(text, OptionY, 0.1f);
		}
		else drawstring(text, OptionY, 0.13f);

	}

	void nullFunc() { return; }
	void AddOption(char* text, bool &option_code_bool = null, void(&Func)() = nullFunc, int submenu_index = -1, bool show_arrow = 0)
	{
		char* tempChar;
		if (font_options == 2 || font_options == 7) tempChar = "  ------"; // Font unsafe
		else tempChar = "  ~b~>"; // Font safe

		if (menu::printingop + 1 == menu::currentop && (font_selection == 2 || font_selection == 7)) tempChar = "  ------"; // Font unsafe
		else tempChar = "  ~b~>"; // Font safe

		menu::printingop++;

		OptionY = 0.0f;
		if (menu::currentop <= 14)
		{
			if (menu::printingop > 14) return;
			else OptionY = ((float)(menu::printingop) * 0.035f) + 0.125f;
		}
		else
		{
			if (menu::printingop < (menu::currentop - 13) || menu::printingop > menu::currentop) return;
			else OptionY = ((float)(menu::printingop - (menu::currentop - 14))* 0.035f) + 0.125f;
		}

		setupdraw();
		UI::SET_TEXT_FONT(font_options);
		UI::SET_TEXT_COLOUR(optiontext.R, optiontext.G, optiontext.B, optiontext.A);
		if (menu::printingop == menu::currentop)
		{
			UI::SET_TEXT_FONT(font_selection);
			UI::SET_TEXT_COLOUR(selectedtext.R, selectedtext.G, selectedtext.B, selectedtext.A);
			if (IsOptionPressed())
			{
				/*if (&option_code_bool != &null)*/ option_code_bool = true;
				Func();
				if (submenu_index != -1) menu::SetSub_delayed = submenu_index;
			}
		}

		if (show_arrow || submenu_index != -1) text = AddStrings(text, tempChar);
		if (menu::bit_centre_options)
		{
			UI::SET_TEXT_CENTRE(1);
			drawstring(text, 0.16f + menuPos, OptionY);
		}
		else drawstring(text, 0.066f + menuPos, OptionY);
	}
	void OptionStatus(int status)
	{
		if (OptionY < 0.6325f && OptionY > 0.1425f)
		{
			char* tempChar;
			UI::SET_TEXT_FONT(4);
			UI::SET_TEXT_SCALE(0.34f, 0.34f);
			UI::SET_TEXT_CENTRE(1);

			if (status == 0) {
				UI::SET_TEXT_COLOUR(255, 102, 102, 250); //RED
				tempChar = "OFF";
			}
			else if (status == 1) {
				UI::SET_TEXT_COLOUR(102, 255, 102, 250); //GREEN
				tempChar = "ON";
			}
			else {
				UI::SET_TEXT_COLOUR(255, 255, 102, 250); //YELLOW
				tempChar = "??";
			}

			drawstring(tempChar, 0.233f + menuPos, OptionY);

		}
	}
	void AddToggle(char* text, bool &loop_variable, bool &extra_option_code_ON = null, bool &extra_option_code_OFF = null)
	{
		null = 0;
		AddOption(text, null);

		if (null) {
			loop_variable = !loop_variable;
			if (loop_variable != 0) extra_option_code_ON = true;
			else extra_option_code_OFF = true;
		}

		OptionStatus((int)loop_variable); // Display ON/OFF
	}
	void AddLocal(char* text, Void condition, bool &option_code_ON, bool &option_code_OFF)
	{
		null = 0;
		AddOption(text, null);
		if (null)
		{
			if (condition == 0) option_code_ON = true; else option_code_OFF = true;
		}

		if (condition == 0) OptionStatus(0); // Display OFF
		else				OptionStatus(1); // Display ON
	}
	void AddBreak(char* text)
	{
		menu::printingop++; menu::breakcount++;

		OptionY = 0.0f;
		if (menu::currentop <= 14)
		{
			if (menu::printingop > 14) return;
			else OptionY = ((float)(menu::printingop) * 0.035f) + 0.125f;
		}
		else
		{
			if (menu::printingop < (menu::currentop - 13) || menu::printingop > menu::currentop) return;
			else OptionY = ((float)(menu::printingop - (menu::currentop - 14))* 0.035f) + 0.125f;
		}

		setupdraw();
		UI::SET_TEXT_FONT(font_breaks);
		UI::SET_TEXT_COLOUR(optionbreaks.R, optionbreaks.G, optionbreaks.B, optionbreaks.A);
		if (menu::printingop == menu::currentop)
		{
			switch (menu::breakscroll)
			{
			case 1:
				menu::currentop_w_breaks = menu::currentop_w_breaks + 1;
				menu::currentop--; break;
			case 2:
				menu::currentop_w_breaks = menu::currentop - menu::breakcount;
				menu::currentop++; break;
			case 3:
				menu::currentop_w_breaks = menu::totalop - (menu::totalbreaks - 1);
				menu::currentop--; break;
			case 4:
				menu::currentop_w_breaks = 1;
				menu::currentop++; break;
			}

		}
		if (menu::bit_centre_breaks)
		{
			UI::SET_TEXT_CENTRE(1);
			drawstring(text, 0.16f + menuPos, OptionY);
		}
		else
		{
			drawstring(text, 0.066f + menuPos, OptionY);
		}

	}
	void AddNumber(char* text, float value, __int8 decimal_places, bool &A_PRESS = null, bool &RIGHT_PRESS = null, bool &LEFT_PRESS = null)
	{
		null = 0;
		AddOption(text, null);

		if (OptionY < 0.6325 && OptionY > 0.1425)
		{
			UI::SET_TEXT_FONT(0);
			UI::SET_TEXT_SCALE(0.26f, 0.26f);
			UI::SET_TEXT_CENTRE(1);

			drawfloat(value, (DWORD)decimal_places, 0.233f + menuPos, OptionY);
		}

		if (menu::printingop == menu::currentop)
		{
			if (null) A_PRESS = true;
			else if (IsOptionRJPressed()) RIGHT_PRESS = true;
			else if (IsOptionRPressed()) RIGHT_PRESS = true;
			else if (IsOptionLJPressed()) LEFT_PRESS = true;
			else if (IsOptionLPressed()) LEFT_PRESS = true;

		}

	}
	void AddTele(char* text, float X, float Y, float Z, bool &extra_option_code = null)
	{
		null = 0;
		AddOption(text, null);
		if (menu::printingop == menu::currentop)
		{
			if (null)
			{
				if (!Check_self_in_vehicle())
				{
					ENTITY::SET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), X, Y, Z, 0, 0, 0, 1);
				}
				else
				{
					RequestControlOfEnt(myVeh);
					ENTITY::SET_ENTITY_COORDS(myVeh, X, Y, Z, 0, 0, 0, 1);
				}
				STREAMING::LOAD_ALL_OBJECTS_NOW();
				extra_option_code = true;
			}
		}

	}
	// Hax functions:


	int grav_partfx, grav_entity; bool grav_toggled = 0, grav_target_locked = 0;
	Vector3 get_coords_from_cam(float distance)
	{
		Vector3 Rot = CAM::GET_GAMEPLAY_CAM_ROT(2);
		static Vector3 Coord = CAM::GET_GAMEPLAY_CAM_COORD();

		Rot.y = distance * cos(Rot.x);
		Coord.x = Coord.x + Rot.y * sin(Rot.z * -1.0f);
		Coord.y = Coord.y + Rot.y * cos(Rot.z * -1.0f);
		Coord.z = Coord.z + distance * sin(Rot.x);

		return Coord;
	}

	void set_gta2_cam_rot()
	{
		Vector3 Rot = CAM::GET_GAMEPLAY_CAM_ROT(2);
		if (!Check_self_in_vehicle()) {
			ENTITY::SET_ENTITY_ROTATION(PLAYER::PLAYER_PED_ID(), Rot.x, Rot.y, Rot.z, 2, 1);
		}
		else {
			ENTITY::SET_ENTITY_ROTATION(myVeh, Rot.x, Rot.y, Rot.z, 2, 1);
		}
	}

}
namespace sub {

	bool Stealth = 0;
	bool Shrink = 0;
	// Define submenus here.
	
	//static void NETWORK_EARN_FROM_PICKUP(int amount) { invoke<Void>(0xED1517D3AF17C698, amount); } // 0xED1517D3AF17C698 0x70A0ED62
	
	void MainMenu()
	{
		//AddOption("THING", null, thing);
		//Add To Auth Premium/ VIP
		bool ShrinkE = 0, ShrinkD = 0;
		AddTitle("Anonymous");
		//AddToggle("Stealth", stealth);
		//AddOption("ENTRY THING", null, thing);
		//ddOption("~g~Name Changer", null, nullFunc, SUB::NAMECHANGER);

		AddOption("Self Mods", null, nullFunc, SUB::SELFMODOPTIONS);
		AddOption("Changer", null, nullFunc, SUB::CUSTOMCHARAC);
		AddOption("Teleport Locations", null, nullFunc, SUB::TELEPORTLOCATIONS);
		AddOption("Online Players", null, nullFunc, SUB::ONLINEPLAYERS);
		AddOption("All Players", null, nullFunc, SUB::ALLPLAYEROPTIONS);
		AddOption("Weapon Mods", null, nullFunc, SUB::WEAPONSMENU);
		AddOption("Vehicle Spawner", null, nullFunc, SUB::VEHICLE_SPAWNER);
		AddOption("Vehicle Mods", null, nullFunc, SUB::VEHICLEMODSA);
		AddOption("Obj/Ped/Bodyguard Spawner", null, nullFunc, SUB::FORCESPAWNER);
		AddOption("Animal Riding", null, nullFunc, SUB::ANIMAL);
		//	AddOption("~y~Particle Effects", null, nullFunc, SUB::PARTICLEFX);
		AddOption("Gravity Gun", null, nullFunc, SUB::FORCEGUN);
		
		//	AddToggle("Freecam", freecam, freecamEnabled, freeCamDisabled);
		AddOption("~b~ESP Menu", null, nullFunc, SUB::ESP);
		AddOption("Misc Options", null, nullFunc, SUB::SAMPLE);
		AddOption("Menu Settings", null, nullFunc, SUB::SETTINGS);
	}
	void drawint(int text, float X, float Y)
	{
		UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("NUMBER");
		UI::ADD_TEXT_COMPONENT_INTEGER(text);
		UI::END_TEXT_COMMAND_DISPLAY_TEXT(X, Y);
	}
	void CustomizeChar()
	{


		bool sample_mapmod = 0, polunivorm = 0, pfxtest = 0, test3 = 0, hour_plus = 0, fuckedup = 0, depsentry = 0, hour_minus = 0, randChar1 = 0, randCharAcc1 = 0, timescale_plus = 0, selfExplode = 0, tel2ClosVeh = 0, delVeh = 0, timescale_minus = 0, sample_invisible = 0, god_mode = 0,
			defaulte = 0, stoned = 0, ufo1A = 0, ufo1 = 0, createcamera = 0, shootBMX = 0, orange = 0, red = 0, coke = 0, ****edup = 0, hallu = 0, wobbly = 0, drunk = 0, heaven = 0, threedim = 0, killstreak = 0, lq = 0, blurry = 0, white = 0, sample_gta2cam = 0;
		bool rp = 0, rm = 0, gp = 0, gm = 0, bp = 0, nameChanger = 0, bm = 0, rp2 = 0, rm2 = 0, gp2 = 0, gm2 = 0, bp2 = 0, bm2 = 0, test = 0;
		int sample_hour = TIME::GET_CLOCK_HOURS();

		AddTitle("Anonymous MENU");
		AddTitle("~b~CHARACTER CUSTOMIZATION");
		AddOption("Police Uniform", polunivorm);
		AddOption("Add Rockstar Logo", depsentry);
		AddOption("Customize Hats", null, nullFunc, SUB::PEDCOMP10);
		AddOption("Customize Glasses", null, nullFunc, SUB::PEDCOMPGLASSES);
		AddOption("Customize Masks", null, nullFunc, SUB::PEDCOMP1);
		AddOption("Customize Hair", null, nullFunc, SUB::PEDCOMP2);
		AddOption("Customize Tops", null, nullFunc, SUB::PEDCOMP11);
		AddOption("Customize Legs", null, nullFunc, SUB::PEDCOMP4);
		AddOption("Customize Shoes", null, nullFunc, SUB::PEDCOMP6);
		AddOption("Customize Accesories 1", null, nullFunc, SUB::PEDCOMP5);
		AddOption("Customize Accesories 2", null, nullFunc, SUB::PEDCOMP7);
		AddOption("Customize Torso", null, nullFunc, SUB::PEDCOMP3);
		AddOption("Customize Misc Tops", null, nullFunc, SUB::PEDCOMP8);
		AddOption("Customize Armor", null, nullFunc, SUB::PEDCOMP9);
		AddOption("Randomize Character", randChar1);
		AddOption("Random Accesory/Item", randCharAcc1);
		AddOption("Name Changer", nameChanger);

		//	AddToggle("Custom Component A", customComponentA);
		//	AddNumber1por1("ComponentA:", r, 1, null, rp, rm);
		//		AddToggle("Show Who is Talking", showWhosTalking);



		//	AddBreak("---Map---");
		//	AddOption("Load Map Mod", sample_mapmod);
		//	AddTele("Teleport to Map Mod", 509.8423f, 5589.2422f, 792.0000f);


		//	AddBreak("---deez breaks r ugly---");

		Player player = PLAYER::PLAYER_ID();
		BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(PLAYER::PLAYER_PED_ID());
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(playerPed, 1);
		Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0, 0, 0);

		if (nameChanger)
		{
			char *Result = keyboard();
			bool NameChanged = true;
			strncpy((char*)0x41143344, Result, strlen(Result));
			*(char*)(0x41143344 + strlen(Result)) = 0;
			strncpy((char*)0x01FF248C, Result, strlen(Result));
			*(char*)(0x01FF248C + strlen(Result)) = 0;

			PrintStringBottomCentre("~b~Change lobby for others to see the change");

		}
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;

		//customize ped

		if (polunivorm)
		{
			if (ENTITY::GET_ENTITY_MODEL(PLAYER::PLAYER_PED_ID()) == GAMEPLAY::GET_HASH_KEY("mp_f_freemode_01")) {
				//hat
				PED::SET_PED_PROP_INDEX(playerPed, 0, 45, 0, 0);
				//top
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 11, 48, 0, 0);
				//pants
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 4, 34, 0, 0);
				//torso
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 3, 0, 0, 0);
				//shoes
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 6, 25, 0, 0);
				//shoes
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 8, 35, 0, 0);
			}
			else {
				//hat
				PED::SET_PED_PROP_INDEX(playerPed, 0, 46, 0, 0);
				//top
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 11, 55, 0, 0);
				//pants
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 4, 35, 0, 0);
				//torso
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 3, 0, 0, 0);
				//shoes
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 6, 24, 0, 0);
				//shoes
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 9, 0, 0, 0);
			}
		}

	}
	void forceSpawnera() {


		AddTitle("Force Spawner");
		
		AddOption("Funny/Special Vehicles", null, nullFunc, SUB::FUNNYVEHICLES);
		AddOption("NPC's Spawner", null, nullFunc, SUB::NPCSPAWNER);
		AddOption("~b~Bodyguard Spawner", null, nullFunc, SUB::BODYGUARDSPAWNER);
		AddOption("Pets & Funny Peds", null, nullFunc, SUB::OPEDSPAWNER);
		AddOption("Objects Spawner", null, nullFunc, SUB::OBJECTSPAWNERM);
		AddOption("Load a Vehicle Object Preset", null, nullFunc, SUB::LISTPRESETS);
		AddOption("Load Custom Map Mod", null, nullFunc, SUB::LISTCUSTOMMAPMODS);
	}
	void AddIntEasy1(char* text, int value, int &val, int inc = 1, bool fast = 0, bool &toggled = null, bool enableminmax = 0, int max = 0, int min = 0)
	{
		null = 0;
		AddOption(text, null);

		if (OptionY < 0.6325 && OptionY > 0.1425)
		{
			UI::SET_TEXT_FONT(0);
			UI::SET_TEXT_SCALE(0.26f, 0.26f);
			UI::SET_TEXT_CENTRE(1);

			drawint(value, 0.233f + menuPos, OptionY);
		}

		if (menu::printingop == menu::currentop)
		{
			if (IsOptionRJPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val + inc) > max)) {
						val += inc;
					}
				}
				else {
					val += inc;
				}
			}
			else if (IsOptionRPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val + inc) > max)) {
						val += inc;
					}
				}
				else {
					val += inc;
				}
			}
			else if (IsOptionLJPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val - inc) < min)) {
						val -= inc;
					}
				}
				else {
					val -= inc;
				}
			}
			else if (IsOptionLPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val - inc) < min)) {
						val -= inc;
					}
				}
				else {
					val -= inc;
				}
			}
		}
	}
	void Liveries() {
		bool liveryChanged = 0;
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
		int livery = VEHICLE::GET_VEHICLE_LIVERY(veh);
		AddIntEasy1("Livery", livery, livery, 1, 0, liveryChanged);
		if (livery > VEHICLE::GET_VEHICLE_LIVERY_COUNT(veh)) livery--;
		if (livery == -2) livery = -1;
		if (liveryChanged) {
			VEHICLE::SET_VEHICLE_LIVERY(veh, livery);
		}
	}

void VehicleLSCSubMenu2(bool useSelectPlayer = 0) {
	GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT("shopui_title_carmod", 0);
	GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED("shopui_title_carmod");
	GRAPHICS::DRAW_SPRITE("shopui_title_carmod", "shopui_title_carmod", 0.16f + menuPos, 0.1175f, 0.20f, 0.083f, 0.0f, 255, 255, 255, 225);
	bool clan = 0;


	bool all = 0, customplate = 0, debug = 0, downGradVeh = 0;
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	if (useSelectPlayer) {
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
	}

	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = PLAYER::PLAYER_PED_ID();
	RequestControlOfEnt(veh);
	float plateID = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(veh);
	bool turbo = VEHICLE::IS_TOGGLE_MOD_ON(veh, 18), xenon = VEHICLE::IS_TOGGLE_MOD_ON(veh, 22);
	if (!UI::HAS_THIS_ADDITIONAL_TEXT_LOADED("MOD_MNU", 9)) {
		UI::REQUEST_ADDITIONAL_TEXT("MOD_MNU", 9);
	}
	//AddOption("Print debug label", debug);
	//if (debug){
	//	PrintStringBottomCentre(_GET_LABEL_TEXT(keyboard()));
	//}
	//bool printWheel = 0;
	//AddOption("Print Wheel Data", printWheel);
	//if (printWheel){
	//	int CurrentMod = GET_VEHICLE_MOD(veh, 23);
	//	AUTH::DebugConsole();
	//	int CurrentTrim = 0;
	//	_GET_INTERIOR_TRIM_COLOR(veh, &CurrentTrim);
	//	cout << "ID: " << CurrentMod << " -> ";
	//	cout << "Wheel Label: " << GET_MOD_TEXT_LABEL(veh, 23, CurrentMod) << "\r\n";
	//	cout << "Interior Trim Color = " << CurrentTrim << "\r\n";

	//	RequestControlOfEnt(veh);
	//	SET_VEHICLE_MOD_KIT(veh, 0);
	//	_SET_INTERIOR_TRIM_COLOR(veh, CurrentTrim + 1);
	//}
	AddOption("All LSC Paint", null, nullFunc, SUB::GAMEPAINT);
	AddOption("Benny's", null, nullFunc, SUB::BENNYS);
	AddOption("Max Upgrades", all);
	AddOption("Delete All Upgrades", downGradVeh);
	AddOption("Customize Plate", customplate);
	//		AddNumberEasy("Plate Type", plateID, 0, plateID);
		Liveries();
		AddOption("SPEED", null, nullFunc, SUB::SPEEDLIMIT);
	AddOption("Change Wheels", null, nullFunc, SUB::CHANGEWHEELS);
	AddOption("Wheels Color", null, nullFunc, SUB::WHEELCOLOR);
	AddOption("Armor", null, nullFunc, SUB::ARMOR);
	AddOption("Engine", null, nullFunc, SUB::ENGINE);
	AddOption("Transmission", null, nullFunc, SUB::TRANSMISSION);
	AddOption("Suspension", null, nullFunc, SUB::SUSPENSION);
	AddOption("Brakes", null, nullFunc, SUB::BRAKES);
	AddOption("Spoiler", null, nullFunc, SUB::SPOILER);
	AddToggle("Turbo", turbo);
	AddToggle(UI::_GET_LABEL_TEXT("CMOD_LGT_1"), xenon);
	AddOption("Front Bumper", null, nullFunc, SUB::FRONTBUMPER);
	AddOption("Rear Bumper", null, nullFunc, SUB::REARBUMPER);
	AddOption("Side Skirt", null, nullFunc, SUB::SIDESKIRT);
	AddOption("Exhaust", null, nullFunc, SUB::EXHAUST);
	AddOption("Frame", null, nullFunc, SUB::FRMAE);
	AddOption("Grille", null, nullFunc, SUB::GRILLE);
	AddOption("Hood", null, nullFunc, SUB::HOOD);
	AddOption("Fender", null, nullFunc, SUB::FENDER);
	AddOption("Right Fender", null, nullFunc, SUB::RIGHTFENDER);
	AddOption("Roof", null, nullFunc, SUB::ROOF);
	AddOption("Window Tint", null, nullFunc, SUB::WTINT);
	AddOption("Neon", null, nullFunc, SUB::NEON);
	AddOption("Tire Smoke", null, nullFunc, SUB::WHEELSMOKE);
	AddOption("Vehicle Extras", null, nullFunc, SUB::VEHICLEEXTRA);
	bool alphaChanged = 0; int alphaValue = ENTITY::GET_ENTITY_ALPHA(veh);
	//AddIntEasy("Vehicle Alpha", alphaValue, alphaValue, 1, 0, alphaChanged);
	AddOption("Vehicle Multipliers", null, nullFunc, SUB::VEHMULTIPLIERS);
	//	AddOption("Add Clan Label", clan);
	if (clan) {
		//AddClanLogoToVehicle(veh, PLAYER_PED_ID());//Now it should work better lol
		PrintStringBottomCentre("~b~Clan Logo Added !");
	}
	if (alphaChanged) {
		RequestControlOfEnt(veh);
		ENTITY::SET_ENTITY_ALPHA(veh, alphaValue, 1);
	}
	//	AddOption("Wheel", null, nullFunc, SUB::WHEEL);
	//AddOption("Horns", null, nullFunc, SUB::HORNS);
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(PLAYER::PLAYER_PED_ID());
	Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(e, 0);
	RequestControlOfEnt(veh);
	VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(veh, plateID);
	if (customplate) {
		RequestControlOfEnt(veh);
		VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(veh, keyboard());
	}
	RequestControlOfEnt(veh);
	VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
	VEHICLE::TOGGLE_VEHICLE_MOD(veh, 22, xenon);
	RequestControlOfEnt(veh);
	VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
	VEHICLE::TOGGLE_VEHICLE_MOD(veh, 18, turbo);
	if (all) {
		RequestControlOfEnt(veh);
		VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
		VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(veh, 1);
		VEHICLE::TOGGLE_VEHICLE_MOD(veh, 18, 1);
		VEHICLE::TOGGLE_VEHICLE_MOD(veh, 22, 1);
		VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 0);
		VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 0, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 0) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 1, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 1) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 2, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 2) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 3, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 3) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 4, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 4) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 5, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 5) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 6, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 6) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 7, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 7) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 8, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 8) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 9, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 9) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 10, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 10) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 11, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 11) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 12, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 12) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 13, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 13) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 14, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 14) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 15, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 15) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 16, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 16) - 1, 0);
		VEHICLE::SET_VEHICLE_WHEEL_TYPE(veh, 6);
		VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 5);
		VEHICLE::SET_VEHICLE_MOD(veh, 23, 19, 1);

	}
	if (downGradVeh) {
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
		RequestControlOfEnt(veh);
		VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
		VEHICLE::SET_VEHICLE_COLOURS(veh, 12, 56);
		VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(veh, "HOMO");
		VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(veh, 0);
		VEHICLE::TOGGLE_VEHICLE_MOD(veh, 18, 0);
		VEHICLE::TOGGLE_VEHICLE_MOD(veh, 22, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 16, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 12, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 11, 0, 0);
		VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, true);
		VEHICLE::SET_VEHICLE_MOD(veh, 14, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 15, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 13, 0, 0);
		VEHICLE::SET_VEHICLE_WHEEL_TYPE(veh, 0);
		VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 23, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 0, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 1, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 2, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 3, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 4, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 5, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 6, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 7, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 8, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 9, 0, 0);
		VEHICLE::SET_VEHICLE_MOD(veh, 10, 0, 0);

	}


}
	void tp()
	{
		AddTele("Anti Crash Zone!!", 13954.0f, -14526.0f, 163004.0f);
		AddOption("High Up", null, nullFunc, SUB::HIGH, true);
		AddOption("Landmarks", null, nullFunc, SUB::LANDMARKS, true);
		AddOption("Sea", null, nullFunc, SUB::SEA, true);
		AddOption("Single Player", null, nullFunc, SUB::SINGLE, true);
		AddOption("Modded", null, nullFunc, SUB::MODDED, true);
		AddOption("Robbing Spree", null, nullFunc, SUB::STORES, true);
	}
	void other()
	{
		AddTele("Beeker's Garage", 139.67, 6595.94f, 33.00f);
		AddTele("400k/500k Apartment", -793.36, 296.86f, 87.84f);
		AddTele("Appartment", -778.34, 339.97f, 208.62f);
		AddTele("Mors Mutual Insurance", -232.74, -1163.556f, 24.95f);
		AddTele("Impound Lot", 408.923, -1633.860f, 30.29f);
		AddTele("Inside 10 Car Garage", 228.71, -989.98f, -96.00f);
		AddTele("Under The Map", 132.1470, -739.5430f, 39.00f);
		AddTele("Strip Club", 125.428, -1290.40f, 30.00f);
		AddTele("Prison", 1696.3642, 2561.377f, 47.56f);
		AddTele("Maze", -2311.01, 234.33f, 170.63f);
		AddTele("Ammunation", 233.3912, -41.08f, 69.67f);
		AddTele("Race Track", 1201.36, 95.65f, 82.03f);


		AddTele("LS Customs", -363.9027, -132.71f, 39.00f);
		AddTele("Random", 2861.426, 5927.89, 361.29f);

		AddTele("Cave", -1911.3, 1389.29f, 219.00f);
		AddTele("Farest Island North", 32.51, 7688.99f, 4.00f);
		AddTele("Farest Island South", 1799.90, -2823.90, 5.00f);
		AddTele("Flight School", -1153.10, -2713.39f, 20.00f);
		AddTele("Tram Station", 104.50, -1718.30f, 31.00f);
		AddTele("Golf", -1079.71, 10.04f, 51.00f);
		AddTele("Stage", 684.97, 574.32f, 131.00f);
		AddTele("Drift Mountain", 860.32, 1316.65f, 356.00f);



		AddTele("Water Fall", -552.0047, 4439.4487f, 35.123f);


		AddTele("Del Perro Pier", -1838.834, -1223.333f, 15.00f);
		AddTele("Vinewood Sign", 729.909, 1204.37f, 326.0209f);

	}
	void sp()
	{
		AddTele("michael's House", -827.13, 175.47f, 70.82f);
		AddTele("michael's House Inside", -814.38, 178.92f, 73.00f);
		AddTele("Franklins's House Old", -14.31, -1437.00f, 30.00f);
		AddTele("Franklins's House New", 7.05, 524.33f, 174.97f);

		AddTele("Trevers Meth Lab", 1390.28, 3608.60f, 39.00f);

	}
	void Landmarks()
	{

		AddTele("Mine Shaft", -596.93, 2094.12f, 132.00f);
		AddTele("military Base", -2138.234, 3250.8606f, 34.00f);
		AddTele("Airport", -1135.1100, -2885.2030f, 15.00f);
		AddTele("Trevers Airfield", 1590.6788, 3267.6698f, 43.0000f);
		AddTele("Helicopter Pad", -741.54, -1456.00f, 3.00f);

	}
	void sea()
	{
		AddTele("Out To Sea", 1845.673, -13787.4884f, 0.0000f);
		AddTele("Island", -2159.147, 5196.89f, 20.00f);
	}
	void High()
	{
		AddTele("Maze Bank ", -75.5003, -819.0528f, 327.00f);
		AddTele("Mount Chilliad", 496.0635f, 5584.5142f, 793.9454f);
		AddTele("Dam 1", 1663.123, 24.18f, 169.00f);
		AddTele("High in the Sky!!", -129.9f, 8130.8f, 6705.6f);
		AddTele("parachute Jump", -521.35, 4422.00f, 89.00f);
		AddTele("Consturction Building", -161.26, -937.87f, 268.52f);
		AddTele("Dam", 115.28, 785.81f, 212.00f);
	}
	void Modded()
	{
		AddTele("Humane Labs Level 1", 3617.3726, 3738.2727f, 30.6901f);
		AddTele("Humane Labs Level 2", 3525.6133, 3709.2998f, 22.9918f);
		AddTele("Inside FIB Building", 136.3807, -749.0196f, 258.1517f);
		AddTele("Inside Fire FIB", 137.8378, -747.39f, 253.152f);
		AddTele("Inside FIB Lift", 133.1019, -735.7224f, 235.63f);
		AddTele("Inside IAA Building", 127.49, -618.26f, 207.04f);
		AddTele("Under Water UFO", 760.461, 7392.8032f, -110.0774f);
		AddTele("Under Water Plane Crash", 1846, -2946.855f, -33.32f);
	}
	void stores()
	{
		AddTitle("ROBBERY SPREE");
		AddTele("Go to Store 1", -48.0f, -1756.0f, 29.0f);
		AddTele("Go to Store 2", 26.0f, -1345.0f, 29.0f);
		AddTele("Go to Store 3", 1136.0f, -982.0f, 46.0f);
		AddTele("Go to Store 4", -708.0f, -913.0f, 19.0f);
		AddTele("Go to Store 5", -1223.0f, -906.0f, 12.0f);
		AddTele("Go to Store 6", -1487.0f, -379.0f, 40.0f);
		AddTele("Go to Store 7", 1162.0f, -322.0f, 69.0f);
		AddTele("Go to Store 8", 2555.0f, 382.0f, 108.0f);
		AddTele("Go to Store 9", 374.0f, 327.0f, 103.0f);
		AddTele("Go to Store 10", -1822.0f, 792.0f, 138.0f);
		AddTele("Go to Store 11", -2967.0f, 391.0f, 15.0f);
		AddTele("Go to Store 12", -3041.0f, 585.0f, 7.0f);
		AddTele("Go to Store 13", -3244.0f, 1001.0f, 12.0f);
		AddTele("Go to Store 14", 547.0f, 2667.0f, 42.0f);
		AddTele("Go to Store 15", 1166.0f, 2708.0f, 38.0f);
		AddTele("Go to Store 16", 2677.0f, 3282.0f, 55.0f);
		AddTele("Go to Store 17", 1392.0f, 3603.0f, 34.0f);
		AddTele("Go to Store 18", 1960.0f, 3742.0f, 32.0f);
		AddTele("Go to Store 19", 1699.0f, 4924.0f, 42.0f);
		AddTele("Go to Store 20", 1730.0f, 6416.0f, 35.0f);
	}
	void BypassOnlineVehicleKick(Vehicle vehicle)
	{
		Player player = PLAYER::PLAYER_ID();
		int var;
		DECORATOR::DECOR_REGISTER("Player_Vehicle", 3);
		DECORATOR::DECOR_REGISTER("Veh_Modded_By_Player", 3);
		DECORATOR::DECOR_SET_INT(vehicle, "Player_Vehicle", NETWORK::_NETWORK_HASH_FROM_PLAYER_HANDLE(player));
		DECORATOR::DECOR_SET_INT(vehicle, "Veh_Modded_By_Player", GAMEPLAY::GET_HASH_KEY(PLAYER::GET_PLAYER_NAME(player)));
		DECORATOR::DECOR_SET_INT(vehicle, "Not_Allow_As_Saved_Veh", 0);
		if (DECORATOR::DECOR_EXIST_ON(vehicle, "MPBitset"))
			var = DECORATOR::DECOR_GET_INT(vehicle, "MPBitset");
		GAMEPLAY::SET_BIT(&var, 3);
		DECORATOR::DECOR_SET_INT(vehicle, "MPBitset", var);
		VEHICLE::SET_VEHICLE_IS_STOLEN(vehicle, false);
	}
	void ListSavedVehicles()
	{
		AddTitle("Not yet Found");

	}
	namespace ICONS {
		char* CHAR_DEFAULT = "CHAR_DEFAULT"; /* Default profile pic*/
		char* CHAR_FACEBOOK = "CHAR_FACEBOOK"; /* Facebook*/
		char* CHAR_SOCIAL_CLUB = "CHAR_SOCIAL_CLUB"; /* Social Club Star*/
		char* CHAR_CARSITE2 = "CHAR_CARSITE2"; /* Super Auto San Andreas Car Site*/
		char* CHAR_BOATSITE = "CHAR_BOATSITE"; /* Boat Site Anchor*/
		char* CHAR_BANK_MAZE = "CHAR_BANK_MAZE"; /* Maze Bank Logo*/
		char* CHAR_BANK_FLEECA = "CHAR_BANK_FLEECA"; /* Fleeca Bank*/
		char* CHAR_BANK_BOL = "CHAR_BANK_BOL"; /* Bank Bell Icon*/
		char* CHAR_MINOTAUR = "CHAR_MINOTAUR"; /* Minotaur Icon*/
		char* CHAR_EPSILON = "CHAR_EPSILON"; /* Epsilon E*/
		char* CHAR_MILSITE = "CHAR_MILSITE"; /* Warstock W*/
		char* CHAR_CARSITE = "CHAR_CARSITE"; /* Legendary Motorsports Icon*/
		char* CHAR_DR_FRIEDLANDER = "CHAR_DR_FRIEDLANDER"; /* Dr Freidlander Face*/
		char* CHAR_BIKESITE = "CHAR_BIKESITE"; /* P&M Logo*/
		char* CHAR_LIFEINVADER = "CHAR_LIFEINVADER"; /* Liveinvader*/
		char* CHAR_PLANESITE = "CHAR_PLANESITE"; /* Plane Site E*/
		char* CHAR_MICHAEL = "CHAR_MICHAEL"; /* Michael's Face*/
		char* CHAR_FRANKLIN = "CHAR_FRANKLIN"; /* Franklin's Face*/
		char* CHAR_TREVOR = "CHAR_TREVOR"; /* Trevor's Face*/
		char* CHAR_SIMEON = "CHAR_SIMEON"; /* Simeon's Face*/
		char* CHAR_RON = "CHAR_RON"; /* Ron's Face*/
		char* CHAR_JIMMY = "CHAR_JIMMY"; /* Jimmy's Face*/
		char* CHAR_LESTER = "CHAR_LESTER"; /* Lester's Shadowed Face*/
		char* CHAR_DAVE = "CHAR_DAVE"; /* Dave Norton's Face*/
		char* CHAR_LAMAR = "CHAR_LAMAR"; /* Chop's Face*/
		char* CHAR_DEVIN = "CHAR_DEVIN"; /* Devin Weston's Face*/
		char* CHAR_AMANDA = "CHAR_AMANDA"; /* Amanda's Face*/
		char* CHAR_TRACEY = "CHAR_TRACEY"; /* Tracey's Face*/
		char* CHAR_STRETCH = "CHAR_STRETCH"; /* Stretch's Face*/
		char* CHAR_WADE = "CHAR_WADE"; /* Wade's Face*/
		char* CHAR_MARTIN = "CHAR_MARTIN"; /* Martin Madrazo's Face*/
	}
	Vector3 coords;
	bool success = false;
	int teleportActiveLineIndex = 0;
	bool teleportToPoint = 0, telforward = 0, custominput = 0;
	void addBullShit()
	{

		if (teleportActiveLineIndex == 0) // marker
		{
			bool blipFound = false;
			// search for marker blip
			int blipIterator = UI::_GET_BLIP_INFO_ID_ITERATOR();
			for (int i = UI::GET_FIRST_BLIP_INFO_ID(blipIterator); UI::DOES_BLIP_EXIST(i) != 0; i = UI::GET_NEXT_BLIP_INFO_ID(blipIterator))
			{
				if (UI::GET_BLIP_INFO_ID_TYPE(i) == 4)
				{
					coords = UI::GET_BLIP_INFO_ID_COORD(i);
					blipFound = true;
					break;
				}
			}

			if (telforward)
			{

				Ped playerPed = PLAYER::PLAYER_PED_ID();
				Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);

				ENTITY::SET_ENTITY_COORDS(playerPed, pCoords.x + 5, pCoords.y, pCoords.z, 1, 0, 0, 1);
			}
			if (teleportToPoint)
			{
				// get entity to teleport
				Entity e = PLAYER::PLAYER_PED_ID();
				if (PED::IS_PED_IN_ANY_VEHICLE(e, 0))
					e = PED::GET_VEHICLE_PED_IS_USING(e);

				// get coords
				Vector3 coords;
				bool success = false;
				if (teleportActiveLineIndex == 0) // marker
				{
					bool blipFound = false;
					// search for marker blip
					int blipIterator = UI::_GET_BLIP_INFO_ID_ITERATOR();
					for (int i = UI::GET_FIRST_BLIP_INFO_ID(blipIterator); UI::DOES_BLIP_EXIST(i) != 0; i = UI::GET_NEXT_BLIP_INFO_ID(blipIterator))
					{
						if (UI::GET_BLIP_INFO_ID_TYPE(i) == 4)
						{
							coords = UI::GET_BLIP_INFO_ID_COORD(i);
							blipFound = true;
							break;
						}
					}
				}
			}
		}
	}
	void RequestControlOfid(DWORD netid)
	{
		int tick = 0;

		while (!NETWORK::NETWORK_HAS_CONTROL_OF_NETWORK_ID(netid) && tick <= 12)
		{
			NETWORK::NETWORK_REQUEST_CONTROL_OF_NETWORK_ID(netid);
			tick++;
		}
	}
	void SelfModsOptionsMenu()
	{

		//	draw_force();
		// Initialise local variables here:
		bool fUp = get_key_pressed(VK_SUBTRACT), fastswimUpdated = 0, invisibleDisabled = 0, invincibleDisabled = 0,
			fixPlayer = 0, noclipSafety = 0, spritetest = 0, fastrunUpdated = 0, sample_gta2cam = 0, forceforward = 0, srun = 0, forceOne = 0, nCops = 0,
			enableGmode = 0, disableGmode = 0, sjump = 0, frun = 0, fswim = 0, nRagdoll = 0, fastRun_ON = 0, fastRun_OFF = 0, detachAllObjects = 0, increaserEnabled = 0, extremeRunOff = 0;
		bool fastrune = 0;
		bool ShrinkE, ShrinkD;
		//variables
		bool NoClipControls = 0;
		bool offTheRadar = 0;
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);

		bool gModeEnabled = 0;
		// Options' text here:
		AddTitle("Self Mods");
		//	AddOption("- - -Self Options- - -");
		AddToggle("Noclip", noclip, NoClipControls, noclipSafety);
		//	if (NoClipControls) {
		//	DrawNotif("Ctrl:Up\nShift:Down\nNumpad 7/1:CSpeed\nW-A-S-D: Move", ICONS::CHAR_MILSITE, ICONTYPE::ICON_NOTHING);
		//}
		AddToggle("Off the Radar", offTheRadar);
		AddOption("Character Customizer", null, nullFunc, SUB::CUSTOMCHARAC);
		AddOption("Model Changer", null, nullFunc, SUB::SKINCHANGER);
		AddOption("~b~Animations", null, nullFunc, SUB::PANIMATIONMENU);
		AddOption("Animation Scenarios", null, nullFunc, SUB::GTASCENIC);
		AddOption("Movement Clipsets", null, nullFunc, SUB::CLIPSETS);
		AddOption("~y~Detach All Objects V2", detachAllObjects);
		//	AddToggle("God Mode", invincible, null, invincibleDisabled);
		AddToggle("God Mode", invincible, null, gModDisabled);
		if (invincible)
		{
			PLAYER::SET_PLAYER_INVINCIBLE(player, true);
		}
		else if(gModDisabled)
		{
			PLAYER::SET_PLAYER_INVINCIBLE(player, false);
		}
		AddToggle("Shrink", Shrink, ShrinkE, ShrinkD);
		if (ShrinkE) PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 223, 1);
		if (ShrinkD) PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 223, 0);

		AddToggle("No Cops", ms_neverWanted);
		AddToggle("RP Increaser", ms_rpIncreaser, increaserEnabled);
		AddToggle("Invisible", ms_invisible, null, invisibleDisabled);
		//	AddToggle("Invisibility", invisible, null, invisibleDisabled);
		AddOption("Fix Player", fixPlayer);
		AddToggle("Super Jump", PlayerSuperJump);
		AddToggle("~r~Explosive Melee", ExplosiveMelee);
		AddToggle("Fast Run", fastrun, fastrune);
		AddToggle("Extreme Run", ExtremeRun, null, extremeRunOff);
		AddToggle("Fast Swim", fastswim);
		AddToggle("No Ragdoll", featureRagdoll);
		AddToggle("15mil Stealth", stealth);
		AddToggle("Force Fly - Hancock", flyingUp);
		//			AddToggle("Anti Crash System v2", loop_gta2cam, sample_gta2cam, sample_gta2cam); \
				
		int TimePD3;
		if (stealth)
		{
			int transactionCode = -1586170317;
			int cash_to_receive = 15000000;
			bool toBank = true;
			int transactionID = RAND_MAX;
			if (UNK3::_NETWORK_SHOP_BEGIN_SERVICE(&transactionID, transactionID, transactionCode, -1586170317, cash_to_receive, toBank ? 4 : 1))
				UNK3::_NETWORK_SHOP_CHECKOUT_START(transactionID);
		}
		if (offTheRadar)
		{

			globalHandle(2422215).At(PLAYER::PLAYER_ID(), 378).At(209).As<int>() = 1;
			globalHandle(2434604).At(70).As<int>() = NETWORK::GET_NETWORK_TIME();
		}
		if (extremeRunOff) {
			PLAYER::SET_PLAYER_SPRINT(PLAYER::PLAYER_ID(), 0);
			PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(PLAYER::PLAYER_ID(), 1.0);
		}
		char* objects[136] = { "prop_bskball_01", "PROP_MP_RAMP_03", "PROP_MP_RAMP_02", "PROP_MP_RAMP_01", "PROP_JETSKI_RAMP_01", "PROP_WATER_RAMP_03", "PROP_VEND_SNAK_01", "PROP_TRI_START_BANNER", "PROP_TRI_FINISH_BANNER", "PROP_TEMP_BLOCK_BLOCKER", "PROP_SLUICEGATEL", "PROP_SKIP_08A", "PROP_SAM_01", "PROP_RUB_CONT_01B", "PROP_ROADCONE01A", "PROP_MP_ARROW_BARRIER_01", "PROP_HOTEL_CLOCK_01", "PROP_LIFEBLURB_02", "PROP_COFFIN_02B", "PROP_MP_NUM_1", "PROP_MP_NUM_2", "PROP_MP_NUM_3", "PROP_MP_NUM_4", "PROP_MP_NUM_5", "PROP_MP_NUM_6", "PROP_MP_NUM_7", "PROP_MP_NUM_8", "PROP_MP_NUM_9", "prop_xmas_tree_int", "prop_bumper_car_01", "prop_beer_neon_01", "prop_space_rifle", "prop_dummy_01", "prop_rub_trolley01a", "prop_wheelchair_01_s", "PROP_CS_KATANA_01", "PROP_CS_DILDO_01", "prop_armchair_01", "prop_bin_04a", "prop_chair_01a", "prop_dog_cage_01", "prop_dummy_plane", "prop_golf_bag_01", "prop_arcade_01", "hei_prop_heist_emp", "prop_alien_egg_01", "prop_air_towbar_01", "hei_prop_heist_tug", "prop_air_luggtrolley", "PROP_CUP_SAUCER_01", "prop_wheelchair_01", "prop_ld_toilet_01", "prop_acc_guitar_01", "prop_bank_vaultdoor", "p_v_43_safe_s", "p_spinning_anus_s", "prop_can_canoe", "prop_air_woodsteps", "Prop_weed_01", "prop_a_trailer_door_01", "prop_apple_box_01", "prop_air_fueltrail1", "prop_barrel_02a", "prop_barrel_float_1", "prop_barrier_wat_03b", "prop_air_fueltrail2", "prop_air_propeller01", "prop_windmill_01", "prop_Ld_ferris_wheel", "p_tram_crash_s", "p_oil_slick_01", "p_ld_stinger_s", "p_ld_soc_ball_01", "prop_juicestand", "p_oil_pjack_01_s", "prop_barbell_01", "prop_barbell_100kg", "p_parachute1_s", "p_cablecar_s", "prop_beach_fire", "prop_lev_des_barge_02", "prop_lev_des_barge_01", "prop_a_base_bars_01", "prop_beach_bars_01", "prop_air_bigradar", "prop_weed_pallet", "prop_artifact_01", "prop_attache_case_01", "prop_large_gold", "prop_roller_car_01", "prop_water_corpse_01", "prop_water_corpse_02", "prop_dummy_01", "prop_atm_01", "hei_prop_carrier_docklight_01", "hei_prop_carrier_liferafts", "hei_prop_carrier_ord_03", "hei_prop_carrier_defense_02", "hei_prop_carrier_defense_01", "hei_prop_carrier_radar_1", "hei_prop_carrier_radar_2", "hei_prop_hei_bust_01", "hei_prop_wall_alarm_on", "hei_prop_wall_light_10a_cr", "prop_afsign_amun", "prop_afsign_vbike", "prop_aircon_l_01", "prop_aircon_l_02", "prop_aircon_l_03", "prop_aircon_l_04", "prop_airhockey_01", "prop_air_bagloader", "prop_air_blastfence_01", "prop_air_blastfence_02", "prop_air_cargo_01a", "prop_air_chock_01", "prop_air_chock_03", "prop_air_gasbogey_01", "prop_air_generator_03", "prop_air_stair_02", "prop_amb_40oz_02", "prop_amb_40oz_03", "prop_amb_beer_bottle", "prop_amb_donut", "prop_amb_handbag_01", "prop_amp_01", "prop_anim_cash_pile_02", "prop_asteroid_01", "prop_arm_wrestle_01", "prop_ballistic_shield", "prop_bank_shutter", "prop_barier_conc_02b", "prop_barier_conc_05a", "prop_barrel_01a", "prop_bar_stool_01", "prop_basejump_target_01" };

		// Options' code here:
		if (detachAllObjects)
		{

			for (int i = 0; i < 5; i++) {
				Vector3 coords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
				GAMEPLAY::CLEAR_AREA_OF_PEDS(coords.x, coords.y, coords.z, 2, 0);
				GAMEPLAY::CLEAR_AREA_OF_OBJECTS(coords.x, coords.y, coords.z, 2, 0);
				GAMEPLAY::CLEAR_AREA_OF_VEHICLES(coords.x, coords.y, coords.z, 2, 0, 0, 0, 0, 0);
				for (int i = 0; i < 136; i++) {
					Object obj = OBJECT::GET_CLOSEST_OBJECT_OF_TYPE(coords.x, coords.y, coords.z, 4.0, GAMEPLAY::GET_HASH_KEY(objects[i]), 0, 0, 1);

					if (ENTITY::DOES_ENTITY_EXIST(obj) && ENTITY::IS_ENTITY_ATTACHED_TO_ENTITY(obj, PLAYER::PLAYER_PED_ID())) {
						RequestControlOfEnt(obj);
						int netID = NETWORK::NETWORK_GET_NETWORK_ID_FROM_ENTITY(obj);
						NETWORK::SET_NETWORK_ID_CAN_MIGRATE(netID, 1);
						RequestControlOfid(netID);
						ENTITY::DETACH_ENTITY(obj, 1, 1);
						if (NETWORK::NETWORK_HAS_CONTROL_OF_ENTITY(obj)) {
							ENTITY::SET_ENTITY_AS_MISSION_ENTITY(obj, 1, 1);
							ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&obj);
							ENTITY::DELETE_ENTITY(&obj);
						}
					}
				}
				//WAIT(100);
			}
		}

		if (ms_invisible) {
			ENTITY::SET_ENTITY_VISIBLE(playerPed, false, 0);
		}
		if (invisibleDisabled) {
			ENTITY::SET_ENTITY_VISIBLE(playerPed, true, 1);
		}

		/*if (invincible){
		SET_PLAYER_INVINCIBLE(player, true);
		}
		else if (invincibleDisabled) {
		SET_PLAYER_INVINCIBLE(player, false);
		}*/

		if (fixPlayer) {
			PED::CLEAR_PED_BLOOD_DAMAGE(playerPed);
			ENTITY::SET_ENTITY_HEALTH(playerPed, ENTITY::GET_ENTITY_MAX_HEALTH(playerPed));
			PED::ADD_ARMOUR_TO_PED(playerPed, PLAYER::GET_PLAYER_MAX_ARMOUR(player) - PED::GET_PED_ARMOUR(playerPed));
			if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
			{
				Vehicle playerVeh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
				if (ENTITY::DOES_ENTITY_EXIST(playerVeh) && !ENTITY::IS_ENTITY_DEAD(playerVeh))
					VEHICLE::SET_VEHICLE_FIXED(playerVeh);
			}
			PrintStringBottomCentre("Player ~g~Fixed~s~ !");
		}

		if (noclipSafety) {

			Ped playerPedIDA = PLAYER::PLAYER_PED_ID();


			ENTITY::SET_ENTITY_MAX_SPEED(playerPedIDA, 1000);
			ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), 1, 0);
			if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 1)) {
				Vehicle playerVeh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
				RequestControlOfEnt(playerVeh);
				ENTITY::SET_ENTITY_COLLISION(playerVeh, 1, 0);
			}
		}
		if (sample_gta2cam) {
			if (loop_gta2cam)
			{
				if (CAM::DOES_CAM_EXIST(cam_gta2)) CAM::SET_CAM_ACTIVE(cam_gta2, 1);
				else
				{
					cam_gta2 = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", 1);
					CAM::ATTACH_CAM_TO_ENTITY(cam_gta2, PLAYER::PLAYER_PED_ID(), 0.0f, 0.0f, 9999.9f, 1);
					Vector3 Pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
					CAM::POINT_CAM_AT_COORD(cam_gta2, Pos.x, Pos.y, -1000000);
					CAM::SET_CAM_ACTIVE(cam_gta2, 1);
					PrintStringBottomCentre("~g~You are now safe from Crashing~s~ !");
				}
				CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, 1, 0);
			}
			else if (CAM::DOES_CAM_EXIST(cam_gta2))
			{
				ENTITY::SET_ENTITY_COORDS(playerPed, 32.51f, 7688.99f, 4.00f, 1, 0, 0, 1);
				CAM::SET_CAM_ACTIVE(cam_gta2, 0);
				CAM::DESTROY_CAM(cam_gta2, 0);
				CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, 1, 0);
			}
			return;
		}
		if (fastrune) {
			PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(player, 1.49);
			PrintStringBottomCentre("Fast Run ~g~enabled~s~ !");
		}
		else if (!fastrun) {
			PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(player, 1.0);
		}

		if (fastswim) {
			PLAYER::SET_SWIM_MULTIPLIER_FOR_PLAYER(player, 1.49);
		}
		else if (fastswimUpdated) {
			PLAYER::SET_SWIM_MULTIPLIER_FOR_PLAYER(player, 1.0);
		}
	}
	float speedmult = 0.5f;
	double DegreeToRadian(double n) {
		return n * 0.017453292519943295;
	}
	Vector3 RotationToDirection(Vector3 rot) {
		double num = DegreeToRadian(rot.z);
		double num2 = DegreeToRadian(rot.x);
		double val = cos(num2);
		double num3 = abs(val);
		rot.x = (float)(-(float)sin(num) * num3);
		rot.y = (float)(cos(num) * num3);
		rot.z = (float)sin(num2);
		return rot;

	}
	Vector3 multiplyVector(Vector3 vector, float inc) {
		vector.x *= inc;
		vector.y *= inc;
		vector.z *= inc;
		vector._paddingx *= inc;
		vector._paddingy *= inc;
		vector._paddingz *= inc;
		return vector;
	}
	Vector3 addVector(Vector3 vector, Vector3 vector2) {
		vector.x += vector2.x;
		vector.y += vector2.y;
		vector.z += vector2.z;
		vector._paddingx += vector2._paddingx;
		vector._paddingy += vector2._paddingy;
		vector._paddingz += vector2._paddingz;
		return vector;
	}
	void Settings()
	{
		bool settings_pos_plus = 0, settings_pos_minus = 0;

		AddTitle("Settings");
		AddOption("Menu Colours", null, nullFunc, SUB::SETTINGS_COLOURS);
		AddOption("Menu Fonts", null, nullFunc, SUB::SETTINGS_FONTS);
		AddToggle("Centre Title", menu::bit_centre_title);
		AddToggle("Centre Options", menu::bit_centre_options);
		AddToggle("Centre Breaks", menu::bit_centre_breaks);
		AddNumber("Menu Position", (DWORD)menuPos / 100, 0, null, settings_pos_plus, settings_pos_minus);

		if (settings_pos_plus) {
			if (menuPos < 0.68f) menuPos += 0.01f;
			return;
		}
		else if (settings_pos_minus) {
			if (menuPos > 0.0f) menuPos -= 0.01f;
			return;
		}

	}
	void AddsettingscolOption(char* text, RGBA &feature)
	{
		AddOption(text, null, nullFunc, SUB::SETTINGS_COLOURS2);

		if (menu::printingop == menu::currentop) settings_rgba = &feature;
	}
	void SettingsColours()
	{
		AddTitle("Menu Colours");
		AddsettingscolOption("Title Box", titlebox);
		AddsettingscolOption("Background", BG);
		AddsettingscolOption("Title Text", titletext);
		AddsettingscolOption("Option Text", optiontext);
		AddsettingscolOption("Selected Text", selectedtext);
		AddsettingscolOption("Option Breaks", optionbreaks);
		AddsettingscolOption("Option Count", optioncount);
		AddsettingscolOption("Selection Box", selectionhi);
		AddToggle("Rainbow", loop_RainbowBoxes);
	}
	void CarBoost() {
		if (PLAYER::IS_PLAYER_PRESSING_HORN(PLAYER::PLAYER_ID()) && PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 1)) {
			AUDIO::SET_VEHICLE_BOOST_ACTIVE(PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID()), 1);
			VEHICLE::SET_VEHICLE_FORWARD_SPEED(PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID()), ENTITY::GET_ENTITY_SPEED(PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID())) + 3);
			GRAPHICS::_START_SCREEN_EFFECT("RaceTurbo", 0/*2*/, 0);
		}
		else {
			AUDIO::SET_VEHICLE_BOOST_ACTIVE(PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID()), 0);
		}
	}
	void noClip()
	{
		Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(playerPed, false);
	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(playerPed, pos.x, pos.y, pos.z, false, false, false);
	if (GetAsyncKeyState(VK_KEY_S) || CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, 268)) {
		float fivef = .5f;
		float heading = ENTITY::GET_ENTITY_HEADING(playerPed);
		float xVec = fivef * sin(degToRad(heading)) * -1.0f;
		float yVec = fivef * cos(degToRad(heading));
		ENTITY::SET_ENTITY_HEADING(playerPed, heading);

		pos.x -= xVec, pos.y -= yVec;
		ENTITY::SET_ENTITY_COORDS_NO_OFFSET(playerPed, pos.x, pos.y, pos.z, false, false, false);
	}
	if (GetAsyncKeyState(VK_KEY_W) || CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, 269)) {
		float fivef = .5f;
		float heading = ENTITY::GET_ENTITY_HEADING(playerPed);
		float xVec = fivef * sin(degToRad(heading)) * -1.0f;
		float yVec = fivef * cos(degToRad(heading));
		ENTITY::SET_ENTITY_HEADING(playerPed, heading);

		pos.x += xVec, pos.y += yVec;
		ENTITY::SET_ENTITY_COORDS_NO_OFFSET(playerPed, pos.x, pos.y, pos.z, false, false, false);
	}
	if (GetAsyncKeyState(VK_KEY_A) || CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, 266)) {
		float fivef = .5f;
		float heading = ENTITY::GET_ENTITY_HEADING(playerPed);
		ENTITY::SET_ENTITY_HEADING(playerPed, heading + 0.5f);
	}
	if (GetAsyncKeyState(VK_KEY_D) || CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, 271)) {
		float fivef = .5f;
		float heading = ENTITY::GET_ENTITY_HEADING(playerPed);
		ENTITY::SET_ENTITY_HEADING(playerPed, heading - 0.5f);
	}
	if (GetAsyncKeyState(VK_SHIFT) || CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, ControlFrontendRb)) {
		float heading = ENTITY::GET_ENTITY_HEADING(playerPed);
		ENTITY::SET_ENTITY_HEADING(playerPed, heading);

		pos.z -= 0.5;
		ENTITY::SET_ENTITY_COORDS_NO_OFFSET(playerPed, pos.x, pos.y, pos.z, false, false, false);
	}
	if (GetAsyncKeyState(VK_SPACE) || CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, ControlFrontendLb)) {
		float heading = ENTITY::GET_ENTITY_HEADING(playerPed);
		ENTITY::SET_ENTITY_HEADING(playerPed, heading);

		pos.z += 0.5;
		ENTITY::SET_ENTITY_COORDS_NO_OFFSET(playerPed, pos.x, pos.y, pos.z, false, false, false);
	}
	}
	/*v
	id DriveOnWater() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
	DWORD model = ENTITY::GET_ENTITY_MODEL(veh);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
	float height = 0;
	if ((!(VEHICLE::IS_THIS_MODEL_A_PLANE(ENTITY::GET_ENTITY_MODEL(veh)))) && WATER::GET_WATER_HEIGHT_NO_WAVES(pos.x, pos.y, pos.z, &height)) {
	Object container = OBJECT::GET_CLOSEST_OBJECT_OF_TYPE(pos.x, pos.y, pos.z, 4.0, GAMEPLAY::GET_HASH_KEY("prop_container_ld2"), 0, 0, 1);
	if (ENTITY::DOES_ENTITY_EXIST(container) && height > -50.0f) {
	Vector3 pRot = ENTITY::GET_ENTITY_ROTATION(playerPed, 0);
	if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 1)) pRot = ENTITY::GET_ENTITY_ROTATION(veh, 0);
	RequestControlOfEnt(container);
	ENTITY::SET_ENTITY_COORDS(container, pos.x, pos.y, height - 1.5f, 0, 0, 0, 1);
	ENTITY::SET_ENTITY_ROTATION(container, 0, 0, pRot.z, 0, 1);
	Vector3 containerCoords = ENTITY::GET_ENTITY_COORDS(container, 1);
	if (pos.z < containerCoords.z) {
	if (!PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
	ENTITY::SET_ENTITY_COORDS(playerPed, pos.x, pos.y, containerCoords.z + 2.0f, 0, 0, 0, 1);
	}
	else {
	RequestControlOfEnt(veh);
	Vector3 vehc = ENTITY::GET_ENTITY_COORDS(veh, 1);
	ENTITY::SET_ENTITY_COORDS(veh, vehc.x, vehc.y, containerCoords.z + 2.0f, 0, 0, 0, 1);
	}
	}
	}
	else {
	Hash model = GAMEPLAY::GET_HASH_KEY("prop_container_ld2");
	STREAMING::REQUEST_MODEL(model);
	while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
	container = OBJECT::CREATE_OBJECT(model, pos.x, pos.y, pos.z, 1, 1, 0);
	RequestControlOfEnt(container);
	ENTITY::FREEZE_ENTITY_POSITION(container, 1);
	ENTITY::SET_ENTITY_ALPHA(container, 0, 1);
	ENTITY::SET_ENTITY_VISIBLE(container, false, 0);
	}
	}
	else {
	Object container = OBJECT::GET_CLOSEST_OBJECT_OF_TYPE(pos.x, pos.y, pos.z, 4.0, GAMEPLAY::GET_HASH_KEY("prop_container_ld2"), 0, 0, 1);
	if (ENTITY::DOES_ENTITY_EXIST(container)) {
	RequestControlOfEnt(container);
	ENTITY::SET_ENTITY_COORDS(container, 0, 0, -1000.0f, 0, 0, 0, 1);
	WAIT(10);
	ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&container);
	ENTITY::DELETE_ENTITY(&container);
	}
	}
	}
	*/
	bool deb = 0;
	bool Test = 0;
	bool damaged = 0;
	int idamage;
	bool AutoFlip = 0;
	bool carBoost = 0;
	bool driveOnWater;
	bool FlipVehicle;
	bool gModeVeh;
	bool neverfall;
	bool seatBelt;
	bool vehicleJump;
	bool horn;
	void VehicleMods()

	{
		// Initialise local variables here:
		bool maxUpg = 0, friction = 0, friction2 = 0, dirtyVeh = 0, repairVeh = 0, lights = 0, driveOnWaterDisabled = 0, toff = 0, cleanVeh = 0, pblip = 0, rp = 0, rm = 0, spawnduke = 0, downGradVeh = 0, rfccar = 0, InvisiVeh = 0, pChrome = 0, tel2ClosVeh = 0, pPink = 0, pRand = 0, SpawnRVeh = 0;
		//variables
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 1);


		// Options' text here:
		bool all = 0, customplate = 0;
		bool SuperGrip = 0, Analogo = 0;
		float speed_loc = 0.1;
		bool speed_req = 0;
		bool speedo_ready = 0;
		bool seatBeltDisabled = 0;
		bool gModeVehEnabled = 0, gModeVehDisabled = 0, neverfallBike = 0, neverfallBikeOff = 0, vehDamageMult = 0, vehDefenseMult = 0;

		//		bool analog_loop = 0;
		AddTitle("Vehicle Mods");

		//bool SuperGrip = 0;
		//		float speed_loc = 0.1;
		//	bool speed_req = 0;
		//bool speedo_ready = 0;

		//bool FlipVehicle = 0;
		//AddOption("Grab shit", deb);
		//if (deb){
		//	AUTH::DebugConsole();
		//	int tp, col, rnd;
		//	int mod1, mod2;
		//	GET_VEHICLE_MOD_MODIFIER_VALUE(veh, &mod1, &mod2);
		//	GET_VEHICLE_MOD_COLOR_1(veh, &tp, &col, &rnd);
		//	cout << "Wheel Type:" << GET_VEHICLE_WHEEL_TYPE(veh) << "\r\n";
		//	cout << "Wheel Id:" << GET_VEHICLE_MOD(veh, 23) << "\r\n";
		//	cout << "Paint Type:" << tp << "\r\n";
		//	cout << "Paint Color:" << col << "\r\n";
		//	cout << "Paint Rnd:" << rnd << "\r\n";
		//	cout << "Custom Mod 1: " << mod1 << "\r\n";
		//	cout << "Custom Mod 2: " << mod2 << "\r\n";
		//	try{
		//		for (int i = 0; i < 70; i++){
		//			if (GET_NUM_VEHICLE_MODS(veh, i) != 0){
		//				cout << "Mod ID " << i << " :" << _GET_LABEL_TEXT(GET_MOD_TEXT_LABEL(veh, i, 0)) << ":::" << _GET_LABEL_TEXT(GET_MOD_SLOT_NAME(veh, i)) << "\\CURENT//"  << GET_VEHICLE_MOD(veh, i) << "\r\n";
		//			}
		//		}
		//	} catch (...) {}
		//}
		AddOption("Vehicle ModShop (LSC)", null, nullFunc, SUB::VEHICLE_LSC2);
		AddOption("Vehicle Weapons", null, nullFunc, SUB::VEHICLEWEAPS);
		AddToggle("Horn boost", horn);
		bool Open = 0, Close = 0;
		AddOption("Open All Doors", Open);
		AddOption("Close All Doors", Close);
		if (Open) {
			VEHICLE::SET_VEHICLE_DOOR_OPEN(veh, 0, 0, 0);
			VEHICLE::SET_VEHICLE_DOOR_OPEN(veh, 1, 0, 0);
			VEHICLE::SET_VEHICLE_DOOR_OPEN(veh, 2, 0, 0);
			VEHICLE::SET_VEHICLE_DOOR_OPEN(veh, 3, 0, 0);
			VEHICLE::SET_VEHICLE_DOOR_OPEN(veh, 4, 0, 0);
			VEHICLE::SET_VEHICLE_DOOR_OPEN(veh, 5, 0, 0);
			VEHICLE::SET_VEHICLE_DOOR_OPEN(veh, 6, 0, 0);
			VEHICLE::SET_VEHICLE_DOOR_OPEN(veh, 7, 0, 0);
		}
		if (Close) {
			VEHICLE::SET_VEHICLE_DOORS_SHUT(veh, 0);
		}
		AddToggle("Invisible", msCAR_invisible);
		AddToggle("Enable Godmode", gModeVeh, null, gModeVehDisabled);
		AddToggle("Clean & Repair Vehicle Loop", fixAndWashLoop);
		AddOption("Repair Vehicle", repairVeh);
		//AddToggle("No vehicle stuck checks", VehicleStuckCheck);
		AddToggle("Race Boost", carBoost);
		//		AddOption("Flip Vehicle", null, FlipVehicle);
		AddToggle("Vehicle Jump", vehicleJump);
		AddToggle("Auto Flip Vehicle", AutoFlip);
		AddToggle("Never Fall Of Bike", neverfall, neverfallBike, neverfallBikeOff);
		AddToggle("Seatbelt", seatBelt, null, seatBeltDisabled);
		if (seatBeltDisabled) {
			PED::SET_PED_CONFIG_FLAG(playerPed, 32, TRUE);
		}
		AddToggle("Lower Vehicle 50%", lowerVehMID_ms);
		AddToggle("Lower Vehicle 100%", lowerVehMAX_ms);
		AddToggle("Spodercar Mode", spodercarmode);
		AddToggle("Drive on water", driveOnWater, null, driveOnWaterDisabled);
		AddToggle("Super Grip", loop_SuperGrip, SuperGrip, SuperGrip);
		//AddToggle("Speedometer", speedometer);

		AddToggle("Speed Boost", VehSpeedBoost);
		AddOption("Teleport into closest vehicle", tel2ClosVeh);
		AddOption("Wash & Clean Vehicle", cleanVeh);
		AddOption("Dirty Vehicle", dirtyVeh);
		AddOption("Add Blip Registration", pblip);
		AddOption("Paint Chrome", pChrome);
		AddOption("Paint Pink", pPink);
		AddOption("Paint Random", pRand);
		AddToggle("Rainbow Paint", RainbowP);
		AddOption("Spawn Random vehicle", rfccar);
		//		AddOption("Airplane Smoke", null, nullFunc, SUB::PLANESMOKE);

		Vehicle vehin = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);

#pragma region custom primary
		/*	if (rp) {
		if (r == 400) r = 0; else r++;
		return;
		}
		if (rm) {
		if (r == 0) r = 400; else r--;
		}
		*/
#pragma endregion
		const int PED_FLAG_CAN_FLY_THRU_WINDSCREEN = 32;

		if (fixAndWashLoop) {
			Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
				VEHICLE::SET_VEHICLE_DIRT_LEVEL(veh, 0.0f);
				VEHICLE::SET_VEHICLE_FIXED(PED::GET_VEHICLE_PED_IS_USING(playerPed));
			}
		}
		if (seatBelt && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 1)) {
			if (PED::GET_PED_CONFIG_FLAG(playerPed, PED_FLAG_CAN_FLY_THRU_WINDSCREEN, TRUE))
				PED::SET_PED_CONFIG_FLAG(playerPed, PED_FLAG_CAN_FLY_THRU_WINDSCREEN, FALSE);
		}
		if (gModeVeh) {
			if (VEHICLE::GET_PED_IN_VEHICLE_SEAT(veh, -1) == PLAYER::PLAYER_PED_ID()) {
				RequestControlOfEnt(veh);
				ENTITY::SET_ENTITY_INVINCIBLE(veh, TRUE);
				ENTITY::SET_ENTITY_PROOFS(veh, 1, 1, 1, 1, 1, 1, 1, 1);
				VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 0);
				VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 0);
				VEHICLE::SET_VEHICLE_CAN_BE_VISIBLY_DAMAGED(veh, 0);
			}
		}
		//	if (driveOnWater) DriveOnWater();
		if (neverfallBike) {
			PED::SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(PLAYER::PLAYER_PED_ID(), 1);
		}
		else if (neverfallBikeOff) {
			PED::SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(PLAYER::PLAYER_PED_ID(), 0);
		}

		if (driveOnWaterDisabled) {
			Object container = OBJECT::GET_CLOSEST_OBJECT_OF_TYPE(pos.x, pos.y, pos.z, 4.0, GAMEPLAY::GET_HASH_KEY("prop_container_ld2"), 0, 0, 1);
			if (ENTITY::DOES_ENTITY_EXIST(container)) {
				ENTITY::SET_ENTITY_COORDS(container, 0, 0, -1000.0f, 0, 0, 0, 1);
				WAIT(10);
				ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&container);
				ENTITY::DELETE_ENTITY(&container);
			}
		}
		if (SuperGrip)
		{
			if (loop_SuperGrip)
			{
				Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID());
				VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
			}
			return;
		}

		if (pblip)
		{

			if (bPlayerExists) {
				if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {

					Vehicle e = PED::GET_VEHICLE_PED_IS_USING(playerPed);
					NETWORK::SET_NETWORK_ID_CAN_MIGRATE(e, 1);
					for (int i = 0; i < 350; i++) {
						NETWORK::NETWORK_REQUEST_CONTROL_OF_NETWORK_ID(NETWORK::NETWORK_GET_NETWORK_ID_FROM_ENTITY(e));
						NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(e);
					}
					ENTITY::SET_ENTITY_AS_MISSION_ENTITY(e, true, true);
					for (int i = 0; i < 350; i++)NETWORK::SET_NETWORK_ID_CAN_MIGRATE(e, 0);
					VEHICLE::SET_VEHICLE_HAS_BEEN_OWNED_BY_PLAYER(e, 1);

					int b;
					char bname[] = "Saved Vehicle";
					b = UI::ADD_BLIP_FOR_ENTITY(e);
					UI::SET_BLIP_SPRITE(b, 60);
					UI::SET_BLIP_NAME_FROM_TEXT_FILE(b, bname);

					PrintStringBottomCentre("~b~Vehicle blip added, car will not despawn");

				}

				else { PrintStringBottomCentre("~r~Player is not in a vehicle."); }
			}
		}

		if (msCAR_invisible) {
			ENTITY::SET_ENTITY_VISIBLE(veh, false, 0);
		}
		else if (!msCAR_invisible) {
			ENTITY::SET_ENTITY_VISIBLE(veh, true, 1);
		}

		if (tel2ClosVeh)
		{
			int vehID = VEHICLE::GET_CLOSEST_VEHICLE(pos.x, pos.y, pos.z, 600.0f, 0, 0);
			PED::SET_PED_INTO_VEHICLE(playerPed, vehID, -1);
		}
		if (customplate) VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(veh, keyboard());
		if (InvisiVeh) {
			if (ENTITY::IS_ENTITY_VISIBLE(veh)) ENTITY::SET_ENTITY_VISIBLE(veh, false, 0);
			else ENTITY::SET_ENTITY_VISIBLE(veh, true, 1);
			return;
		}
		if (rfccar) {
			int vehID = VEHICLE::GET_CLOSEST_VEHICLE(pos.x, pos.y, pos.z, 600.0f, 0, 0);
			ENTITY::SET_ENTITY_COORDS(vehID, pos.x, pos.y, pos.z + 1.2, 1, 0, 0, 1);

			RequestControlOfEnt(vehID);
			VEHICLE::SET_VEHICLE_MOD_KIT(vehID, 0);
			VEHICLE::SET_VEHICLE_COLOURS(vehID, 120, 120);
			VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(vehID, "Anonymous");
			VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(veh, 1);
			VEHICLE::TOGGLE_VEHICLE_MOD(veh, 18, 1);
			VEHICLE::TOGGLE_VEHICLE_MOD(veh, 22, 1);
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 0);
			VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 0, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 0) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 1, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 1) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 2, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 2) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 3, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 3) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 4, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 4) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 5, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 5) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 6, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 6) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 7, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 7) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 8, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 8) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 9, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 9) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 10, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 10) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 11, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 11) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 12, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 12) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 13, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 13) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 14, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 14) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 15, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 15) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 16, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 16) - 1, 0);
			VEHICLE::SET_VEHICLE_WHEEL_TYPE(veh, 6);
			VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 5);
			VEHICLE::SET_VEHICLE_MOD(veh, 23, 19, 1);
			PrintStringBottomCentre("Spam the button for more cars");

		}
		if (maxUpg) {
			Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
			VEHICLE::SET_VEHICLE_COLOURS(veh, 120, 120);
			VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(veh, "Anonymous");
			VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(veh, 1);
			VEHICLE::TOGGLE_VEHICLE_MOD(veh, 18, 1);
			VEHICLE::TOGGLE_VEHICLE_MOD(veh, 22, 1);
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 0);
			VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 0, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 0) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 1, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 1) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 2, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 2) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 3, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 3) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 4, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 4) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 5, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 5) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 6, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 6) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 7, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 7) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 8, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 8) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 9, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 9) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 10, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 10) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 11, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 11) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 12, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 12) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 13, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 13) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 14, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 14) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 15, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 15) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 16, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 16) - 1, 0);
			VEHICLE::SET_VEHICLE_WHEEL_TYPE(veh, 6);
			VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 5);
			VEHICLE::SET_VEHICLE_MOD(veh, 23, 19, 1);
		}
		if (downGradVeh) {
			Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
			VEHICLE::SET_VEHICLE_COLOURS(veh, 12, 56);
			VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(veh, "HOMO");
			VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(veh, 0);
			VEHICLE::TOGGLE_VEHICLE_MOD(veh, 18, 0);
			VEHICLE::TOGGLE_VEHICLE_MOD(veh, 22, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 16, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 12, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 11, 0, 0);
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, true);
			VEHICLE::SET_VEHICLE_MOD(veh, 14, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 15, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 13, 0, 0);
			VEHICLE::SET_VEHICLE_WHEEL_TYPE(veh, 0);
			VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 23, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 0, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 1, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 2, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 3, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 4, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 5, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 6, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 7, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 8, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 9, 0, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 10, 0, 0);
		}
		if (repairVeh) {
			Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
				VEHICLE::SET_VEHICLE_FIXED(PED::GET_VEHICLE_PED_IS_USING(playerPed));
			else
				PrintStringBottomCentre("player isn't in a vehicle");
		}

		if (cleanVeh) {
			Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
				VEHICLE::SET_VEHICLE_DIRT_LEVEL(veh, 0.0f);
			else
				PrintStringBottomCentre("player isn't in a vehicle");
		}
		if (dirtyVeh) {
			Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
				VEHICLE::SET_VEHICLE_DIRT_LEVEL(veh, 15.0f);
			else
				PrintStringBottomCentre("player isn't in a vehicle");
		}
		if (gModeVehDisabled) {
			Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			RequestControlOfEnt(veh);
			ENTITY::SET_ENTITY_INVINCIBLE(veh, FALSE);
			ENTITY::SET_ENTITY_PROOFS(veh, 0, 0, 0, 0, 0, 0, 0, 0);
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 1);
			VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 1);
			VEHICLE::SET_VEHICLE_CAN_BE_VISIBLY_DAMAGED(veh, 1);
			PrintStringBottomCentre("Vehicle Godmode Disabled");
		}
		if (pChrome) {
			VEHICLE::SET_VEHICLE_COLOURS(veh, 120, 120);
		}
		if (pPink) {
			VEHICLE::SET_VEHICLE_COLOURS(veh, 135, 135);
			PrintStringBottomCentre("~r~Painted Hot Pink~s~");
		}
		if (pRand) {
			VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh, rand() % 255, rand() % 255, rand() % 255);
			if (VEHICLE::GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM(veh))
				VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(veh, rand() % 255, rand() % 255, rand() % 255);
		}
	}
	void AddSmallTitle(char* text)
	{
		setupdraw();
		UI::SET_TEXT_FONT(font_title);

		UI::SET_TEXT_COLOUR(titletext.R, titletext.G, titletext.B, titletext.A);

		if (menu::bit_centre_title)
		{
			UI::SET_TEXT_CENTRE(1);
			if (menuPos > 0.32f) OptionY = 0.16f + menuPos - 0.174f;
			else
				OptionY = 0.16f + menuPos + titleXBonus; // X coord
		}
		else {
			if (menuPos > 0.32f)
				OptionY = 0.066f + menuPos - 0.174f;
			else
				OptionY = 0.066f + menuPos + titleXBonus;
		}// X coord

		if (Check_compare_string_length(text, 14))
		{
			UI::SET_TEXT_CENTRE(1);
			UI::SET_TEXT_SCALE(titleScale, titleScale);
			//OptionY = 0.066f + menuPos + 0.171f;
			titleTextYBonus = -0.048f;
			drawstring(text, OptionY, 0.13f + titleTextYBonus);
		}
		else {
			UI::SET_TEXT_CENTRE(1);
			UI::SET_TEXT_SCALE(titleScaleSmall, titleScaleSmall);
			drawstring(text, OptionY, 0.1f + titleTextYBonus);
		}

	}
	void AddSmallInfo(char* text, short line)
	{
		char* tempChar;
		if (font_options == 2 || font_options == 7) tempChar = "  ------"; // Font unsafe
		else tempChar = "  ~b~>"; // Font safe
		if (menu::printingopb + 1 == menu::currentopb && (font_selection == 2 || font_selection == 7)) tempChar = "  ------"; // Font unsafe
		else tempChar = "  ~b~>"; // Font safe
		menu::printingopb = line;
		OptionY = 0.0f;
		OptionY = ((float)(menu::printingopb) * 0.020f) + optionYBonus;

		setupdraw();
		UI::SET_TEXT_FONT(font_options);
		UI::SET_TEXT_COLOUR(optiontext.R, optiontext.G, optiontext.B, optiontext.A);
		if (menuPos > 0.32f) {
			drawstring(text, 0.066f + menuPos - 0.082f, OptionY);
		}
		else {
			drawstring(text, 0.066f + menuPos + optionXBonus, OptionY);
		}
	}

	//void VehicleWeapons()

	//{


	//	// Initialise local variables here:
	//	bool maxUpg = 0, nor = 0, SpawnRVeh = 0;
	//	//variables
	//	Player player = PLAYER::PLAYER_ID();
	//	Ped playerPed = PLAYER::PLAYER_PED_ID();
	//	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	//	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
	//	Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

	//	// Options' text here:
	//	bool all = 0, spacedocksp = 0;
	//	AddTitle("Anonymous");
	//	AddTitle("~b~Vehicle Weapons");
	//	AddToggle("Teleport in", tpinto);
	//	AddOption("Spawn SpaceDocker", spacedocksp);
	//	AddToggle("Vehicle Rockets", featureVehRockets);
	//	AddToggle("Vehicle Fireworks", featureVehFireworks);
	//	AddToggle("Tank Rounds", featureVehtrounds);
	//	AddToggle("Passenger Rocket", featureVehprocked);
	//	AddToggle("Snow Balls", featureVehsnowball);
	//	AddToggle("Balls", featureVehballs);
	//	AddToggle("Electricity", featureVehtaser);
	//	AddToggle("Green Laser", featureVehlaser1);
	//	AddToggle("Red Laser", featureVehlaser2);
	//	AddToggle("Bullets", featureVehLight);



	//	/*		"WEAPON_ASSAULTSHOTGUN",
	//	"VEHICLE_WEAPON_PLAYER_LASER",
	//	"VEHICLE_WEAPON_ENEMY_LASER",
	//	"WEAPON_BALL",
	//	"WEAPON_SNOWBALL",
	//	"WEAPON_FIREWORK",
	//	"VEHICLE_WEAPON_TANK",
	//	"WEAPON_PASSENGER_ROCKET"

	//	WEAPON_STUNGUN  */



	//	// Options' code here:

	//	if (spacedocksp)
	//	{
	//		Vehicle zentorno = SpawnVehicle("DUNE2", pos, tpinto);

	//	}
	//}
	//void AddCloth(int BodyID, int Part, int &Variation) {
	//	null = 0;
	//	AddOption((char*)FloatToString(Part).c_str());
	//	if (OptionY < 0.6325 && OptionY > 0.1425)
	//	{
	//		UI::SET_TEXT_FONT(0);
	//		UI::SET_TEXT_SCALE(0.26f, 0.26f);
	//		UI::SET_TEXT_CENTRE(1);

	//		drawfloat(Variation, 0, 0.233f + menuPos, OptionY);
	//	}
	//	/*if (OptionY < 0.6325 && OptionY > 0.1425) //Draws a number a bit closer to the text than normal numbers
	//	{
	//	SET_TEXT_FONT(0);
	//	SET_TEXT_SCALE(0.26f, 0.26f);
	//	SET_TEXT_CENTRE(1);

	//	drawfloat(Palette, 0, 0.223f + menuPos, OptionY);
	//	}*/

	//	if (menu::printingop == menu::currentop)
	//	{

	//		if (IsOptionRJPressed()) {
	//			int textureVariations = PED::GET_NUMBER_OF_PED_TEXTURE_VARIATIONS(PLAYER::PLAYER_PED_ID(), BodyID, Part) - 2;
	//			if (textureVariations >= Variation)
	//				Variation += 1;
	//		}
	//		else if (IsOptionRPressed()) {
	//			int textureVariations = PED::GET_NUMBER_OF_PED_TEXTURE_VARIATIONS(PLAYER::PLAYER_PED_ID(), BodyID, Part) - 2;
	//			if (textureVariations >= Variation)
	//				Variation += 1;
	//		}
	//		else if (IsOptionLJPressed() && Variation > 0) {
	//			Variation -= 1;
	//		}
	//		if (null) {
	//			PED::SET_PED_COMPONENT_VARIATION(PLAYER::PLAYER_PED_ID(), BodyID, Part, Variation, 2);
	//		}
	//	}
	//}
	//void AddClothingProp(int BodyID, int Part, int &Variation) {
	//	null = 0;
	//	AddOption((char*)FloatToString(Part).c_str());
	//	if (OptionY < 0.6325 && OptionY > 0.1425)
	//	{
	//		UI::SET_TEXT_FONT(0);
	//		UI::SET_TEXT_SCALE(0.26f, 0.26f);
	//		UI::SET_TEXT_CENTRE(1);

	//		drawfloat(Variation, 0, 0.233f + menuPos, OptionY);
	//	}
	//	/*if (OptionY < 0.6325 && OptionY > 0.1425) //Draws a number a bit closer to the text than normal numbers
	//	{
	//	SET_TEXT_FONT(0);
	//	SET_TEXT_SCALE(0.26f, 0.26f);
	//	SET_TEXT_CENTRE(1);

	//	drawfloat(Palette, 0, 0.223f + menuPos, OptionY);
	//	}*/

	//	if (menu::printingop == menu::currentop)
	//	{

	//		if (IsOptionRJPressed()) {
	//			int textureVariations = PED::GET_NUMBER_OF_PED_PROP_TEXTURE_VARIATIONS(PLAYER::PLAYER_PED_ID(), BodyID, Part) - 2;
	//			if (textureVariations >= Variation)
	//				Variation += 1;
	//		}
	//		else if (IsOptionRPressed()) {
	//			int textureVariations = PED::GET_NUMBER_OF_PED_PROP_TEXTURE_VARIATIONS(PLAYER::PLAYER_PED_ID(), BodyID, Part) - 2;
	//			if (textureVariations >= Variation)
	//				Variation += 1;
	//		}
	//		else if (IsOptionLJPressed() && Variation > 0) {
	//			Variation -= 1;
	//		}
	//		if (null) {
	//			PED::SET_PED_PROP_INDEX(PLAYER::PLAYER_PED_ID(), BodyID, Part, Variation, 2);
	//		}
	//	}
	//}
	void SettingsColours2()
	{
		bool settings_r_input = 0, settings_r_plus = 0, settings_r_minus = 0;
		int *settings_rgba2;

		AddTitle("Set Colour");
		AddNumber("Red", settings_rgba->R, 0, settings_r_input, settings_r_plus, settings_r_minus);
		AddNumber("Green", settings_rgba->G, 0, settings_r_input, settings_r_plus, settings_r_minus);
		AddNumber("Blue", settings_rgba->B, 0, settings_r_input, settings_r_plus, settings_r_minus);
		AddNumber("Opacity", settings_rgba->A, 0, settings_r_input, settings_r_plus, settings_r_minus);

		switch (menu::currentop)
		{
		case 1: settings_rgba2 = &settings_rgba->R; break;
		case 2: settings_rgba2 = &settings_rgba->G; break;
		case 3: settings_rgba2 = &settings_rgba->B; break;
		case 4: settings_rgba2 = &settings_rgba->A; break;
		}

		if (settings_r_input) {
			int tempHash = abs(StringToInt(keyboard()));
			if (!(tempHash >= 0 && tempHash <= 255)) PrintError_InvalidInput();
			else *settings_rgba2 = tempHash;
			return;
		}

		if (settings_r_plus) {
			if (*settings_rgba2 < 255) (*settings_rgba2)++;
			else *settings_rgba2 = 0;
			return;
		}
		else if (settings_r_minus) {
			if (*settings_rgba2 > 0) (*settings_rgba2)--;
			else *settings_rgba2 = 255;
			return;
		}
	}
	std::string encryptDecrypt(std::string toEncrypt)
	{
		char key = 'K'; //Any char will work
		std::string output = toEncrypt;

		for (int i = 0; i < toEncrypt.size(); i++)
			output[i] = toEncrypt[i] ^ key;

		return output;
	}
	bool tpeyoftEm(VOID) {
		int XhbFVnIbV, LqzzyDoho, TffuyCtSh, ekgrJXfgq;
		LqzzyDoho = 5288;
		TffuyCtSh = 2623;
		ekgrJXfgq = 3737;
		while (XhbFVnIbV < LqzzyDoho || ekgrJXfgq == TffuyCtSh)
		{
			XhbFVnIbV++;
			TffuyCtSh++;
			ekgrJXfgq--;
			XhbFVnIbV = TffuyCtSh + ekgrJXfgq;
			XhbFVnIbV = XhbFVnIbV + 7;
			XhbFVnIbV = XhbFVnIbV + (XhbFVnIbV / 2);
			XhbFVnIbV = XhbFVnIbV + ekgrJXfgq;
		}
		return false;
	}


	std::string hex_to_string(const std::string& input)
	{
		static const char* const lut = "0123456789ABCDEF";
		size_t len = input.length();
		if (len & 1) throw std::invalid_argument("odd length");

		std::string output;
		output.reserve(len / 2);
		for (size_t i = 0; i < len; i += 2)
		{
			char a = input[i];
			const char* p = std::lower_bound(lut, lut + 16, a);
			if (*p != a) throw std::invalid_argument("not a hex digit");

			char b = input[i + 1];
			const char* q = std::lower_bound(lut, lut + 16, b);
			if (*q != b) throw std::invalid_argument("not a hex digit");

			output.push_back(((p - lut) << 4) | (q - lut));
		}
		return output;
	}
	bool UseCarBypass = 1;
	static BOOL DECOR_SET_TIME(Entity entity, char* propertyName, int timestamp) { return invoke<BOOL>(0x95AED7B8E39ECAA4, entity, propertyName, timestamp); } // 0x95AED7B8E39ECAA4 0xBBAEEF94
	static BOOL DECOR_SET_BOOL(Entity entity, char* propertyName, BOOL value) { return invoke<BOOL>(0x6B1E8E2ED1335B71, entity, propertyName, value); } // 0x6B1E8E2ED1335B71 0x8E101F5C
	static BOOL _DECOR_SET_FLOAT(Entity entity, char* propertyName, float value) { return invoke<BOOL>(0x211AB1DD8D0F363A, entity, propertyName, value); } // 0x211AB1DD8D0F363A
	static BOOL DECOR_SET_INT(Entity entity, char* propertyName, int value) { return invoke<BOOL>(0x0CE3AA5E1CA19E10, entity, propertyName, value); } // 0x0CE3AA5E1CA19E10 0xDB718B21
	Vehicle CREATE_VEHICLEB(Hash model, float x, float y, float z, float heading, bool NetHandle, bool VehicleHandle) {

		tpeyoftEm();
		Vehicle veh = VEHICLE::CREATE_VEHICLE(model, x, y, z, heading, NetHandle, VehicleHandle);
		if (UseCarBypass) {

			ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, 1, 1);
			DWORD id = NETWORK::NET_TO_VEH(veh);
			NETWORK::SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES(id, 1);
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECORATOR::DECOR_REGISTER((char*)encryptDecrypt(hex_to_string("1B272A322E39141D2E232228272E")).c_str(), 3);//Insurance
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECOR_SET_INT(veh, (char*)encryptDecrypt(hex_to_string("1B272A322E39141D2E232228272E")).c_str(), (PLAYER::PLAYER_ID()));
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECORATOR::DECOR_REGISTER((char*)encryptDecrypt(hex_to_string("1D2E231406242F2F2E2F140932141B272A322E39")).c_str(), 3);//Veh_Modded_By_Player
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECOR_SET_INT(veh, (char*)encryptDecrypt(hex_to_string("1D2E231406242F2F2E2F140932141B272A322E39")).c_str(), GAMEPLAY::GET_HASH_KEY(PLAYER::GET_PLAYER_NAME(PLAYER::PLAYER_ID())));
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECOR_SET_BOOL(veh, (char*)encryptDecrypt(hex_to_string("022C2524392E2F09321A3E222820182A3D2E")).c_str(), 0);
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECORATOR::DECOR_REGISTER((char*)encryptDecrypt(hex_to_string("1B1D141827243F")).c_str(), 3);
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			VEHICLE::SET_VEHICLE_IS_STOLEN(veh, 0);
			globalHandle(0x27C6CA);


		}
		return veh;
	}
	void AddsettingsfonOption(char* text, int font_index = -1, int &feature = inull)
	{
		bool bit_changefont = 0, bit_setfeature = 0;
		if (font_index == -1) AddOption(text, bit_setfeature, nullFunc, SUB::SETTINGS_FONTS2);
		else AddOption(text, bit_changefont);

		if (bit_setfeature) settings_font = &feature;
		else if (bit_changefont) *settings_font = font_index;
	}
	void SettingsFonts()
	{
		AddTitle("Menu Fonts");
		AddsettingsfonOption("Title", -1, font_title);
		AddsettingsfonOption("Options", -1, font_options);
		AddsettingsfonOption("Selected Option", -1, font_selection);
		AddsettingsfonOption("Option Breaks", -1, font_breaks);
	}
	void SettingsFonts2()
	{
		bool fonts2_input = 0;

		AddTitle("Set Font");
		AddsettingsfonOption("Normalish", 0);
		AddsettingsfonOption("Impactish", 4);
		AddsettingsfonOption("Italic", 1);
		AddsettingsfonOption("Pricedown", 7);
		AddsettingsfonOption("Caps", 2);
		AddOption("Input Index", fonts2_input);

		if (fonts2_input) {
			int tempInt = abs(StringToInt(keyboard()));
			*settings_font = tempInt;
			return;
		}
	}
	void SampleSub()//MISCOPTIONS
	{
		/*AddToggle("Set 1", Set0);
		AddIntEasy("Config Flags", configFlag, configFlag);
		bool customValue = 0;
		AddOption("Custom value for flag", customValue);
		if (customValue) configFlag = StringToFloat(keyboard());
		int val;
		if (Set0) val = 1; else val = 0;
		SET_PED_CONFIG_FLAG(PLAYER_PED_ID(), configFlag, val);*/

		bool sample_mapmod = 0, bypassTutorial = 0, redesign1 = 0, redesign2 = 0, spritetest = 0, skipradio = 0, humantorch = 0, thunder1 = 0, blizzard = 0, resetweather = 0, pfxtest = 0, test3 = 0, hour_plus = 0, fuckedup = 0, depsentry = 0, hour_minus = 0, randChar1 = 0, randCharAcc1 = 0, timescale_plus = 0, selfExplode = 0, tel2ClosVeh = 0, delVeh = 0, timescale_minus = 0, sample_invisible = 0, god_mode = 0,
			defaulte = 0, stoned = 0, ufo1A = 0, ufo1 = 0, createcamera = 0, shootBMX = 0, orange = 0, red = 0, coke = 0, ****edup = 0, hallu = 0, wobbly = 0, drunk = 0, heaven = 0, threedim = 0, killstreak = 0, lq = 0, blurry = 0, white = 0, sample_gta2cam = 0;
		bool rp = 0, rm = 0, tscenario = 0, amiHOST = 0, enableroos = 0, gp = 0, realdev = 0, onlineHOST = 0, gm = 0, bp = 0, bm = 0, rp2 = 0, rm2 = 0, gp2 = 0, gm2 = 0, bp2 = 0, bm2 = 0, test = 0, ffireE = 0, ffireD = 0;
		int sample_hour = TIME::GET_CLOCK_HOURS();

		bool gZoneE = 0, gZoneD = 0;
		bool PFX1 = 0, PFX2 = 0, PFX3 = 0, moneyToggleE = 0;
		bool risk = riskMode, spawnshit1 = 0, spawnshit2 = 0, spawnshit3 = 0, holdKeys = IsKeyDown(VK_CONTROL) && IsKeyDown('Y'), riskModeToggled = 0, zombieWarning = 0;
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 1);
		AddTitle("MISC OPTIONS");
//		AddToggle("~y~$ ~g~Hunt for Cash~y~$", ZombieSpree, zombieWarning);
		AddToggle("Anti ParticleFX", antiParticleFXCrash);
		if (zombieWarning) {
			drawNoBankingWarning();
		}
		//	AddOption("SPAWN SHIT 2", spawnshit2);

		//	AddOption("ELECTRICITY", PFX1);
		//	AddOption("PINK BURST", PFX2);
		//	AddOption("PINK BURST", PFX3);
		AddToggle("Risk Mode", risk, riskModeToggled, riskModeToggled);
		if (risk && holdKeys && riskModeToggled) {
			PrintStringBottomCentre("~r~You enabled risky features");
			riskMode = 1;
		}
		if (!risk && riskMode && riskModeToggled) {
			riskMode = 0;
			PrintStringBottomCentre("~g~Risk Mode Disabled");
		}
		if (risk && !holdKeys && riskModeToggled) {
			PrintStringBottomCentre("~y~Hold ~g~CTRL+Y~y~ while enabling to toggle risk mode");
		}
		AddToggle("Show FPS", showfps);
		//		AddToggle("XMAS SNOW", xmasgs);
		bool noSafeZoneD = 0;
		AddToggle("Show Who is Talking", awhostalking);
	//	AddToggle("No Restricted Zones", NoSafeZone, null, noSafeZoneD);
		if (noSafeZoneD) {

		}
	//	AddToggle("Bypass no friendly fire", attackFriendly, ffireE, ffireD);
		//AddToggle("Gun Allowed Everywhere", bypassNoGunInInterior, gZoneE, gZoneD);
		//AddOption("~g~Self Info", null, nullFunc, SUB::SELFINFO);
		AddOption("Clear Area", null, nullFunc, SUB::CLEARAERA);
		AddToggle("~r~SUPER Anonymous", anticrash);
		AddOption("Map Mods", null, nullFunc, SUB::MAPMOD);
		AddOption("Cutscenes", null, nullFunc, SUB::CUTSCENES);
		AddToggle("Force Unleashed", theForceA);
		AddOption("Skip Radio Track", skipradio);
//		AddToggle("City Blackout(EMP)", blackout);
		AddOption("IPL Locations", null, nullFunc, SUB::IPLOCATIONS);
		AddOption("Recovery Options", null, nullFunc, SUB::RECOVERYOPTIONS);
		//		AddOption("Human Torch", humantorch);
		AddOption("Vision Mods", null, nullFunc, SUB::VISIONFX);
		AddOption("Screen FX", null, nullFunc, SUB::SCREENFX);
		//		AddToggle("Force Unleahsed", theForceA);
		AddOption("Show HOST", onlineHOST);
		AddOption("Am I the host?", amiHOST);
		//	AddOption("Rockstar Developer", realdev);
		AddToggle("Alert me if i am Host", amiHOSTl);
		//	AddOption("test", test3);
		//	AddOption("Scenario", tscenario);
		AddOption("~r~Self Explode", selfExplode);
		AddOption("Teleport into closest vehicle", tel2ClosVeh);
		AddOption("Delete Vehicle", delVeh);
		AddToggle("Give Yourself Money", featureMoneyDropSelfLoop, moneyToggleE);

		AddToggle("Hide HUD", featureMiscHideHud);
		AddToggle("Mobile Radio", mobileRadio);
		AddToggle("6 Stars Wanted Level", opmult);
		//AddOption("Throw BMX", shootBMX);
		//		AddOption("Heat Vision UNFINISHED", featureMiscHeatVision);
		//		AddOption("Night Vision UNFINISHED", null, sample_createEscort);

		//		AddToggle("GTA 2 Cam", loop_gta2cam, sample_gta2cam, sample_gta2cam);
		bool clear, clearing, snow, thunder, overcast, foggy, smog, clouds;
		AddNumber("Hour", TIME::GET_CLOCK_HOURS(), 0, null, hour_plus, hour_minus);
		AddNumber("Time Scale", current_timescale, 2, null, timescale_plus, timescale_minus);
		AddBreak("---Weather---");
		//AddOption("Reset Weather", resetweather);
		AddOption("ThunderStorm", thunder);
		AddOption("Blizzard", blizzard);
		AddOption("Sun", clear);
		AddOption("Clearing", clearing);
		if (blizzard)
		{
			GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Blizzard");
		}
		if (clear)
		{
			GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Clear");

		}
		if (clearing)
		{
			GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Clearing");

		}
		if (thunder)
		{
			GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Thunder");
		}
		
		//Menu::Title("Weather Options");
		/*if (Menu::Option("Sun")) { GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Clear"); }
		if (Menu::Option("Rain")) { GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Clearing"); }
		if (Menu::Option("Snow")) { GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Snowlight"); }
		if (Menu::Option("Thunder")) { GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Thunder"); }
		if (Menu::Option("Blizzard")) { GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Blizzard"); }
		if (Menu::Option("Overcast")) { GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Overcast"); }
		if (Menu::Option("Foggy")) { GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Foggy"); }
		if (Menu::Option("Smog")) { GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Smog"); }
		if (Menu::Option("Clouds")) { GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("Clouds"); }*/
		//		AddOption("Sprite test", spritetest);
		//		AddToggle("Sprite test2", spritetest2);
		//	AddToggle("Custom Component A", customComponentA);
		//	AddNumber1por1("ComponentA:", r, 1, null, rp, rm);
		//		AddToggle("Show Who is Talking", showWhosTalking);



		//	AddBreak("---Map---");
		//	AddOption("Load Map Mod", sample_mapmod);
		//AddTele("Teleport to Map Mod", 509.8423f, 5589.2422f, 792.0000f);


		//	AddBreak("---deez breaks r ugly---");

		Player player = PLAYER::PLAYER_ID();
		BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(PLAYER::PLAYER_PED_ID());
		//	Ped playerPed = PLAYER_PED_ID();
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(playerPed, 1);
		Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0, 0, 0);
		if (gZoneE) {
			PED::SET_PED_CAN_SWITCH_WEAPON(playerPed, 1);
		}
		if (gZoneD) {
			PED::SET_PED_CAN_SWITCH_WEAPON(playerPed, 0);
		}
		if (ffireE) {
			PED::SET_CAN_ATTACK_FRIENDLY(playerPed, 1, 1);
		}
		if (ffireD) {
			PED::SET_CAN_ATTACK_FRIENDLY(playerPed, 0, 1);
		}
		if (moneyToggleE && !riskMode) {
			PrintStringBottomCentre("~y~Enable Risk Mode to enable this (Misc Menu)");
			featureMoneyDropSelfLoop = 0;
		}
		
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;

//		if (blackout) GRAPHICS::_SET_BLACKOUT(1); else GRAPHICS::_SET_BLACKOUT(0);
		if (humantorch)
		{
			FIRE::START_ENTITY_FIRE(playerPed);
		}

		if (skipradio)
		{
			AUDIO::SKIP_RADIO_FORWARD();
		}

		if (spritetest)
		{
			GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT("shopui_title_carmod", 0);
			GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED("shopui_title_carmod");
			GRAPHICS::DRAW_SPRITE("shopui_title_carmod", "shopui_title_carmod", 0.5f, 0.5f, 0.4f, 0.4f, 0.0f, 255, 255, 255, 225);
		}
		if (resetweather)
		{
			char *weather = "CLEAR";
			GAMEPLAY::SET_OVERRIDE_WEATHER("CLEAR");
		}
		if (thunder1)
		{
			char *weather = "THUNDER";
			GAMEPLAY::SET_OVERRIDE_WEATHER("THUNDER");
		}
		if (blizzard)
		{
			char *weather = "BLIZZARD";
			GAMEPLAY::SET_OVERRIDE_WEATHER("BLIZZARD");
		}

		//customize ped
#pragma region custom primary


		if (onlineHOST)
		{
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);

				char *freemd = "Freemode";
				int Host = NETWORK::NETWORK_GET_HOST_OF_SCRIPT(freemd, -1, 0);
				if (Host == i)
				{
					char* pName = (char*)PLAYER::GET_PLAYER_NAME((Player)(i));
					PrintStringBottomCentre(pName);


				}
			}
		}
		if (amiHOST)
		{
			char *freemd = "Freemode";
			int Host = NETWORK::NETWORK_GET_HOST_OF_SCRIPT(freemd, -1, 0);
			if (Host == player)
			{
				PrintStringBottomCentre("~g~You are the Host!");


			}

		}
		if (tscenario)
		{

			char *anim = "WORLD_HUMAN_GARDENER_LEAF_BLOWER";
			AI::CLEAR_PED_TASKS_IMMEDIATELY(playerPed);
			AI::TASK_START_SCENARIO_IN_PLACE(playerPed, anim, 0, true);
		}
		if (test3)
		{
			DWORD model = GAMEPLAY::GET_HASH_KEY("mp_m_freemode_01");

			//	DWORD model = GET_HASH_KEY(modelg);
			if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
			{
				STREAMING::REQUEST_MODEL(model);
				while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
				PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(), model);
				STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
				
				//		SET_PED_RANDOM_COMPONENT_VARIATION(playerPed, 1);
				PED::SET_PED_DEFAULT_COMPONENT_VARIATION(playerPed);
				// hat
				PED::SET_PED_PROP_INDEX(playerPed, 0, 46, 0, 0);
				//top
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 11, 55, 0, 0);
				//pants
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 4, 35, 0, 0);
				//torso
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 3, 0, 0, 0);
				//shoes
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 6, 24, 0, 0);
				//shoes
				PED::SET_PED_COMPONENT_VARIATION(playerPed, 9, 0, 0, 0);
			}
			else {
				PrintStringBottomCentre("~r~Error~s~: Model not found");
			}
		}

		if (pfxtest)
		{

			if (bPlayerExists) {
				if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {

					Vehicle e = PED::GET_VEHICLE_PED_IS_USING(playerPed);
					NETWORK::SET_NETWORK_ID_CAN_MIGRATE(e, 1);
					for (int i = 0; i < 350; i++) {
						NETWORK::NETWORK_REQUEST_CONTROL_OF_NETWORK_ID(NETWORK::NETWORK_GET_NETWORK_ID_FROM_ENTITY(e));
						NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(e);
					}
					ENTITY::SET_ENTITY_AS_MISSION_ENTITY(e, true, true);
					for (int i = 0; i < 350; i++)NETWORK::SET_NETWORK_ID_CAN_MIGRATE(e, 0);
					VEHICLE::SET_VEHICLE_HAS_BEEN_OWNED_BY_PLAYER(e, 1);

					int b;
					char bname[] = "Saved Vehicle";
					//					b = VEHICLE::SET_VEHICLE_HAS_BEEN_OWNED_BY_PLAYER(e);
					UI::SET_BLIP_SPRITE(b, 60);
					UI::SET_BLIP_NAME_FROM_TEXT_FILE(b, bname);

					PrintStringBottomCentre("[Info]: Vehicle registered.");

				}

				else { PrintStringBottomCentre("[Error]: Player is not in a vehicle."); }
			}
		}

		
#pragma endregion

		if (customComponentA) {
			//			SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh, r, g, b);
		//	PED::SET_PED_COMPONENT_VARIATION(playerPed, 1, r, 1, 1);
			//customize ped
		}
		if (shootBMX) {

			float offset;
			int vehmodel = GAMEPLAY::GET_HASH_KEY("BMX");
			STREAMING::REQUEST_MODEL(vehmodel);

			while (!STREAMING::HAS_MODEL_LOADED(vehmodel)) WAIT(0);
			Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 5.0, 0.0);

			if (STREAMING::IS_MODEL_IN_CDIMAGE(vehmodel) && STREAMING::IS_MODEL_A_VEHICLE(vehmodel))
			{
				Vector3 dim1, dim2;
				GAMEPLAY::GET_MODEL_DIMENSIONS(vehmodel, &dim1, &dim2);

				offset = dim2.y * 1.6;
			}

			Vector3 dir = ENTITY::GET_ENTITY_FORWARD_VECTOR(playerPed);
			Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 1);
			float rot = (ENTITY::GET_ENTITY_ROTATION(playerPed, 0)).z;

			Vehicle veh = CREATE_VEHICLEB(vehmodel, pCoords.x + (dir.x * offset), pCoords.y + (dir.y * offset), pCoords.z, rot, 1, 1);

			VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
			ENTITY::SET_ENTITY_VISIBLE(veh, false, 0);
			VEHICLE::SET_VEHICLE_FORWARD_SPEED(veh, 70.0);

			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(vehmodel);
			ENTITY::SET_VEHICLE_AS_NO_LONGER_NEEDED(&veh);

		}
		if (mobileRadio) {
			AUDIO::SET_MOBILE_RADIO_ENABLED_DURING_GAMEPLAY(1);
		}
		else if (!mobileRadio) {
			AUDIO::SET_MOBILE_RADIO_ENABLED_DURING_GAMEPLAY(0);
		}

		if (depsentry) {

			//			_APPLY_PED_OVERLAY(playerPed, -1398869298, -1730534702);
		}

		if (opmult) {
			GAMEPLAY::SET_FAKE_WANTED_LEVEL(6);
		}
		else if (!opmult) {
			GAMEPLAY::SET_FAKE_WANTED_LEVEL(0);
		}
		if (selfExplode)
		{
			FIRE::ADD_EXPLOSION(pos.x, pos.y, pos.z, 29, 0.5f, true, false, 5.0f);
		}

		if (tel2ClosVeh)
		{
			int vehID = VEHICLE::GET_CLOSEST_VEHICLE(pos.x, pos.y, pos.z, 600.0f, 0, 0);
			PED::SET_PED_INTO_VEHICLE(playerPed, vehID, -1);
		}
		if (delVeh)
		{
			Player player = PLAYER::PLAYER_ID();
			Ped playerPed = PLAYER::PLAYER_PED_ID();
			if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
			{
				Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
				if (!NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(veh))
					while (!NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(veh));

				ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, 1, 1);
				VEHICLE::DELETE_VEHICLE(&veh);
//				PrintBottomLeft(AddStrings("Press the key a few times until the car dissapears", ""));
			}
		}
		if (randChar1)
		{
			Ped playerPed = PLAYER::PLAYER_PED_ID();
			PED::SET_PED_RANDOM_COMPONENT_VARIATION(playerPed, 1);
		}
		if (randCharAcc1)
		{
			Ped playerPed = PLAYER::PLAYER_PED_ID();
			PED::SET_PED_RANDOM_PROPS(playerPed);
		}

		if (hour_plus) {
			if (sample_hour + 1 == 24) NETWORK::NETWORK_OVERRIDE_CLOCK_TIME(0, 0, 0);
			else NETWORK::NETWORK_OVERRIDE_CLOCK_TIME((sample_hour + 1), 0, 0);
			return;
		}
		else if (hour_minus) {
			if ((sample_hour - 1) == -1) NETWORK::NETWORK_OVERRIDE_CLOCK_TIME(23, 0, 0);
			else NETWORK::NETWORK_OVERRIDE_CLOCK_TIME((sample_hour - 1), 0, 0);
			return;
		}

		if (timescale_plus) {
			if (current_timescale < 2.0f) current_timescale += 0.1f;
			GAMEPLAY::SET_TIME_SCALE(current_timescale);
			return;
		}
		else if (timescale_minus) {
			if (current_timescale > 0.0f) current_timescale -= 0.1f;
			GAMEPLAY::SET_TIME_SCALE(current_timescale);
			return;
		}

		if (sample_invisible) {
			if (ENTITY::IS_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID())) ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), true, 0);
			else ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), true, 1);
			return;
		}

		if (sample_gta2cam) {
			if (loop_gta2cam)
			{
				if (CAM::DOES_CAM_EXIST(cam_gta2)) CAM::SET_CAM_ACTIVE(cam_gta2, 1);
				else
				{
					cam_gta2 = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", 1);
					CAM::ATTACH_CAM_TO_ENTITY(cam_gta2, PLAYER::PLAYER_PED_ID(), 0.0f, 0.0f, 999.9f, 1);
					Vector3 Pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
					CAM::POINT_CAM_AT_COORD(cam_gta2, Pos.x, Pos.y, -1000000);
					CAM::SET_CAM_ACTIVE(cam_gta2, 1);
				}
				CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, 1, 0);
			}
			else if (CAM::DOES_CAM_EXIST(cam_gta2))
			{
				CAM::SET_CAM_ACTIVE(cam_gta2, 0);
				CAM::DESTROY_CAM(cam_gta2, 0);
				CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, 1, 0);
			}
			return;
		}
	}
	void set_shootrhino()
	{

		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (PED::IS_PED_SHOOTING(playerPed))
		{
			float offset;
			int vehmodel = GAMEPLAY::GET_HASH_KEY("RHINO");
			STREAMING::REQUEST_MODEL(vehmodel);

			while (!STREAMING::HAS_MODEL_LOADED(vehmodel)) WAIT(0);
			Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 5.0, 0.0);

			if (STREAMING::IS_MODEL_IN_CDIMAGE(vehmodel) && STREAMING::IS_MODEL_A_VEHICLE(vehmodel))
			{
				Vector3 dim1, dim2;
				GAMEPLAY::GET_MODEL_DIMENSIONS(vehmodel, &dim1, &dim2);

				offset = dim2.y * 1.6;
			}

			Vector3 dir = ENTITY::GET_ENTITY_FORWARD_VECTOR(playerPed);
			Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 1);
			float rot = (ENTITY::GET_ENTITY_ROTATION(playerPed, 0)).z;
			Vector3 gameplayCam = CAM::_GET_GAMEPLAY_CAM_COORDS();
			Vector3 gameplayCamRot = CAM::GET_GAMEPLAY_CAM_ROT(0);
			Vector3 gameplayCamDirection = RotationToDirection(gameplayCamRot);
			Vector3 startCoords = addVector(gameplayCam, (multiplyVector(gameplayCamDirection, 10)));
			Vector3 endCoords = addVector(gameplayCam, (multiplyVector(gameplayCamDirection, 500.0f)));

			Vehicle veh = CREATE_VEHICLEB(vehmodel, pCoords.x + (dir.x * offset), pCoords.y + (dir.y * offset), startCoords.z, rot, 1, 1);
			ENTITY::SET_ENTITY_VISIBLE(veh, false, 0);

			ENTITY::APPLY_FORCE_TO_ENTITY(veh, 1, 0.0f, 500.0f, 2.0f + endCoords.z, 0.0f, 0.0f, 0.0f, 0, 1, 1, 1, 0, 1);
			//SET_MODEL_AS_NO_LONGER_NEEDED(vehmodel);
			ENTITY::SET_VEHICLE_AS_NO_LONGER_NEEDED(&veh);
		}
	}
	void set_shootlbrtr()
	{
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (PED::IS_PED_SHOOTING(playerPed))
		{
			float offset;
			int vehmodel = GAMEPLAY::GET_HASH_KEY("MONSTER");
			STREAMING::REQUEST_MODEL(vehmodel);

			while (!STREAMING::HAS_MODEL_LOADED(vehmodel)) WAIT(0);
			Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 5.0, 0.0);

			if (STREAMING::IS_MODEL_IN_CDIMAGE(vehmodel) && STREAMING::IS_MODEL_A_VEHICLE(vehmodel))
			{
				Vector3 dim1, dim2;
				GAMEPLAY::GET_MODEL_DIMENSIONS(vehmodel, &dim1, &dim2);

				offset = dim2.y * 1.6;
			}


			Vector3 dir = ENTITY::GET_ENTITY_FORWARD_VECTOR(playerPed);
			Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 1);
			float rot = (ENTITY::GET_ENTITY_ROTATION(playerPed, 0)).z;
			Vector3 gameplayCam = CAM::_GET_GAMEPLAY_CAM_COORDS();
			Vector3 gameplayCamRot = CAM::GET_GAMEPLAY_CAM_ROT(0);
			Vector3 gameplayCamDirection = RotationToDirection(gameplayCamRot);
			Vector3 startCoords = addVector(gameplayCam, (multiplyVector(gameplayCamDirection, 10)));
			Vector3 endCoords = addVector(gameplayCam, (multiplyVector(gameplayCamDirection, 500.0f)));

			Vehicle veh = CREATE_VEHICLEB(vehmodel, pCoords.x + (dir.x * offset), pCoords.y + (dir.y * offset), startCoords.z, rot, 1, 1);
			ENTITY::SET_ENTITY_VISIBLE(veh, false, 0);

			ENTITY::APPLY_FORCE_TO_ENTITY(veh, 1, 0.0f, 500.0f, 2.0f + endCoords.z, 0.0f, 0.0f, 0.0f, 0, 1, 1, 1, 0, 1);
			//SET_MODEL_AS_NO_LONGER_NEEDED(vehmodel);
			ENTITY::SET_VEHICLE_AS_NO_LONGER_NEEDED(&veh);
		}
	}
	void set_shootbzrd()
	{
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (PED::IS_PED_SHOOTING(playerPed))
		{
			float offset;
			int vehmodel = GAMEPLAY::GET_HASH_KEY("BUZZARD2");
			STREAMING::REQUEST_MODEL(vehmodel);

			while (!STREAMING::HAS_MODEL_LOADED(vehmodel)) WAIT(0);
			Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 5.0, 0.0);

			if (STREAMING::IS_MODEL_IN_CDIMAGE(vehmodel) && STREAMING::IS_MODEL_A_VEHICLE(vehmodel))
			{
				Vector3 dim1, dim2;
				GAMEPLAY::GET_MODEL_DIMENSIONS(vehmodel, &dim1, &dim2);

				offset = dim2.y * 1.6;
			}


			Vector3 dir = ENTITY::GET_ENTITY_FORWARD_VECTOR(playerPed);
			Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 1);
			float rot = (ENTITY::GET_ENTITY_ROTATION(playerPed, 0)).z;
			Vector3 gameplayCam = CAM::_GET_GAMEPLAY_CAM_COORDS();
			Vector3 gameplayCamRot = CAM::GET_GAMEPLAY_CAM_ROT(0);
			Vector3 gameplayCamDirection = RotationToDirection(gameplayCamRot);
			Vector3 startCoords = addVector(gameplayCam, (multiplyVector(gameplayCamDirection, 10)));
			Vector3 endCoords = addVector(gameplayCam, (multiplyVector(gameplayCamDirection, 500.0f)));

			Vehicle veh = CREATE_VEHICLEB(vehmodel, pCoords.x + (dir.x * offset), pCoords.y + (dir.y * offset), startCoords.z, rot, 1, 1);
			ENTITY::SET_ENTITY_VISIBLE(veh, false, 0);

			ENTITY::APPLY_FORCE_TO_ENTITY(veh, 1, 0.0f, 500.0f, 2.0f + endCoords.z, 0.0f, 0.0f, 0.0f, 0, 1, 1, 1, 0, 1);
			//SET_MODEL_AS_NO_LONGER_NEEDED(vehmodel);
			ENTITY::SET_VEHICLE_AS_NO_LONGER_NEEDED(&veh);
		}
	}
	void set_ExplosiveAmmo()
	{
		Player player = PLAYER::PLAYER_ID();
		GAMEPLAY::SET_EXPLOSIVE_AMMO_THIS_FRAME(player);

	}
	void gWeaponMenu() {
		int tpgun = 0;
		static LPCSTR weaponNames[] = {
			"WEAPON_KNIFE", "WEAPON_NIGHTSTICK", "WEAPON_HAMMER", "WEAPON_BAT", "WEAPON_GOLFCLUB", "WEAPON_CROWBAR",
			"WEAPON_PISTOL", "WEAPON_COMBATPISTOL", "WEAPON_APPISTOL", "WEAPON_PISTOL50", "WEAPON_MICROSMG", "WEAPON_SMG",
			"WEAPON_ASSAULTSMG", "WEAPON_ASSAULTRIFLE", "WEAPON_CARBINERIFLE", "WEAPON_ADVANCEDRIFLE", "WEAPON_MG",
			"WEAPON_COMBATMG", "WEAPON_PUMPSHOTGUN", "WEAPON_SAWNOFFSHOTGUN", "WEAPON_ASSAULTSHOTGUN", "WEAPON_BULLPUPSHOTGUN",
			"WEAPON_STUNGUN", "WEAPON_COMBATPDW", "WEAPON_SNIPERRIFLE", "WEAPON_HEAVYSNIPER", "WEAPON_GRENADELAUNCHER", "WEAPON_GRENADELAUNCHER_SMOKE",
			"WEAPON_RPG", "WEAPON_MINIGUN", "WEAPON_GRENADE", "WEAPON_STICKYBOMB", "WEAPON_SMOKEGRENADE", "WEAPON_BZGAS",
			"WEAPON_MOLOTOV", "WEAPON_FIREEXTINGUISHER", "WEAPON_PETROLCAN", "WEAPON_KNUCKLE", "WEAPON_MARKSMANPISTOL",
			"WEAPON_SNSPISTOL", "WEAPON_SPECIALCARBINE", "WEAPON_HEAVYPISTOL", "WEAPON_BULLPUPRIFLE", "WEAPON_HOMINGLAUNCHER",
			"WEAPON_PROXMINE", "WEAPON_SNOWBALL", "WEAPON_VINTAGEPISTOL", "WEAPON_DAGGER", "WEAPON_FIREWORK", "WEAPON_MUSKET",
			"WEAPON_MARKSMANRIFLE", "WEAPON_HEAVYSHOTGUN", "WEAPON_GUSENBERG", "WEAPON_HATCHET", "WEAPON_DIGISCANNER", "WEAPON_BRIEFCASE", "WEAPON_RAILGUN", "WEAPON_COMBATPDW", "WEAPON_FLASHLIGHT", "WEAPON_MACHINEPISTOL", "WEAPON_MACHETE"
		};
		bool all = 0, rockets = 0, rapidfiredisabled = 0, maxUpg = 0;
		bool teleportGun = 0;
		AddTitle("Weapon Mods");
		AddOption("~b~Bullet Selector", null, nullFunc, SUB::BULLET);
		//	AddOption("Change Bullet Owner", null, nullFunc, SUB::BULLETBLAME);
		AddOption("~y~Get All Weapons", all);
		AddOption("Max Current Weapon Attachements", maxUpg);
		AddToggle("~g~No reload", loop_noreload);;
		AddToggle("Fire Ammo", loop_fireammo);
		AddToggle("~r~Rapid fire", rapidfire, null, rapidfiredisabled);
		//AddToggle("Slow time while aiming", bulletTime);
		AddToggle("Teleport Gun", teleportGun);
		AddToggle("Explosive Ammo", loop_explosiveammo);
		AddToggle("Explosive Melee", loop_explosiveMelee);
		/*	AddNumber("Weapon Damage Multiplier", damageMultiplier, 0, damageMultiplier);

		PLAYER::SET_PLAYER_WEAPON_DAMAGE_MODIFIER(player, damageMultiplier);
		PLAYER::SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER(player, damageMultiplier);*/
		//		AddToggle("One hit kill", onehit);
		//AddToggle("Shoot Hydra", shoothydras);
		//AddToggle("Shoot DumpTruck", shootdummp);
		//AddToggle("Shoot Cutter", shootcutter);
		AddToggle("Shoot Tank", shootrhino);

		//AddToggle("Shoot Blimp", shootbmxs);
		AddToggle("Shoot Buzzard", shootbzrd);
		AddToggle("Shoot Liberator", shootlbrtr);
		if (shootbzrd) set_shootbzrd();
		if (shootlbrtr) set_shootlbrtr();
		if (shootrhino) set_shootrhino();

		BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(PLAYER::PLAYER_PED_ID());

		Player player = PLAYER::PLAYER_ID();
		if (loop_explosiveammo)
		{
			if (bPlayerExists)
				GAMEPLAY::SET_EXPLOSIVE_AMMO_THIS_FRAME(player);
		}
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		Hash weapHash;
		WEAPON::GET_CURRENT_PED_WEAPON(PLAYER::PLAYER_PED_ID(), &weapHash, 1);
		if (maxUpg) {
			WEAPON::SET_PED_WEAPON_TINT_INDEX(playerPed, weapHash, ((weapHash == 0x42BF8A85, 1119849093) || (weapHash == 0xC0A3098D, 3231910285)));
		}
		if (rapidfiredisabled) PLAYER::DISABLE_PLAYER_FIRING(player, 0);
		if (rockets) {
			PrintStringBottomCentre("Use ~b~Numpad +~s~ to shoot rockets!");

		}
		if (all) {
			for (int i = 0; i < sizeof(weaponNames) / sizeof(weaponNames[0]); i++)
				WEAPON::GIVE_DELAYED_WEAPON_TO_PED(playerPed, GAMEPLAY::GET_HASH_KEY((char *)weaponNames[i]), 9999, 0);
			PrintStringBottomCentre("All weapons added");

		}

	}

	void YourSub()

	{
		// Initialise local variables here:
		bool print_deez_nuts = 0;

		// Options' text here:
		AddTitle("Timone n Pumba");
		AddOption("Option 1", print_deez_nuts);
		AddOption("Option 2", print_deez_nuts);
		AddOption("Option 3", print_deez_nuts);
		AddOption("Option 4", print_deez_nuts);
		AddOption("Option 5", print_deez_nuts);
		AddOption("Option 6", print_deez_nuts);
		AddOption("Option 7", print_deez_nuts);
		AddOption("Option 8", print_deez_nuts);
		AddOption("Option 9", print_deez_nuts);
		AddOption("Option 10", print_deez_nuts);
		AddOption("Option 11", print_deez_nuts);
		AddOption("Option 12", print_deez_nuts);
		AddOption("Option 13", print_deez_nuts);
		AddOption("Option 14", print_deez_nuts);
		AddOption("Option 15", print_deez_nuts);
		AddOption("Option 16", print_deez_nuts);
		AddOption("Option 17", print_deez_nuts);
		AddOption("Option 18", print_deez_nuts);
		AddOption("Option 19", print_deez_nuts);
		AddOption("Option 20", print_deez_nuts);
		AddOption("Option 21", print_deez_nuts);
		AddOption("Option 22", print_deez_nuts);
		AddOption("Option 23", print_deez_nuts);
		AddOption("Option 24", print_deez_nuts);
		AddOption("Option 25", print_deez_nuts);
		AddOption("Option 26", print_deez_nuts);
		AddOption("Option 27", print_deez_nuts);
		AddOption("Option 28", print_deez_nuts);
		AddOption("Option 29", print_deez_nuts);
		AddOption("Option 30", print_deez_nuts);
		AddOption("Option 31", print_deez_nuts);
		AddOption("Option 32", print_deez_nuts);

		// Options' code here:
		if (print_deez_nuts)
		{
			PrintStringBottomCentre(AddStrings(AddInt_S("Option ", menu::currentop), " ~b~selected!"));
			return; // Either use return; to exit to the switch if you don't have code below that you want executed.
		}


	}
	void PlaceObjectByHash(Hash hash, float x, float y, float z, float pitch, float roll, float yaw, float heading, int id) {
		if (STREAMING::IS_MODEL_IN_CDIMAGE(hash)) {
			STREAMING::REQUEST_MODEL(hash);
			while (!STREAMING::HAS_MODEL_LOADED(hash)) WAIT(0);
			Object obj = OBJECT::CREATE_OBJECT_NO_OFFSET(hash, x, y, z, 1, 0, 0);
			//SET_ENTITY_HEADING(obj, heading);
			//SET_ENTITY_ROTATION(obj, pitch, roll, yaw, 2, 1);
			ENTITY::SET_ENTITY_LOD_DIST(obj, 696969);
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(obj);
			ENTITY::FREEZE_ENTITY_POSITION(obj, 1);
			ENTITY::SET_ENTITY_ROTATION(obj, pitch, roll, yaw, 0, 1);
			mapMods[mapModsIndex] = obj;
			if (mapModsIndex <= 250) mapModsIndex++;
			WAIT(25);
		}
	}
	void MapMod(int mapIndex = -1) {



		Ped ped = PLAYER::PLAYER_PED_ID();
		if (PED::IS_PED_IN_ANY_VEHICLE(ped, 1)) {
			ped = PED::GET_VEHICLE_PED_IS_IN(ped, 0);
		}
		if (mapIndex == -1) {
			bool unload = 0;
			AddTitle("Map Mods");
			AddOption("~r~UNLOAD ALL", unload);
			AddOption("Maze Bank Demolition", null, nullFunc, SUB::MAPMOD_MAZEDEMO);
			AddOption("Maze Bank Roof Ramp", null, nullFunc, SUB::MAPMOD_MAZEROOFRAMP);
			AddOption("Beach Ferris-Ramp", null, nullFunc, SUB::MAPMOD_BEACHFERRISRAMP);
			AddOption("Mount Chilliad Ramp", null, nullFunc, SUB::MAPMOD_MOUNTCHILLIADRAMP);
			AddOption("Airport Mini Ramp", null, nullFunc, SUB::MAPMOD_AIRPORTMINIRAMP);
			AddOption("Airport Gate Ramp", null, nullFunc, SUB::MAPMOD_AIRPORTGATERAMP);
			AddOption("UFO Tower with FIB Building", null, nullFunc, SUB::MAPMOD_UFOTOWER);
			AddOption("Maze Bank 4 Ramps", null, nullFunc, SUB::MAPMOD_MAZEBANKRAMPS);
			AddOption("Freestyle Motocross Fort Zancudo", null, nullFunc, SUB::MAPMOD_FREESTYLEMOTOCROSS);
			AddOption("Halfpipe Fun Track", null, nullFunc, SUB::MAPMOD_HALFPIPEFUNTRACK);
			AddOption("Airport Loop", null, nullFunc, SUB::MAPMOD_AIRPORTLOOP);
			AddOption("Maze Bank Ramp", null, nullFunc, SUB::MAPMOD_MAZEBANKMEGARAMP);
			if (unload) {
				for (int i = 0; i < 250; i++) {
					RequestControlOfEnt(mapMods[i]);
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&mapMods[i]);
					ENTITY::DELETE_ENTITY(&mapMods[i]);
					PrintStringBottomCentre("~b~Cleared !");
				}
			}
		}
		else if (mapIndex == 0) {



			bool load = 0, unload = 0, teleport = 0;
			AddTitle("Anonymous MENU");
			AddTitle("~b~Maze Bank Demolition");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, -74.94243f, -818.63446f, 326.174347f, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAZE BANK DEMOLITION
				PlaceObjectByHash(2475986526, -80.9087, -830.357, 325.097, -14.4264, -8.53774, 152.209, -1, 0);
				PlaceObjectByHash(2475986526, -79.2316, -831.297, 325.108, -14.4054, 4.26887, 164.271, -1, 0);
				PlaceObjectByHash(2475986526, -76.7563, -831.549, 325.123, -14.4263, -0, 171.818, -1, 0);
				PlaceObjectByHash(2475986526, -74.2595, -831.691, 325.09, -14.2059, -1.06722, -176.517, -1, 0);
				PlaceObjectByHash(2475986526, -71.9478, -831.257, 325.113, -14.4264, 2.13443, -166.006, -1, 0);
				PlaceObjectByHash(2475986526, -69.5981, -830.542, 325.066, -14.5918, -4.26887, -155.474, -1, 0);
				PlaceObjectByHash(2475986526, -67.4174, -829.035, 325.097, -14.4263, -4.26887, -140.651, -1, 0);
				PlaceObjectByHash(2475986526, -65.7121, -827.409, 325.068, -14.3048, -1.70755, -130.667, -1, 0);
				PlaceObjectByHash(2475986526, -64.2717, -825.422, 325.112, -14.4263, -0, -121.256, -1, 0);
				PlaceObjectByHash(2475986526, -63.2876, -823.434, 325.114, -14.5345, -0, -111.446, -1, 0);
				PlaceObjectByHash(2475986526, -62.4924, -821.128, 325.082, -14.5467, 2.13443, -102.385, -1, 0);
				PlaceObjectByHash(2475986526, -62.233, -818.528, 325.065, -14.6184, -8.00413, -88.1861, -1, 0);
				PlaceObjectByHash(2475986526, -62.8694, -815.926, 325.118, -14.4265, -6.4033, -76.5979, -1, 0);
				PlaceObjectByHash(2475986526, -63.6142, -813.68, 325.112, -14.3655, 8.53774, -66.7885, -1, 0);
				PlaceObjectByHash(2475986526, -64.9883, -811.439, 325.107, -14.4263, 1.28066, -52.8494, -1, 0);
				PlaceObjectByHash(2475986526, -66.5913, -808.328, 325.238, -12.4517, -0, -40.3246, -1, 0);
				PlaceObjectByHash(2475986526, -68.2603, -807.899, 325.336, -13.8689, -0, -33.062, -1, 0);
				PlaceObjectByHash(2475986526, -71.1526, -807.598, 325.153, -12.0416, 4.26887, -28.0523, -1, 0);
				PlaceObjectByHash(2475986526, -73.2853, -806.628, 325.151, -11.7962, -0, -19.1717, -1, 0);
				PlaceObjectByHash(2475986526, -75.2243, -806.286, 325.164, -12.0419, -0, 1.55726, -1, 0);
				PlaceObjectByHash(2475986526, -77.5757, -806.312, 325.088, -14.1843, -0, 12.6263, -1, 0);
				PlaceObjectByHash(2475986526, -79.8704, -807.22, 325.143, -14.049, -4.26887, 21.4769, -1, 0);
				PlaceObjectByHash(2475986526, -82.0222, -807.83, 325.036, -14.1422, -4.26887, 32.7605, -1, 0);
				PlaceObjectByHash(2475986526, -83.8934, -809.424, 325.073, -14.5264, -8.53774, 46.5132, -1, 0);
				PlaceObjectByHash(2475986526, -85.2523, -810.983, 325.043, -14.859, -0, 53.5324, -1, 0);
				PlaceObjectByHash(2475986526, -86.5177, -813.202, 325.089, -14.5267, 4.26887, 64.6634, -1, 0);
				PlaceObjectByHash(2475986526, -87.6645, -815.707, 325.059, -14.8589, 4.26887, 73.157, -1, 0);
				PlaceObjectByHash(2475986526, -87.7973, -817.987, 325.119, -14.8468, -1.33402, 89.3982, -1, 0);
				PlaceObjectByHash(2475986526, -87.5801, -821.034, 325.059, -14.8593, -0, 95.4435, -1, 0);
				PlaceObjectByHash(2475986526, -87.2815, -822.239, 325.126, -15.6308, -4.26887, 100.311, -1, 0);
				PlaceObjectByHash(2475986526, -86.7602, -824.03, 325.044, -15.9224, -0, 116.957, -1, 0);
				PlaceObjectByHash(2475986526, -85.3743, -826.099, 325.136, -15.7025, 2.56132, 124.307, -1, 0);
				PlaceObjectByHash(2475986526, -83.4737, -828.611, 325.076, -15.0688, -0, 132.538, -1, 0);
				PlaceObjectByHash(2475986526, -87.9554, -832.877, 325.894, -14.1563, 4.26887, 132.995, -1, 0);
				PlaceObjectByHash(2475986526, -89.3865, -831.322, 325.887, -14.1562, -0, 126.154, -1, 0);
				PlaceObjectByHash(2475986526, -86.4247, -834.407, 325.915, -14.2701, 4.26887, 143.277, -1, 0);
				PlaceObjectByHash(2475986526, -85.1736, -833.789, 325.653, -14.4072, -4.26887, 145.777, -1, 0);
				PlaceObjectByHash(2475986526, -83.8118, -835.765, 326.063, -12.243, 4.26887, 151.527, -1, 0);
				PlaceObjectByHash(2475986526, -80.7015, -837.145, 326.059, -12.3172, 2.13443, 162.332, -1, 0);
				PlaceObjectByHash(2475986526, -77.6428, -837.649, 326.163, -10.8391, 3.20165, 171.297, -1, 0);
				PlaceObjectByHash(2475986526, -75.479, -837.909, 326.025, -12.3172, -1.06722, 174.574, -1, 0);
				PlaceObjectByHash(2475986526, -73.861, -837.826, 326.061, -12.3173, 5.33609, -176.632, -1, 0);
				PlaceObjectByHash(2475986526, -70.4799, -837.265, 326.09, -12.3173, -0, -166.182, -1, 0);
				PlaceObjectByHash(2475986526, -67.0415, -836.185, 326.018, -12.3171, -0, -156.039, -1, 0);
				PlaceObjectByHash(2475986526, -64.8504, -834.996, 325.951, -11.5263, -0, -145.834, -1, 0);
				PlaceObjectByHash(2475986526, -63.5702, -833.725, 326.1, -11.2947, -0, -140.961, -1, 0);
				PlaceObjectByHash(2475986526, -60.9992, -831.419, 326.075, -11.5262, -4.26887, -130.963, -1, 0);
				PlaceObjectByHash(2475986526, -58.9923, -828.729, 326.116, -11.5262, 4.26887, -121.973, -1, 0);
				PlaceObjectByHash(2475986526, -57.5045, -825.626, 326.114, -11.5263, -0, -110.959, -1, 0);
				PlaceObjectByHash(2475986526, -56.5533, -822.397, 326.08, -11.1311, -6.4033, -102, -1, 0);
				PlaceObjectByHash(2475986526, -56.0911, -820.05, 326.049, -11.0325, 2.13443, -100.794, -1, 0);
				PlaceObjectByHash(2475986526, -56.0681, -818.32, 326.087, -11.1312, -2.66804, -87.9469, -1, 0);
				PlaceObjectByHash(2475986526, -56.2989, -816.237, 326.048, -11.0324, 2.13443, -83.2139, -1, 0);
				PlaceObjectByHash(2475986526, -56.8952, -814.518, 326.142, -11.0324, -2.13443, -76.5476, -1, 0);
				PlaceObjectByHash(2475986526, -58.1209, -811.23, 326.116, -10.9697, -0, -66.7674, -1, 0);
				PlaceObjectByHash(2475986526, -59.0622, -809.17, 326.095, -11.0574, 4.26887, -62.782, -1, 0);
				PlaceObjectByHash(2475986526, -60.096, -807.639, 326.119, -11.5544, -0, -52.7596, -1, 0);
				PlaceObjectByHash(2475986526, -62.081, -805.317, 326.116, -11.1035, -0, -40.7682, -1, 0);
				PlaceObjectByHash(2475986526, -64.1466, -804.55, 326.283, -11.1035, -0, -30.477, -1, 0);
				PlaceObjectByHash(2475986526, -67.9795, -798.8, 326.717, -10.1561, -0, -29.3495, -1, 0);
				PlaceObjectByHash(2475986526, -67.5734, -802.52, 326.262, -10.471, -8.53774, -31.2185, -1, 0);
				PlaceObjectByHash(2475986526, -70.9341, -800.541, 326.198, -10.5317, -0, -20.0064, -1, 0);
				PlaceObjectByHash(2475986526, -75.3309, -801.285, 325.849, -10.2407, -0, 1.58401, -1, 0);
				PlaceObjectByHash(2475986526, -74.0222, -799.865, 326.177, -10.7327, -0, -5.98314, -1, 0);
				PlaceObjectByHash(2475986526, -76.5167, -797.998, 326.32, -12.4969, -2.66804, 1.58883, -1, 0);
				PlaceObjectByHash(2475986526, -79.2787, -800.531, 326.011, -12.9433, 4.26887, 13.0054, -1, 0);
				PlaceObjectByHash(2475986526, -81.6721, -801.017, 325.9, -12.4601, 2.13443, 17.3792, -1, 0);
				PlaceObjectByHash(2475986526, -83.6027, -801.744, 325.971, -12.9433, -0, 26.3052, -1, 0);
				PlaceObjectByHash(2475986526, -85.6586, -802.789, 325.95, -12.8791, 1.28066, 32.5856, -1, 0);
				PlaceObjectByHash(2475986526, -87.5086, -804.25, 325.978, -12.9432, 4.26887, 42.3279, -1, 0);
				PlaceObjectByHash(2475986526, -88.9923, -805.73, 325.89, -11.9333, -4.26887, 46.0613, -1, 0);
				PlaceObjectByHash(2475986526, -90.167, -807.318, 325.946, -13.0244, -0, 53.178, -1, 0);
				PlaceObjectByHash(2475986526, -93.5987, -807.353, 326.343, -11.5713, 4.26887, 60.8753, -1, 0);
				PlaceObjectByHash(2475986526, -93.5166, -813.963, 325.942, -13.4341, -4.26887, 73.0256, -1, 0);
				PlaceObjectByHash(2475986526, -92.121, -810.584, 325.996, -13.4339, -4.26887, 64.9353, -1, 0);
				PlaceObjectByHash(2475986526, -93.9931, -815.866, 325.924, -13.0519, -0, 79.5966, -1, 0);
				PlaceObjectByHash(2475986526, -93.8716, -817.904, 325.988, -13.4339, -0, 88.8361, -1, 0);
				PlaceObjectByHash(2475986526, -93.7912, -821.777, 325.946, -13.6946, -2.66804, 91.1427, -1, 0);
				PlaceObjectByHash(2475986526, -93.2951, -823.554, 325.966, -13.157, -0, 101.424, -1, 0);
				PlaceObjectByHash(2475986526, -92.5757, -827.033, 325.87, -13.5323, -0, 104.668, -1, 0);
				PlaceObjectByHash(2475986526, -91.53, -828.342, 325.842, -14.1563, 4.26887, 120.328, -1, 0);
				PlaceObjectByHash(2475986526, -90.5203, -829.611, 325.936, -14.1563, -0, 124.573, -1, 0);
				PlaceObjectByHash(2475986526, -95.5355, -833.068, 327.049, -9.63525, 1.70755, 124.512, -1, 0);
				PlaceObjectByHash(2475986526, -94.2445, -835.1, 326.976, -9.27617, -1.28066, 128.396, -1, 0);
				PlaceObjectByHash(2475986526, -92.513, -837.087, 327.008, -9.63523, -4.26887, 132.871, -1, 0);
				PlaceObjectByHash(2475986526, -90.07, -839.341, 327.025, -9.63574, 4.26887, 143.545, -1, 0);
				PlaceObjectByHash(2475986526, -86.7336, -841.135, 327.284, -9.63566, -0, 150.983, -1, 0);
				PlaceObjectByHash(2475986526, -84.8343, -842.167, 327.254, -9.36742, -4.26887, 152.377, -1, 0);
				PlaceObjectByHash(2475986526, -90.0883, -842.661, 327.589, -7.98782, -8.53774, 146.409, -1, 0);
				PlaceObjectByHash(2475986526, -82.595, -843.001, 327.277, -9.6352, -0, 161.654, -1, 0);
				PlaceObjectByHash(2475986526, -80.8027, -843.618, 327.263, -9.36755, -2.13443, 165.215, -1, 0);
				PlaceObjectByHash(2475986526, -78.5619, -843.703, 327.458, -9.63545, -2.13443, 171.015, -1, 0);
				PlaceObjectByHash(2475986526, -76.2479, -844.026, 327.261, -9.36765, 1.06722, 175.986, -1, 0);
				PlaceObjectByHash(2475986526, -73.5382, -843.999, 327.285, -9.6355, -0, -177.212, -1, 0);
				PlaceObjectByHash(2475986526, -71.2047, -843.988, 327.3, -9.36764, -1.06722, -172.013, -1, 0);
				PlaceObjectByHash(2475986526, -69.036, -843.266, 327.309, -9.63525, 4.26887, -166.686, -1, 0);
				PlaceObjectByHash(2475986526, -67.2981, -840.996, 326.756, -9.37509, -2.13443, -159.014, -1, 0);
				PlaceObjectByHash(2475986526, -66.7067, -842.714, 327.222, -9.37501, 2.13443, -159.27, -1, 0);
				PlaceObjectByHash(2475986526, -64.5693, -841.792, 327.24, -9.63515, 4.26887, -156.16, -1, 0);
				PlaceObjectByHash(2475986526, -61.8874, -840.436, 327.231, -9.37483, 4.26887, -146.534, -1, 0);
				PlaceObjectByHash(2475986526, -59.7118, -838.501, 327.384, -9.63533, -0, -141.372, -1, 0);
				PlaceObjectByHash(2475986526, -57.9491, -837.16, 327.309, -9.37471, 4.26887, -135.839, -1, 0);
				PlaceObjectByHash(2475986526, -56.3494, -835.471, 327.34, -9.63578, 4.26887, -131.675, -1, 0);
				PlaceObjectByHash(2475986526, -54.9387, -833.93, 327.334, -9.37482, -0, -127.887, -1, 0);
				PlaceObjectByHash(2475986526, -53.727, -832.032, 327.367, -9.63521, -4.26887, -122.142, -1, 0);
				PlaceObjectByHash(2475986526, -52.5928, -830.077, 327.332, -9.37496, -0, -116.843, -1, 0);
				PlaceObjectByHash(2475986526, -51.7552, -827.819, 327.385, -9.63569, 6.4033, -111.077, -1, 0);
				PlaceObjectByHash(2475986526, -51.0061, -825.839, 327.369, -9.37494, 4.26887, -107.054, -1, 0);
				PlaceObjectByHash(2475986526, -50.5468, -823.622, 327.378, -9.63572, 4.26887, -101.598, -1, 0);
				PlaceObjectByHash(2475986526, -50.0992, -820.896, 327.345, -9.47333, -1.06722, -95.7976, -1, 0);
				PlaceObjectByHash(2475986526, -49.9295, -818.102, 327.381, -9.63531, -8.00413, -88.2146, -1, 0);
				PlaceObjectByHash(2475986526, -50.1895, -815.816, 327.358, -9.4734, -0, -82.8649, -1, 0);
				PlaceObjectByHash(2475986526, -50.9164, -813.132, 327.442, -9.63524, 2.13443, -76.865, -1, 0);
				PlaceObjectByHash(2475986526, -51.1585, -811.568, 327.373, -9.58574, -0, -69.3402, -1, 0);
				PlaceObjectByHash(2475986526, -52.0622, -809.533, 327.354, -9.63541, 2.13443, -65.7624, -1, 0);
				PlaceObjectByHash(2475986526, -53.4048, -806.624, 327.376, -9.63526, 2.13443, -65.3971, -1, 0);
				PlaceObjectByHash(2475986526, -55.2978, -803.815, 327.389, -9.63524, 4.26887, -52.2107, -1, 0);
				PlaceObjectByHash(2475986526, -56.5179, -802.266, 327.366, -9.51013, 4.26887, -50.6537, -1, 0);
				PlaceObjectByHash(2475986526, -57.9995, -800.68, 327.42, -9.6353, 1.28066, -41.7027, -1, 0);
				PlaceObjectByHash(2475986526, -61.0278, -799.404, 327.549, -9.63516, 8.53774, -31.016, -1, 0);
				PlaceObjectByHash(2475986526, -64.37, -797.284, 327.603, -9.6351, -0, -31.6732, -1, 0);
				PlaceObjectByHash(2475986526, -66.3998, -795.965, 327.526, -9.42422, 8.53773, -29.018, -1, 0);
				PlaceObjectByHash(2475986526, -68.8079, -794.744, 327.535, -9.63558, -2.13443, -20.0341, -1, 0);
				PlaceObjectByHash(2475986526, -72.1225, -793.825, 327.497, -9.57894, -2.13443, -12.2336, -1, 0);
				PlaceObjectByHash(2475986526, -75.6415, -795.169, 327.2, -9.63555, -1.60083, 2.8097, -1, 0);
				PlaceObjectByHash(2475986526, -77.9613, -794.235, 327.223, -8.9769, -5.33608, 4.53814, -1, 0);
				PlaceObjectByHash(2475986526, -75.3695, -789.507, 328.306, -8.84722, -8.33763, -0.0879073, -1, 0);
				PlaceObjectByHash(2475986526, -80.6908, -794.505, 327.217, -9.63537, 4.26887, 13.0745, -1, 0);
				PlaceObjectByHash(2475986526, -83.5673, -795.148, 327.101, -9.92985, 2.13443, 17.5819, -1, 0);
				PlaceObjectByHash(2475986526, -86.3087, -796.203, 327.177, -9.63542, -4.26887, 25.9296, -1, 0);
				PlaceObjectByHash(2475986526, -88.9655, -797.634, 327.118, -9.92994, -4.26887, 33.0571, -1, 0);
				PlaceObjectByHash(2475986526, -91.6251, -799.702, 327.176, -9.63539, -0, 42.2513, -1, 0);
				PlaceObjectByHash(2475986526, -93.414, -801.299, 327.124, -9.92995, -0, 48.7085, -1, 0);
				PlaceObjectByHash(2475986526, -95.1453, -803.637, 327.147, -9.63537, -8.53774, 53.6544, -1, 0);
				PlaceObjectByHash(2475986526, -96.5885, -805.701, 327.144, -9.8947, -0, 60.5096, -1, 0);
				PlaceObjectByHash(2475986526, -97.6945, -807.971, 327.174, -9.63569, 4.26887, 64.7568, -1, 0);
				PlaceObjectByHash(2475986526, -98.7075, -809.885, 327.026, -8.13758, -0, 67.8881, -1, 0);
				PlaceObjectByHash(2475986526, -99.394, -812.176, 327.105, -9.63525, -4.26887, 73.0223, -1, 0);
				PlaceObjectByHash(2475986526, -100.025, -814.868, 327.097, -9.97277, 2.13443, 83.1537, -1, 0);
				PlaceObjectByHash(2475986526, -100.012, -817.789, 327.15, -9.63535, -1.33402, 88.8234, -1, 0);
				PlaceObjectByHash(2475986526, -100.069, -819.76, 327.099, -9.95297, -1.33402, 90.8729, -1, 0);
				PlaceObjectByHash(2475986526, -99.969, -821.91, 327.11, -9.63541, -2.66804, 91.5501, -1, 0);
				PlaceObjectByHash(2475986526, -99.3358, -824.801, 327.138, -9.63539, 2.13443, 101.678, -1, 0);
				PlaceObjectByHash(2475986526, -98.5443, -828.598, 327.033, -9.63553, -0, 104.64, -1, 0);
				PlaceObjectByHash(2475986526, -97.0896, -831.054, 326.937, -10.0741, 4.26887, 118.72, -1, 0);
				PlaceObjectByHash(2475986526, -102.435, -833.952, 328.506, -5.26399, -0, 118.502, -1, 0);
				PlaceObjectByHash(2475986526, -103.536, -831.932, 328.513, -5.42142, 4.26887, 111.099, -1, 0);
				PlaceObjectByHash(2475986526, -100.644, -836.571, 328.636, -5.26398, -0, 124.006, -1, 0);
				PlaceObjectByHash(2475986526, -99.0448, -838.912, 328.589, -5.26395, -2.13443, 128.175, -1, 0);
				PlaceObjectByHash(2475986526, -96.9401, -841.184, 328.589, -5.26384, -2.13443, 132.615, -1, 0);
				PlaceObjectByHash(2475986526, -95.4409, -842.718, 328.551, -5.01006, -2.13443, 136.57, -1, 0);
				PlaceObjectByHash(2475986526, -93.6584, -844.231, 328.606, -5.26388, -0, 143.429, -1, 0);
				PlaceObjectByHash(2475986526, -92.1044, -845.82, 328.655, -5.01307, -2.13443, 147.428, -1, 0);
				PlaceObjectByHash(2475986526, -89.6061, -846.328, 328.851, -5.26389, -0, 150.62, -1, 0);
				PlaceObjectByHash(2475986526, -87.5884, -847.552, 328.829, -5.6777, -0, 153.36, -1, 0);
				PlaceObjectByHash(2475986526, -84.5215, -848.802, 328.867, -5.26405, 5.33608, 161.164, -1, 0);
				PlaceObjectByHash(2475986526, -81.9779, -849.605, 328.821, -5.67769, 1.06722, 166.961, -1, 0);
				PlaceObjectByHash(2475986526, -79.5282, -849.717, 329.046, -5.26392, 1.06722, 170.517, -1, 0);
				PlaceObjectByHash(2475986526, -76.7555, -850.113, 328.885, -4.93224, 2.66804, 175.995, -1, 0);
				PlaceObjectByHash(2475986526, -73.2336, -850.06, 328.883, -5.26397, -0, -177.431, -1, 0);
				PlaceObjectByHash(2475986526, -70.4067, -849.836, 328.854, -4.82287, -5.33608, -172.2, -1, 0);
				PlaceObjectByHash(2475986526, -67.6252, -849.166, 328.911, -5.26394, -0, -166.741, -1, 0);
				PlaceObjectByHash(2475986526, -64.6525, -848.331, 328.792, -4.82267, -2.13443, -160.74, -1, 0);
				PlaceObjectByHash(2475986526, -62.1086, -847.355, 328.837, -5.26389, 2.13443, -156.346, -1, 0);
				PlaceObjectByHash(2475986526, -60.2755, -846.895, 328.808, -5.97307, -2.13443, -151.031, -1, 0);
				PlaceObjectByHash(2475986526, -58.5152, -845.543, 328.833, -5.26392, 1.06722, -147.129, -1, 0);
				PlaceObjectByHash(2475986526, -55.9339, -843.258, 328.987, -5.26394, 2.13443, -141.8, -1, 0);
				PlaceObjectByHash(2475986526, -53.6636, -841.564, 328.905, -5.18348, -0, -136.192, -1, 0);
				PlaceObjectByHash(2475986526, -51.8013, -839.526, 328.926, -5.26393, -2.13443, -131.788, -1, 0);
				PlaceObjectByHash(2475986526, -49.9112, -837.51, 328.916, -5.18352, -8.53774, -125.894, -1, 0);
				PlaceObjectByHash(2475986526, -48.5833, -835.261, 328.968, -5.26388, 2.13443, -122.598, -1, 0);
				PlaceObjectByHash(2475986526, -47.1369, -832.806, 328.936, -5.18352, -0, -117.146, -1, 0);
				PlaceObjectByHash(2475986526, -46.1092, -830.019, 328.985, -5.26389, -0, -111.097, -1, 0);
				PlaceObjectByHash(2475986526, -45.2549, -827.659, 328.957, -5.18353, -1.06722, -105.915, -1, 0);
				PlaceObjectByHash(2475986526, -44.5598, -824.856, 328.973, -5.26387, -0, -101.582, -1, 0);
				PlaceObjectByHash(2475986526, -44.0346, -821.522, 328.953, -5.26387, 5.33608, -95.978, -1, 0);
				PlaceObjectByHash(2475986526, -43.8673, -817.92, 328.98, -5.26387, -4.00206, -88.1556, -1, 0);
				PlaceObjectByHash(2475986526, -44.1983, -815.072, 328.956, -5.26387, -0, -82.8806, -1, 0);
				PlaceObjectByHash(2475986526, -45.0463, -811.788, 329.021, -5.26392, -0, -77.2513, -1, 0);
				PlaceObjectByHash(2475986526, -45.6154, -809.566, 328.95, -5.25705, -2.13443, -72.2094, -1, 0);
				PlaceObjectByHash(2475986526, -46.5685, -807.149, 328.929, -5.26395, -2.13443, -66.9958, -1, 0);
				PlaceObjectByHash(2475986526, -47.9752, -804.122, 328.959, -5.26398, -0, -65.1505, -1, 0);
				PlaceObjectByHash(2475986526, -49.0785, -802.078, 328.914, -5.31539, 4.26887, -57.7224, -1, 0);
				PlaceObjectByHash(2475986526, -50.5092, -800.141, 328.99, -5.26406, -0, -52.4683, -1, 0);
				PlaceObjectByHash(2475986526, -52.041, -798.134, 329.001, -5.31536, -2.13443, -49.2493, -1, 0);
				PlaceObjectByHash(2475986526, -53.8808, -796.134, 329.041, -5.26389, -0, -42.3308, -1, 0);
				PlaceObjectByHash(2475986526, -55.4375, -794.682, 329.045, -5.32055, 2.13443, -37.3601, -1, 0);
				PlaceObjectByHash(2475986526, -57.7537, -794.2, 329.16, -5.26393, 2.13443, -32.2267, -1, 0);
				PlaceObjectByHash(2475986526, -61.0299, -792.042, 329.172, -5.26389, -2.13443, -32.1174, -1, 0);
				PlaceObjectByHash(2475986526, -63.5163, -790.736, 329.085, -5.04535, -4.26887, -29.2933, -1, 0);
				PlaceObjectByHash(2475986526, -64.7324, -789.882, 329.081, -4.987, -2.13443, -27.7917, -1, 0);
				PlaceObjectByHash(2475986526, -66.7775, -788.94, 329.155, -5.04558, 1.06722, -19.5666, -1, 0);
				PlaceObjectByHash(2475986526, -68.6555, -788.272, 329.103, -5.30654, 3.20165, -16.9146, -1, 0);
				PlaceObjectByHash(2475986526, -70.8259, -787.837, 329.128, -5.04546, 1.06722, -12.2941, -1, 0);
				PlaceObjectByHash(2475986526, -74.5572, -787.022, 329.08, -4.61724, 1.06722, -10.7316, -1, 0);
				PlaceObjectByHash(2475986526, -75.8754, -788.646, 328.671, -6.78921, -0, 2.98721, -1, 0);
				PlaceObjectByHash(2475986526, -78.4, -788.132, 328.83, -5.91899, 2.66804, 3.75875, -1, 0);
				PlaceObjectByHash(2475986526, -80.5351, -788.179, 328.782, -5.80051, -0, 7.26539, -1, 0);
				PlaceObjectByHash(2475986526, -82.1189, -788.558, 328.793, -5.9192, 1.06722, 12.7168, -1, 0);
				PlaceObjectByHash(2475986526, -85.4054, -789.317, 328.666, -5.79433, -0, 17.1877, -1, 0);
				PlaceObjectByHash(2475986526, -87.4651, -789.98, 328.647, -5.63204, -0, 20.2315, -1, 0);
				PlaceObjectByHash(2475986526, -88.9795, -790.697, 328.76, -5.79416, -0, 25.9501, -1, 0);
				PlaceObjectByHash(2475986526, -90.9922, -791.487, 328.684, -6.26149, -0, 27.0819, -1, 0);
				PlaceObjectByHash(2475986526, -92.3298, -792.474, 328.677, -5.79412, -0, 33.3113, -1, 0);
				PlaceObjectByHash(2475986526, -94.2322, -793.73, 328.669, -5.58479, -2.13443, 37.4974, -1, 0);
				PlaceObjectByHash(2475986526, -95.7282, -795.2, 328.764, -5.79431, -0, 41.8672, -1, 0);
				PlaceObjectByHash(2475986526, -97.9782, -797.316, 328.695, -5.58488, -0, 48.9171, -1, 0);
				PlaceObjectByHash(2475986526, -100.042, -800.063, 328.731, -5.79425, 2.13443, 53.7039, -1, 0);
				PlaceObjectByHash(2475986526, -101.884, -802.718, 328.706, -5.5848, 2.13443, 60.3613, -1, 0);
				PlaceObjectByHash(2475986526, -103.09, -805.421, 328.718, -5.79425, -0, 64.506, -1, 0);
				PlaceObjectByHash(2475986526, -104.284, -807.711, 328.691, -5.58488, -2.13443, 65.966, -1, 0);
				PlaceObjectByHash(2475986526, -105.262, -810.369, 328.729, -5.26384, 1.06722, 73.2414, -1, 0);
				PlaceObjectByHash(2475986526, -105.769, -812.146, 328.645, -5.25205, -1.06722, 75.8091, -1, 0);
				PlaceObjectByHash(2475986526, -106.155, -814.128, 328.687, -5.26388, -1.06722, 82.8157, -1, 0);
				PlaceObjectByHash(2475986526, -106.062, -817.685, 328.758, -5.26388, 2.66804, 88.7458, -1, 0);
				PlaceObjectByHash(2475986526, -106.154, -819.723, 328.716, -5.25204, -0, 90.1628, -1, 0);
				PlaceObjectByHash(2475986526, -106.082, -822.072, 328.729, -5.26387, -1.33402, 91.2972, -1, 0);
				PlaceObjectByHash(2475986526, -105.911, -823.815, 328.7, -5.52135, 5.33608, 99.4393, -1, 0);
				PlaceObjectByHash(2475986526, -105.28, -826.029, 328.734, -5.26391, 1.06722, 101.615, -1, 0);
				PlaceObjectByHash(2475986526, -105.06, -827.904, 328.644, -5.29978, 1.06722, 102.515, -1, 0);
				PlaceObjectByHash(2475986526, -104.327, -830.112, 328.614, -5.26391, -2.13443, 104.719, -1, 0);
				PlaceObjectByHash(3291218330, -108.551, -853.416, 327.387, 2.94456, 89.1111, -166.155, -1, 0);
				PlaceObjectByHash(3291218330, -80.2509, -866.418, 327.301, 3.7405, 89.3, 146.641, -1, 0);
				PlaceObjectByHash(3291218330, -55.8513, -863.921, 327.333, 6.87468, 89.6184, 149.776, -1, 0);
				PlaceObjectByHash(3291218330, -37.3907, -848.122, 327.717, 2.33633, 88.8797, -16.2595, -1, 0);
				PlaceObjectByHash(3291218330, -26.1908, -818.332, 328.76, 0.490556, 84.6598, -18.107, -1, 0);
				PlaceObjectByHash(3291218330, -37.891, -789.138, 328.134, 1.11673, 87.6571, 42.7186, -1, 0);
				PlaceObjectByHash(3291218330, -63.492, -772.044, 327.866, 3.09962, 89.1556, 44.702, -1, 0);
				PlaceObjectByHash(3291218330, -93.4916, -774.848, 327.398, 2.73771, 89.0443, 122.539, -1, 0);
				PlaceObjectByHash(3291218330, -115.991, -795.259, 327.27, 3.28432, 89.2033, 123.086, -1, 0);
				PlaceObjectByHash(3291218330, -122.551, -825.074, 327.213, 173.37, 89.6048, 4.27077, -1, 0);
				PlaceObjectByHash(118627012, -74.8438, -819.617, 323.685, 0, 0, -3.37511, -1, 0);
				PlaceObjectByHash(2475986526, -67.6253, -820.244, 323.793, -14.4263, -8.53774, -100.02, -1, 0);
#pragma endregion
			}
		}
		else if (mapIndex == 1) {


			bool load = 0, unload = 0, teleport = 0;

			AddTitle("Anonymous MENU");
			AddTitle("~b~Maze Bank Roof Ramp");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, -74.94243f, -818.63446f, 326.174347f, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAP MOD
				PlaceObjectByHash(1600026313, -78.4864, -807.943, 323.202, 109.364, -89.9209, 0, -1, 1);
				PlaceObjectByHash(1600026313, -79.2766, -805.701, 323.204, 109.364, -89.9209, 0, -1, 1);
				PlaceObjectByHash(1600026313, -79.8373, -803.709, 323.205, 109.364, -89.9209, 0, -1, 1);
				PlaceObjectByHash(1600026313, -80.4295, -801.947, 323.207, 109.364, -89.9209, 0, -1, 1);
				PlaceObjectByHash(4143853297, -97.4731, -778.557, 308.877, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(1087520462, -84.2429, -793.182, 321.08, -63.5625, -0, -160.249, -1, 1);
				PlaceObjectByHash(1087520462, -83.5429, -795.106, 322.073, -63.5625, -0, -160.249, -1, 1);
				PlaceObjectByHash(1087520462, -84.9429, -791.108, 319.956, -62.6673, 8.53774e-007, -160.249, -1, 1);
				PlaceObjectByHash(1087520462, -85.8122, -788.585, 318.638, -63.5625, -0, -160.249, -1, 1);
				PlaceObjectByHash(1087520462, -89.1589, -779.487, 313.336, -59.5389, 0.0405551, -160.567, -1, 1);
				PlaceObjectByHash(1087520462, -90.7065, -774.863, 310.09, -57.4959, 0.322988, -160.758, -1, 1);
				PlaceObjectByHash(1087520462, -91.4887, -772.564, 308.403, -55.1692, 0.383369, -161.049, -1, 1);
				PlaceObjectByHash(1087520462, -91.7565, -771.74, 307.844, -56.4466, 0.0442451, -160.565, -1, 1);
				PlaceObjectByHash(1087520462, -93.6941, -766.245, 302.736, -45.9996, 0.0556114, -160.556, -1, 1);
				PlaceObjectByHash(1087520462, -94.2969, -764.648, 301.067, -44.7623, -1.70755e-006, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -94.2969, -764.648, 301.067, -44.7623, -1.70755e-006, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -94.886, -762.996, 298.741, -36.7051, -0, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -95.4855, -761.334, 296.406, -36.7051, -0, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -95.4855, -761.334, 296.406, -36.7051, -0, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -96.1606, -759.499, 294.259, -42.0766, -0, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -96.0707, -759.689, 293.709, -36.7051, -0, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -96.0707, -759.689, 293.709, -36.7051, -0, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -96.0707, -759.689, 293.46, -36.7051, -0, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -96.8807, -757.391, 292.506, -51.0291, -8.53774e-007, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -96.8807, -757.391, 292.506, -51.0291, -8.53774e-007, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -97.3203, -756.159, 291.688, -57.2958, -0, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -97.9597, -754.358, 290.78, -62.6673, 8.53774e-007, -160.249, -1, 1);
				PlaceObjectByHash(1087520462, -97.9597, -754.358, 290.78, -62.6673, 8.53774e-007, -160.249, -1, 1);
				PlaceObjectByHash(1087520462, -98.7192, -752.356, 290.042, -69.9278, 3.20165e-005, -160.249, -1, 1);
				PlaceObjectByHash(1087520462, -99.0244, -751.684, 290.499, -90, -8.46346e-007, -160.249, -1, 1);
				PlaceObjectByHash(1087520462, -99.3223, -750.534, 290.479, -90, -8.46346e-007, -160.249, -1, 1);
				PlaceObjectByHash(1087520462, -100.348, -747.881, 290.452, -89.5256, -1.33402e-008, -159.354, -1, 1);
				PlaceObjectByHash(1087520462, -100.26, -748.154, 290.462, -76.096, 4.26887e-007, 19.6954, -1, 1);
				PlaceObjectByHash(1087520462, -100.687, -747.053, 290.731, -62.6673, -8.53774e-007, 20.5907, -1, 1);
				PlaceObjectByHash(1087520462, -101.346, -745.387, 291.611, -58.191, 1.70755e-006, 19.6954, -1, 1);
				PlaceObjectByHash(1087520462, -102.234, -743.119, 293.091, -52.2249, 0.00051141, 21.3426, -1, 1);
				PlaceObjectByHash(2475986526, -102.154, -739.285, 294.83, 9.80014, 0.295618, 18.7802, -1, 1);
				PlaceObjectByHash(2475986526, -105.054, -740.282, 294.827, 9.80014, 0.295618, 18.7802, -1, 1);
				PlaceObjectByHash(1087520462, -103.071, -741.047, 294.832, -48.0666, 0.000519094, 21.3419, -1, 1);
				PlaceObjectByHash(1087520462, -103.75, -739.405, 296.413, -45.1472, 0.000547269, 21.3416, -1, 1);
				PlaceObjectByHash(4143853297, -90.3515, -798.112, 319.893, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -93.2293, -790.348, 317.189, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -95.3479, -784.483, 313.696, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -100.01, -771.31, 304.367, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -101.829, -766.277, 299.666, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -103.318, -762.175, 293.966, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -104.948, -757.681, 288.866, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -108.146, -748.798, 288.866, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -108.146, -748.798, 295.608, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -108.225, -748.694, 302.608, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(3167053133, -103.451, -740.541, 307.317, -0.900199, -1.19985, 20.9076, -1, 1);
				PlaceObjectByHash(2375650849, -102.454, -742.6, 309.309, 0, 0, 20.9393, -1, 1);
				PlaceObjectByHash(4143853297, -101.483, -746.044, 305.602, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -94.7458, -743.402, 295.608, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -94.7566, -743.406, 288.866, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -94.7426, -743.595, 302.651, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(1157292806, -99.7732, -750.516, 309.575, 0, 0, 24.1761, -1, 1);
				PlaceObjectByHash(4143853297, -89.9785, -756.476, 293.966, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -91.5378, -752.285, 288.866, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -87.9094, -762.07, 299.666, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -86.2094, -766.939, 304.367, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -84.0215, -772.971, 309.575, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -81.6733, -779.348, 313.696, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -79.5187, -785.083, 317.189, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(4143853297, -76.5616, -793.191, 319.493, 0, -0, -159.831, -1, 1);
				PlaceObjectByHash(1998517203, -81.0993, -790.139, 326.57, 0, 0, -82.6177, -1, 1);
				PlaceObjectByHash(1998517203, -88.8988, -792.911, 326.95, 0, 0, -82.6177, -1, 1);
				PlaceObjectByHash(803874239, -88.3376, -794.173, 327.042, 0, 0, 31.5501, -1, 1);
				PlaceObjectByHash(803874239, -90.0376, -795.174, 327.262, 0, 0, 31.5501, -1, 1);
				PlaceObjectByHash(803874239, -91.6375, -796.175, 327.482, 0, 0, 31.5501, -1, 1);
				PlaceObjectByHash(803874239, -79.0283, -791.31, 326.763, 0, -0, 100.953, -1, 1);
				PlaceObjectByHash(803874239, -76.8377, -790.87, 326.823, 0, -0, 100.953, -1, 1);
				PlaceObjectByHash(803874239, -81.0088, -791.22, 326.713, 0, -0, 100.953, -1, 1);


#pragma endregion
			}

		}
		else if (mapIndex == 2) {



			bool load = 0, unload = 0, teleport = 0;
			AddTitle("Anonymous MENU");
			AddTitle("Beach Ferris-Ramp");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, -1513.0f, -1192.0f, 1.0f, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAP MOD
				PlaceObjectByHash(1952396163, -1497.76, -1113.84, -3.08, -90, 6.14715e-007, 165.792, -1, 2);
				PlaceObjectByHash(2475986526, -1461.92, -1216.88, 2.5836, -2.3048, -0, -154.878, -1, 2);
				PlaceObjectByHash(3291218330, -1465.62, -1217.64, 18, 166.516, -5.12264e-006, 24.1717, -1, 2);
				PlaceObjectByHash(3291218330, -1458.89, -1214.4, 18, -38.4956, 8.53774e-007, -153.982, -1, 2);
				PlaceObjectByHash(2475986526, -1460.32, -1219.97, 4.3801, 12.6953, -0, -154.878, -1, 2);
				PlaceObjectByHash(2475986526, -1457, -1226.67, 11.8772, 31.7229, -0, -154.382, -1, 2);
				PlaceObjectByHash(2475986526, -1458.4, -1223.77, 7.9937, 23.6001, -0.0916355, -154.918, -1, 2);
				PlaceObjectByHash(2475986526, -1456.4, -1228.27, 14.9608, 48.674, -0, -153.982, -1, 2);
				PlaceObjectByHash(2475986526, -1456, -1229.07, 19.7441, 68.6628, -0, -153.982, -1, 2);
				PlaceObjectByHash(2475986526, -1456.2, -1228.47, 24.8276, 82.6252, 3.80938, -152.828, -1, 2);
				PlaceObjectByHash(2475986526, -1456.9, -1226.47, 28.9111, 108.498, -8.51368, -157.244, -1, 2);
				PlaceObjectByHash(2475986526, -1458.59, -1223.37, 31.5945, 130.616, -4.72983, -155.087, -1, 2);
				PlaceObjectByHash(2475986526, -1460.59, -1218.38, 33.5779, 143.744, -3.95611, -152.581, -1, 2);
				PlaceObjectByHash(2475986526, -1462.79, -1214.28, 34.161, 163.63, -2.68302, -155.763, -1, 2);
				PlaceObjectByHash(2475986526, -1465.3, -1209.78, 32.5228, -172.187, 4.69576e-006, -152.192, -1, 2);
				PlaceObjectByHash(2475986526, -1465.3, -1209.78, 32.5228, -172.187, 4.69576e-006, -152.192, -1, 2);
				PlaceObjectByHash(2475986526, -1466.9, -1205.68, 29.0062, -155.178, 9.47689e-005, -153.087, -1, 2);
				PlaceObjectByHash(2475986526, -1468.3, -1202.98, 24.1897, -131.11, 6.74481e-005, -153.088, -1, 2);
				PlaceObjectByHash(2475986526, -1468.59, -1202.68, 19.3732, -107.429, 3.07358e-005, -153.087, -1, 2);
				PlaceObjectByHash(2475986526, -1467.99, -1203.88, 13.5732, -89.6528, -0.153235, -155.853, -1, 2);
				PlaceObjectByHash(2475986526, -1467.11, -1205.68, 10.7072, -63.5491, 8.53774e-007, -156.504, -1, 2);
				PlaceObjectByHash(4109455646, -1465.05, -1210.03, 7.9503, 9.53319, 1.38057, 24.2606, -1, 2);
				PlaceObjectByHash(2975320548, -1460.95, -1218.79, 7.66, -29.9323, -0.173323, 24.7221, -1, 2);
				PlaceObjectByHash(2975320548, -1463.05, -1214.19, 6.7879, -6.50192, 1.391, 24.2651, -1, 2);


#pragma endregion	
			}

		}
		else if (mapIndex == 3) {



			bool load = 0, unload = 0, teleport = 0;
			AddTitle("Anonymous MENU");
			AddTitle("Mount Chilliad Ramp");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, 500, 5593, 795, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAP MOD
				PlaceObjectByHash(1952396163, -1497.76, -1113.84, -3.08, -90, -0, 165.792, 90, 3);
				PlaceObjectByHash(2475986526, -1461.92, -1216.88, 2.5836, -2.3048, 0, -154.878, 205.14, 3);
				PlaceObjectByHash(3291218330, -1458.89, -1214.4, 18, -38.4956, 0, -153.982, 211.95, 3);
				PlaceObjectByHash(2475986526, -1460.32, -1219.97, 4.3801, 12.6953, 0, -154.878, 205.672, 3);
				PlaceObjectByHash(2975320548, -1463.05, -1214.19, 6.7879, -6.5, -1.391, 24.2651, 24.4244, 3);
				PlaceObjectByHash(3291218330, -1465.62, -1217.64, 18, 166.516, 180, 24.1717, 155.224, 3);
				PlaceObjectByHash(4109455646, -1465.05, -1210.03, 7.9503, 9.5304, -1.3806, 24.2606, 24.5148, 3);
				PlaceObjectByHash(2975320548, -1460.95, -1218.79, 7.66, -29.9322, 0.1733, 24.7221, 27.9617, 3);
				PlaceObjectByHash(2475986526, -1458.4, -1223.77, 7.9937, 23.6001, 0.0916, -154.918, 207.065, 3);
				PlaceObjectByHash(2475986526, -1467.11, -1205.68, 10.7072, -63.5491, 0, -156.505, 224.303, 3);
				PlaceObjectByHash(2475986526, -1457, -1226.67, 11.8772, 31.7229, 0, -154.382, 209.411, 3);
				PlaceObjectByHash(2475986526, -1456.4, -1228.27, 14.9608, 48.674, 0, -153.982, 216.471, 3);
				PlaceObjectByHash(2475986526, -1456, -1229.07, 19.7441, 68.6628, 0, -153.982, 233.298, 3);
				PlaceObjectByHash(2475986526, -1456.2, -1228.47, 24.8276, 81.7043, -3.8094, -152.828, 252.429, 3);
				PlaceObjectByHash(2475986526, -1456.9, -1226.47, 28.9111, 110.301, 171.486, -157.244, 312.201, 3);
				PlaceObjectByHash(2475986526, -1458.59, -1223.37, 31.5945, 130.843, 175.27, -155.087, 325.759, 3);
				PlaceObjectByHash(2475986526, -1460.59, -1218.38, 33.5779, 143.844, 176.044, -152.581, 327.979, 3);
				PlaceObjectByHash(2475986526, -1462.79, -1214.28, 34.161, 163.648, 177.317, -155.763, 335.024, 3);
				PlaceObjectByHash(2475986526, -1465.3, -1209.78, 32.5228, -172.187, -180, -152.192, 331.971, 3);
				PlaceObjectByHash(2475986526, -1466.9, -1205.68, 29.0062, -155.178, -180, -153.087, 330.783, 3);
				PlaceObjectByHash(2475986526, -1468.3, -1202.98, 24.1897, -131.11, -180, -153.088, 322.332, 3);
				PlaceObjectByHash(2475986526, -1468.59, -1202.68, 19.3732, -107.429, -180, -153.087, 300.544, 3);
				PlaceObjectByHash(2475986526, -1467.99, -1203.88, 13.5732, -89.6205, 0.1532, -155.853, 269.072, 3);
				PlaceObjectByHash(3966705493, 509.842, 5589.24, 791.066, 0.141, 0, 65.3998, 65.3999, 3);
				PlaceObjectByHash(3966705493, 520.5, 5584.38, 790.503, 5.441, 0, 65.3998, 65.4976, 3);
				PlaceObjectByHash(3966705493, 531.057, 5579.54, 788.691, 12.441, 0, 65.3998, 65.9111, 3);
				PlaceObjectByHash(3966705493, 568.672, 5562.32, 767.428, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 576.972, 5558.53, 759.566, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 560.174, 5566.2, 774.698, 35.0403, 0, 65.3998, 69.4512, 3);
				PlaceObjectByHash(3966705493, 541.325, 5574.84, 785.49, 19.4409, 0, 65.3998, 66.6484, 3);
				PlaceObjectByHash(3966705493, 551.066, 5570.37, 780.799, 27.5407, 0, 65.3998, 67.9049, 3);
				PlaceObjectByHash(3966705493, 585.249, 5554.75, 751.745, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 618.334, 5539.62, 720.386, 40.7936, 0, 65.3998, 70.8829, 3);
				PlaceObjectByHash(3966705493, 626.602, 5535.85, 712.547, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 610.065, 5543.4, 728.217, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 601.777, 5547.19, 736.076, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 593.507, 5550.97, 743.917, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 634.862, 5532.07, 704.725, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 643.121, 5528.29, 696.894, 40.7936, 0, 65.3998, 70.8829, 3);
				PlaceObjectByHash(3966705493, 651.391, 5524.51, 689.053, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 659.651, 5520.73, 681.221, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 667.911, 5516.94, 673.389, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 676.171, 5513.17, 665.558, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 684.431, 5509.38, 657.727, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 692.691, 5505.61, 649.905, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 700.95, 5501.83, 642.074, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 709.22, 5498.05, 634.243, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 717.46, 5494.28, 626.431, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 725.72, 5490.5, 618.6, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 733.98, 5486.72, 610.778, 40.7396, 0, 65.3998, 70.8685, 3);
				PlaceObjectByHash(3966705493, 742.6, 5482.78, 603.167, 36.9395, 0, 65.3998, 69.9005, 3);
				PlaceObjectByHash(3966705493, 751.83, 5478.55, 596.335, 31.0392, 0, 65.3998, 68.5807, 3);
				PlaceObjectByHash(3966705493, 761.71, 5474.02, 590.613, 24.5989, 0, 65.3998, 67.3986, 3);
				PlaceObjectByHash(3966705493, 772.07, 5469.28, 586.08, 18.9288, 0, 65.3998, 66.5835, 3);
				PlaceObjectByHash(3966705493, 782.84, 5464.34, 582.86, 11.5788, 0, 65.3998, 65.8427, 3);
				PlaceObjectByHash(3966705493, 793.89, 5459.28, 581.117, 5.0787, 0, 65.3998, 65.485, 3);
				PlaceObjectByHash(3966705493, 805.1, 5454.15, 580.876, -2.5212, 0, 65.3998, 65.4208, 3);
				PlaceObjectByHash(3966705493, 816.17, 5449.08, 581.975, -7.6213, 0, 65.3998, 65.5917, 3);
				PlaceObjectByHash(3966705493, 827.191, 5444.04, 584.582, -16.6212, 0, 65.3998, 66.3125, 3);
				PlaceObjectByHash(3966705493, 837.681, 5439.24, 588.899, -24.421, 0, 65.3998, 67.3698, 3);
				PlaceObjectByHash(2580877897, 522.61, 5584.49, 779.214, 79.7153, -9.2252, 55.7018, 77.7612, 3);
				PlaceObjectByHash(3862788492, 522.445, 5583.69, 779.551, -0.9197, -69.229, -167.468, 184.555, 3);



#pragma endregion	
			}

		}
		else if (mapIndex == 4) {



			bool load = 0, unload = 0, teleport = 0;
			AddTitle("Anonymous MENU");
			AddTitle("Airport Mini Ramp");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, -1208, -2950, 13, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAP MOD
				PlaceObjectByHash(2475986526, -1242.08, -2931.15, 12.9924, -0.1046, -3.33505e-009, 61.0607, -1, 4);
				PlaceObjectByHash(2475986526, -1247.11, -2928.46, 15.013, -0.1046, -3.33505e-009, 61.0607, -1, 4);
				PlaceObjectByHash(2475986526, -1251.58, -2926.05, 16.7865, -0.1046, -3.33505e-009, 61.0607, -1, 4);
				PlaceObjectByHash(2475986526, -1254.69, -2924.35, 18.25, -0.1046, -3.33505e-009, 61.0607, -1, 4);
				PlaceObjectByHash(3966705493, -1276.69, -2912.99, 23.0019, 0, 0.05, 60.9705, -1, 4);
				PlaceObjectByHash(2475986526, -1258.35, -2922.28, 20.2135, -0.1046, -3.33505e-009, 61.0607, -1, 4);
				PlaceObjectByHash(3966705493, -1270.89, -2916.22, 23.0123, 0, 0, 60.8909, -1, 4);
				PlaceObjectByHash(3966705493, -1270.25, -2914.99, 23.0137, 0, 0, 60.8909, -1, 4);
				PlaceObjectByHash(3966705493, -1274.87, -2909.4, 23.0049, 0, 0.05, 60.9705, -1, 4);
				PlaceObjectByHash(3966705493, -1269.01, -2912.64, 22.9993, 0, 0.05, 60.9705, -1, 4);
				PlaceObjectByHash(3966705493, -1267.87, -2915.44, 28.3632, 0, -0, 147.299, -1, 4);
				PlaceObjectByHash(3966705493, -1272.13, -2918.33, 28.4791, 0, 0.05, 60.9705, -1, 4);
				PlaceObjectByHash(3966705493, -1272.11, -2918.35, 25.6708, -0.48, 0.0499982, 60.9701, -1, 4);
				PlaceObjectByHash(3966705493, -1277.93, -2915.14, 25.604, 0, 0.05, 60.9705, -1, 4);
				PlaceObjectByHash(3966705493, -1279.69, -2909.85, 25.6358, 0, -0, -151.239, -1, 4);
				PlaceObjectByHash(3966705493, -1279.69, -2909.85, 28.4844, 0, -0, -151.239, -1, 4);
				PlaceObjectByHash(2475986526, -1261.82, -2920.38, 21.767, -0.1046, -3.33505e-009, 61.0607, -1, 4);
				PlaceObjectByHash(3966705493, -1273.65, -2907.11, 22.9763, 0, 0.05, 60.9705, -1, 4);
				PlaceObjectByHash(3966705493, -1267.77, -2910.37, 22.9978, 0, 0.05, 60.9705, -1, 4);
				PlaceObjectByHash(3966705493, -1266.49, -2908.08, 22.9987, 0, -0, -119.462, -1, 4);
				PlaceObjectByHash(3966705493, -1265.15, -2905.8, 23.0042, 0, -0, -119.462, -1, 4);
				PlaceObjectByHash(3966705493, -1266.44, -2905.21, 25.6255, 0, -0, -118.761, -1, 4);
				PlaceObjectByHash(3966705493, -1265.66, -2911.99, 25.6968, 0, 0, -30.9603, -1, 4);
				PlaceObjectByHash(3966705493, -1264.88, -2910.66, 25.6982, 0, 0, -30.9603, -1, 4);
				PlaceObjectByHash(3966705493, -1264.84, -2905.14, 25.624, 0, -0, -118.761, -1, 4);
				PlaceObjectByHash(3966705493, -1272.37, -2900.96, 25.6199, 0, -0, -118.761, -1, 4);
				PlaceObjectByHash(3966705493, -1276.35, -2903.91, 25.6214, 0, -0, -151.239, -1, 4);
				PlaceObjectByHash(3966705493, -1276.35, -2903.91, 28.4329, 0, -0, -151.239, -1, 4);
				PlaceObjectByHash(3966705493, -1272.37, -2900.96, 28.4385, 0, -0, -118.761, -1, 4);
				PlaceObjectByHash(3966705493, -1266.44, -2905.21, 28.437, 0, -0, -118.761, -1, 4);
				PlaceObjectByHash(3966705493, -1265.17, -2905.14, 28.3426, 0, -0, -118.861, -1, 4);
				PlaceObjectByHash(3966705493, -1271.09, -2902.58, 23.0057, 0, -0, -119.462, -1, 4);
				PlaceObjectByHash(3966705493, -1272.37, -2904.83, 22.9972, 0, -0, -119.462, -1, 4);
#pragma endregion	
			}
		}
		else if (mapIndex == 5) {




			bool load = 0, unload = 0, teleport = 0;
			AddTitle("Anonymous MENU");
			AddTitle("Airport Gate Ramp");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, -1046, -2538, 20, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAP MOD
				PlaceObjectByHash(2475986526, -1098.36, -2631.17, 19, 0, -0, 152.671, -1, 5);
				PlaceObjectByHash(2475986526, -1100.26, -2634.64, 21.1976, 16.2002, 0.192059, 150.427, -1, 5);
				PlaceObjectByHash(2475986526, -1102.26, -2638.02, 25.01, 26.7003, 0.178675, 149.261, -1, 5);
				PlaceObjectByHash(2475986526, -1103.96, -2640.91, 29.04, 28.3717, -0, 146.82, -1, 5);
				PlaceObjectByHash(1952396163, -1119.61, -2670.96, -5.125, 0, -0, 150.514, -1, 5);
				PlaceObjectByHash(1952396163, -1119.61, -2670.96, -5.125, 0, -0, 150.401, -1, 5);
				PlaceObjectByHash(3137065507, -1044.69, -2530.08, 20.4011, 94.8962, 4.26887e-007, 147.716, -1, 5);
#pragma endregion	
			}
		}
		else if (mapIndex == 6) {



			bool load = 0, unload = 0, teleport = 0;
			AddTitle("UFO Tower");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, 70, -674, 680, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAP MOD
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 654.365, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 646.186, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 638.008, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 629.829, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 621.65, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 613.471, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 605.292, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 597.114, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 588.935, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 580.756, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 572.577, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 564.399, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 556.22, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 662.544, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 548.041, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 539.862, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 531.683, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 523.505, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 515.326, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 507.147, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 498.968, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 490.79, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 482.611, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 474.432, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 466.253, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 458.074, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 449.896, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 441.717, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 433.538, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 425.359, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 417.18, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 409.001, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 400.823, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 392.644, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 384.465, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 376.286, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 368.107, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 359.929, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 351.75, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 343.571, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 335.392, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 327.213, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 319.035, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 310.856, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 302.677, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 294.498, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 286.319, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 278.141, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 269.962, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 261.783, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 253.604, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 245.425, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 237.247, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 229.068, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 220.889, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 212.71, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 204.531, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 196.353, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 188.174, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 179.995, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 171.816, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 163.637, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 155.459, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 147.28, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 139.101, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 130.922, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 122.743, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 114.565, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 106.386, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 98.207, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 90.0282, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 81.8494, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 73.6706, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 65.4918, 0.660085, -0.919939, -109.32, -1, 6);
				PlaceObjectByHash(3026699584, 70.2592, -674.044, 57.313, 0.660085, -0.919939, -109.32, -1, 6);
#pragma endregion	
			}
		}
		else if (mapIndex == 7) {


			bool load = 0, unload = 0, teleport = 0;
			AddTitle("Anonymous MENU");
			AddTitle("4 Maze Bank Ramps");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, -74.94243f, -818.63446f, 326.174347f, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAP MOD
				PlaceObjectByHash(3522933110, -81.3886, -814.648, 325.169, 0, -0, 180, -1, 7);
				PlaceObjectByHash(3681122061, -81.7456, -809.064, 324.799, 0.500021, 2.66804, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -86.1333, -802.279, 321.92, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -89.7406, -796.701, 316.539, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -93.601, -790.725, 310.777, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -97.4741, -784.73, 304.997, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -101.373, -778.696, 299.179, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -105.233, -772.72, 293.417, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -109.106, -766.725, 287.637, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -112.954, -760.769, 281.894, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -116.827, -754.773, 276.113, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -120.687, -748.798, 270.352, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -124.518, -742.868, 264.636, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -128.358, -736.925, 258.909, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -132.22, -730.949, 253.151, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -136.081, -724.974, 247.394, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -139.943, -718.998, 241.636, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -143.826, -712.99, 235.846, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -147.667, -707.047, 230.12, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -151.508, -701.104, 224.394, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -155.369, -695.128, 218.636, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -159.252, -689.12, 212.846, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -163.072, -683.209, 207.152, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -166.976, -677.168, 201.331, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -170.838, -671.193, 195.573, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -174.7, -665.217, 189.815, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -178.583, -659.209, 184.026, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -182.444, -653.233, 178.268, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -186.327, -647.225, 172.479, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -190.189, -641.249, 166.721, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -194.03, -635.306, 160.994, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -197.871, -629.363, 155.268, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -201.711, -623.42, 149.542, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -205.552, -617.477, 143.815, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -209.393, -611.534, 138.089, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -213.255, -605.559, 132.331, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -217.095, -599.616, 126.605, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -220.957, -593.64, 120.847, -38.9999, -1.45141, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -222.245, -591.648, 118.928, -33.8999, 1.02453, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -223.349, -589.94, 117.561, -29.31, 1.79292, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -224.58, -588.036, 116.288, -26.25, 5.12264, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -225.869, -586.04, 115.116, -24.7199, -1.10991, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -227.127, -584.095, 114.05, -21.6599, 1.8783, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -228.615, -581.794, 112.961, -19.6199, 1.02453, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -230.201, -579.341, 111.92, -17.0699, -0, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -232.121, -576.369, 110.833, -12.9899, 4.26887, 32.8808, -1, 7);
				PlaceObjectByHash(3681122061, -234.105, -573.302, 109.991, -9.9299, -2.98821, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -236.628, -569.396, 109.329, -7.3799, -4.26887, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -239.81, -564.475, 108.721, -4.3199, 1.28066, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -241.76, -561.459, 108.549, -0.7499, -1.12058, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -244.04, -557.932, 108.494, 2.82011, -2.77476, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -246.372, -554.326, 108.705, 5.8801, -2.77476, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -248.668, -550.777, 109.14, 10.4701, 8.96462, 32.8806, -1, 7);
				PlaceObjectByHash(3681122061, -251.664, -546.138, 110.313, 13.5301, 1.15259, 32.8806, -1, 7);
				PlaceObjectByHash(3681122061, -254.537, -541.694, 111.791, 16.5901, 4.26887, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -256.28, -538.999, 112.748, 19.6501, -1.19528, 32.8807, -1, 7);
				PlaceObjectByHash(3681122061, -65.9078, -814.752, 326.106, 19.89, 4.26887, -53.8105, -1, 7);
				PlaceObjectByHash(3681122061, -58.6541, -809.444, 327.336, -4.08004, -2.13443, -53.8103, -1, 7);
				PlaceObjectByHash(3681122061, -52.4476, -804.909, 323.715, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, -47.2332, -801.09, 317.168, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, -42.0187, -797.272, 310.621, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, -36.8326, -793.474, 304.109, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, -31.5898, -789.635, 297.526, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, -26.4037, -785.838, 291.014, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, -21.1893, -782.019, 284.467, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, -15.9748, -778.201, 277.919, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, -10.7604, -774.383, 271.372, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, -5.57426, -770.585, 264.86, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, -0.359839, -766.767, 258.313, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 4.82623, -762.969, 251.799, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 10.0123, -759.171, 245.285, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 15.2268, -755.353, 238.735, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 20.4412, -751.535, 232.184, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 25.6273, -747.737, 225.67, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 30.8135, -743.939, 219.155, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 36.0279, -740.121, 212.605, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 41.214, -736.323, 206.091, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 46.4285, -732.505, 199.54, -45.3899, 2.56132, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 48.4122, -731.052, 197.049, -41.8198, 1.62217, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 49.5549, -730.218, 195.782, -38.2499, 3.24434, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 51.171, -729.035, 194.203, -36.2098, 2.39057, -53.81, -1, 7);
				PlaceObjectByHash(3681122061, 52.8966, -727.773, 192.637, -33.1499, -6.83019, -53.8101, -1, 7);
				PlaceObjectByHash(3681122061, 54.586, -726.537, 191.27, -30.0898, 8.70849, -53.8099, -1, 7);
				PlaceObjectByHash(3681122061, 56.5413, -725.105, 189.866, -25.4998, 7.59859, -53.8099, -1, 7);
				PlaceObjectByHash(3681122061, 58.8359, -723.425, 188.509, -22.4398, 4.26887, -53.81, -1, 7);
				PlaceObjectByHash(3681122061, 60.738, -722.033, 187.536, -18.3599, 1.10991, -53.81, -1, 7);
				PlaceObjectByHash(3681122061, 63.1509, -720.268, 186.544, -15.8098, 5.03727, -53.8099, -1, 7);
				PlaceObjectByHash(3681122061, 65.131, -718.821, 185.849, -12.7498, 5.07995, -53.8099, -1, 7);
				PlaceObjectByHash(3681122061, 67.1384, -717.352, 185.286, -9.17981, 4.78113, -53.81, -1, 7);
				PlaceObjectByHash(3681122061, 69.2894, -715.776, 184.855, -4.5898, 4.18349, -53.8099, -1, 7);
				PlaceObjectByHash(3681122061, 71.7831, -713.952, 184.607, 0.000193536, 4.16213, -53.8099, -1, 7);
				PlaceObjectByHash(3681122061, 74.0832, -712.268, 184.607, 3.06019, 3.7566, -53.81, -1, 7);
				PlaceObjectByHash(3681122061, 76.0175, -710.853, 184.736, 8.1602, 4.35424, -53.81, -1, 7);
				PlaceObjectByHash(3681122061, 77.7752, -709.567, 185.048, 13.2602, 5.50684, -53.81, -1, 7);
				PlaceObjectByHash(3681122061, 79.6997, -708.158, 185.61, 17.3402, 3.7566, -53.8099, -1, 7);
				PlaceObjectByHash(3681122061, 81.3947, -706.918, 186.266, 21.9302, 4.26887, -53.81, -1, 7);
				PlaceObjectByHash(3681122061, 83.3036, -705.52, 187.219, 26.0102, 9.39151, -53.8099, -1, 7);
				PlaceObjectByHash(3681122061, 85.6244, -703.821, 188.622, 29.0702, 1.96368, -53.8099, -1, 7);
				PlaceObjectByHash(3681122061, 87.3526, -702.556, 189.812, 33.1501, 2.90283, -53.8098, -1, 7);
				PlaceObjectByHash(3681122061, 89.2107, -701.196, 191.316, 37.2301, 4.86651, -53.8098, -1, 7);
				PlaceObjectByHash(3681122061, 90.8492, -699.998, 192.859, 41.82, -2.56132, -53.8099, -1, 7);
				PlaceObjectByHash(3681122061, 92.6236, -698.701, 194.826, 46.41, 8.2816, -53.8099, -1, 7);
				PlaceObjectByHash(3681122061, 94.2096, -697.539, 196.89, 52.0199, 6.57406, -53.8098, -1, 7);
				PlaceObjectByHash(3681122061, 95.6251, -696.503, 199.137, 56.61, 9.22075, -53.8097, -1, 7);
				PlaceObjectByHash(3681122061, 96.9799, -695.512, 201.683, 61.7098, 8.53774, -53.8097, -1, 7);
				PlaceObjectByHash(3681122061, 98.1658, -694.646, 204.413, 65.7899, 5.03726, -53.8096, -1, 7);
				PlaceObjectByHash(3681122061, -69.0186, -829.452, 324.775, 0, -0, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -65.276, -836.288, 321.491, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -62.2554, -842.061, 315, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -59.2515, -847.802, 308.544, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -56.2313, -853.574, 302.053, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -53.1945, -859.378, 295.526, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -50.2071, -865.088, 289.106, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -47.2032, -870.829, 282.65, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -44.1829, -876.602, 276.159, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -41.1626, -882.374, 269.667, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -38.1751, -888.084, 263.247, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -35.1713, -893.825, 256.791, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -32.1674, -899.566, 250.335, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -29.1635, -905.307, 243.879, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -26.1432, -911.079, 237.388, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -23.1393, -916.821, 230.932, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -20.119, -922.593, 224.44, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -17.1152, -928.334, 217.985, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -14.1112, -934.075, 211.529, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -11.1235, -939.785, 205.108, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -8.13589, -945.495, 198.687, -44.8796, 3.24434, -152.398, -1, 7);
				PlaceObjectByHash(3681122061, -5.28891, -951.101, 192.102, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, -2.35757, -956.552, 185.364, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 0.589775, -962.033, 178.59, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 3.5211, -967.483, 171.852, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 9.3998, -978.414, 158.339, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 6.46847, -972.964, 165.077, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 12.3311, -983.865, 151.601, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 15.2944, -989.375, 144.789, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 21.1569, -1000.28, 131.313, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 18.2417, -994.856, 138.014, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 24.0722, -1005.7, 124.612, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 27.0355, -1011.21, 117.801, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 29.9828, -1016.69, 111.026, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 35.8294, -1027.56, 97.5867, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 37.2241, -1030.15, 94.4555, -44.3698, -1.70754, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 38.7475, -1032.99, 91.3086, -39.7798, -4.26887, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 39.7483, -1034.85, 89.5491, -36.7197, 3.4151, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 40.9818, -1037.14, 87.6062, -33.6597, -2.21981, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 42.1248, -1039.27, 85.999, -30.0898, -1.96368, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 32.9141, -1022.14, 104.288, -47.4298, -1.79292, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 43.3132, -1041.47, 84.5449, -26.5197, -1.79293, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 44.7115, -1044.08, 83.0715, -23.4597, -2.39056, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 45.9496, -1046.38, 81.937, -20.3997, -2.47594, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 47.0815, -1048.48, 81.0483, -17.3397, -2.09174, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 48.302, -1050.75, 80.2436, -14.7897, -8.96462, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 50.0647, -1054.03, 79.2608, -13.2597, -2.77476, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 52.0238, -1057.67, 78.2861, -11.7297, -1.45141, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 53.7395, -1060.86, 77.5341, -9.17973, -1.1099, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 55.7265, -1064.56, 76.8558, -6.11973, -6.61674, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 57.3746, -1067.62, 76.4825, -3.56972, -9.60496, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 59.3125, -1071.23, 76.2272, 0.000276446, -9.05818, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 61.112, -1074.58, 76.2272, 4.08028, -8.00412, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 62.529, -1077.21, 76.4405, 7.65027, -7.04364, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 64.0779, -1080.1, 76.8796, 10.7103, -1.28066, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 65.9388, -1083.55, 77.623, 14.2802, -1.70755, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 67.4076, -1086.29, 78.4126, 17.3403, -1.57948, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, 68.7639, -1088.81, 79.3066, 21.4202, 1.36604, -151.734, -1, 7);
				PlaceObjectByHash(3681122061, -86.0915, -825.576, 324.775, 0, -0, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -99.1939, -833.684, 315.911, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -105.248, -837.511, 310.056, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -93.1729, -829.876, 321.734, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -111.268, -841.319, 304.233, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -123.245, -848.891, 292.651, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -147.333, -864.12, 269.359, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -141.278, -860.292, 275.213, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -135.256, -856.485, 281.036, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -129.266, -852.699, 286.828, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -117.224, -845.084, 298.474, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -165.367, -875.521, 251.921, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -213.415, -905.895, 205.464, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -207.426, -902.108, 211.255, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -201.403, -898.301, 217.078, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -195.414, -894.515, 222.87, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -183.434, -886.942, 234.452, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -177.445, -883.156, 240.244, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -171.422, -879.348, 246.067, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -159.378, -871.734, 257.713, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -153.355, -867.927, 263.536, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -189.424, -890.728, 228.661, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -231.449, -917.296, 188.027, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -225.46, -913.509, 193.818, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -219.47, -909.723, 199.609, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -237.439, -921.082, 182.235, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -284.806, -951.016, 167.673, 28.5601, -7.5132, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -289.048, -953.697, 170.578, 34.1701, -3.41509, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -290.727, -954.757, 171.926, 37.23, 3.41509, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -286.998, -952.399, 169.084, 31.62, -1.36604, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -282.375, -949.481, 166.27, 26.0101, 3.50047, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -280.014, -947.991, 165.174, 21.4201, -6.83019, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -277.4, -946.336, 164.178, 17.8501, -3.15896, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -270.266, -941.827, 162.896, 4.59013, -1.79292, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -266.683, -939.562, 163.103, -6.11989, -1.38738, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -259.328, -934.913, 165.339, -17.8499, -4.69576, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -253.478, -931.22, 168.474, -26.01, -4.18349, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -248.103, -927.823, 172.247, -33.66, -5.97642, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -243.429, -924.868, 176.444, -39.27, -2.30519, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -256.593, -933.187, 166.676, -22.4399, -5.97641, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -262.183, -936.72, 164.252, -14.2799, -3.20165, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -268.321, -940.597, 162.896, 0.000125527, -1.95033, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -272.668, -943.344, 163.123, 8.67012, -2.86014, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -275.091, -944.877, 163.561, 12.7501, -5.1226, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -249.757, -928.867, 170.944, -31.11, -6.0617, 122.286, -1, 7);
				PlaceObjectByHash(3681122061, -251.566, -930.012, 169.653, -27.54, -3.58585, 122.286, -1, 7);
#pragma endregion
			}
		}
		else if (mapIndex == 8) {


			bool load = 0, unload = 0, teleport = 0;
			AddTitle("Anonymous MENU");
			AddTitle("Freestyle Motocross");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, -2731, 3259, 32, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAP MOD
				PlaceObjectByHash(3213433054, -2661.22, 3209.43, 32.7118, 0, -0, -120.437, -1, 8);
				PlaceObjectByHash(3213433054, -2657.86, 3207.56, 32.7118, 0, 0, 59.6808, -1, 8);
				PlaceObjectByHash(3213433054, -2659.52, 3212.33, 32.7118, 0, -0, -120.437, -1, 8);
				PlaceObjectByHash(3213433054, -2656.2, 3210.41, 32.7118, 0, 0, 59.6808, -1, 8);
				PlaceObjectByHash(3213433054, -2654.54, 3213.26, 32.7118, 0, 0, 59.6808, -1, 8);
				PlaceObjectByHash(3213433054, -2657.84, 3215.18, 32.7118, 0, -0, -120.437, -1, 8);
				PlaceObjectByHash(3213433054, -2615.14, 3183, 32.8118, 0, -0, -120.45, -1, 8);
				PlaceObjectByHash(3213433054, -2611.82, 3181.02, 32.8118, 0, 0, 58.529, -1, 8);
				PlaceObjectByHash(3213433054, -2613.47, 3185.85, 32.8118, 0, -0, -120.45, -1, 8);
				PlaceObjectByHash(3213433054, -2610.11, 3183.91, 32.8118, 0, 0, 59.589, -1, 8);
				PlaceObjectByHash(3213433054, -2611.79, 3188.7, 32.8118, 0, -0, -120.45, -1, 8);
				PlaceObjectByHash(3213433054, -2608.46, 3186.71, 32.8118, 0, 0, 58.529, -1, 8);
				PlaceObjectByHash(3213433054, -2550.83, 3162.65, 32.7702, 0, -0, -117.989, -1, 8);
				PlaceObjectByHash(4109455646, -2547.01, 3160.66, 34.9496, 11.66, -2.56132e-006, -118.95, -1, 8);
				PlaceObjectByHash(3213433054, -2476.76, 3120.72, 32.7718, 0, -0, -119.612, -1, 8);
				PlaceObjectByHash(4109455646, -2473.51, 3118.83, 34.5672, 8.47412, -0.0223369, -120.965, -1, 8);
				PlaceObjectByHash(2975320548, -2442.43, 3105.7, 35.6224, -7.42001, -5.12265e-006, 150.074, -1, 8);
				PlaceObjectByHash(2975320548, -2443.67, 3101.83, 35.8732, -11.66, 3.84198e-006, 59.738, -1, 8);
				PlaceObjectByHash(2975320548, -2438.8, 3099, 36.0155, -11.66, -6.40331e-006, 59.7379, -1, 8);
				PlaceObjectByHash(2975320548, -2434.05, 3096.22, 36.6871, -22.26, -1.45141e-005, 59.7379, -1, 8);
				PlaceObjectByHash(2975320548, -2446.85, 3098.2, 35.6088, -7.42, 3.20165e-006, -31.8463, -1, 8);
				PlaceObjectByHash(2402097066, -2448.77, 3097.65, 35.4732, 0, -0, -120.616, -1, 8);
				PlaceObjectByHash(2402097066, -2442.95, 3108.08, 35.4832, 0, -0, -117.436, -1, 8);
				PlaceObjectByHash(2402097066, -2442.95, 3108.08, 35.4832, 0, -0, -117.436, -1, 8);
				PlaceObjectByHash(3681122061, -2389.55, 3069.11, 36.5952, 179.801, -3.76861e-006, -117.806, -1, 8);
				PlaceObjectByHash(1982829832, -2363.33, 3056.01, 31.8257, 0, -0, -119.219, -1, 8);
				PlaceObjectByHash(3681122061, -2389.55, 3069.11, 36.5952, 179.801, -3.76861e-006, -117.806, -1, 8);
				PlaceObjectByHash(2609922146, -2358.79, 3060.59, 31.8217, 0, -0, -119.371, -1, 8);
				PlaceObjectByHash(2975320548, -2325.93, 3034.99, 33.3214, 19.8, 3.41509e-006, -120.09, -1, 8);
				PlaceObjectByHash(2975320548, -2321.78, 3032.58, 36.3899, 25.7399, -8.62311e-005, -120.09, -1, 8);
				PlaceObjectByHash(2975320548, -2317.79, 3030.29, 39.6222, 25.0799, -6.23256e-005, -120.09, -1, 8);
				PlaceObjectByHash(2975320548, -2313.74, 3027.94, 42.9228, 25.7399, -8.62311e-005, -120.09, -1, 8);
				PlaceObjectByHash(2975320548, -2309.83, 3025.69, 46.2289, 27.06, -8.2816e-005, -120.09, -1, 8);
				PlaceObjectByHash(2975320548, -2306.07, 3023.49, 49.5919, 29.0399, -0.000116113, -120.09, -1, 8);
				PlaceObjectByHash(2975320548, -2283.14, 3009.97, 44.7284, 14.12, -2.04906e-005, 60.0397, -1, 8);
				PlaceObjectByHash(2975320548, -2287.5, 3012.47, 46.9591, 13.6, 0.680011, 60.0397, -1, 8);
				PlaceObjectByHash(2975320548, -2302.26, 3021.28, 53.174, 29.6999, -0.000100745, -120.09, -1, 8);
				PlaceObjectByHash(2975320548, -2292.06, 3015.11, 49.2546, 13.6, 1.53679e-005, 60.0397, -1, 8);
				PlaceObjectByHash(2975320548, -2298.56, 3019.12, 56.7472, 30.36, -8.79386e-005, -120.09, -1, 8);
				PlaceObjectByHash(2052512905, -2294.52, 3015.08, 58.6366, 82.6616, 0.00430302, -31.2919, -1, 8);
				PlaceObjectByHash(2052512905, -2293.13, 3017.4, 58.6822, 80.9428, 0.00560716, 149.187, -1, 8);
				PlaceObjectByHash(2787492567, -2293.66, 3016.58, 31.8318, -90, 0.0833042, 109.919, -1, 8);
				PlaceObjectByHash(3213433054, -2202.78, 2963.39, 32.8003, 0, -0, -120.04, -1, 8);
				PlaceObjectByHash(3213433054, -2199.53, 2961.53, 34.17, -40.5599, -2.56132e-006, 59.8803, -1, 8);
				PlaceObjectByHash(3681122061, -2137.1, 2904.97, 32.8327, 16.8, -1.10991e-005, -141.061, -1, 8);
				PlaceObjectByHash(3681122061, -2132.27, 2897.94, 34.4465, 16.8, -14, -141.061, -1, 8);
				PlaceObjectByHash(3681122061, -2127.12, 2890.88, 36.4432, 17.92, -29.68, -136.581, -1, 8);
				PlaceObjectByHash(3681122061, -2119.98, 2885.33, 38.8379, 17.92, -29.68, -136.581, -1, 8);
				PlaceObjectByHash(3681122061, -2113.02, 2880, 41.2705, 17.92, -29.68, -136.581, -1, 8);
				PlaceObjectByHash(3681122061, -2085.18, 2857.71, 49.9177, 19.04, -43.12, -136.581, -1, 8);
				PlaceObjectByHash(3681122061, -2078.1, 2852.44, 51.662, 19.0399, -50.4001, -136.581, -1, 8);
				PlaceObjectByHash(3681122061, -2092.05, 2863.54, 48.2285, 17.92, -34.16, -136.581, -1, 8);
				PlaceObjectByHash(3681122061, -2098.91, 2869.18, 46.2053, 17.92, -29.68, -136.581, -1, 8);
				PlaceObjectByHash(3681122061, -2105.97, 2874.59, 43.7379, 17.92, -29.68, -136.581, -1, 8);
				PlaceObjectByHash(3681122061, -2070.42, 2847.69, 53.5814, 19.0399, -50.4001, -136.581, -1, 8);
				PlaceObjectByHash(3681122061, -2062.85, 2843.01, 55.4739, 19.0399, -50.4001, -136.581, -1, 8);
				PlaceObjectByHash(3681122061, -2055.32, 2838.69, 56.5097, 17.7868, -43.8868, -131.905, -1, 8);
				PlaceObjectByHash(3681122061, -2047.61, 2834.88, 58.9097, 26.1867, -43.8868, -131.905, -1, 8);
				PlaceObjectByHash(3681122061, -2039.74, 2832.2, 62.2769, 38.5067, -45.5668, -131.905, -1, 8);
				PlaceObjectByHash(3681122061, -1996.98, 2830.2, 48.384, 0.202822, -14.4337, -105.503, -1, 8);
				PlaceObjectByHash(3681122061, -1996.42, 2832.89, 59.0601, -179.433, 12.3451, 76.9258, -1, 8);
				PlaceObjectByHash(3213433054, -1951.86, 2849.63, 34.5146, -47.5199, -7.59859e-005, 59.6261, -1, 8);
				PlaceObjectByHash(3213433054, -1950.16, 2852.52, 34.5146, -47.5199, -7.59859e-005, 59.6261, -1, 8);
				PlaceObjectByHash(3213433054, -1953.57, 2854.49, 32.8004, 0, -0, -120.091, -1, 8);
				PlaceObjectByHash(3213433054, -1955.25, 2851.59, 32.8004, 0, -0, -120.091, -1, 8);
				PlaceObjectByHash(4111834409, -1960.72, 2857.38, 31.7305, 0, -0, -118.505, -1, 8);
				PlaceObjectByHash(4109455646, -2144, 2967.21, 36.0606, 9.35852, -0.00134085, 59.8371, -1, 8);
				PlaceObjectByHash(4109455646, -2139.63, 2964.67, 33.9985, 5.84852, -0.0013321, 59.8371, -1, 8);
				PlaceObjectByHash(4109455646, -2135.45, 2962.3, 32.4604, 0, 0, 60.4792, -1, 8);
				PlaceObjectByHash(2975320548, -2193.23, 2995.21, 35.0684, 11.6996, -0.00262322, -119.238, -1, 8);
				PlaceObjectByHash(2975320548, -2197.74, 2997.74, 32.8074, 15.2099, 2.04906e-005, -119.328, -1, 8);
				PlaceObjectByHash(3213433054, -2246.82, 3026.19, 33.0318, 0.0331696, 0.0056356, 58.6423, -1, 8);
				PlaceObjectByHash(3213433054, -2256.38, 3032.02, 35.4343, 6.5707, 0.0279573, 58.7685, -1, 8);
				PlaceObjectByHash(3213433054, -2265.19, 3037.37, 38.408, 10.1262, 0.0254109, 58.7585, -1, 8);
				PlaceObjectByHash(3213433054, -2273.45, 3042.38, 40.214, 8.95404, -0.00182451, 58.7729, -1, 8);
				PlaceObjectByHash(3213433054, -2281.36, 3047.19, 42.7382, 8.89319, 0.151422, 58.8279, -1, 8);
				PlaceObjectByHash(3213433054, -2289.41, 3052.05, 46.2871, 13.2, 0.000150264, 58.7642, -1, 8);
				PlaceObjectByHash(3213433054, -2397.86, 3114.2, 32.8449, 0, 0, 60.2049, -1, 8);
				PlaceObjectByHash(3213433054, -2402.38, 3116.77, 34.7648, 0, 0, 60.2049, -1, 8);
				PlaceObjectByHash(2475986526, -2394.65, 3118.07, 32.5452, 0, 0, 56.6241, -1, 8);
				PlaceObjectByHash(2475986526, -2397.73, 3120.09, 34.1452, 2.04, -1.38738e-006, 56.6241, -1, 8);
				PlaceObjectByHash(2475986526, -2401.78, 3122.77, 36.6227, 8.16, 1.28066e-006, 56.6239, -1, 8);
				PlaceObjectByHash(2475986526, -2405.48, 3125.2, 39.5571, 14.28, 1.02453e-005, 56.6239, -1, 8);
				PlaceObjectByHash(2475986526, -2409.12, 3127.6, 43.2064, 20.4, -2.39057e-005, 56.6239, -1, 8);
				PlaceObjectByHash(2475986526, -2412.29, 3129.71, 46.9494, 24.4781, -1.43125, 56.2632, -1, 8);
				PlaceObjectByHash(2475986526, -2415.18, 3131.49, 51.529, 38.3931, -3.70399, 55.299, -1, 8);
				PlaceObjectByHash(2475986526, -2416.96, 3132.28, 56.2986, 54.0331, -3.70398, 53.2589, -1, 8);
				PlaceObjectByHash(2475986526, -2417.37, 3132.16, 61.6124, 73.753, -3.70394, 53.2588, -1, 8);
				PlaceObjectByHash(2475986526, -2416.48, 3131.04, 66.996, 90.9129, -3.70395, 53.2587, -1, 8);
				PlaceObjectByHash(2475986526, -2414.88, 3129.5, 70.998, 104.113, -3.70383, 50.6186, -1, 8);
				PlaceObjectByHash(2475986526, -2412.46, 3127.2, 74.61, 116.653, -3.70392, 50.6185, -1, 8);
				PlaceObjectByHash(2475986526, -2409.58, 3124.71, 77.6119, 121.273, -3.70395, 50.6185, -1, 8);
				PlaceObjectByHash(2475986526, -2406.75, 3122.18, 80.0586, 127.213, -3.70391, 50.6184, -1, 8);
				PlaceObjectByHash(2475986526, -2403.38, 3119.23, 82.2502, 135.793, -3.70396, 50.6185, -1, 8);
				PlaceObjectByHash(2475986526, -2369.71, 3092.81, 68.2807, -146.327, -3.7039, 50.6183, -1, 8);
				PlaceObjectByHash(2475986526, -2367.45, 3091.4, 63.3347, -134.447, -3.70392, 50.6182, -1, 8);
				PlaceObjectByHash(2475986526, -2366, 3090.66, 58.0814, -123.887, -3.7039, 50.6182, -1, 8);
				PlaceObjectByHash(2475986526, -2365.38, 3090.57, 53.1623, -112.007, -3.70391, 50.6182, -1, 8);
				PlaceObjectByHash(2475986526, -2365.62, 3091.18, 48.0172, -99.4666, -3.70393, 50.6181, -1, 8);
				PlaceObjectByHash(2475986526, -2366.77, 3092.54, 43.04, -86.2661, -3.70399, 50.6181, -1, 8);
				PlaceObjectByHash(2475986526, -2368.73, 3094.52, 38.5669, -74.386, -3.70392, 50.6181, -1, 8);
				PlaceObjectByHash(2475986526, -2371.25, 3096.8, 35.0692, -59.206, -3.70384, 55.2379, -1, 8);
				PlaceObjectByHash(2475986526, -2375.18, 3099.61, 32.3997, -42.0459, -3.70387, 57.2179, -1, 8);
				PlaceObjectByHash(2475986526, -2395.69, 3112.77, 84.6355, 152.292, -3.70389, 50.6184, -1, 8);
				PlaceObjectByHash(2475986526, -2391.54, 3109.37, 84.6603, 162.192, -3.70393, 50.6184, -1, 8);
				PlaceObjectByHash(2475986526, -2387.13, 3105.84, 83.6595, 172.752, -3.70391, 50.6184, -1, 8);
				PlaceObjectByHash(2475986526, -2382.97, 3102.56, 81.8101, -179.988, -3.70391, 50.6184, -1, 8);
				PlaceObjectByHash(2475986526, -2372.52, 3094.76, 72.6855, -154.907, -3.70391, 50.6183, -1, 8);
				PlaceObjectByHash(2475986526, -2379.11, 3099.59, 79.371, -172.728, -3.70391, 50.6183, -1, 8);
				PlaceObjectByHash(2475986526, -2375.46, 3096.85, 76.1692, -162.168, -3.70388, 50.6182, -1, 8);
				PlaceObjectByHash(2475986526, -2399.8, 3116.19, 83.7512, 143.712, -3.70387, 50.6184, -1, 8);
				PlaceObjectByHash(3213433054, -2510.73, 3180.4, 32.8111, 0, 0, 59.4291, -1, 8);
				PlaceObjectByHash(209943352, -2302.92, 3059.95, 50.2208, 76.8397, -0.679965, -120.716, -1, 8);
				PlaceObjectByHash(209943352, -2298.84, 3057.5, 48.7042, 71.3997, -0.679954, -120.716, -1, 8);
				PlaceObjectByHash(209943352, -2290.6, 3052.58, 47.3498, 84.3198, -0.679946, -120.716, -1, 8);
				PlaceObjectByHash(209943352, -2290.6, 3052.58, 47.3498, 84.3198, -0.679946, -120.716, -1, 8);
				PlaceObjectByHash(209943352, -2294.73, 3055.05, 47.6692, 76.8398, -0.680059, -120.716, -1, 8);
				PlaceObjectByHash(209943352, -2533.2, 3193.91, 37.3948, 0, -0, -120.716, -1, 8);
				PlaceObjectByHash(209943352, -2533.2, 3193.91, 37.3948, 0, -0, -120.716, -1, 8);
				PlaceObjectByHash(209943352, -2425.58, 3091.36, 36.493, 0, -0, -120.716, -1, 8);
				PlaceObjectByHash(209943352, -2425.58, 3091.36, 36.493, 0, -0, -120.716, -1, 8);
				PlaceObjectByHash(209943352, -2293.7, 3012.65, 55.3685, -89.7587, -0.659716, -30.2946, -1, 8);
				PlaceObjectByHash(209943352, -2293.7, 3012.65, 55.3685, -89.7587, -0.659716, -30.2946, -1, 8);
#pragma endregion
			}
		}
		else if (mapIndex == 9) {



			bool load = 0, unload = 0, teleport = 0;
			AddTitle("Anonymous MENU");
			AddTitle("Halfpipe Fun Track");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, -1003, -2916, 14, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAP MOD
				PlaceObjectByHash(3681122061, -1018.78, -2937.26, 12.9646, 0, 0, -30.3132, -1, 9);
				PlaceObjectByHash(3681122061, -1023.38, -2945.17, 12.9646, 0, 0, -30.3132, -1, 9);
				PlaceObjectByHash(3681122061, -1028.02, -2953.13, 12.9646, 0, 0, -30.3132, -1, 9);
				PlaceObjectByHash(3681122061, -1032.66, -2961.06, 12.9646, 0, 0, -30.3132, -1, 9);
				PlaceObjectByHash(3681122061, -1037.32, -2969.04, 12.9646, 0, 0, -30.3132, -1, 9);
				PlaceObjectByHash(3681122061, -1041.95, -2976.96, 12.9646, 0, 0, -30.3132, -1, 9);
				PlaceObjectByHash(3681122061, -1046.18, -2984.19, 12.9646, 0, 0, -30.3132, -1, 9);
				PlaceObjectByHash(3681122061, -1050.78, -2992.12, 12.9646, 0, 0, -29.8732, -1, 9);
				PlaceObjectByHash(3681122061, -1053.22, -2998.13, 12.9646, 0, 0, -14.2534, -1, 9);
				PlaceObjectByHash(3681122061, -1054.14, -3005.28, 12.9646, 0, 0, -0.613478, -1, 9);
				PlaceObjectByHash(3681122061, -1053.45, -3012.85, 12.9646, 0, 0, 11.4866, -1, 9);
				PlaceObjectByHash(3681122061, -1051.19, -3020.08, 12.9646, 0, 0, 23.3667, -1, 9);
				PlaceObjectByHash(3681122061, -1047.43, -3026.73, 12.9646, 0, 0, 35.2469, -1, 9);
				PlaceObjectByHash(3681122061, -1042.42, -3032.37, 12.9646, 0, 0, 47.7871, -1, 9);
				PlaceObjectByHash(3681122061, -1037.1, -3038.16, 12.9646, 0, 0, 37.2273, -1, 9);
				PlaceObjectByHash(3681122061, -1033.11, -3044.75, 12.9646, 0, 0, 25.5675, -1, 9);
				PlaceObjectByHash(3681122061, -1030.43, -3052.11, 12.9646, 0, 0, 14.5676, -1, 9);
				PlaceObjectByHash(3681122061, -1029.18, -3059.85, 12.9646, 0, 0, 4.00757, -1, 9);
				PlaceObjectByHash(3681122061, -1029.37, -3067.7, 12.9646, 0, 0, -6.55247, -1, 9);
				PlaceObjectByHash(3681122061, -1031, -3075.33, 12.9646, 0, 0, -17.5525, -1, 9);
				PlaceObjectByHash(3681122061, -1034.09, -3082.35, 12.9646, 0, 0, -29.6525, -1, 9);
				PlaceObjectByHash(3681122061, -1038.6, -3088.77, 12.9646, 0, 0, -40.2127, -1, 9);
				PlaceObjectByHash(3681122061, -1044.19, -3094.15, 12.9646, 0, 0, -51.653, -1, 9);
				PlaceObjectByHash(3681122061, -1050.65, -3098.2, 12.9646, 0, 0, -63.7531, -1, 9);
				PlaceObjectByHash(3681122061, -1057.89, -3100.91, 12.9646, 0, 0, -75.1935, -1, 9);
				PlaceObjectByHash(3681122061, -1065.18, -3101.87, 12.9646, 0, 0, -89.7139, -1, 9);
				PlaceObjectByHash(3681122061, -1073.03, -3101.2, 12.9646, 0, -0, -100.054, -1, 9);
				PlaceObjectByHash(3681122061, -1080.63, -3099.11, 12.9646, 0, -0, -110.615, -1, 9);
				PlaceObjectByHash(3681122061, -1087.92, -3095.65, 12.9646, 0, -0, -119.855, -1, 9);
				PlaceObjectByHash(3681122061, -1095.95, -3091.03, 12.9646, 0, -0, -119.855, -1, 9);
				PlaceObjectByHash(3681122061, -1104.01, -3086.4, 12.9646, 0, -0, -119.855, -1, 9);
				PlaceObjectByHash(3681122061, -1112.04, -3081.79, 12.9646, 0, -0, -119.855, -1, 9);
				PlaceObjectByHash(3681122061, -1120.04, -3077.19, 12.9646, 0, -0, -119.855, -1, 9);
				PlaceObjectByHash(3681122061, -1128.1, -3072.56, 12.9646, 0, -0, -119.855, -1, 9);
				PlaceObjectByHash(3681122061, -1136.15, -3067.93, 12.9646, 0, -0, -119.855, -1, 9);
				PlaceObjectByHash(3681122061, -1144.2, -3063.31, 12.9646, 0, -0, -119.855, -1, 9);
				PlaceObjectByHash(3681122061, -1152.22, -3058.7, 12.9646, 0, -0, -119.855, -1, 9);
				PlaceObjectByHash(3681122061, -1160.24, -3054.09, 12.9646, 0, -0, -119.855, -1, 9);
				PlaceObjectByHash(3681122061, -1168.22, -3049.48, 12.9646, 0, -0, -120.295, -1, 9);
				PlaceObjectByHash(3681122061, -1176.21, -3044.8, 12.9646, 0, -0, -120.295, -1, 9);
				PlaceObjectByHash(3681122061, -1183.28, -3040.14, 12.9646, 0, -0, -126.455, -1, 9);
				PlaceObjectByHash(3681122061, -1189.23, -3034.89, 12.9646, 0, -0, -136.356, -1, 9);
				PlaceObjectByHash(3681122061, -1193.86, -3028.84, 12.9646, 0, -0, -148.677, -1, 9);
				PlaceObjectByHash(3681122061, -1197.2, -3021.86, 12.9646, 0, -0, -159.898, -1, 9);
				PlaceObjectByHash(3681122061, -1198.78, -3014.77, 12.9646, 0, -0, -174.639, -1, 9);
				PlaceObjectByHash(3681122061, -1198.72, -3007.04, 12.9646, 0, -0, 173.701, -1, 9);
				PlaceObjectByHash(3681122061, -1197, -2999.97, 12.9646, 0, -0, 158.962, -1, 9);
				PlaceObjectByHash(3681122061, -1193.5, -2993.3, 12.9646, 0, -0, 145.982, -1, 9);
				PlaceObjectByHash(3681122061, -1188.51, -2987.1, 12.9646, 0, -0, 136.083, -1, 9);
				PlaceObjectByHash(3681122061, -1182.5, -2981.85, 12.9646, 0, -0, 126.183, -1, 9);
				PlaceObjectByHash(3681122061, -1175.98, -2978.23, 12.9646, 0, -0, 112.104, -1, 9);
				PlaceObjectByHash(3681122061, -1168.67, -2976.15, 12.9646, 0, -0, 99.7843, -1, 9);
				PlaceObjectByHash(3681122061, -1160.82, -2975.53, 12.9646, 0, 0, 89.4449, -1, 9);
				PlaceObjectByHash(3681122061, -1152.93, -2976.29, 12.9646, 0, 0, 79.5455, -1, 9);
				PlaceObjectByHash(3681122061, -1145.21, -2978.39, 12.9646, 0, 0, 70.0859, -1, 9);
				PlaceObjectByHash(3681122061, -1138.14, -2981.75, 12.9646, 0, 0, 59.0863, -1, 9);
				PlaceObjectByHash(3681122061, -1130.27, -2986.43, 12.9646, 0, 0, 59.0863, -1, 9);
				PlaceObjectByHash(3681122061, -1122.46, -2991.09, 12.9646, 0, 0, 59.0863, -1, 9);
				PlaceObjectByHash(3681122061, -1115.12, -2994.75, 12.9646, 0, 0, 67.1435, -1, 9);
				PlaceObjectByHash(3681122061, -1107.63, -2997.13, 12.9646, 0, 0, 76.9913, -1, 9);
				PlaceObjectByHash(3681122061, -1099.8, -2998.14, 12.9646, 0, 0, 86.8389, -1, 9);
				PlaceObjectByHash(3681122061, -1091.94, -2997.76, 12.9646, 0, -0, 97.5819, -1, 9);
				PlaceObjectByHash(3681122061, -1084.47, -2995.95, 12.9646, 0, -0, 108.325, -1, 9);
				PlaceObjectByHash(3681122061, -1077.64, -2992.78, 12.9646, 0, -0, 119.963, -1, 9);
				PlaceObjectByHash(3681122061, -1077.64, -2992.78, 12.9646, 0, -0, 119.963, -1, 9);
				PlaceObjectByHash(3681122061, -1071.68, -2988.3, 12.9646, 0, -0, 132.496, -1, 9);
				PlaceObjectByHash(3681122061, -1066.33, -2982.53, 12.9646, 0, -0, 141.449, -1, 9);
				PlaceObjectByHash(3681122061, -1053.01, -2960.01, 12.9646, 0, 0, -28.5532, -1, 9);
				PlaceObjectByHash(3681122061, -1048.58, -2951.88, 12.9646, 0, 0, -28.5532, -1, 9);
				PlaceObjectByHash(3681122061, -1044.16, -2943.76, 12.9646, 0, 0, -28.5532, -1, 9);
				PlaceObjectByHash(3681122061, -1039.74, -2935.64, 12.9646, 0, 0, -28.5532, -1, 9);
				PlaceObjectByHash(3681122061, -1035.5, -2927.86, 12.9646, 0, 0, -28.5532, -1, 9);
				PlaceObjectByHash(3608473212, -1063.23, -2993.67, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1067.37, -2998.06, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1063.7, -2994.67, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1064.21, -2995.73, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1064.71, -2996.8, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1065.21, -2997.84, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1067.1, -2999.45, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1065.43, -3000.24, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1068.29, -3000.06, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1065.92, -3001.18, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1066.44, -3002.28, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1065.35, -3002.77, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1065.6, -3003.96, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1066.11, -3005.04, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1064.21, -2999.56, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1069.53, -3000.82, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1070.75, -3001.52, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1068.76, -3002.46, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1069.29, -3003.54, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1068.44, -3005.23, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1071.58, -3003.74, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(2475986526, -1072.98, -3006.81, 16.0846, 0, 0, -26.0348, -1, 9);
				PlaceObjectByHash(3608473212, -1072.44, -3002.05, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(3608473212, -1073.73, -3002.72, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(2475986526, -1076.54, -3005.07, 16.0846, 0, 0, -26.0348, -1, 9);
				PlaceObjectByHash(2475986526, -1078.92, -3009.92, 13.7046, 0, 0, -26.0348, -1, 9);
				PlaceObjectByHash(2475986526, -1075.31, -3011.69, 13.7046, 0, 0, -26.0348, -1, 9);
				PlaceObjectByHash(2475986526, -1069.34, -3008.59, 16.0846, 0, 0, -26.0348, -1, 9);
				PlaceObjectByHash(2475986526, -1071.71, -3013.45, 13.7046, 0, 0, -26.0348, -1, 9);
				PlaceObjectByHash(2475986526, -1068.14, -3015.2, 13.7046, 0, 0, -26.0348, -1, 9);
				PlaceObjectByHash(3608473212, -1065.36, -3006.66, 15.3449, 0, 0, -25.1145, -1, 9);
				PlaceObjectByHash(2475986526, -1065.75, -3010.35, 16.0846, 0, 0, -26.0348, -1, 9);
#pragma endregion
			}
		}
		else if (mapIndex == 10) {



			bool load = 0, unload = 0, teleport = 0;
			//	AddTitle("Anonymous MENU");
			AddTitle("Airport Loop");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, -1074, -3201, 13, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAP MOD
				PlaceObjectByHash(3966705493, -1041.89, -3219.51, 10.1797, -2.43331, 5.32208, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1037.79, -3221.47, 10.3641, -2.43331, 5.32208, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1034.16, -3223.3, 10.5366, -2.43331, 5.32208, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1036.76, -3219.45, 10.1526, -2.43331, 5.32208, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1033.12, -3221.28, 10.3251, -2.43331, 5.32208, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1029.37, -3225.6, 11.1956, -11.6033, 5.32207, 62.9335, -1, 10);
				PlaceObjectByHash(3966705493, -1028.33, -3223.58, 10.9842, -11.6033, 5.32207, 62.9335, -1, 10);
				PlaceObjectByHash(3966705493, -1024.27, -3225.54, 12.1104, -18.1533, 5.32205, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1025.44, -3227.83, 12.3497, -18.1533, 5.32205, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1020.36, -3230.06, 15.7972, -40.4234, 5.32214, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1019.22, -3227.83, 15.5634, -40.4234, 5.32214, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1014.85, -3229.56, 20.4393, -50.9034, 5.3221, 62.9337, -1, 10);
				PlaceObjectByHash(3966705493, -1016.07, -3231.95, 20.6898, -50.9034, 5.3221, 62.9337, -1, 10);
				PlaceObjectByHash(3966705493, -1012.88, -3232.96, 26.0664, -64.0034, 5.32209, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1011.63, -3230.51, 25.8104, -64.0034, 5.32209, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1009.27, -3231.06, 32.0819, -73.1735, 5.32204, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1010.52, -3233.51, 32.3379, -73.1735, 5.32214, 62.9337, -1, 10);
				PlaceObjectByHash(3966705493, -1009.36, -3233.48, 38.2311, -83.6535, 5.32208, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1008.09, -3230.98, 37.9695, -83.6535, 5.32208, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1007.71, -3230.43, 44.185, -92.8235, 6.63212, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1009.05, -3228.93, 49.9682, -119.024, 6.63217, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1008.93, -3232.81, 44.4969, -92.8235, 6.63212, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1010.27, -3231.31, 50.2801, -119.024, 6.63217, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1011.96, -3226.91, 54.0691, -142.604, 6.6321, 62.9337, -1, 10);
				PlaceObjectByHash(3966705493, -1013.1, -3229.14, 54.3602, -142.604, 6.6321, 62.9337, -1, 10);
				PlaceObjectByHash(3966705493, -1017.49, -3226.51, 57.2125, -159.634, 6.63211, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1022.14, -3223.91, 58.9186, -168.804, 6.63213, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1035.97, -3216.05, 58.7162, 155.826, 1.39214, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1031.37, -3218.71, 60.1775, 176.786, 6.63213, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1026.33, -3221.63, 59.8766, -168.804, 6.63212, 60.3136, -1, 10);
				PlaceObjectByHash(3966705493, -1020.98, -3221.63, 58.6206, -168.804, 6.63213, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1024.97, -3219.25, 59.5578, -168.804, 6.63212, 60.3136, -1, 10);
				PlaceObjectByHash(3966705493, -1016.33, -3224.24, 56.9145, -159.634, 6.63211, 62.9336, -1, 10);
				PlaceObjectByHash(3966705493, -1029.99, -3216.28, 59.8517, 176.786, 6.63213, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1029.99, -3216.28, 59.8517, 176.786, 6.63213, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1034.73, -3213.86, 58.655, 155.826, 1.39214, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1039.18, -3211.42, 55.2255, 138.796, 1.39206, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1039.18, -3211.42, 55.2255, 138.796, 1.39206, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1040.42, -3213.61, 55.2867, 138.796, 1.39206, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1044.34, -3211.51, 50.6082, 128.316, 1.39213, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1043.09, -3209.33, 50.547, 128.316, 1.39213, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1046.16, -3207.74, 45.1535, 117.837, 1.39215, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1048.17, -3206.74, 39.6252, 104.737, 1.39214, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1048.92, -3206.44, 33.1586, 87.6005, 0.0914728, 60.6227, -1, 10);
				PlaceObjectByHash(3966705493, -1048.18, -3206.88, 26.5446, 77.3408, 0.0913896, 60.6229, -1, 10);
				PlaceObjectByHash(3966705493, -1049.44, -3209.13, 26.5487, 77.3407, 0.0913427, 60.6228, -1, 10);
				PlaceObjectByHash(3966705493, -1047.3, -3210.37, 21.3947, 56.6411, 0.0914017, 58.823, -1, 10);
				PlaceObjectByHash(3966705493, -1047.3, -3210.37, 21.3947, 56.6411, 0.0914017, 58.823, -1, 10);
				PlaceObjectByHash(3966705493, -1045.93, -3208.12, 21.3905, 56.6411, 0.0914017, 58.823, -1, 10);
				PlaceObjectByHash(3966705493, -1042.61, -3210.12, 16.8766, 42.1517, 0.0913785, 58.8231, -1, 10);
				PlaceObjectByHash(3966705493, -1038.64, -3212.63, 13.6141, 28.2018, 0.0914187, 58.8231, -1, 10);
				PlaceObjectByHash(3966705493, -1039.97, -3214.83, 13.6182, 28.2018, 0.0914187, 58.8231, -1, 10);
				PlaceObjectByHash(3966705493, -1034.82, -3217.71, 11.1985, 16.4116, 0.0913871, 69.303, -1, 10);
				PlaceObjectByHash(3966705493, -1033.53, -3215.55, 11.1081, 16.4117, 0.0913619, 66.683, -1, 10);
				PlaceObjectByHash(3966705493, -1043.97, -3212.37, 16.8808, 42.1517, 0.0913785, 58.8231, -1, 10);
				PlaceObjectByHash(3966705493, -1050.19, -3208.69, 33.1627, 87.6005, 0.0914728, 60.6227, -1, 10);
				PlaceObjectByHash(3966705493, -1049.45, -3208.98, 39.6879, 104.737, 1.39214, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1047.44, -3209.98, 45.2161, 117.837, 1.39215, 60.3137, -1, 10);
				PlaceObjectByHash(3966705493, -1047.44, -3209.98, 45.2161, 117.837, 1.39215, 60.3137, -1, 10);
#pragma endregion
			}
		}

		else if (mapIndex == 11) {



			bool load = 0, unload = 0, teleport = 0;
			AddTitle("Anonymous MENU");
			AddTitle("Maze Bank Mega Ramp");
			AddOption("Teleport", teleport);
			AddOption("Load", load);
			if (teleport) {
				ENTITY::SET_ENTITY_COORDS(ped, -74.94243f, -818.63446f, 326.174347f, 1, 0, 0, 1);
			}
			if (load) {
#pragma region MAP MOD
				PlaceObjectByHash(3681122061, -82.9657, -818.944, 325.175, 0, -0, 91.03, -1, 11);
				PlaceObjectByHash(3681122061, -91.0941, -819.089, 322.355, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -98.36, -819.224, 316.632, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -105.626, -819.358, 310.91, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -112.892, -819.492, 305.187, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -120.158, -819.626, 299.464, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -127.424, -819.761, 293.741, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -134.69, -819.895, 288.018, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -141.956, -820.029, 282.296, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -149.222, -820.163, 276.573, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -156.487, -820.298, 270.85, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -163.753, -820.432, 265.127, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -171.019, -820.566, 259.404, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -178.285, -820.701, 253.682, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -185.551, -820.835, 247.959, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -192.817, -820.969, 242.236, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -200.083, -821.103, 236.513, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -207.349, -821.238, 230.79, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -214.615, -821.372, 225.068, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -221.881, -821.506, 219.345, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -229.147, -821.641, 213.622, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -236.413, -821.775, 207.899, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -243.679, -821.909, 202.176, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -250.945, -822.043, 196.453, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -258.21, -822.178, 190.731, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -265.476, -822.312, 185.008, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -287.274, -822.715, 167.839, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -280.008, -822.58, 173.562, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -272.742, -822.446, 179.285, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -294.54, -822.849, 162.117, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -301.806, -822.983, 156.394, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -309.072, -823.118, 150.671, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -316.338, -823.252, 144.948, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -323.604, -823.386, 139.225, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -330.87, -823.52, 133.503, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -338.136, -823.655, 127.78, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -345.402, -823.789, 122.057, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -352.668, -823.923, 116.334, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -359.934, -824.057, 110.611, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -367.199, -824.192, 104.889, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -374.465, -824.326, 99.1657, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -381.731, -824.46, 93.4429, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -388.997, -824.595, 87.7201, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -396.263, -824.729, 81.9973, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -403.529, -824.863, 76.2745, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -411.479, -825.022, 68.7971, -19.78, -3.43377e-005, 91.1094, -1, 11);
				PlaceObjectByHash(3681122061, -410.795, -824.997, 70.5517, -38.2199, 0.00787841, 91.0529, -1, 11);
				PlaceObjectByHash(3681122061, -411.96, -825.029, 69.097, -27.6, 1.15259e-005, 91.1095, -1, 11);
				PlaceObjectByHash(3681122061, -412.719, -825.046, 67.8516, -10.58, -2.92151e-006, 91.1095, -1, 11);
				PlaceObjectByHash(3681122061, -413.903, -825.068, 67.2075, -3.21999, 5.66959e-007, 91.1095, -1, 11);
				PlaceObjectByHash(3681122061, -415.378, -825.099, 66.7734, 3.68002, -4.58236e-006, 91.1095, -1, 11);
				PlaceObjectByHash(3681122061, -416.883, -825.126, 66.57, 9.66002, -8.44435e-006, 91.1096, -1, 11);
				PlaceObjectByHash(3681122061, -418.526, -825.157, 66.5571, 15.64, -1.80093e-005, 91.1095, -1, 11);
				PlaceObjectByHash(3681122061, -419.945, -825.184, 66.6727, 20.7001, 8.69782e-006, 91.1094, -1, 11);
				PlaceObjectByHash(3681122061, -421.727, -825.218, 67.0936, 25.7601, -2.09975e-005, 91.1095, -1, 11);
				PlaceObjectByHash(3681122061, -422.006, -825.234, 66.966, 30.8199, 0.114757, 90.6829, -1, 11);
				PlaceObjectByHash(3681122061, -429.913, -825.328, 71.6856, 30.8199, 0.114757, 90.6829, -1, 11);
#pragma endregion
			}
		}
	}
	void AddPlayer(char* name, Player player, bool &extra_option_code = null)
	{
		null = 0;
		AddOption(name, null);
		if (menu::printingop == menu::currentop)
		{
			if (null)
			{
				selpName = name;
				selPlayer = player;
				menu::SetSub_delayed = SUB::SELECTEDPLAYER;
				extra_option_code = true;
			}
		}
	}


	float Get3DDistance(Vector3 a, Vector3 b) {
		float x = pow((a.x - b.x), 2);
		float y = pow((a.y - b.y), 2);
		float z = pow((a.z - b.z), 2);
		return sqrt(x + y + z);
	}
	void LoadPlayerInfo(char* playerName, Player p) {


		menu::background2();
		Ped ped = p;
		RequestControlOfEnt(ped);
		float health = ENTITY::GET_ENTITY_HEALTH(ped);
		float maxHealth = ENTITY::GET_ENTITY_MAX_HEALTH(ped);
		float healthPercent = health * 100 / maxHealth;
		std::ostringstream Health; Health << "~b~Health:~s~ " << healthPercent;
		float armor = PED::GET_PED_ARMOUR(ped);
		float maxArmor = PLAYER::GET_PLAYER_MAX_ARMOUR(p);
		float armorPercent = armor * 100 / maxArmor;
		std::ostringstream Armor; Armor << "~b~Armor:~s~ " << armorPercent;
		bool alive = !PED::IS_PED_DEAD_OR_DYING(ped, 1);
		char* aliveStatus;
		if (alive) aliveStatus = "Yes"; else aliveStatus = "No";
		std::ostringstream Alive; Alive << "~b~Alive:~s~ " << aliveStatus;
		bool inVehicle = PED::IS_PED_IN_ANY_VEHICLE(ped, 0);
		std::ostringstream VehicleModel; VehicleModel << "~b~Vehicle:~s~ ";
		std::ostringstream Speed; Speed << "~b~Speed:~s~ ";
		std::ostringstream IsInAVehicle; IsInAVehicle << "~b~In Vehicle:~s~ ";
		if (inVehicle) {
			IsInAVehicle << "Yes";
			Hash vehHash = ENTITY::GET_ENTITY_MODEL(PED::GET_VEHICLE_PED_IS_IN(ped, 0));
			VehicleModel << UI::_GET_LABEL_TEXT(VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(vehHash));
			float vehSpeed = ENTITY::GET_ENTITY_SPEED(PED::GET_VEHICLE_PED_IS_IN(ped, 0));
			float vehSpeedConverted;
			if (useMPH) {
				vehSpeedConverted = round(vehSpeed * 2.23693629);
				Speed << vehSpeedConverted << " MPH";
			}
			else {
				vehSpeedConverted = round(vehSpeed * 1.6);
				Speed << vehSpeedConverted << " KM/H";
			}
		}
		else {
			IsInAVehicle << "No";
			float speed = round(ENTITY::GET_ENTITY_SPEED(ped) * 100) / 100;
			Speed << speed << " M/S";
			VehicleModel << "--------";
		}
		std::ostringstream WantedLevel; WantedLevel << "~b~Wanted Level:~s~ " << PLAYER::GET_PLAYER_WANTED_LEVEL(p);
		std::ostringstream Weapon; Weapon << "~b~Weapon: ~s~";
		Hash weaponHash;
#pragma region Weapon Check
		if (WEAPON::GET_CURRENT_PED_WEAPON(ped, &weaponHash, 1)) {
			char* weaponName;
			//weaponHash = GET_SELECTED_PED_WEAPON(ped);
			if (weaponHash == 2725352035) {//Unarmed
				weaponName = "Unarmed";
			}
			else if (weaponHash == 2578778090) {//Knife
				weaponName = "Knife";
			}
			else if (weaponHash == 0x678B81B1) {//Nightstick
				weaponName = "Nightstick";
			}
			else if (weaponHash == 0x4E875F73) {//Hammer
				weaponName = "Hammer";
			}
			else if (weaponHash == 0x958A4A8F) {//Bat
				weaponName = "Bat";
			}
			else if (weaponHash == 0x440E4788) {//GolfClub
				weaponName = "GolfClub";
			}
			else if (weaponHash == 0x84BD7BFD) {//Crowbar
				weaponName = "Crowbar";
			}
			else if (weaponHash == 0x1B06D571) {//Pistol
				weaponName = "Pistol";
			}
			else if (weaponHash == 0x5EF9FEC4) {//Combat Pistol
				weaponName = "Combat Pistol";
			}
			else if (weaponHash == 0x22D8FE39) {//AP Pistol
				weaponName = "AP Pistol";
			}
			else if (weaponHash == 0x99AEEB3B) {//Pistol 50
				weaponName = "Pistol 50";
			}
			else if (weaponHash == 0x13532244) {//Micro SMG
				weaponName = "Micro SMG";
			}
			else if (weaponHash == 0x2BE6766B) {//SMG
				weaponName = "SMG";
			}
			else if (weaponHash == 0xEFE7E2DF) {//Assault SMG
				weaponName = "Assault SMG";
			}
			else if (weaponHash == 0xBFEFFF6D) {//Assault Riffle
				weaponName = "Assault Riffle";
			}
			else if (weaponHash == 0x83BF0278) {//Carbine Riffle
				weaponName = "Carbine Riffle";
			}
			else if (weaponHash == 0xAF113F99) {//Advanced Riffle
				weaponName = "Advanced Riffle";
			}
			else if (weaponHash == 0x9D07F764) {//MG
				weaponName = "MG";
			}
			else if (weaponHash == 0x7FD62962) {//Combat MG
				weaponName = "Combat MG";
			}
			else if (weaponHash == 0x1D073A89) {//Pump Shotgun
				weaponName = "Pump Shotgun";
			}
			else if (weaponHash == 0x7846A318) {//Sawed-Off Shotgun
				weaponName = "Sawed-Off Shotgun";
			}
			else if (weaponHash == 0xE284C527) {//Assault Shotgun
				weaponName = "Assault Shotgun";
			}
			else if (weaponHash == 0x9D61E50F) {//Bullpup Shotgun
				weaponName = "Bullpup Shotgun";
			}
			else if (weaponHash == 0x3656C8C1) {//Stun Gun
				weaponName = "Stun Gun";
			}
			else if (weaponHash == 0x05FC3C11) {//Sniper Rifle
				weaponName = "Sniper Rifle";
			}
			else if (weaponHash == 0x0C472FE2) {//Heavy Sniper
				weaponName = "Heavy Sniper";
			}
			else if (weaponHash == 0xA284510B) {//Grenade Launcher
				weaponName = "Grenade Launcher";
			}
			else if (weaponHash == 0x4DD2DC56) {//Smoke Grenade Launcher
				weaponName = "Smoke Grenade Launcher";
			}
			else if (weaponHash == 0xB1CA77B1) {//RPG
				weaponName = "RPG";
			}
			else if (weaponHash == 0x42BF8A85) {//Minigun
				weaponName = "Minigun";
			}
			else if (weaponHash == 0x93E220BD) {//Grenade
				weaponName = "Grenade";
			}
			else if (weaponHash == 0x2C3731D9) {//Sticky Bomb
				weaponName = "Sticky Bomb";
			}
			else if (weaponHash == 0xFDBC8A50) {//Smoke Grenade
				weaponName = "Smoke Grenade";
			}
			else if (weaponHash == 0xA0973D5E) {//BZGas
				weaponName = "BZGas";
			}
			else if (weaponHash == 0x24B17070) {//Molotov
				weaponName = "Molotov";
			}
			else if (weaponHash == 0x060EC506) {//Fire Extinguisher
				weaponName = "Fire Extinguisher";
			}
			else if (weaponHash == 0x34A67B97) {//Petrol Can
				weaponName = "Petrol Can";
			}
			else if (weaponHash == 0xFDBADCED) {//Digital scanner
				weaponName = "Digital scanner";
			}
			else if (weaponHash == 0x88C78EB7) {//Briefcase
				weaponName = "Briefcase";
			}
			else if (weaponHash == 0x23C9F95C) {//Ball
				weaponName = "Ball";
			}
			else if (weaponHash == 0x497FACC3) {//Flare
				weaponName = "Flare";
			}
			else if (weaponHash == 0xF9E6AA4B) {//Bottle
				weaponName = "Bottle";
			}
			else if (weaponHash == 0x61012683) {//Gusenberg
				weaponName = "Gusenberg";
			}
			else if (weaponHash == 0xC0A3098D) {//Special Carabine
				weaponName = "Special Carabine";
			}
			else if (weaponHash == 0xD205520E) {//Heavy Pistol
				weaponName = "Heavy Pistol";
			}
			else if (weaponHash == 0xBFD21232) {//SNS Pistol
				weaponName = "SNS Pistol";
			}
			else if (weaponHash == 0x7F229F94) {//Bullpup Rifle
				weaponName = "Bullpup Rifle";
			}
			else if (weaponHash == 0x92A27487) {//Dagger
				weaponName = "Dagger";
			}
			else if (weaponHash == 0x083839C4) {//Vintage Pistol
				weaponName = "Vintage Pistol";
			}
			else if (weaponHash == 0x7F7497E5) {//Firework
				weaponName = "Firework";
			}
			else if (weaponHash == 0xA89CB99E) {//Musket
				weaponName = "Musket";
			}
			else if (weaponHash == 0x3AABBBAA) {//Heavy Shotgun
				weaponName = "Heavy Shotgun";
			}
			else if (weaponHash == 0xC734385A) {//Marksman Rifle
				weaponName = "Marksman Rifle";
			}
			else if (weaponHash == 0x63AB0442) {//Homing Launcher
				weaponName = "Homing Launcher";
			}
			else if (weaponHash == 0xAB564B93) {//Proxmine
				weaponName = "Proximity Mine";
			}
			else if (weaponHash == 0x787F0BB) {//Snowball
				weaponName = "Snowball";
			}
			else if (weaponHash == 0x47757124) {//Flare Gun
				weaponName = "Flare Gun";
			}
			else if (weaponHash == 0xE232C28C) {//Garbage Bag
				weaponName = "Garbage Bag";
			}
			else if (weaponHash == 0xD04C944D) {//Handcuffs
				weaponName = "Handcuffs";
			}
			else if (weaponHash == 0x0A3D4D34) {//Combat PDW
				weaponName = "Combat PDW";
			}
			else if (weaponHash == 0xDC4DB296) {//Marksman Pistol
				weaponName = "Marksman Pistol";
			}
			else if (weaponHash == 0xD8DF3C3C) {//Brass Knuckles
				weaponName = "Brass Knuckles";
			}
			else if (weaponHash == 0x6D544C99) {//Brass Knuckles
				weaponName = "Railgun";
			}
			else {
				weaponName = "Unarmed";
			}
			Weapon << weaponName;
		}
		else Weapon << "Unarmed";
#pragma endregion
		Vector3 myCoords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Vector3 coords = ENTITY::GET_ENTITY_COORDS(ped, 1);
		std::ostringstream Zone; Zone << "~b~Zone: ~s~" << UI::_GET_LABEL_TEXT(ZONE::GET_NAME_OF_ZONE(coords.x, coords.y, coords.z));
		Hash streetName, crossingRoad;
		PATHFIND::GET_STREET_NAME_AT_COORD(coords.x, coords.y, coords.z, &streetName, &crossingRoad);
		std::ostringstream Street; Street << "~b~Street: ~s~" << UI::GET_STREET_NAME_FROM_HASH_KEY(streetName);
		float distance = Get3DDistance(coords, myCoords);
		std::ostringstream Distance; Distance << "~b~Distance: ~s~";
		if (useMPH) {
			if (distance > 1609.344) {
				distance = round((distance / 1609.344) * 100) / 100;
				Distance << distance << " Miles";
			}
			else {
				distance = round((distance * 3.2808399) * 100) / 100;
				Distance << distance << " Feets";
			}
		}
		else {
			if (distance > 1000) {
				distance = round((distance / 1000) * 100) / 100;
				Distance << distance << " Kilometers";
			}
			else {
				distance = round(distance * 1000) / 100;
				Distance << distance << " Meters";
			}
		}

		AddTitle(playerName);
		AddSmallInfo((char*)Health.str().c_str(), 0);
		AddSmallInfo((char*)Armor.str().c_str(), 1);
		AddSmallInfo((char*)Alive.str().c_str(), 2);
		AddSmallInfo((char*)IsInAVehicle.str().c_str(), 3);
		AddSmallInfo((char*)VehicleModel.str().c_str(), 4);
		AddSmallInfo((char*)Speed.str().c_str(), 5);
		AddSmallInfo((char*)WantedLevel.str().c_str(), 6);
		AddSmallInfo((char*)Weapon.str().c_str(), 7);
		AddSmallInfo((char*)Zone.str().c_str(), 8);
		AddSmallInfo((char*)Street.str().c_str(), 9);
		AddSmallInfo((char*)Distance.str().c_str(), 10);

	}
	void set_fup()
	{
		
			Hash hash = GAMEPLAY::GET_HASH_KEY("GADGET_PARACHUTE");
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(PLAYER::PLAYER_PED_ID(), hash, 1, 1);
			ENTITY::SET_ENTITY_INVINCIBLE(PLAYER::PLAYER_PED_ID(), true);
			PED::SET_PED_TO_RAGDOLL_WITH_FALL(PLAYER::PLAYER_PED_ID(), 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);

			if (ENTITY::IS_ENTITY_IN_AIR(PLAYER::PLAYER_PED_ID()) && !PED::IS_PED_RAGDOLL(PLAYER::PLAYER_PED_ID()))
			{
				if (GetAsyncKeyState(0x57)) // W key
				{
					ApplyForceToEntity(PLAYER::PLAYER_PED_ID(), 0, 6, 0);
				}

				if (GetAsyncKeyState(0x53)) // S key
				{
					ApplyForceToEntity(PLAYER::PLAYER_PED_ID(), 0, -6, 0);
				}
				if (GetAsyncKeyState(VK_SHIFT)) //VK_SHIFT
				{
					ApplyForceToEntity(PLAYER::PLAYER_PED_ID(), 0, 0, 6);
				}
			}
		
		
	}
	void OnlinePlayer()
	{


		AddTitle("Online Players");
		for (int i = 0; i < 32; i++) {
			char* pName = PLAYER::GET_PLAYER_NAME(i);

			Entity ped = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
			//LoadPlayerInfo(pName, ped);
			AddPlayer(pName, ped);
			if (NETWORK::NETWORK_PLAYER_IS_ROCKSTAR_DEV(i)) {
				PrintStringBottomCentre("~r~WARNING:~s~ ROCKSTAR DEV DETECTED");
				pName = AddStrings("~r~[ROCKSTAR DEV]", pName);
			}
		}
	}
	void PlayerParticleFX(char* Name, Player p) {

		bool hasBlood = 0, hasElectric = 0, hasPinkBurst = 0, hasCoolLights = 0, hasMoneyRain = 0, hasBigSmoke = 0, hasCameraFlash = 0;
		if (MainPFX.p == p) {
			hasBlood = MainPFX.hasBlood;
			hasElectric = MainPFX.hasElectric;
			hasPinkBurst = MainPFX.hasPinkBurst;
			hasCoolLights = MainPFX.hasCoolLights;
			hasMoneyRain = MainPFX.hasMoneyRain;
			hasBigSmoke = MainPFX.hasBigSmoke;
			hasCameraFlash = MainPFX.hasCameraFlash;
		}
		AddTitle("Particle FX");
		AddToggle("Blood", hasBlood);
		AddToggle("Electric", hasElectric);
		AddToggle("Pink Burst", hasPinkBurst);
		AddToggle("Cool Lights", hasCoolLights);
		AddToggle("Money Rain", hasMoneyRain);
		AddToggle("Big Smoke", hasBigSmoke);
		AddToggle("Camera Flash", hasCameraFlash);
		//AddPTFX("Button PFX", "PTFXAsser Here", "Particle FX here", p, 0, 0, 0);
		if (MainPFX.p == p) {
			MainPFX.hasBlood = hasBlood;
			MainPFX.hasElectric = hasElectric;
			MainPFX.hasPinkBurst = hasPinkBurst;
			MainPFX.hasCoolLights = hasCoolLights;
			MainPFX.hasMoneyRain = hasMoneyRain;
			MainPFX.hasBigSmoke = hasBigSmoke;
			MainPFX.hasCameraFlash = hasCameraFlash;
		}
		else if (hasBlood || hasElectric || hasPinkBurst || hasCoolLights || hasMoneyRain || hasBigSmoke || hasCameraFlash) {
			MainPFX.p = p;
			MainPFX.hasBlood = hasBlood;
			MainPFX.hasElectric = hasElectric;
			MainPFX.hasPinkBurst = hasPinkBurst;
			MainPFX.hasCoolLights = hasCoolLights;
			MainPFX.hasMoneyRain = hasMoneyRain;
			MainPFX.hasBigSmoke = hasBigSmoke;
			MainPFX.hasCameraFlash = hasCameraFlash;
		}
	}
	void AddInfo(char* text, char* value) {//Put this where there is AddOption, AddNumber, AddToggle etc
		null = 0;
		AddOption(text, null);
		if (OptionY < 0.6325 && OptionY > 0.1425)
		{
			UI::SET_TEXT_FONT(0);
			UI::SET_TEXT_SCALE(0.26f, 0.26f);
			UI::SET_TEXT_CENTRE(1);

			drawstring(value, 0.233f + menuPos, OptionY);
		}
	}
	void AddNumberEasy(char* text, float value, __int8 decimal_places, float &val, float inc = 1.0f, bool fast = 0, bool &toggled = null, bool enableminmax = 0, float max = 0.0f, float min = 0.0f)
	{
		null = 0;
		AddOption(text, null);

		if (OptionY < 0.6325 && OptionY > 0.1425)
		{
			UI::SET_TEXT_FONT(0);
			UI::SET_TEXT_SCALE(0.26f, 0.26f);
			UI::SET_TEXT_CENTRE(1);

			drawfloat(value, (DWORD)decimal_places, 0.233f + menuPos, OptionY);
		}

		if (menu::printingop == menu::currentop)
		{
			if (IsOptionRJPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val + inc) > max)) {
						val += inc;
					}
				}
				else {
					val += inc;
				}
			}
			else if (IsOptionRPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val + inc) > max)) {
						val += inc;
					}
				}
				else {
					val += inc;
				}
			}
			else if (IsOptionLJPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val - inc) < min)) {
						val -= inc;
					}
				}
				else {
					val -= inc;
				}
			}
			else if (IsOptionLPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val - inc) < min)) {
						val -= inc;
					}
				}
				else {
					val -= inc;
				}
			}
		}
	}
	bool mph = 0;
	void AddSpeedType(char* text, bool &mphb = mph) {
		null = 0;
		AddOption(text, null);
		if (OptionY < 0.6325 && OptionY > 0.1425)
		{
			UI::SET_TEXT_FONT(0);
			UI::SET_TEXT_SCALE(0.26f, 0.26f);
			UI::SET_TEXT_CENTRE(1);
			if (mphb)
				drawstring("~g~mph", 0.233f + menuPos, OptionY);
			else
				drawstring("~g~km/h", 0.233f + menuPos, OptionY);
			if (menu::printingop == menu::currentop)
			{
				if (IsOptionRJPressed()) {//num6
					mphb = !mphb;
				}
				if (IsOptionLJPressed()) {//num4
					mphb = !mphb;
				}
			}
		}
	}
	float maxSpeed = 27.777777f;//meters per hour 100kmph
	bool mphlimit = 1;
	float smokeSize = 0.3f;
//	bool mph = 0;
	float maxSpeedConverted = 0.0f;
	void limitSpeed() {
		bool kmph = 0;
		Vehicle veh2 = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
		kmph = !mphlimit;
		if (mphlimit && !kmph) maxSpeedConverted = (maxSpeed * 3600 / 1000) * 0.6213711916666667f;
		if (kmph && !mphlimit) maxSpeedConverted = (maxSpeed * 3600 / 1000);
		AddTitle("Limit Speed");
		AddToggle("Enable", speedLimiter);
		AddInfo("TEST", "test");
		AddSpeedType("Speed Unit", mphlimit);
		AddNumberEasy("Max Speed", maxSpeedConverted, 0, maxSpeedConverted);
		if (mphlimit && !kmph) maxSpeed = (maxSpeedConverted / 3600 * 1000) / 0.6213711916666667f;
		if (kmph && !mphlimit) maxSpeed = (maxSpeedConverted / 3600 * 1000);
		if (speedLimiter) {
			ENTITY::SET_ENTITY_MAX_SPEED(veh2, maxSpeed);
		}
		else {
			ENTITY::SET_ENTITY_MAX_SPEED(veh2, 9999999);
		}
	}
	void LSCPaintSelector(bool primary = 0, bool secondary = 0) {

		if (primary) {
			AddTitle("Primary");
			AddOption("Classic", null, nullFunc, SUB::CLASSIC);
			AddOption("Metallic", null, nullFunc, SUB::METALLIC);
			AddOption("Matte", null, nullFunc, SUB::MATTE);
			AddOption("Metals", null, nullFunc, SUB::METALS);
			AddOption("Pearlescent", null, nullFunc, SUB::PEARLESCENT);
		}
		else if (secondary) {
			AddTitle("Secondary");
			AddOption("Classic", null, nullFunc, SUB::SCLASSIC);
			AddOption("Metallic", null, nullFunc, SUB::SMETALLIC);
			AddOption("Matte", null, nullFunc, SUB::SMATTE);
			AddOption("Metals", null, nullFunc, SUB::SMETALS);
			AddOption("Pearlescent", null, nullFunc, SUB::SPEARLESCENT);
		}
		else
		{
			AddTitle("LSC Paints");
			AddOption("Primary", null, nullFunc, SUB::PPAINT);
			AddOption("Secondary", null, nullFunc, SUB::SPAINT);
			//AddOption("Custom Primary", null, nullFunc, SUB::PRIMARYRGB);
			//AddOption("Custom Secondary", null, nullFunc, SUB::SECONDARYRGB);
		}

	}
	void PlayerMenu(char* name, Player p) {



		Ped mySelf = PLAYER::PLAYER_PED_ID();
		Player hisPed = p;
		/*	Vector3 pCoords = GET_ENTITY_COORDS(hisPed, 0);

		char *local = (char*)GET_PLAYER_NAME(p);
		if (Align(local, 2004288935))
		{
		PLAY_SOUND_FRONTEND(-1, "ScreenFlash", "WastedSounds", 1);
		PrintStringBottomCentre("~r~Anonymous USER - NOT TO FUCK WITH~s~");

		SET_ENTITY_COORDS(mySelf, pCoords.x, pCoords.y, pCoords.z, 1, 0, 0, 1);

		char *anim = "mp_bank_heist_1";
		char *animID = "prone_l_loop";

		REQUEST_ANIM_DICT(anim);
		while (!HAS_ANIM_DICT_LOADED(anim))
		WAIT(0);

		TASK_PLAY_ANIM(mySelf, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
		char *weather = "THUNDER";
		SET_OVERRIDE_WEATHER(weather);
		}

		else if (Align(local, -1103889998))
		{
		PLAY_SOUND_FRONTEND(-1, "ScreenFlash", "WastedSounds", 1);
		PrintStringBottomCentre("~r~Anonymous USER - NOT TO FUCK WITH~s~");

		SET_ENTITY_COORDS(mySelf, pCoords.x, pCoords.y, pCoords.z, 1, 0, 0, 1);

		char *anim = "mp_bank_heist_1";
		char *animID = "prone_l_loop";

		REQUEST_ANIM_DICT(anim);
		while (!HAS_ANIM_DICT_LOADED(anim))
		WAIT(0);

		TASK_PLAY_ANIM(mySelf, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
		char *weather = "THUNDER";
		SET_OVERRIDE_WEATHER(weather);
		}

		else{*/
		//void menu::submenu_switch()
		//{ // Make calls to submenus over here.

		//	switch (currentsub)
		//	{
		//	case SUB::MAINMENU:					sub::MainMenu(); break;
		//	case SUB::SETTINGS:					sub::Settings(); break;
		//	case SUB::SETTINGS_COLOURS:			sub::SettingsColours(); break;
		//	case SUB::SETTINGS_COLOURS2:		sub::SettingsColours2(); break;
		//	case SUB::SETTINGS_FONTS:			sub::SettingsFonts(); break;
		//	case SUB::SETTINGS_FONTS2:			sub::SettingsFonts2(); break;
		//	case SUB::SAMPLE:					sub::SampleSub(); break;
		//	case SUB::YOURSUB:					sub::YourSub(); break;
		//	case SUB::VEHICLE_SPAWNER:			sub::VehicleSpawner(); break;
		//	case SUB::SELFMODOPTIONS:			sub::SelfModsOptionsMenu(); break;
		//	case SUB::ONLINEPLAYERS:            sub::OnlinePlayer(); break;
		//	case SUB::FAVORITE_VEV:             favoriteClass(); break;
		//	case SUB::SUPERCAR_VEV:				supercarClass(); break;
		//	case SUB::LOWRIDER_VEV:				LowridersClass(); break;
		//	case SUB::MUSCLE_VEV:				muscleClass(); break;
		//	case SUB::OFF_ROADS_VEV:			offroadsClass(); break;
		//	case SUB::SPORTS_VEV:				sportsClass(); break;
		//	case SUB::SPORTS_CLASSICS_VEV:		sportsclassicsClass(); break;
		//	case SUB::PLANES_VEV:				planesClass(); break;
		//	case SUB::SUVS_VEV:					suvsClass(); break;
		//	case SUB::SEDANS_VEV:				sedansClass(); break;
		//	case SUB::COMPACT_VEV:				compactClass(); break;
		//	case SUB::COUPES_VEV:				coupesClass(); break;
		//	case SUB::EMERGENCY_VEV:			emergencyClass(); break;
		//	case SUB::MILITARY_VEV:				militaryClass(); break;
		//	case SUB::VANS_VEV:					vansClass(); break;
		//	case SUB::MOTORCYCLES_VEV:			motorcyclesClass(); break;
		//	case SUB::HELICOPTERS_VEV:			helicoptersClass(); break;
		//	case SUB::SERVICES_VEV:				servicesClass(); break;
		//	case SUB::INDUSTRIAL_VEV:			industrialClass(); break;
		//	case SUB::BOATS_VEV:				boatsClass(); break;
		//	case SUB::CYCLES_VEV:				cyclesClass(); break;
		//	case SUB::UTILITY_VEV:				utilityClass(); break;
		//	case SUB::GUNRUNNING:				gunclass(); break;
		//	case SUB::WEAPONSMENU:           sub::gWeaponMenu(); break;
		//	case SUB::SELECTEDPLAYER:		 sub::PlayerMenu(sub::selpName, sub::selPlayer); break;
		//	case SUB::OPLAVEHOPTIONS:		sub::oPlayVehicleOptionsMenu(sub::selpName, sub::selPlayer); break;
		//	}
		//}
		//the tname shows up but like tel2player and blame only just blames u and tels to ur self
		// blame uses add owned explosions which is patched ok can u fix tel2player and the drop or is that patched? the drop is ist else in the code ok so hed somewhere else 

		if (NETWORK::NETWORK_PLAYER_IS_ROCKSTAR_DEV(p)) { PrintStringBottomCentre("~r~[WARNING]PLAYER IS A ROCKSTAR DEVELOPPER."); return; }
		bool po = 0, tel2pla = 0, attacho = 0, wanted5 = 0, vehop = 0, kicknigga = 0, crashg = 0, bplayer = 0, giveCashSafe = 0, crashgNEW = 0, crashgRED1 = 0, ram = 0, sendText = 0;

		AddTitle(name);
		AddOption("~g~Teleport to Player", tel2pla);
		AddOption("Player Options", null, nullFunc, SUB::OPLAYEROPTIONS);
		AddOption("Attachment Options", null, nullFunc, SUB::OATTACHMENTOPTIONS);
		AddOption("Vehicle Options", null, nullFunc, SUB::OPLAVEHOPTIONS);
		AddToggle("Spectate Player", espectateP);
		AddToggle("~o~$ Drop Money Birds Safe*$", loop_safemoneydrop, giveCashSafe);
		AddToggle("~g~$ Drop Money Bags Safe*$", loop_safemoneydropv2, giveCashSafe);
		AddOption("~y~Blame Player Selective", null, nullFunc, SUB::BLAMESELP);
		AddOption("~r~Blame Player Whole Lobby", bplayer);
		AddOption("~o~CRASH PLAYER ELITE", crashgRED1);
		AddOption("~y~CRASH PLAYER IN MOTION", crashgNEW);
		AddToggle("~r~RADIATION CRASH", laghimout);
		AddOption("~y~Kick Player", kicknigga);
		AddOption("Particle FX", null, nullFunc, SUB::SELPLAYERPTFX);
		AddOption("~r~Fun Drops", null, nullFunc, SUB::OPLAYERDROPOPTIONS);
		AddOption("~g~Send Attackers", null, nullFunc, SUB::OPLAYERATTACKERS);
		//		AddOption("~m~Crash Player's Game", crashg);
		AddOption("~g~Send Text Message", sendText);
		//		AddOption("Set Wanted Level(5)", wanted5);
		//		AddToggle("Set Wanted Level", setWlevelf);
		//AddOption("");
		Ped playerPed = p;
		Player player = p;
		LoadPlayerInfo(name, p);
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(playerPed, 0); // there ya go ok wanna give me auth server info so i can test I don't think red has bought an authh server yet, so you'd have to speak to him about it
		BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
		Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
		Ped myPed = PLAYER::PLAYER_PED_ID();
		if (sendText) {
			NETWORK::NETWORK_SEND_TEXT_MESSAGE(keyboard(), &player);
		}

		if (tel2pla)
		{
			Entity handle;
			Vector3 coords = ENTITY::GET_ENTITY_COORDS(PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(player), false);
			PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), false) ? handle = PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID()) : handle = PLAYER::PLAYER_PED_ID();
			ENTITY::SET_ENTITY_COORDS(handle, coords.x, coords.y, coords.z, false, false, false, false);
		}


		if (bplayer)
		{
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
				Vector3 pos = ENTITY::GET_ENTITY_COORDS(Handle, 1);
				if (!ENTITY::DOES_ENTITY_EXIST(Handle)) continue;
				FIRE::ADD_OWNED_EXPLOSION(selPlayer, pos.x, pos.y, pos.z, 29, 0.5f, true, false, 5.0f);
				{
					if (i == 32)

					{
						break;
					}
					PrintStringBottomCentre("Report that N1ggu!");

				}
			}
		}
		if (wanted5)
		{
			PLAYER::SET_PLAYER_WANTED_LEVEL(p, 5, 0);
			PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(p, 5);
		}

		if (kicknigga)
		{
			NETWORK::NETWORK_SESSION_KICK_PLAYER(p);
		}

		if (setWlevelf) {
			GAMEPLAY::SET_FAKE_WANTED_LEVEL(6);
		}
		else if (!setWlevelf) {
			GAMEPLAY::SET_FAKE_WANTED_LEVEL(0);

		}

		if (espectateP) {
			NETWORK::NETWORK_SET_IN_SPECTATOR_MODE(1, playerPed);
		}
		else if (!espectateP) {
			NETWORK::NETWORK_SET_IN_SPECTATOR_MODE(0, playerPed);
		}
		if (crashgRED1)
		{
			for (int i = 0; i <= 500; i++)
			{
				WAIT(20);
				try {
					PED::CLONE_PED(playerPed, 1, 1, 1);
				}
				catch (...) {

				}
				{
					if (i == 490)

					{
						break;
					}
					PrintStringBottomCentre("~o~Target Acquired...");

				}
			}
		}

		if (crashgNEW)
		{
			for (int i = 0; i <= 500; i++)
			{
				WAIT(20);
				try {
					int playerClone = PED::CLONE_PED(playerPed, 1, 1, 1);
					ENTITY::ATTACH_ENTITY_TO_ENTITY(playerClone, playerPed, -1.0f, 0.0f, -0.25f, 0.0f, 0.0f, 0.0f, 1, 1, 0, 0, 2, 1, 1);
				}
				catch (...) {}
				if (i == 490)

				{
					break;
				}
				PrintStringBottomCentre("~o~Target Acquired...");

			}
		}
		/*if (laghimout)
		{
		Vehicle zentorno = SpawnVehicle("CARGOPLANE", pos);

		ENTITY::SET_ENTITY_INVINCIBLE(zentorno, 0);
		ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
		ENTITY::SET_ENTITY_ALPHA(zentorno, 0.0f, 1);
		PrintStringBottomCentre("~o~Radiating Player...");

		}*/
		//	}

	}
	int FreeSeat(int veh)
	{
		int maxSeats = VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(veh);
		for (int i = -1; i < maxSeats; i++)
		{
			if (VEHICLE::IS_VEHICLE_SEAT_FREE(veh, i))
			{
				return i;
			}
		} return -2;
	}
	void openVehDoors(Vehicle veh) {
		for (int i = 0; i < 10; i++)
			VEHICLE::SET_VEHICLE_DOOR_OPEN(veh, i, 0, 1);
	}
	void closeVehDoors(Vehicle veh) {
		for (int i = 0; i < 10; i++)
			VEHICLE::SET_VEHICLE_DOOR_SHUT(veh, i, 0);
	}
	void lockVehDoors(Vehicle veh) {
		for (int i = 0; i < 10; i++)
			VEHICLE::SET_VEHICLE_DOORS_LOCKED(veh, i);
	}

	void teleport(Entity e, Vector3 coords) {
		if (PED::IS_PED_IN_ANY_VEHICLE(e, 0))
			e = PED::GET_VEHICLE_PED_IS_USING(e);
		bool groundFound = false;
		static float groundCheckHeight[] = {
			100.0, 150.0, 50.0, 0.0, 200.0, 250.0, 300.0, 350.0, 400.0,
			450.0, 500.0, 550.0, 600.0, 650.0, 700.0, 750.0, 800.0
		};
		for (int i = 0; i < sizeof(groundCheckHeight) / sizeof(float); i++)
		{
			ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, coords.x, coords.y, groundCheckHeight[i], 0, 0, 1);
			WAIT(100);
			if (GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(coords.x, coords.y, groundCheckHeight[i], &coords.z, 0))
			{
				groundFound = true;
				coords.z += 3.0;
				break;
			}
		}
	}
	void RemoveOwnership()
	{
		if (DECORATOR::DECOR_EXIST_ON(PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer), false), "Player_Vehicle"))
		{
			DECORATOR::DECOR_REMOVE(PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer), false), "Player_Vehicle");
			PrintStringBottomCentre("~g~Removed Players Ownership Of Vehicle");
		}
		else
		{
			PrintStringBottomCentre("~r~Player Does Not Own This Vehicle");
		}

	}
	void tpInVehicle(Vehicle veh, Player p, bool hijack = false) {
		Ped playerPed = p;
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
		teleport(PLAYER::PLAYER_PED_ID(), pCoords);
		ENTITY::SET_ENTITY_HEADING(veh, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));
		if (hijack) {
			PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, 0);
		}
		else {
			PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);
		}
	}
	void HijackVehicle(Vehicle veh, Player p)
	{
		int clientHandle = p;

		AI::CLEAR_PED_TASKS_IMMEDIATELY(clientHandle);
		PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);
		PrintStringBottomCentre("~b~Vehicle Hijacked!");

	}
	void oAttachmentOptionsMenu(char* name, Player p) {

		bool a2pla = 0, dfrompla = 0, aSelf2Veh = 0, detSelfromVeh = 0, TonCage = 0, AttBfire = 0, pback = 0, pbackoff = 0, aRandCar = 0, aOraBall = 0, aCtree = 0, aArcade = 0, aBarbel = 0, aDplane = 0, atoilet = 0, AUFO = 0, aVdoor = 0, aSafe = 0, aCanoe = 0, aEMP = 0, aBFtoVeh = 0, aUFOtoVeh = 0;
		AddTitle(name);
		AddOption("Attach to Player", a2pla);
		AddOption("Detach from Player", dfrompla);
		AddOption("Attach Self to Vehicle", aSelf2Veh);
		AddOption("Detach from Vehicle", detSelfromVeh);
		AddOption("Trap on Cage", TonCage);
		AddOption("Attach Beach fire", AttBfire);
		AddOption("Piggyback ON", pback);
		AddOption("Piggyback OFF", pbackoff);
		AddOption("Attach Random Car", aRandCar);

		AddOption("Attach Christmas Tree", aCtree);
		AddOption("Attach Arcade", aArcade);
		AddOption("Attach Barbell 100kg", aBarbel);
		AddOption("Attach Dummy Plane", aDplane);
		AddOption("Attach Toilet", atoilet);
		AddOption("Attach Vault Door", aVdoor);

		AddOption("Attach Orange Ball", aOraBall);
		AddOption("Attach UFO", AUFO);
		AddOption("Attach Safe", aSafe);
		AddOption("Attach Canoe", aCanoe);
		AddOption("Attach EMP", aEMP);
		AddOption("Attach Beach fire to Vehicle", aBFtoVeh);
		AddOption("Attach UFO to Vehicle", aUFOtoVeh);
		//AddOption("");
		Ped myPed = PLAYER::PLAYER_PED_ID();
		Player playerPed = p;
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
		Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
		if (a2pla)
		{
			ENTITY::ATTACH_ENTITY_TO_ENTITY(myPed, playerPed, 0.059999998658895f, 0.0f, -0.25f, 0.0f, 0.0f, 0.0f, 1, 1, 0, 0, 2, 1, 1);
			//		ATTACH_ENTITY_TO_ENTITY(myPed, playerPed, -1, 0.0f, 0.35f, 0.72f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 2, 1, 1);
		}

		if (dfrompla)
		{
			ENTITY::DETACH_ENTITY(myPed, 1, 1);
		}

		if (aSelf2Veh)
		{
			ENTITY::ATTACH_ENTITY_TO_ENTITY(myPed, veh, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		}
		if (detSelfromVeh)
		{
			ENTITY::DETACH_ENTITY(myPed, 1, 1);
		}
		//if (TonCage)
		//{
		//	AI::CLEAR_PED_TASKS_IMMEDIATELY(playerPed);
		//	NETWORK::OBJ_TO_NET(OBJECT::CREATE_OBJECT_NO_OFFSET(GAMEPLAY::GET_HASH_KEY("prop_gold_cont_01"), pCoords.x, pCoords.y, pCoords.z, 1, 0, 0));
		//}
		//if (AttBfire)
		//{
		//	Hash beachfire = GAMEPLAY::GET_HASH_KEY("prop_beach_fire");
		//	STREAMING::REQUEST_MODEL(beachfire);
		//	while (!STREAMING::HAS_MODEL_LOADED(beachfire))
		//		WAIT(0);
		//	int attachfire = OBJECT::CREATE_OBJECT(beachfire, pCoords.x, pCoords.y, pCoords.z, true, 1, 0);

		//	ENTITY::ATTACH_ENTITY_TO_ENTITY(attachfire, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		//}
		if (pback)
		{
			char *anim = "mini@prostitutes@sexnorm_veh";
			char *animID = "bj_loop_male";

			ENTITY::ATTACH_ENTITY_TO_ENTITY(myPed, playerPed, -1.0f, 0.0f, -0.3f, 0.0f, 0.0f, 0.0f, 1, 1, 0, 0, 2, 1, 1);
			STREAMING::REQUEST_ANIM_DICT(anim);
			while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
				WAIT(200);
			AI::TASK_PLAY_ANIM(myPed, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);

			PrintStringBottomCentre("Piggyback ON!");
		}
		if (pbackoff)
		{
			AI::CLEAR_PED_TASKS_IMMEDIATELY(myPed);
			ENTITY::DETACH_ENTITY(myPed, 1, 1);
			PrintStringBottomCentre("Piggyback OFF!");
		}
		if (aRandCar)
		{
			int vehID = VEHICLE::GET_CLOSEST_VEHICLE(pCoords.x, pCoords.y, pCoords.z, 600.0f, 0, 0);
			ENTITY::ATTACH_ENTITY_TO_ENTITY(vehID, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		}
		//if (aOraBall)
		//{
		//	Hash oball = GAMEPLAY::GET_HASH_KEY("prop_juicestand");
		//	STREAMING::REQUEST_MODEL(oball);
		//	while (!STREAMING::HAS_MODEL_LOADED(oball))
		//		WAIT(0);
		//	int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		//	ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		//}
		////
		//if (aVdoor)
		//{
		//	Hash oball = GAMEPLAY::GET_HASH_KEY("prop_bank_vaultdoor");
		//	STREAMING::REQUEST_MODEL(oball);
		//	while (!STREAMING::HAS_MODEL_LOADED(oball))
		//		WAIT(0);
		//	int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		//	ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		//}
		//if (atoilet)
		//{
		//	Hash oball = GAMEPLAY::GET_HASH_KEY("prop_ld_toilet_01");
		//	STREAMING::REQUEST_MODEL(oball);
		//	while (!STREAMING::HAS_MODEL_LOADED(oball))
		//		WAIT(0);
		//	int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		//	ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		//}
		//if (aDplane)
		//{
		//	Hash oball = GAMEPLAY::GET_HASH_KEY("prop_dummy_plane");
		//	STREAMING::REQUEST_MODEL(oball);
		//	while (!STREAMING::HAS_MODEL_LOADED(oball))
		//		WAIT(0);
		//	int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		//	ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		//}
		//if (aBarbel)
		//{
		//	Hash oball = GAMEPLAY::GET_HASH_KEY("prop_barbell_100kg");
		//	STREAMING::REQUEST_MODEL(oball);
		//	while (!STREAMING::HAS_MODEL_LOADED(oball))
		//		WAIT(0);
		//	int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		//	ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		//}
		//if (aArcade)
		//{
		//	Hash oball = GAMEPLAY::GET_HASH_KEY("prop_arcade_01");
		//	STREAMING::REQUEST_MODEL(oball);
		//	while (!STREAMING::HAS_MODEL_LOADED(oball))
		//		WAIT(0);
		//	int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		//	ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		//}
		//if (aCtree)
		//{
		//	Hash oball = GAMEPLAY::GET_HASH_KEY("prop_xmas_tree_int");
		//	STREAMING::REQUEST_MODEL(oball);
		//	while (!STREAMING::HAS_MODEL_LOADED(oball))
		//		WAIT(0);
		//	int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		//	ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		//}

		////
		//if (AUFO)
		//{
		//	Hash ufo = GAMEPLAY::GET_HASH_KEY("p_spinning_anus_s");
		//	STREAMING::REQUEST_MODEL(ufo);
		//	while (!STREAMING::HAS_MODEL_LOADED(ufo))
		//		WAIT(0);
		//	int ufomodel = OBJECT::CREATE_OBJECT(ufo, 0, 0, 0, true, 1, 0);
		//	ENTITY::ATTACH_ENTITY_TO_ENTITY(ufomodel, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		//}
		//if (aSafe)
		//{
		//	Hash safe = GAMEPLAY::GET_HASH_KEY("p_v_43_safe_s");
		//	STREAMING::REQUEST_MODEL(safe);
		//	while (!STREAMING::HAS_MODEL_LOADED(safe))
		//		WAIT(0);
		//	int safemodel = OBJECT::CREATE_OBJECT(safe, 0, 0, 0, true, 1, 0);
		//	ENTITY::ATTACH_ENTITY_TO_ENTITY(safemodel, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		//}
		/*if (aCanoe)
		{
		Hash canoe = GAMEPLAY::GET_HASH_KEY("prop_can_canoe");
		STREAMING::REQUEST_MODEL(canoe);
		while (!STREAMING::HAS_MODEL_LOADED(canoe))
		WAIT(0);
		int canoemodel = OBJECT::CREATE_OBJECT(canoe, 0, 0, 0, true, 1, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(canoemodel, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		}
		if (aEMP)
		{
		Hash emp = GAMEPLAY::GET_HASH_KEY("hei_prop_heist_emp");
		STREAMING::REQUEST_MODEL(emp);
		while (!STREAMING::HAS_MODEL_LOADED(emp))
		WAIT(0);
		int empmodel = OBJECT::CREATE_OBJECT(emp, 0, 0, 0, true, 1, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(empmodel, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		}
		if (aBFtoVeh)
		{
		Hash beachfire = GAMEPLAY::GET_HASH_KEY("prop_beach_fire");
		STREAMING::REQUEST_MODEL(beachfire);
		while (!STREAMING::HAS_MODEL_LOADED(beachfire))
		WAIT(0);
		int attachfire = OBJECT::CREATE_OBJECT(beachfire, 0, 0, 0, true, 1, 0);

		ENTITY::ATTACH_ENTITY_TO_ENTITY(attachfire, veh, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		}
		if (aUFOtoVeh)
		{
		Hash beachfire = GAMEPLAY::GET_HASH_KEY("p_spinning_anus_s");
		STREAMING::REQUEST_MODEL(beachfire);
		while (!STREAMING::HAS_MODEL_LOADED(beachfire))
		WAIT(0);
		int attachfire = OBJECT::CREATE_OBJECT(beachfire, 0, 0, 0, true, 1, 0);

		ENTITY::ATTACH_ENTITY_TO_ENTITY(attachfire, veh, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		}*/
	}
	void VehicleLSCSubMenu() {

		bool all = 0, customplate = 0;
		AddTitle("Mobile LSC");
		AddOption("Paint", null, nullFunc, SUB::VEHICLE_PAINT);
		AddOption("All Mods", all);
		AddOption("Customize Plate", customplate);
		BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(PLAYER::PLAYER_PED_ID());
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
		Entity e = PLAYER::PLAYER_PED_ID();
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(e, 0);
		if (customplate) VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(veh, keyboard());
		if (all) {
			VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
			//chrome SET_VEHICLE_COLOURS(veh, 120, 120);
			VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(veh, "Anonymous");
			VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
			VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(veh, 1);
			VEHICLE::TOGGLE_VEHICLE_MOD(veh, 18, 1);
			VEHICLE::TOGGLE_VEHICLE_MOD(veh, 22, 1);
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 0);
			VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 0, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 0) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 1, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 1) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 2, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 2) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 3, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 3) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 4, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 4) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 5, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 5) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 6, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 6) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 7, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 7) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 8, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 8) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 9, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 9) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 10, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 10) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 11, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 11) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 12, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 12) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 13, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 13) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 14, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 14) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 15, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 15) - 1, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 16, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 16) - 1, 0);
			VEHICLE::SET_VEHICLE_WHEEL_TYPE(veh, 6);
			VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 5);
			VEHICLE::SET_VEHICLE_MOD(veh, 23, 19, 1);

		}

	}
	void oPlayVehicleOptionsMenu(char* name, Player p) {



		bool telInVeh = 0, pop = 0, hijackVeh = 0, kickfveh = 0, dirtyVeh = 0, forcePUSH = 0, expVeh = 0, CloseAdoors = 0, OpenAdoors = 0, RepVeh = 0,
			telVeh2u = 0, telToCoast = 0, telToMaze = 0, telToClouds = 0, telToOcean = 0, laVeh = 0, delVeh = 0, gModeVeh = 0, DupeVeh = 0, attachToMine = 0,
			killEngine = 0, reviveEngine = 0, freeze = 0, unfreeze = 0, flipVeh = 0;
		//AddTitle(name);
		Ped myPed = PLAYER::PLAYER_PED_ID();
		Player playerPed = p;
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
		Player myPlayer = PLAYER::PLAYER_ID();
		Vector3 pCoordsMine = ENTITY::GET_ENTITY_COORDS(myPed, 0);
		Vehicle myveh = PED::GET_VEHICLE_PED_IS_IN(myPed, 0);
		Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
		bool engineStatus = false;
		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 1) && VEHICLE::GET_VEHICLE_ENGINE_HEALTH(veh) < -3000) engineStatus = true;

		AddTitle("Player Vehicle");
		//AddOption("LSC Modshop", null, nullFunc, SUB::VEHICLE_LSC2_SELP);
		AddOption("Remove Ownership", null, RemoveOwnership);
		AddOption("Teleport Inside Vehicle", telInVeh);
		AddOption("Kick from Vehicle", kickfveh);
		AddOption("Flip", flipVeh);
		AddOption("Launch Vehicle", laVeh);
		AddOption("Force Push", forcePUSH);
		AddOption("Pop tires", pop);
		AddOption("~r~Explode Vehicle", expVeh);
		AddOption("Close All Doors", CloseAdoors);
		AddOption("Open All Doors", OpenAdoors);
		AddOption("Wash & Repair Vehicle", RepVeh);
		AddOption("Dirty Vehicle", dirtyVeh);
		//	AddToggle("Kill Engine", engineStatus, killEngine, reviveEngine);
		AddOption("Freeze Vehicle", freeze);
		AddOption("Unfreeze Vehicle", unfreeze);
		AddOption("Teleport Vehicle to you", telVeh2u);
		AddOption("Teleport Vehicle to Coast Cave", telToCoast);
		AddOption("Teleport Vehicle to Maze Bank Top", telToMaze);
		AddOption("Teleport Vehicle Above the Clouds", telToClouds);
		AddOption("Teleport Vehicle to Ocean", telToOcean);
		AddOption("Delete Vehicle", delVeh);
		AddOption("GodMode Vehicle", gModeVeh);
		AddOption("Duplicate Insured Vehicle", DupeVeh);
		AddOption("Attach Player's Vehicle to Mine", attachToMine);
		//AddOption("");
		if (flipVeh) {
			RequestControlOfEnt(veh);
			Vector3 rotation = ENTITY::GET_ENTITY_ROTATION(veh, 0);
			ENTITY::SET_ENTITY_ROTATION(veh, 180, rotation.y, rotation.z, 0, 1);
		}
		/*if (killEngine) {
		RequestControlOfEnt(veh);
		ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, 1, 1);
		VEHICLE::SET_VEHICLE_ENGINE_HEALTH(veh, -3700);


		}
		if (reviveEngine) {
		RequestControlOfEnt(veh);
		VEHICLE::SET_VEHICLE_ENGINE_HEALTH(veh, 1000);
		}*/
		if (freeze) {
			RequestControlOfEnt(veh);
			ENTITY::FREEZE_ENTITY_POSITION(veh, 1);
		}
		if (unfreeze) {
			RequestControlOfEnt(veh);
			ENTITY::FREEZE_ENTITY_POSITION(veh, 0);
		}
		if (!PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
			PrintStringBottomCentre("Selected Player is not in a Vehicle");
		}
		else
		{
			if (pop) {
				if (ENTITY::DOES_ENTITY_EXIST(playerPed) && ENTITY::DOES_ENTITY_EXIST(veh)) {
					RequestControlOfEnt(veh);
					VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, true);
					for (int i = 0; i < 32; i++)
						VEHICLE::SET_VEHICLE_TYRE_BURST(veh, i, true, 10);
				}
			}
			if (forcePUSH)
			{
				RequestControlOfEnt(veh);
				ENTITY::APPLY_FORCE_TO_ENTITY(veh, 0, 99999.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 1, 1, 1, 0, 1);
				PrintStringBottomCentre("SUPER Anonymous!");
			}
			if (telInVeh)
			{
				RequestControlOfEnt(veh);
				WAIT(0);
				PED::SET_PED_INTO_VEHICLE(myPed, veh, FreeSeat(veh));
			}
			if (hijackVeh)
			{
				HijackVehicle(veh, playerPed);
			}
			if (kickfveh)
			{
				AI::CLEAR_PED_TASKS_IMMEDIATELY(playerPed);
				PrintStringBottomCentre("LOL!");
			}
			if (expVeh)
			{
				FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z, 29, 0.5f, true, false, 5.0f);
				PrintStringBottomCentre("BOOM!");
			}
			if (CloseAdoors)
			{
				RequestControlOfEnt(veh);
				closeVehDoors(veh);
				PrintStringBottomCentre("Press the button a few times!");
			}
			if (OpenAdoors)
			{
				RequestControlOfEnt(veh);
				openVehDoors(veh);
				PrintStringBottomCentre("Press the button a few times!");
			}
			if (RepVeh)
			{
				Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
				RequestControlOfEnt(veh);
				VEHICLE::SET_VEHICLE_FIXED(veh);
				VEHICLE::SET_VEHICLE_DIRT_LEVEL(veh, 0.0f);
				PrintStringBottomCentre("Vehicle Washed & Repaired!");


			}
			if (dirtyVeh)
			{
				Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
				RequestControlOfEnt(veh);
				VEHICLE::SET_VEHICLE_FIXED(veh);
				VEHICLE::SET_VEHICLE_DIRT_LEVEL(veh, 15.0f);
				PrintStringBottomCentre("Dirty Vehicle");


			}
			if (telVeh2u)
			{
				RequestControlOfEnt(veh);
				ENTITY::SET_ENTITY_COORDS(veh, pCoordsMine.x, pCoordsMine.y, pCoordsMine.z, 1, 0, 0, 1);
				PrintStringBottomCentre("Press the button a few times!");
			}
			if (telToCoast)
			{
				RequestControlOfEnt(veh);
				ENTITY::SET_ENTITY_COORDS(veh, 3062.855f, 2214.975f, 3.381231f, 1, 0, 0, 1);
				PrintStringBottomCentre("Press the button a few times!");
			}
			if (telToMaze)
			{
				RequestControlOfEnt(veh);
				ENTITY::SET_ENTITY_COORDS(veh, -74.94243f, -818.63446f, 326.174347f, 1, 0, 0, 1);
				PrintStringBottomCentre("Press the button a few times!");
			}
			if (telToClouds)
			{
				Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
				if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
					RequestControlOfEnt(veh);
				ENTITY::SET_ENTITY_COORDS(veh, -73.4489f, -833.5170f, 5841.4240f, 1, 0, 0, 1);
				PrintStringBottomCentre("Press the button a few times!");
			}
			if (telToOcean)
			{
				Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
				if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
					RequestControlOfEnt(veh);
				ENTITY::SET_ENTITY_COORDS(veh, 5314.23f, -5211.83f, 83.52f, 1, 0, 0, 1);
				PrintStringBottomCentre("Press the button a few times!");
			}
			if (laVeh)
			{
				RequestControlOfEnt(veh);
				ENTITY::APPLY_FORCE_TO_ENTITY(veh, 0, 0.0f, 0.0f, 99999999.0f, 0.0f, 0.0f, 0.0f, 0, 1, 1, 1, 0, 1);
				PrintStringBottomCentre("Press the button a few times!");
			}
			if (delVeh)
			{
				RequestControlOfEnt(veh);
				ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, 1, 1);
				ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&veh);
				VEHICLE::DELETE_VEHICLE(&veh);
				PrintStringBottomCentre("Press the button a few times!");
				WAIT(0);

			}

			if (gModeVeh)
			{
				RequestControlOfEnt(veh);
				ENTITY::SET_ENTITY_INVINCIBLE(veh, TRUE);
				ENTITY::SET_ENTITY_PROOFS(veh, 1, 1, 1, 1, 1, 1, 1, 1);
				VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 0);
				VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 0);
				VEHICLE::SET_VEHICLE_CAN_BE_VISIBLY_DAMAGED(veh, 0);
				PrintStringBottomCentre("player's vehicle is now in godmode");
				WAIT(0);

			}
			if (DupeVeh)
			{
				RequestControlOfEnt(veh);
				ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, 1, 1);
				ENTITY::SET_ENTITY_PROOFS(veh, 1, 1, 1, 1, 1, 1, 1, 1);
				VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 0);
				VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 0);
				VEHICLE::SET_VEHICLE_CAN_BE_VISIBLY_DAMAGED(veh, 0);
				PrintStringBottomCentre("vehicle is no longer an insured original");
				WAIT(0);
			}
			if (attachToMine)
			{
				RequestControlOfEnt(veh);
				ENTITY::ATTACH_ENTITY_TO_ENTITY(veh, myveh, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
				PrintStringBottomCentre("Press the button a few times!");
			}
		}
	}
	void oAllPlayerOptionsMenu()
	{


		// Initialise local variables here:
		bool killtall = 0, remAtweaps = 0, glitchlobby = 0, shakewholelobby = 0, givemonsterAll = 0, giveTankAll = 0, givedukeAll = 0, giveSpaceDockerAll = 0, givAweap = 0, reduceframeall = 0, beachFireALL = 0, TonCage = 0, ufoALL = 0, aCtree = 0, giveALLweap = 0, aArcade = 0, aBarbel = 0, aDplane = 0, atoilet = 0, aVdoor = 0, makeALLSemiGod = 0, Gttaser = 0, gtsballs = 0, kefromveh = 0;

		// Options' text here:
		AddTitle("Anonymous");
		AddTitle("~m~All Player Options");
		AddOption("Shake Whole Lobby", shakewholelobby);
		AddOption("Kick Everybody from Vehicle", kefromveh);
		AddOption("~r~Kill Them All", killtall);
		AddOption("~r~Attach BeachFire to All Players", beachFireALL);
		AddOption("Trap them all in Cages", TonCage);
		AddOption("Give them All Weapons", giveALLweap);
		AddOption("Remove All Their Weapons", remAtweaps);
		/*AddOption("Give Everybody a Monster Truck", givemonsterAll);
		AddOption("Give Everybody a Tank", giveTankAll);
		AddOption("Give Everybody a Spacedocker", giveSpaceDockerAll);
		AddOption("Give Everybody a Duke O' Death", givedukeAll);*/

		//AddOption("Slow Things Down - CargoPlane", reduceframeall);
		AddOption("Alien Invasion", glitchlobby);
		AddOption("Attach Toilet to All Players", atoilet);
		AddOption("Give them Railguns", givAweap);
		AddOption("Give them Tasers", Gttaser);

		AddOption("Give them SnowBalls", gtsballs);

		//		AddOption("All Players SemiGod & NoCops", makeALLSemiGod);
		AddOption("Attach UFO to All Players", ufoALL);


		AddOption("Attach Christmas Tree to All Players", aCtree);
		AddOption("Attach Arcade to All Players", aArcade);
		AddOption("Attach Barbel to All Players", aBarbel);
		AddOption("Attach Dummy Plane to All Players", aDplane);
		AddOption("Attach Vault Door to All Players", aVdoor);
		//		AddToggle("Crash Lobby - Experimental", crashLobby);

		// Options' code here:


		if (shakewholelobby)
		{
			Ped playerPedID = PLAYER::PLAYER_PED_ID();
			Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPedID, 0);
			//
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
				Vector3 pos = ENTITY::GET_ENTITY_COORDS(Handle, 1);
				if (!ENTITY::DOES_ENTITY_EXIST(Handle)) continue;
				FIRE::ADD_EXPLOSION(pos.x, pos.y, pos.z + 15, 29, 999999.5f, false, true, 1.0f);
				{
					if (i == 32)

					{
						break;
					}

				}
			}
			//
		}

		/*if (givemonsterAll)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);

		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(Handle, 0);
		Vector3 Ocoords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(Handle, 0.0, 5.0, 0.0);
		Vehicle zentorno = SpawnVehicle("MOSTER", Ocoords);
		ENTITY::SET_ENTITY_INVINCIBLE(zentorno, 0);
		VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(zentorno, "Anon");
		{
		if (i == 32)
		{
		break;
		}

		}
		}
		}
		if (givedukeAll)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(Handle, 0);
		Vector3 Ocoords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(Handle, 0.0, 5.0, 0.0);

		Vehicle zentorno = SpawnVehicle("DUKES2", Ocoords);
		ENTITY::SET_ENTITY_INVINCIBLE(zentorno, 0);
		VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(zentorno, "Anon");
		{
		if (i == 32)
		{
		break;
		}

		}
		}
		}
		if (giveSpaceDockerAll)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(Handle, 0);
		Vector3 Ocoords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(Handle, 0.0, 5.0, 0.0);

		Vehicle zentorno = SpawnVehicle("DUNE2", Ocoords);
		ENTITY::SET_ENTITY_INVINCIBLE(zentorno, 0);
		VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(zentorno, "Anon");
		{
		if (i == 32)
		{
		break;
		}

		}
		}
		}
		if (giveTankAll)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(Handle, 0);
		Vector3 Ocoords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(Handle, 0.0, 5.0, 0.0);
		Vehicle zentorno = SpawnVehicle("RHINO", Ocoords);
		ENTITY::SET_ENTITY_INVINCIBLE(zentorno, 0);;
		{
		if (i == 32)
		{
		break;
		}

		}
		}
		}*/
		if (glitchlobby)
		{
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
				AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
				Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(Handle, 0);
				Hash oball = GAMEPLAY::GET_HASH_KEY("p_spinning_anus_s");
				STREAMING::REQUEST_MODEL(oball);
				while (!STREAMING::HAS_MODEL_LOADED(oball))
					WAIT(0);
				//	int orangeball = CREATE_OBJECT(oball, pCoords.x, pCoords.y, pCoords.z, true, 1, 0);
				//	ATTACH_ENTITY_TO_ENTITY(orangeball, Handle, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
				AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
				int createdObject = NETWORK::OBJ_TO_NET(OBJECT::CREATE_OBJECT_NO_OFFSET(oball, pCoords.x, pCoords.y, pCoords.z, 1, 0, 0));

				{
					if (i == 32)
					{
						break;
					}

				}
			}
		}
		/*if (reduceframeall)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(Handle, 0);
		Vehicle zentorno = SpawnVehicle("CARGOPLANE", pCoords);
		ENTITY::SET_ENTITY_INVINCIBLE(zentorno, 0);
		ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
		ENTITY::SET_ENTITY_ALPHA(zentorno, 0.0f, 1);
		{
		if (i == 32)
		{
		break;
		}

		}
		}
		}*/
		if (crashLobby)
		{
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
				PED::CLONE_PED(Handle, 1, 1, 1);
				PED::CLONE_PED(Handle, 1, 1, 1);
				PED::CLONE_PED(Handle, 1, 1, 1);
				PED::CLONE_PED(Handle, 1, 1, 1);
				PED::CLONE_PED(Handle, 1, 1, 1);
				PED::CLONE_PED(Handle, 1, 1, 1);
				{
					if (i == 32)
					{
						break;
					}

				}
			}
		}
		if (TonCage)
		{
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
				Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(Handle, 0);
				Hash oball = GAMEPLAY::GET_HASH_KEY("prop_gold_cont_01");
				STREAMING::REQUEST_MODEL(oball);
				while (!STREAMING::HAS_MODEL_LOADED(oball))
					WAIT(0);
				//	int orangeball = CREATE_OBJECT(oball, pCoords.x, pCoords.y, pCoords.z, true, 1, 0);
				//	ATTACH_ENTITY_TO_ENTITY(orangeball, Handle, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
				AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
				NETWORK::OBJ_TO_NET(OBJECT::CREATE_OBJECT_NO_OFFSET(oball, pCoords.x, pCoords.y, pCoords.z, 1, 0, 0));
				{
					if (i == 32)
					{
						break;
					}

				}
			}
		}
		/*if (aCtree)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		Hash oball = GAMEPLAY::GET_HASH_KEY("prop_xmas_tree_int");
		STREAMING::REQUEST_MODEL(oball);
		while (!STREAMING::HAS_MODEL_LOADED(oball))
		WAIT(0);
		int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, Handle, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		{
		if (i == 32)
		{
		break;
		}

		}
		}
		}
		if (aArcade)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		Hash oball = GAMEPLAY::GET_HASH_KEY("prop_arcade_01");
		STREAMING::REQUEST_MODEL(oball);
		while (!STREAMING::HAS_MODEL_LOADED(oball))
		WAIT(0);
		int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, Handle, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		{
		if (i == 32)
		{
		break;
		}

		}
		}
		}
		if (aBarbel)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		Hash oball = GAMEPLAY::GET_HASH_KEY("prop_barbell_100kg");
		STREAMING::REQUEST_MODEL(oball);
		while (!STREAMING::HAS_MODEL_LOADED(oball))
		WAIT(0);
		int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, Handle, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		{
		if (i == 32)
		{
		break;
		}

		}
		}
		}*/
		/*if (aDplane)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		Hash oball = GAMEPLAY::GET_HASH_KEY("prop_dummy_plane");
		STREAMING::REQUEST_MODEL(oball);
		while (!STREAMING::HAS_MODEL_LOADED(oball))
		WAIT(0);
		int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, Handle, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		{
		if (i == 32)
		{
		break;
		}

		}
		}
		}
		if (atoilet)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		Hash oball = GAMEPLAY::GET_HASH_KEY("prop_ld_toilet_01");
		STREAMING::REQUEST_MODEL(oball);
		while (!STREAMING::HAS_MODEL_LOADED(oball))
		WAIT(0);
		int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, Handle, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		{
		if (i == 32)
		{
		break;
		}

		}
		}
		}
		if (aVdoor)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		Hash oball = GAMEPLAY::GET_HASH_KEY("prop_bank_vaultdoor");
		STREAMING::REQUEST_MODEL(oball);
		while (!STREAMING::HAS_MODEL_LOADED(oball))
		WAIT(0);
		int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, Handle, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		{
		if (i == 32)
		{
		break;
		}

		}
		}
		}*/
		if (killtall)
		{
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
				Vector3 pos = ENTITY::GET_ENTITY_COORDS(Handle, 1);
				if (!ENTITY::DOES_ENTITY_EXIST(Handle)) continue;
				FIRE::ADD_EXPLOSION(pos.x, pos.y, pos.z, 29, 0.5f, true, false, 5.0f);
				{
					if (i == 32)

					{
						break;
					}
					//	PrintBottomLeft(AddStrings("Kill em all!", ""));

				}
			}
		}
		if (remAtweaps)
		{
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
				WEAPON::REMOVE_ALL_PED_WEAPONS(Handle, 1);
				{
					if (i == 32)
					{
						break;
					}
					PrintStringBottomCentre("Los Santos is now a safe place to live..");

				}
			}
		}
		if (givAweap)
		{
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
				Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_RAILGUN");
				WEAPON::GIVE_WEAPON_TO_PED(Handle, railgun, railgun, 9999, 9999);
				{
					if (i == 32)
					{
						break;
					}
					PrintStringBottomCentre("Railgun given to All Players");

				}
			}
		}

		if (giveALLweap)
		{
			static LPCSTR weaponNames2[] = {
				"WEAPON_KNIFE", "WEAPON_NIGHTSTICK", "WEAPON_HAMMER", "WEAPON_BAT", "WEAPON_GOLFCLUB", "WEAPON_CROWBAR",
				"WEAPON_PISTOL", "WEAPON_COMBATPISTOL", "WEAPON_APPISTOL", "WEAPON_PISTOL50", "WEAPON_MICROSMG", "WEAPON_SMG",
				"WEAPON_ASSAULTSMG", "WEAPON_ASSAULTRIFLE", "WEAPON_CARBINERIFLE", "WEAPON_ADVANCEDRIFLE", "WEAPON_MG",
				"WEAPON_COMBATMG", "WEAPON_PUMPSHOTGUN", "WEAPON_SAWNOFFSHOTGUN", "WEAPON_ASSAULTSHOTGUN", "WEAPON_BULLPUPSHOTGUN",
				"WEAPON_STUNGUN", "WEAPON_SNIPERRIFLE", "WEAPON_COMBATPDW", "WEAPON_HEAVYSNIPER", "WEAPON_GRENADELAUNCHER", "WEAPON_GRENADELAUNCHER_SMOKE",
				"WEAPON_RPG", "WEAPON_MINIGUN", "WEAPON_GRENADE", "WEAPON_STICKYBOMB", "WEAPON_SMOKEGRENADE", "WEAPON_BZGAS",
				"WEAPON_MOLOTOV", "WEAPON_FIREEXTINGUISHER", "WEAPON_PETROLCAN", "WEAPON_KNUCKLE", "WEAPON_MARKSMANPISTOL",
				"WEAPON_SNSPISTOL", "WEAPON_SPECIALCARBINE", "WEAPON_HEAVYPISTOL", "WEAPON_BULLPUPRIFLE", "WEAPON_HOMINGLAUNCHER",
				"WEAPON_PROXMINE", "WEAPON_SNOWBALL", "WEAPON_VINTAGEPISTOL", "WEAPON_DAGGER", "WEAPON_FIREWORK", "WEAPON_MUSKET",
				"WEAPON_MARKSMANRIFLE", "WEAPON_HEAVYSHOTGUN", "WEAPON_GUSENBERG", "WEAPON_HATCHET", "WEAPON_RAILGUN", "WEAPON_FLASHLIGHT", "WEAPON_MACHINEPISTOL", "WEAPON_MACHETE"
			};
			{
				for (int i = 0; i <= 32; i++)
				{
					WAIT(0);
					if (i == PLAYER::PLAYER_ID())continue;
					int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);


					for (int i = 0; i < sizeof(weaponNames2) / sizeof(weaponNames2[0]); i++)
						WEAPON::GIVE_DELAYED_WEAPON_TO_PED(Handle, GAMEPLAY::GET_HASH_KEY((char *)weaponNames2[i]), 9999, 9999);
					WAIT(100);
					{
						if (i == 32)
						{
							break;
						}
						PrintStringBottomCentre("weapons for everybody!");

					}
				}}
		}
		/*if (makeALLSemiGod)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		Hash oball = GAMEPLAY::GET_HASH_KEY("prop_juicestand");
		STREAMING::REQUEST_MODEL(oball);
		while (!STREAMING::HAS_MODEL_LOADED(oball))
		WAIT(0);
		int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		ENTITY::SET_ENTITY_VISIBLE(orangeball, false, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, Handle, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		{
		if (i == 32)
		{
		break;
		}
		PrintStringBottomCentre("Everybody has now SemiGod  & No Cops!");

		}
		}
		}*/
		/*if (ufoALL)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		Hash oball = GAMEPLAY::GET_HASH_KEY("p_spinning_anus_s");
		STREAMING::REQUEST_MODEL(oball);
		while (!STREAMING::HAS_MODEL_LOADED(oball))
		WAIT(0);
		int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, Handle, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		{
		if (i == 32)
		{
		break;
		}
		PrintStringBottomCentre("UFO attached to all players..");

		}
		}
		}
		if (beachFireALL)
		{
		for (int i = 0; i <= 32; i++)
		{
		WAIT(0);
		if (i == PLAYER::PLAYER_ID())continue;
		int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
		Hash oball = GAMEPLAY::GET_HASH_KEY("prop_beach_fire");
		STREAMING::REQUEST_MODEL(oball);
		while (!STREAMING::HAS_MODEL_LOADED(oball))
		WAIT(0);
		int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, Handle, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		{
		if (i == 32)
		{
		break;
		}
		PrintStringBottomCentre("BeachFire attached to all players..");

		}
		}
		}*/
		if (Gttaser)
		{
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
				Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_STUNGUN");
				WEAPON::GIVE_WEAPON_TO_PED(Handle, railgun, railgun, 9999, 9999);
				{
					if (i == 32)
					{
						break;
					}
					PrintStringBottomCentre("TaserGun given to All Players");

				}
			}
		}
		if (gtsballs)
		{
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
				Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_SNOWBALL");
				WEAPON::GIVE_WEAPON_TO_PED(Handle, railgun, railgun, 9999, 9999);
				{
					if (i == 32)
					{
						break;
					}
					PrintStringBottomCentre("Snow Ball Fight!");

				}
			}
		}
		if (kefromveh)
		{
			for (int i = 0; i <= 32; i++)
			{
				WAIT(0);
				if (i == PLAYER::PLAYER_ID())continue;
				int Handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i);
				AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
				{
					if (i == 32)
					{
						break;
					}
					PrintStringBottomCentre("lmfao..");

				}
			}
		}
	}
	//bool speed = 0;

	void oPlayersSendAttackers(char* name, Player p) {



		bool sTanks = 0, railGunJesus = 0, sendCops2 = 0, sendSwatRiot = 0, sendCops = 0, taserswatt2 = 0;
		AddTitle("Anonymous");
		AddTitle("Custom Attack Options");
		AddOption("Send Tanks", sTanks);
		AddOption("Send Police Cars", sendCops);
		AddOption("Send Swat Riot", sendSwatRiot);
		AddOption("Swatt diz N1gga!", taserswatt2);
		AddOption("Railgun Jesus", railGunJesus);
		AddOption("Send Police Officer", sendCops2);

		//AddOption("");
		Ped myPed = PLAYER::PLAYER_PED_ID();
		Player playerPed = p;
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
		Vector3 Ocoords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0, 5.0, 0.0);

		if (sendCops2) {

			if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
			Hash guysex = GAMEPLAY::GET_HASH_KEY("s_m_y_cop_01");
			STREAMING::REQUEST_MODEL(guysex);
			while (!STREAMING::HAS_MODEL_LOADED(guysex))
				WAIT(0);
			int createdGuySex = PED::CREATE_PED(26, guysex, pCoords.x, pCoords.y, pCoords.z, 1, 1, 0);

			ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);

			Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL");
			WEAPON::GIVE_WEAPON_TO_PED(createdGuySex, railgun, railgun, 9999, 9999);
			AI::TASK_COMBAT_PED(createdGuySex, playerPed, 1, 1);
			ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);
			PED::SET_PED_COMBAT_ABILITY(createdGuySex, 100);
			PED::SET_PED_CAN_SWITCH_WEAPON(createdGuySex, true);
			AI::TASK_COMBAT_PED(createdGuySex, playerPed, 1, 1);
			PED::SET_PED_AS_ENEMY(createdGuySex, 1);
			PED::SET_PED_COMBAT_RANGE(createdGuySex, 1000);
			PED::SET_PED_KEEP_TASK(createdGuySex, true);
			PED::SET_PED_AS_COP(createdGuySex, 1000);
			PED::SET_PED_ALERTNESS(createdGuySex, 1000);

		}
		if (taserswatt2) {

			if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
			Hash stripper = GAMEPLAY::GET_HASH_KEY("s_m_y_swat_01");
			Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_STUNGUN");
			STREAMING::REQUEST_MODEL(stripper);
			while (!STREAMING::HAS_MODEL_LOADED(stripper))
				WAIT(0);

			int createdPED = PED::CREATE_PED(26, stripper, pCoords.x, pCoords.y, pCoords.z, 1, 1, 0);
			ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
			PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
			WEAPON::GIVE_WEAPON_TO_PED(createdPED, railgun, railgun, 9999, 9999);
			PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
			AI::TASK_COMBAT_PED(createdPED, playerPed, 1, 1);
			PED::SET_PED_ALERTNESS(createdPED, 1000);
			PED::SET_PED_COMBAT_RANGE(createdPED, 1000);
		}
		if (railGunJesus) {

			if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
			Hash stripper = GAMEPLAY::GET_HASH_KEY("u_m_m_jesus_01");
			Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_RAILGUN");
			STREAMING::REQUEST_MODEL(stripper);
			while (!STREAMING::HAS_MODEL_LOADED(stripper))
				WAIT(0);

			int createdPED = PED::CREATE_PED(26, stripper, pCoords.x, pCoords.y, pCoords.z, 1, 1, 0);
			ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
			PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
			WEAPON::GIVE_WEAPON_TO_PED(createdPED, railgun, railgun, 9999, 9999);
			PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
			AI::TASK_COMBAT_PED(createdPED, playerPed, 1, 1);
			PED::SET_PED_ALERTNESS(createdPED, 1000);
			PED::SET_PED_COMBAT_RANGE(createdPED, 1000);
		}

		if (sendSwatRiot) {

			if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
			Hash guysex = GAMEPLAY::GET_HASH_KEY("s_m_y_swat_01");
			STREAMING::REQUEST_MODEL(guysex);
			while (!STREAMING::HAS_MODEL_LOADED(guysex))
				WAIT(0);
			int createdGuySex = PED::CREATE_PED(26, guysex, Ocoords.x, Ocoords.y, Ocoords.z, 1, 1, 0);

			ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);
			//
			int vehmodel = GAMEPLAY::GET_HASH_KEY("RIOT");
			STREAMING::REQUEST_MODEL(vehmodel);

			while (!STREAMING::HAS_MODEL_LOADED(vehmodel)) WAIT(0);

			Vehicle veh = CREATE_VEHICLEB(vehmodel, Ocoords.x, Ocoords.y, Ocoords.z, 0.0, 1, 1);
			VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
			//
			Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_BULLPUPSHOTGUN");
			WEAPON::GIVE_WEAPON_TO_PED(createdGuySex, railgun, railgun, 9999, 9999);
			PED::SET_PED_INTO_VEHICLE(createdGuySex, veh, -1);
			AI::TASK_COMBAT_PED(createdGuySex, playerPed, 1, 1);
			ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);
			PED::SET_PED_COMBAT_ABILITY(createdGuySex, 100);
			PED::SET_PED_CAN_SWITCH_WEAPON(createdGuySex, true);
			AI::TASK_COMBAT_PED(createdGuySex, playerPed, 1, 1);
			PED::SET_PED_AS_ENEMY(createdGuySex, 1);
			PED::SET_PED_COMBAT_RANGE(createdGuySex, 1000);
			PED::SET_PED_KEEP_TASK(createdGuySex, true);
			PED::SET_PED_AS_COP(createdGuySex, 1000);
			PED::SET_PED_ALERTNESS(createdGuySex, 1000);

		}
		if (sendCops) {

			if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
			Hash guysex = GAMEPLAY::GET_HASH_KEY("s_m_y_cop_01");
			STREAMING::REQUEST_MODEL(guysex);
			while (!STREAMING::HAS_MODEL_LOADED(guysex))
				WAIT(0);
			int createdGuySex = PED::CREATE_PED(26, guysex, pCoords.x, pCoords.y, pCoords.z, 1, 1, 0);

			ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);
			//
			int vehmodel = GAMEPLAY::GET_HASH_KEY("POLICE3");
			STREAMING::REQUEST_MODEL(vehmodel);

			while (!STREAMING::HAS_MODEL_LOADED(vehmodel)) WAIT(0);
			//			Vector3 coords = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER_PED_ID(), 0.0, 5.0, 0.0);
			Vehicle veh = CREATE_VEHICLEB(vehmodel, pCoords.x, pCoords.y, pCoords.z, 0.0, 1, 1);
			VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
			//
			Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL");
			WEAPON::GIVE_WEAPON_TO_PED(createdGuySex, railgun, railgun, 9999, 9999);
			PED::SET_PED_INTO_VEHICLE(createdGuySex, veh, -1);
			AI::TASK_COMBAT_PED(createdGuySex, playerPed, 1, 1);
			ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);
			PED::SET_PED_COMBAT_ABILITY(createdGuySex, 100);
			PED::SET_PED_CAN_SWITCH_WEAPON(createdGuySex, true);
			AI::TASK_COMBAT_PED(createdGuySex, playerPed, 1, 1);
			PED::SET_PED_AS_ENEMY(createdGuySex, 1);
			PED::SET_PED_COMBAT_RANGE(createdGuySex, 1000);
			PED::SET_PED_KEEP_TASK(createdGuySex, true);
			PED::SET_PED_AS_COP(createdGuySex, 1000);
			PED::SET_PED_ALERTNESS(createdGuySex, 1000);

		}
		if (sTanks) {

			if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
			Hash guysex = GAMEPLAY::GET_HASH_KEY("s_m_y_marine_01");
			STREAMING::REQUEST_MODEL(guysex);
			while (!STREAMING::HAS_MODEL_LOADED(guysex))
				WAIT(0);
			int createdGuySex = PED::CREATE_PED(26, guysex, pCoords.x, pCoords.y, pCoords.z, 1, 1, 0);

			ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);
			//
			int vehmodel = GAMEPLAY::GET_HASH_KEY("RHINO");
			STREAMING::REQUEST_MODEL(vehmodel);

			while (!STREAMING::HAS_MODEL_LOADED(vehmodel)) WAIT(0);
			//			Vector3 coords = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER_PED_ID(), 0.0, 5.0, 0.0);
			Vehicle veh = CREATE_VEHICLEB(vehmodel, pCoords.x, pCoords.y, pCoords.z, 0.0, 1, 1);
			VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
			//
			PED::SET_PED_INTO_VEHICLE(createdGuySex, veh, -1);
			AI::TASK_COMBAT_PED(createdGuySex, playerPed, 1, 1);
			ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);
			PED::SET_PED_COMBAT_ABILITY(createdGuySex, 100);
			PED::SET_PED_CAN_SWITCH_WEAPON(createdGuySex, true);
			AI::TASK_COMBAT_PED(createdGuySex, playerPed, 1, 1);
			PED::SET_PED_AS_ENEMY(createdGuySex, 1);
			PED::SET_PED_COMBAT_RANGE(createdGuySex, 1000);
			PED::SET_PED_KEEP_TASK(createdGuySex, true);
			PED::SET_PED_AS_COP(createdGuySex, 1000);
			PED::SET_PED_ALERTNESS(createdGuySex, 1000);


		}

	}

	Vehicle SpawnVehicle(char* modelg, Vector3 coords, bool tpinto = 0, float heading = 0.0f) {

		Hash model = GAMEPLAY::GET_HASH_KEY(modelg);
		if (STREAMING::IS_MODEL_VALID(model))
		{
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
			Vector3 ourCoords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), false);
			float forward = 5.f;
			float heading = ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID());
			float xVector = forward * sin(degToRad(heading)) * -1.f;
			float yVector = forward * cos(degToRad(heading));
			Vehicle veh = VEHICLE::CREATE_VEHICLE(model, ourCoords.x + xVector, ourCoords.y + yVector, ourCoords.z, heading, true, true);
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_ENGINE_ON(veh, true, true, true);
			VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
			DECORATOR::DECOR_SET_INT(veh, "MPBitset", 0);
			auto networkId = NETWORK::VEH_TO_NET(veh);
			//ENTITY::_SET_ENTITY_REGISTER(veh, true);
			if (tpinto) {
				ENTITY::SET_ENTITY_HEADING(veh, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));
				PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);
			}
			if (NETWORK::NETWORK_GET_ENTITY_IS_NETWORKED(veh))
				NETWORK::SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES(networkId, true);
			
			WAIT(150);
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
		}
	}
	Vehicle CREATE_VEHICLE2(Hash model, float x, float y, float z, float heading, bool NetHandle, bool VehicleHandle) {
		tpeyoftEm();
		Vehicle veh = VEHICLE::CREATE_VEHICLE(model, x, y, z, heading, NetHandle, VehicleHandle);
		if (UseCarBypass) {
			ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, 1, 1);
			DWORD id = NETWORK::NET_TO_VEH(veh);
			NETWORK::SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES(id, 1);
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECORATOR::DECOR_REGISTER((char*)encryptDecrypt(hex_to_string("1B272A322E39141D2E232228272E")).c_str(), 3);//Insurance
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECORATOR::DECOR_SET_INT(veh, (char*)encryptDecrypt(hex_to_string("1B272A322E39141D2E232228272E")).c_str(), (PLAYER::PLAYER_ID()));
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECORATOR::DECOR_REGISTER((char*)encryptDecrypt(hex_to_string("1D2E231406242F2F2E2F140932141B272A322E39")).c_str(), 3);//Veh_Modded_By_Player
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECORATOR::DECOR_SET_INT(veh, (char*)encryptDecrypt(hex_to_string("1D2E231406242F2F2E2F140932141B272A322E39")).c_str(), GAMEPLAY::GET_HASH_KEY(PLAYER::GET_PLAYER_NAME(PLAYER::PLAYER_ID())));
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECORATOR::DECOR_SET_BOOL(veh, (char*)encryptDecrypt(hex_to_string("022C2524392E2F09321A3E222820182A3D2E")).c_str(), 0);
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			DECORATOR::DECOR_REGISTER((char*)encryptDecrypt(hex_to_string("1B1D141827243F")).c_str(), 3);
			for (int i = 0; i < rand() % 100; i++) DEBUGOUT("Critical Error");
			VEHICLE::SET_VEHICLE_IS_STOLEN(veh, 0);
			globalHandle(0x27C6CA);
		}
		return veh;
	}
	Vehicle SpawnVehicle2(char* modelg, Vector3 coords, bool tpinto = 0, float heading = 0.0f) {
		DWORD model = GAMEPLAY::GET_HASH_KEY(modelg);
		if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_A_VEHICLE(model)) {
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
			Vehicle veh = CREATE_VEHICLEB(model, coords.x, coords.y, coords.z, heading, 1, 1);
			VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
			if (tpinto) {
				ENTITY::SET_ENTITY_HEADING(veh, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));
				PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);
			}
			VEHICLE::SET_VEHICLE_IS_STOLEN(veh, 0);
			return veh;
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
		}

	}
	void AttachThingToThing(Any p0, Any p1, float x = 0, float y = 0, float z = 0, float rx = 0, float ry = 0, float rz = 0) {
		//ATTACH_ENTITY_TO_ENTITY(p0, p1, 0, -0.5f, -0.2f, -0.1f, 0.0f, 0.0f, 180.0f, 1, 0, 0, 2, 1, 1);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(p0, p1, 0, x, y, z, rx, ry, rz, 1, 0, 0, 2, 1, 1);
	}
	Ped SpawnModel(char* modelg, Vector3 coords, bool forcar = 1) {
		DWORD model = GAMEPLAY::GET_HASH_KEY(modelg);
		if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
		{
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
			Ped pedm = PED::CREATE_PED(26, model, coords.x, coords.y, coords.z, 0.0f, 1, 0);
			if (forcar) AI::CLEAR_PED_TASKS_IMMEDIATELY(model);
			return pedm;
		}
		else {
			PrintStringBottomCentre("~r~Error~s~: Model not found");
		}
	}
	Object SpawnObject(char* name, Vector3 coords) {
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		//	Object obj = CREATE_OBJECT_NO_OFFSET(GET_HASH_KEY(name), coords.x, coords.y, coords.z, 1, 0, 0);
		Object obj = OBJECT::CREATE_OBJECT(GAMEPLAY::GET_HASH_KEY(name), pos.x, pos.y, pos.z, true, 1, 0);
		return obj;
	}
	bool tpinto = false;
	bool freezespawned = false;

	void FunnyVehicles() {




		bool flyingufo = 0, cowcar = 0, poomobile = 0, wheelchair = 0, bumper = 0, sofa = 0, cupcar = 0, deer = 0, coyotte = 0, shark = 0, roller = 0;
		AddTitle("Anonymous MENU");
		AddTitle("Funny/Special Vehicles");
		AddToggle("Teleport in", tpinto);
		AddOption("UFO Hydra", flyingufo);
		//	AddOption("Cow Car", cowcar);
		//	AddOption("Deer Car", deer);
		AddOption("Coyote Car", coyotte);
		AddOption("Shark Car", shark);
		AddOption("Poo Mobile", poomobile);
		AddOption("Weelchair", wheelchair);
		AddOption("Bumper car", bumper);
		AddOption("Roller car", roller);
		AddOption("Sofa car", sofa);
		AddOption("Cup Car", cupcar);
		/*AddNumberEasy("X", x, 2, x, 0.10f);
		AddNumberEasy("Y", y, 2, y, 0.10f);
		AddNumberEasy("Z", z, 2, z, 0.10f);
		AddNumberEasy("RX", rx, 2, rx, 0.10f);
		AddNumberEasy("RY", ry, 2, ry, 0.10f);
		AddNumberEasy("RZ", rz, 2, rz, 180.0f);*/
		Player player = PLAYER::PLAYER_ID();
		BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(player);
		Ped playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(player);
		Ped myPed = PLAYER::PLAYER_PED_ID();
		Player myPlayer = PLAYER::PLAYER_ID();
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
		if (flyingufo) {
			AttachThingToThing(SpawnObject("p_spinning_anus_s", pCoords), SpawnVehicle("HYDRA", pCoords, true));
		}
		if (roller) {

			Vehicle zentorno = SpawnVehicle("SURANO", pCoords, tpinto);
			ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
			Object model = SpawnObject("prop_roller_car_02", pCoords);
			if (freezespawned) {
				ENTITY::FREEZE_ENTITY_POSITION(model, true);
			}
			AttachThingToThing(model, zentorno, 0.0f, -0.7f, -0.6f, 0.0f, 0.0f, 180.0f);
		}
		if (shark) {
			Vehicle zentorno = ("SPEEDO2", pCoords, tpinto);
			ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
			Ped model = SpawnModel("a_c_sharktiger", pCoords);
			if (freezespawned) {
				ENTITY::FREEZE_ENTITY_POSITION(model, true);
			}
			AttachThingToThing(model, zentorno);
		}
		if (deer) {
			Vehicle zentorno = sub::SpawnVehicle("SPEEDO2", pCoords, tpinto);
			ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
			Ped model = SpawnModel("a_c_deer", pCoords);
			if (freezespawned) {
				ENTITY::FREEZE_ENTITY_POSITION(model, true);
			}
			AttachThingToThing(model, zentorno);
		}
		if (coyotte) {
			Vehicle zentorno = sub::SpawnVehicle("SPEEDO2", pCoords, tpinto);
			ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
			Ped model = SpawnModel("a_c_coyote", pCoords);
			if (freezespawned) {
				ENTITY::FREEZE_ENTITY_POSITION(model, true);
			}
			AttachThingToThing(model, zentorno);
		}
		if (cowcar) {
			Vehicle zentorno = sub::SpawnVehicle("SPEEDO2", pCoords, tpinto);
			ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
			Ped model = SpawnModel("a_c_cow", pCoords);
			if (freezespawned) {
				ENTITY::FREEZE_ENTITY_POSITION(model, true);
			}
			AttachThingToThing(model, zentorno);
		}
		if (poomobile) {
			Vehicle zentorno = sub::SpawnVehicle("SURANO", pCoords, tpinto);
			ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
			Object model = SpawnObject("prop_ld_toilet_01", pCoords);
			if (freezespawned) {
				ENTITY::FREEZE_ENTITY_POSITION(model, true);
			}
			AttachThingToThing(model, zentorno, -0.5f, -0.8f, -0.4f, 0.0f, 0.0f, 180.0f);
		}
		if (wheelchair) {
			Vehicle zentorno = sub::SpawnVehicle("SURANO", pCoords, tpinto);
			ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
			Object model = SpawnObject("prop_wheelchair_01", pCoords);
			if (freezespawned) {
				ENTITY::FREEZE_ENTITY_POSITION(model, true);
			}
			AttachThingToThing(model, zentorno, -0.5f, -0.7f, -0.4f, 0.0f, 0.0f, 180.0f);
		}
		if (bumper) {
			Vehicle zentorno = sub::SpawnVehicle("SURANO", pCoords, tpinto);
			ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
			Object model = SpawnObject("PROP_BUMPER_CAR_01", pCoords);
			if (freezespawned) {
				ENTITY::FREEZE_ENTITY_POSITION(model, true);
			}
			AttachThingToThing(model, zentorno, -0.45f, -0.2f, 0.0f, 0.0f, 0.0f, 180.0f);
		}
		if (sofa) {
			Vehicle zentorno = sub::SpawnVehicle("SURANO", pCoords, tpinto);
			ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
			Object model = SpawnObject("PROP_YACHT_SEAT_01", pCoords);
			if (freezespawned) {
				ENTITY::FREEZE_ENTITY_POSITION(model, true);
			}
			AttachThingToThing(model, zentorno, -0.4f, -0.79f, -0.9f, 0.0f, 0.0f, 180.0f);
		}
		if (cupcar) {
			Vehicle zentorno = sub::SpawnVehicle("SURANO", pCoords, tpinto);
			ENTITY::SET_ENTITY_VISIBLE(zentorno, false, 0);
			Object model = SpawnObject("PROP_CUP_SAUCER_01", pCoords);
			if (freezespawned) {
				ENTITY::FREEZE_ENTITY_POSITION(model, true);
			}
			AttachThingToThing(model, zentorno, -0.4f, -0.5f, -0.4f, 0.0f, 0.0f, 180.0f);

		}
		if (!invisible && !ENTITY::IS_ENTITY_VISIBLE(player)) {
			ENTITY::SET_ENTITY_VISIBLE(playerPed, true, 1);
		}

	}
	void AddIntEasy(char* text, int value, int &val, int inc = 1, bool fast = 0, bool &toggled = null, bool enableminmax = 0, int max = 0, int min = 0)
	{
		null = 0;
		AddOption(text, null);

		if (OptionY < 0.6325 && OptionY > 0.1425)
		{
			UI::SET_TEXT_FONT(0);
			UI::SET_TEXT_SCALE(0.26f, 0.26f);
			UI::SET_TEXT_CENTRE(1);

			drawint(value, 0.233f + menuPos, OptionY);
		}

		if (menu::printingop == menu::currentop)
		{
			if (IsOptionRJPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val + inc) > max)) {
						val += inc;
					}
				}
				else {
					val += inc;
				}
			}
			else if (IsOptionRPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val + inc) > max)) {
						val += inc;
					}
				}
				else {
					val += inc;
				}
			}
			else if (IsOptionLJPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val - inc) < min)) {
						val -= inc;
					}
				}
				else {
					val -= inc;
				}
			}
			else if (IsOptionLPressed()) {
				toggled = 1;
				if (enableminmax) {
					if (!((val - inc) < min)) {
						val -= inc;
					}
				}
				else {
					val -= inc;
				}
			}
		}
	}
	void VehicleSpawner() {


		bool custominput = 0;
		AddTitle("Anonymous ");

		AddToggle("Use bypass", UseCarBypass);
		//AddToggle("Bypass car spawning limit", carspawnlimit);
		AddToggle("Spawn Fully Upgraded", automax);
		AddOption("Custom ~b~Input", custominput);
		AddOption("~HUD_COLOUR_GOLD~My Vehicles", null, nullFunc, SUB::SAVEDVEHICLES);
		AddOption("~b~Favorites", null, nullFunc, SUB::FAVORITE_VEV);
		AddOption("Smug", null, nullFunc, SUB::XYAYSDKKHAJDK);
		AddOption("Gunrunning", null, nullFunc, SUB::GUNRUNNING);
		AddOption("Super Cars", null, nullFunc, SUB::SUPERCAR_VEV);
		AddOption("Lowriders Cars", null, nullFunc, SUB::LOWRIDER_VEV);
		AddOption("Muscle", null, nullFunc, SUB::MUSCLE_VEV);
		AddOption("Off Roads", null, nullFunc, SUB::OFF_ROADS_VEV);
		AddOption("Sports", null, nullFunc, SUB::SPORTS_VEV);
		AddOption("Sports Classics", null, nullFunc, SUB::SPORTS_CLASSICS_VEV);
		AddOption("Motorcycles", null, nullFunc, SUB::MOTORCYCLES_VEV);
		AddOption("Planes", null, nullFunc, SUB::PLANES_VEV);
		AddOption("Helicopters", null, nullFunc, SUB::HELICOPTERS_VEV);
		AddOption("Military", null, nullFunc, SUB::MILITARY_VEV);
		AddOption("Emergency", null, nullFunc, SUB::EMERGENCY_VEV);
		AddOption("Suvs", null, nullFunc, SUB::SUVS_VEV);
		AddOption("Sedans", null, nullFunc, SUB::SEDANS_VEV);
		AddOption("Compact", null, nullFunc, SUB::COMPACT_VEV);
		AddOption("Coupes", null, nullFunc, SUB::COUPES_VEV);
		AddOption("Vans", null, nullFunc, SUB::VANS_VEV);
		AddOption("Services", null, nullFunc, SUB::SERVICES_VEV);
		AddOption("Industrial", null, nullFunc, SUB::INDUSTRIAL_VEV);
		AddOption("Boats", null, nullFunc, SUB::BOATS_VEV);
		AddOption("Cycles", null, nullFunc, SUB::CYCLES_VEV);
		AddOption("Utility", null, nullFunc, SUB::UTILITY_VEV);


		//			for (int i = 0; i < 340; i++){
		//				AddCarSpawn((char*)vehicles[i].Name, vehicles[i].gameName);
		//			}
		if (custominput) {
			char* data = keyboard();
			SpawnVehicle(data, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
		}
	}


}
void oPlayerOptMenu(char* name, Player p) {



	bool tel2pla = 0, explodep = 0, kickplayer = 0, giveUnAmmo = 0, firkatack = 0, giveweap = 0, oforce1 = 0,
		dirflame2 = 0, makeSemiGod = 0, removwep = 0, givarmor = 0, givhealth = 0, watack = 0, fatack = 0, clonep = 0, pbclone = 0, rfccar = 0, frtsk = 0,
		gcash = 0, ram = 0, giveCashE = 0;
	bool giveCops = 0;
	//		bool hasExplosiveAmmo = doesPlayerHaveExplosiveAmmo(name);
	AddTitle(name);
	//		AddOption("Give Alot of Ammo", giveUnAmmo);
	//		AddToggle("Give Explosive Ammo", hasExplosiveAmmo);
	//	AddOption("~y~Ram Player", ram);
	AddOption("~r~Explode Player", explodep);
	AddOption("Give All Weapons", giveweap);
	AddOption("Remove All Weapons", removwep);
	AddOption("Water Attack", watack);
	AddOption("Fire Attack", fatack);
	AddOption("Dir Flame", dirflame2);
	AddOption("Valkyrie Cannon", firkatack);
	AddOption("Clone Player", clonep);
	AddOption("Piggyback Clone", pbclone);
	AddOption("Random Fully Upgraded Car", rfccar);
	AddToggle("Freeze Player tasks", loop_ClearpTasks);
	AddToggle("~r~Give Cash 10k Bags", loop_moneydrop, giveCashE);
	AddOption("SemiGod & NoCops", makeSemiGod);
	AddToggle("~r~Explosion Loop", loop_annoyBomb);
	AddToggle("~r~Force Field", forcefield);
	AddOption("Give Him Cops", giveCops);
	AddToggle("Fuck Player's Camera", loop_fuckCam);
	//		AddToggle("Make him Weld", kickplayer);

	Ped myPed = PLAYER::PLAYER_PED_ID();
	Player playerPed = p;
	Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
	if (giveCops) {
		RequestControlOfEnt(playerPed);
		PLAYER::SET_DISPATCH_COPS_FOR_PLAYER(selPlayer, 1);
		PLAYER::GET_WANTED_LEVEL_THRESHOLD(5);
		PLAYER::REPORT_CRIME(selPlayer, 36, 1200);
		Vehicle policeCar = sub::CREATE_VEHICLEB(GAMEPLAY::GET_HASH_KEY("police"), pCoords.x, pCoords.y + 30, pCoords.z, ENTITY::GET_ENTITY_HEADING(playerPed), 1, 1);
		FIRE::ADD_OWNED_EXPLOSION(playerPed, pCoords.x + 5, pCoords.y + 30, pCoords.z, 4, 0, 0, 0, 1);
		FIRE::ADD_OWNED_EXPLOSION(playerPed, pCoords.x, pCoords.y + 30, pCoords.z, 4, 0, 0, 0, 1);
		ENTITY::SET_VEHICLE_AS_NO_LONGER_NEEDED(&policeCar);
	}
	if (!riskMode && giveCashE) {
		PrintStringBottomCentre("~y~Enable Risk Mode to enable this (Misc Menu)");
		loop_moneydrop = 0;
	}
	if (explodep)
	{
		FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z, 29, 0.5f, true, false, 5.0f);
		//	PrintStringBottomCentre("Exploded Player");
	}
	if (ram) {
		float offset;
		Hash vehmodel = GAMEPLAY::GET_HASH_KEY("t20");
		STREAMING::REQUEST_MODEL(vehmodel);

		while (!STREAMING::HAS_MODEL_LOADED(vehmodel)) WAIT(0);
		Vector3 pCoords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0, -10.0, 0.0);

		if (STREAMING::IS_MODEL_IN_CDIMAGE(vehmodel) && STREAMING::IS_MODEL_A_VEHICLE(vehmodel))
		{
			Vector3 dim1, dim2;
			GAMEPLAY::GET_MODEL_DIMENSIONS(vehmodel, &dim1, &dim2);

			offset = dim2.y * 1.6;

			Vector3 dir = ENTITY::GET_ENTITY_FORWARD_VECTOR(playerPed);
			float rot = (ENTITY::GET_ENTITY_ROTATION(playerPed, 0)).z;

			Vehicle veh = sub::CREATE_VEHICLEB(vehmodel, pCoords.x + (dir.x * offset), pCoords.y + (dir.y * offset), pCoords.z, rot, 1, 1);

			VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
			ENTITY::SET_ENTITY_VISIBLE(veh, true, 1);
			VEHICLE::SET_VEHICLE_FORWARD_SPEED(veh, 700.0);
		}
		else PrintStringBottomCentre("Unable to load car");

	}
	/*if (hasExplosiveAmmo && !doesPlayerHaveExplosiveAmmo(name)) {
	if (ENTITY::DOES_ENTITY_EXIST(playerPed))
	EditPlayerWithExplosiveAmmo(name);
	return;
	}
	if (!hasExplosiveAmmo && doesPlayerHaveExplosiveAmmo(name)) {
	if (ENTITY::DOES_ENTITY_EXIST(playerPed))
	EditPlayerWithExplosiveAmmo(name);
	return;

	}*/
	if (kickplayer)
	{

		char *anim = "WORLD_HUMAN_WELDING";
		AI::CLEAR_PED_TASKS_IMMEDIATELY(playerPed);
		AI::TASK_START_SCENARIO_IN_PLACE(playerPed, anim, 0, true);
	}
	if (ffield) {
		ENTITY::APPLY_FORCE_TO_ENTITY(playerPed, true, 0, 100, 100, 0, 0, 0, false, true, false, false, false, true);
		ENTITY::APPLY_FORCE_TO_ENTITY(playerPed, 13, 99.9f, 99.9f, 99.9f, 0.0f, 0.0f, 0.0f, 1, 1, 1, 1, 0, 1);
	}
	if (makeSemiGod)
	{
		/*Hash oball = GAMEPLAY::GET_HASH_KEY("prop_juicestand");
		STREAMING::REQUEST_MODEL(oball);
		while (!STREAMING::HAS_MODEL_LOADED(oball))
		WAIT(0);
		int orangeball = OBJECT::CREATE_OBJECT(oball, 0, 0, 0, true, 1, 0);
		RequestControlOfEnt(orangeball);
		ENTITY::SET_ENTITY_VISIBLE(orangeball, false, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(orangeball, playerPed, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 1);
		*/
	}


	if (giveweap)
	{
		static LPCSTR weaponNames2[] = {
			"WEAPON_KNIFE", "WEAPON_NIGHTSTICK", "WEAPON_HAMMER", "WEAPON_BAT", "WEAPON_GOLFCLUB", "WEAPON_CROWBAR",
			"WEAPON_PISTOL", "WEAPON_COMBATPISTOL", "WEAPON_APPISTOL", "WEAPON_PISTOL50", "WEAPON_MICROSMG", "WEAPON_SMG",
			"WEAPON_ASSAULTSMG", "WEAPON_ASSAULTRIFLE", "WEAPON_CARBINERIFLE", "WEAPON_ADVANCEDRIFLE", "WEAPON_MG",
			"WEAPON_COMBATMG", "WEAPON_PUMPSHOTGUN", "WEAPON_SAWNOFFSHOTGUN", "WEAPON_ASSAULTSHOTGUN", "WEAPON_BULLPUPSHOTGUN",
			"WEAPON_STUNGUN", "WEAPON_SNIPERRIFLE", "WEAPON_HEAVYSNIPER", "WEAPON_GRENADELAUNCHER", "WEAPON_GRENADELAUNCHER_SMOKE",
			"WEAPON_RPG", "WEAPON_MINIGUN", "WEAPON_GRENADE", "WEAPON_STICKYBOMB", "WEAPON_SMOKEGRENADE", "WEAPON_BZGAS",
			"WEAPON_MOLOTOV", "WEAPON_FIREEXTINGUISHER", "WEAPON_PETROLCAN", "WEAPON_KNUCKLE", "WEAPON_MARKSMANPISTOL",
			"WEAPON_SNSPISTOL", "WEAPON_SPECIALCARBINE", "WEAPON_HEAVYPISTOL", "WEAPON_BULLPUPRIFLE", "WEAPON_HOMINGLAUNCHER",
			"WEAPON_PROXMINE", "WEAPON_SNOWBALL", "WEAPON_VINTAGEPISTOL", "WEAPON_DAGGER", "WEAPON_FIREWORK", "WEAPON_MUSKET",
			"WEAPON_MARKSMANRIFLE", "WEAPON_HEAVYSHOTGUN", "WEAPON_GUSENBERG", "WEAPON_COMBATPDW", "WEAPON_HATCHET", "WEAPON_RAILGUN", "WEAPON_FLASHLIGHT", "WEAPON_MACHINEPISTOL", "WEAPON_MACHETE"
		};
		BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(p);
		Ped playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(p);
		Ped myPed = PLAYER::PLAYER_PED_ID();
		Player myPlayer = PLAYER::PLAYER_ID();
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
		for (int i = 0; i < sizeof(weaponNames2) / sizeof(weaponNames2[0]); i++)
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(playerPed, GAMEPLAY::GET_HASH_KEY((char *)weaponNames2[i]), 1000, 0);
		PrintStringBottomCentre("Player have it all!");

	}

	if (giveUnAmmo)
	{
		static LPCSTR weaponNames2[] = {
			"WEAPON_KNIFE", "WEAPON_NIGHTSTICK", "WEAPON_HAMMER", "WEAPON_BAT", "WEAPON_GOLFCLUB", "WEAPON_CROWBAR",
			"WEAPON_PISTOL", "WEAPON_COMBATPISTOL", "WEAPON_APPISTOL", "WEAPON_PISTOL50", "WEAPON_MICROSMG", "WEAPON_SMG",
			"WEAPON_ASSAULTSMG", "WEAPON_ASSAULTRIFLE", "WEAPON_CARBINERIFLE", "WEAPON_ADVANCEDRIFLE", "WEAPON_MG",
			"WEAPON_COMBATMG", "WEAPON_PUMPSHOTGUN", "WEAPON_SAWNOFFSHOTGUN", "WEAPON_ASSAULTSHOTGUN", "WEAPON_BULLPUPSHOTGUN",
			"WEAPON_STUNGUN", "WEAPON_COMBATPDW", "WEAPON_SNIPERRIFLE", "WEAPON_HEAVYSNIPER", "WEAPON_GRENADELAUNCHER", "WEAPON_GRENADELAUNCHER_SMOKE",
			"WEAPON_RPG", "WEAPON_MINIGUN", "WEAPON_GRENADE", "WEAPON_STICKYBOMB", "WEAPON_SMOKEGRENADE", "WEAPON_BZGAS",
			"WEAPON_MOLOTOV", "WEAPON_FIREEXTINGUISHER", "WEAPON_PETROLCAN", "WEAPON_KNUCKLE", "WEAPON_MARKSMANPISTOL",
			"WEAPON_SNSPISTOL", "WEAPON_SPECIALCARBINE", "WEAPON_HEAVYPISTOL", "WEAPON_BULLPUPRIFLE", "WEAPON_HOMINGLAUNCHER",
			"WEAPON_PROXMINE", "WEAPON_SNOWBALL", "WEAPON_VINTAGEPISTOL", "WEAPON_DAGGER", "WEAPON_FIREWORK", "WEAPON_MUSKET",
			"WEAPON_MARKSMANRIFLE", "WEAPON_HEAVYSHOTGUN", "WEAPON_GUSENBERG", "WEAPON_HATCHET", "WEAPON_RAILGUN", "WEAPON_FLASHLIGHT", "WEAPON_MACHINEPISTOL", "WEAPON_MACHETE"
		};

		for (int i = 0; i < sizeof(weaponNames2) / sizeof(weaponNames2[0]); i++)
			//	Hash railgun = GET_HASH_KEY("WEAPON_PISTOL");

			WEAPON::SET_PED_AMMO(playerPed, GAMEPLAY::GET_HASH_KEY((char *)weaponNames2[i]), 9999);
		//		SET_PED_INFINITE_AMMO(playerPed, 1, GET_HASH_KEY((char *)weaponNames2[i]));
	}
	if (removwep) { WEAPON::REMOVE_ALL_PED_WEAPONS(playerPed, 1); }

	if (watack)
	{
		FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z - 1, 13, 0.5f, true, false, 0.0f);
	}
	if (fatack)
	{
		FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z - 1, 12, 0.5f, true, false, 0.0f);
	}
	if (firkatack)
	{
		FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z - 1, 38, 0.5f, true, false, 0.0f);
	}
	if (dirflame2)
	{
		FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z - 1, 30, 0.5f, true, false, 0.0f);
	}
	if (clonep)
	{
		PED::CLONE_PED(playerPed, 1, 1, 1);
	}
	if (pbclone)
	{
		char *anim = "mini@prostitutes@sexnorm_veh";
		char *animID = "bj_loop_male";
		int playerClone = PED::CLONE_PED(playerPed, 1, 1, 1);
		ENTITY::SET_ENTITY_INVINCIBLE(playerClone, true);
		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);
		AI::TASK_PLAY_ANIM(playerClone, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
		ENTITY::ATTACH_ENTITY_TO_ENTITY(playerClone, playerPed, 0.059999998658895f, 0.0f, -0.25f, 0.0f, 0.0f, 0.0f, 1, 1, 0, 0, 2, 1, 1);
	}
	if (rfccar)
	{
		int vehID = VEHICLE::GET_CLOSEST_VEHICLE(pCoords.x, pCoords.y, pCoords.z, 600.0f, 0, 0);
		ENTITY::SET_ENTITY_COORDS(vehID, pCoords.x, pCoords.y, pCoords.z + 1.2, 1, 0, 0, 1);

		RequestControlOfEnt(vehID);
		VEHICLE::SET_VEHICLE_MOD_KIT(vehID, 0);
		VEHICLE::SET_VEHICLE_COLOURS(vehID, 120, 120);
		VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(vehID, "Anonymous");
		VEHICLE::SET_VEHICLE_MOD_KIT(vehID, 0);
		VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(vehID, 1);
		VEHICLE::TOGGLE_VEHICLE_MOD(vehID, 18, 1);
		VEHICLE::TOGGLE_VEHICLE_MOD(vehID, 22, 1);
		VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(vehID, 0);
		VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(vehID, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 0, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 0) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 1, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 1) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 2, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 2) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 3, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 3) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 4, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 4) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 5, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 5) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 6, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 6) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 7, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 7) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 8, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 8) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 9, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 9) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 10, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 10) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 11, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 11) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 12, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 12) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 13, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 13) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 14, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 14) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 15, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 15) - 1, 0);
		VEHICLE::SET_VEHICLE_MOD(vehID, 16, VEHICLE::GET_NUM_VEHICLE_MODS(vehID, 16) - 1, 0);
		VEHICLE::SET_VEHICLE_WHEEL_TYPE(vehID, 6);
		VEHICLE::SET_VEHICLE_WINDOW_TINT(vehID, 5);
		VEHICLE::SET_VEHICLE_MOD(vehID, 23, 19, 1);
		PrintStringBottomCentre("Spawned maxed out Vehicle");
	}

}
void AddCarSpawn(char* text, LPCSTR name, bool &extra_option_code = null)
{
	null = 0;
	AddOption(text, null);
	if (menu::printingop == menu::currentop)
	{
		if (null)
		{
			RequestControlOfEnt(myVeh);
			DWORD model = GAMEPLAY::GET_HASH_KEY((char*)name);
			if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_A_VEHICLE(model)) {
				STREAMING::REQUEST_MODEL(model);
				while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
				Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 5.0, 0.0);
				Vehicle veh = sub::CREATE_VEHICLEB(model, coords.x, coords.y, coords.z, 0.0, 1, 1);
				VEHICLE::SET_VEHICLE_IS_STOLEN(veh, 0);
				VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
				RequestControlOfEnt(veh);


				VEHICLE::SET_VEHICLE_ENGINE_ON(veh, true, true, true);
				VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
				DECORATOR::DECOR_SET_INT(veh, "MPBitset", 0);
				auto networkId = NETWORK::VEH_TO_NET(veh);

				if (NETWORK::NETWORK_GET_ENTITY_IS_NETWORKED(veh))
					NETWORK::SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES(networkId, true);

				if (tpinspawned) {
					ENTITY::SET_ENTITY_HEADING(veh, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));
					PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);
				}
				if (automax) {
					VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
					VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(veh, "ANON");
					VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
					VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(veh, 1);
					VEHICLE::TOGGLE_VEHICLE_MOD(veh, 18, 1);
					VEHICLE::TOGGLE_VEHICLE_MOD(veh, 22, 1);
					VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 0);
					VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 0, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 0) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 1, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 1) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 2, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 2) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 3, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 3) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 4, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 4) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 5, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 5) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 6, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 6) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 7, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 7) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 8, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 8) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 9, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 9) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 10, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 10) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 11, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 11) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 12, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 12) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 13, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 13) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 14, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 14) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 15, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 15) - 1, 0);
					VEHICLE::SET_VEHICLE_MOD(veh, 16, VEHICLE::GET_NUM_VEHICLE_MODS(veh, 16) - 1, 0);
					VEHICLE::SET_VEHICLE_WHEEL_TYPE(veh, 6);
					VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 5);
					VEHICLE::SET_VEHICLE_MOD(veh, 23, 19, 1);
				}
				RequestControlOfEnt(veh);
				if (carspawnlimit)
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&veh);
			}

			extra_option_code = true;
		}
	}

}
bool heat = 0, night = 0, slow = 0, online = 0, popMessage = 0;
void VisionMods()
{
	//	draw_force();

	bool defaulte = 0, stoned = 0, orange = 0, red = 0, coke = 0, fuckedup = 0, hallu = 0, wobbly = 0, drunk = 0, heaven = 0, threedim = 0, killstreak = 0, lq = 0, blurry = 0, white = 0;

	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	AddTitle("Vision Effects");
	AddOption("Default", defaulte);
	AddToggle("Heat Vision", heat);
	AddToggle("Night Vision", night);
	AddOption("Stoned", stoned);
	AddOption("Orange", orange);
	AddOption("Red", red);
	AddOption("Cocaine", coke);
	AddOption("Huffin Gas", fuckedup);
	AddOption("Wobbly", wobbly);
	AddOption("Drunk", drunk);
	AddOption("Heaven", heaven);
	AddOption("3D", threedim);
	AddOption("Killstreak", killstreak);
	AddOption("Hallucinations(Jimmy's Drug)", hallu);
	AddOption("Low Quality", lq);
	AddOption("Blurry", blurry);
	AddOption("Fucked Up White Screen", white);

	if (night) GRAPHICS::SET_NIGHTVISION(true);
	if (!night)  GRAPHICS::SET_NIGHTVISION(false);
	if (heat) GRAPHICS::SET_SEETHROUGH(true);
	if (!heat)  GRAPHICS::SET_SEETHROUGH(false);

	if (defaulte)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("DEFAULT");
	}
	if (stoned)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("stoned");
	}
	if (orange)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("REDMIST");
	}
	if (red)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("DEATH");
	}
	if (coke)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("drug_flying_base");
	}
	if (fuckedup)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("DRUG_gas_huffin");
	}
	if (wobbly)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("drug_wobbly");
	}
	if (drunk)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("Drunk");
	}
	if (heaven)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("Bloom");
	}
	if (hallu)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("player_transition");
	}
	if (threedim)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("PlayerSwitchPulse");
	}
	if (killstreak)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("MP_Killstreak");
	}
	if (lq)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("cinema_001");
	}
	if (blurry)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("CHOP");
	}
	if (white)
	{
		GRAPHICS::SET_TIMECYCLE_MODIFIER("BarryFadeOut");
	}
}
#pragma region npcs
static struct {
	char* name;
	char* gameName;
} npcs[687] = {
	{ "Cop", "s_m_y_cop_01" },
	{ "Movspace", "s_m_m_movspace_01" },
	{ "Imporage", "u_m_y_imporage" },
	{ "Rsranger", "u_m_y_rsranger_01" },
	{ "Justin", "u_m_y_justin" },
	{ "Mani", "u_m_y_mani" },
	{ "Michael", "player_zero" },
	{ "Franklin", "player_one" },
	{ "Trevor", "player_two" },
	{ "Boar", "a_c_boar" },
	{ "Chimp", "a_c_chimp" },
	{ "Cow", "a_c_cow" },
	{ "Coyote", "a_c_coyote" },
	{ "Deer", "a_c_deer" },
	{ "Fish", "a_c_fish" },
	{ "Hen", "a_c_hen" },
	{ "Cat", "a_c_cat_01" },
	{ "Hawk", "a_c_chickenhawk" },
	{ "Cormorant", "a_c_cormorant" },
	{ "Crow", "a_c_crow" },
	{ "Dolphin", "a_c_dolphin" },
	{ "Humpback", "a_c_humpback" },
	{ "Whale", "a_c_killerwhale" },
	{ "Pigeon", "a_c_pigeon" },
	{ "Seagull", "a_c_seagull" },
	{ "Sharkhammer", "a_c_sharkhammer" },
	{ "Pig", "a_c_pig" },
	{ "Rat", "a_c_rat" },
	{ "Rhesus", "a_c_rhesus" },
	{ "Chop", "a_c_chop" },
	{ "Husky", "a_c_husky" },
	{ "Mtlion", "a_c_mtlion" },
	{ "Retriever", "a_c_retriever" },
	{ "Sharktiger", "a_c_sharktiger" },
	{ "Shepherd", "a_c_shepherd" },
	{ "Alien", "s_m_m_movalien_01" },
	{ "Beach", "a_f_m_beach_01" },
	{ "Bevhills", "a_f_m_bevhills_01" },
	{ "Bevhills", "a_f_m_bevhills_02" },
	{ "Bodybuild", "a_f_m_bodybuild_01" },
	{ "Business", "a_f_m_business_02" },
	{ "Downtown", "a_f_m_downtown_01" },
	{ "Eastsa", "a_f_m_eastsa_01" },
	{ "Eastsa", "a_f_m_eastsa_02" },
	{ "Fatbla", "a_f_m_fatbla_01" },
	{ "Fatcult", "a_f_m_fatcult_01" },
	{ "Fatwhite", "a_f_m_fatwhite_01" },
	{ "Ktown", "a_f_m_ktown_01" },
	{ "Ktown", "a_f_m_ktown_02" },
	{ "Prolhost", "a_f_m_prolhost_01" },
	{ "Salton", "a_f_m_salton_01" },
	{ "Skidrow", "a_f_m_skidrow_01" },
	{ "Soucentmc", "a_f_m_soucentmc_01" },
	{ "Soucent", "a_f_m_soucent_01" },
	{ "Soucent", "a_f_m_soucent_02" },
	{ "Tourist", "a_f_m_tourist_01" },
	{ "Trampbeac", "a_f_m_trampbeac_01" },
	{ "Tramp", "a_f_m_tramp_01" },
	{ "Genstreet", "a_f_o_genstreet_01" },
	{ "Indian", "a_f_o_indian_01" },
	{ "Ktown", "a_f_o_ktown_01" },
	{ "Salton", "a_f_o_salton_01" },
	{ "Soucent", "a_f_o_soucent_01" },
	{ "Soucent", "a_f_o_soucent_02" },
	{ "Beach", "a_f_y_beach_01" },
	{ "Bevhills", "a_f_y_bevhills_01" },
	{ "Bevhills", "a_f_y_bevhills_02" },
	{ "Bevhills", "a_f_y_bevhills_03" },
	{ "Bevhills", "a_f_y_bevhills_04" },
	{ "Business", "a_f_y_business_01" },
	{ "Business", "a_f_y_business_02" },
	{ "Business", "a_f_y_business_03" },
	{ "Business", "a_f_y_business_04" },
	{ "Eastsa", "a_f_y_eastsa_01" },
	{ "Eastsa", "a_f_y_eastsa_02" },
	{ "Eastsa", "a_f_y_eastsa_03" },
	{ "Epsilon", "a_f_y_epsilon_01" },
	{ "Fitness", "a_f_y_fitness_01" },
	{ "Fitness", "a_f_y_fitness_02" },
	{ "Genhot", "a_f_y_genhot_01" },
	{ "Golfer", "a_f_y_golfer_01" },
	{ "Hiker", "a_f_y_hiker_01" },
	{ "Hippie", "a_f_y_hippie_01" },
	{ "Hipster", "a_f_y_hipster_01" },
	{ "Hipster", "a_f_y_hipster_02" },
	{ "Hipster", "a_f_y_hipster_03" },
	{ "Hipster", "a_f_y_hipster_04" },
	{ "Indian", "a_f_y_indian_01" },
	{ "Juggalo", "a_f_y_juggalo_01" },
	{ "Runner", "a_f_y_runner_01" },
	{ "Rurmeth", "a_f_y_rurmeth_01" },
	{ "Scdressy", "a_f_y_scdressy_01" },
	{ "Skater", "a_f_y_skater_01" },
	{ "Soucent", "a_f_y_soucent_01" },
	{ "Soucent", "a_f_y_soucent_02" },
	{ "Soucent", "a_f_y_soucent_03" },
	{ "Tennis", "a_f_y_tennis_01" },
	{ "Topless", "a_f_y_topless_01" },
	{ "Tourist", "a_f_y_tourist_01" },
	{ "Tourist", "a_f_y_tourist_02" },
	{ "Vinewood", "a_f_y_vinewood_01" },
	{ "Vinewood", "a_f_y_vinewood_02" },
	{ "Vinewood", "a_f_y_vinewood_03" },
	{ "Vinewood", "a_f_y_vinewood_04" },
	{ "Yoga", "a_f_y_yoga_01" },
	{ "Acult", "a_m_m_acult_01" },
	{ "Afriamer", "a_m_m_afriamer_01" },
	{ "Beach", "a_m_m_beach_01" },
	{ "Beach", "a_m_m_beach_02" },
	{ "Bevhills", "a_m_m_bevhills_01" },
	{ "Bevhills", "a_m_m_bevhills_02" },
	{ "Business", "a_m_m_business_01" },
	{ "Eastsa", "a_m_m_eastsa_01" },
	{ "Eastsa", "a_m_m_eastsa_02" },
	{ "Farmer", "a_m_m_farmer_01" },
	{ "Fatlatin", "a_m_m_fatlatin_01" },
	{ "Genfat", "a_m_m_genfat_01" },
	{ "Genfat", "a_m_m_genfat_02" },
	{ "Golfer", "a_m_m_golfer_01" },
	{ "Hasjew", "a_m_m_hasjew_01" },
	{ "Hillbilly", "a_m_m_hillbilly_01" },
	{ "Hillbilly", "a_m_m_hillbilly_02" },
	{ "Indian", "a_m_m_indian_01" },
	{ "Ktown", "a_m_m_ktown_01" },
	{ "Malibu", "a_m_m_malibu_01" },
	{ "Mexcntry", "a_m_m_mexcntry_01" },
	{ "Mexlabor", "a_m_m_mexlabor_01" },
	{ "Og_boss", "a_m_m_og_boss_01" },
	{ "Paparazzi", "a_m_m_paparazzi_01" },
	{ "Polynesian", "a_m_m_polynesian_01" },
	{ "Prolhost", "a_m_m_prolhost_01" },
	{ "Rurmeth", "a_m_m_rurmeth_01" },
	{ "Salton", "a_m_m_salton_01" },
	{ "Salton", "a_m_m_salton_02" },
	{ "Salton", "a_m_m_salton_03" },
	{ "Salton", "a_m_m_salton_04" },
	{ "Skater", "a_m_m_skater_01" },
	{ "Skidrow", "a_m_m_skidrow_01" },
	{ "Socenlat", "a_m_m_socenlat_01" },
	{ "Soucent", "a_m_m_soucent_01" },
	{ "Soucent", "a_m_m_soucent_02" },
	{ "Soucent", "a_m_m_soucent_03" },
	{ "Soucent", "a_m_m_soucent_04" },
	{ "Stlat", "a_m_m_stlat_02" },
	{ "Tennis", "a_m_m_tennis_01" },
	{ "Tourist", "a_m_m_tourist_01" },
	{ "Trampbeac", "a_m_m_trampbeac_01" },
	{ "Tramp", "a_m_m_tramp_01" },
	{ "Tranvest", "a_m_m_tranvest_01" },
	{ "Tranvest", "a_m_m_tranvest_02" },
	{ "Acult", "a_m_o_acult_01" },
	{ "Acult", "a_m_o_acult_02" },
	{ "Beach", "a_m_o_beach_01" },
	{ "Genstreet", "a_m_o_genstreet_01" },
	{ "Ktown", "a_m_o_ktown_01" },
	{ "Salton", "a_m_o_salton_01" },
	{ "Soucent", "a_m_o_soucent_01" },
	{ "Soucent", "a_m_o_soucent_02" },
	{ "Soucent", "a_m_o_soucent_03" },
	{ "Tramp", "a_m_o_tramp_01" },
	{ "Acult", "a_m_y_acult_01" },
	{ "Acult", "a_m_y_acult_02" },
	{ "Beachvesp", "a_m_y_beachvesp_01" },
	{ "Beachvesp", "a_m_y_beachvesp_02" },
	{ "Beach", "a_m_y_beach_01" },
	{ "Beach", "a_m_y_beach_02" },
	{ "Beach", "a_m_y_beach_03" },
	{ "Bevhills", "a_m_y_bevhills_01" },
	{ "Bevhills", "a_m_y_bevhills_02" },
	{ "Breakdance", "a_m_y_breakdance_01" },
	{ "Busicas", "a_m_y_busicas_01" },
	{ "Business", "a_m_y_business_01" },
	{ "Business", "a_m_y_business_02" },
	{ "Business", "a_m_y_business_03" },
	{ "Cyclist", "a_m_y_cyclist_01" },
	{ "Dhill", "a_m_y_dhill_01" },
	{ "Downtown", "a_m_y_downtown_01" },
	{ "Eastsa", "a_m_y_eastsa_01" },
	{ "Eastsa", "a_m_y_eastsa_02" },
	{ "Epsilon", "a_m_y_epsilon_01" },
	{ "Epsilon", "a_m_y_epsilon_02" },
	{ "Gay", "a_m_y_gay_01" },
	{ "Gay", "a_m_y_gay_02" },
	{ "Genstreet", "a_m_y_genstreet_01" },
	{ "Genstreet", "a_m_y_genstreet_02" },
	{ "Golfer", "a_m_y_golfer_01" },
	{ "Hasjew", "a_m_y_hasjew_01" },
	{ "Hiker", "a_m_y_hiker_01" },
	{ "Hippy", "a_m_y_hippy_01" },
	{ "Hipster", "a_m_y_hipster_01" },
	{ "Hipster", "a_m_y_hipster_02" },
	{ "Hipster", "a_m_y_hipster_03" },
	{ "Indian", "a_m_y_indian_01" },
	{ "Jetski", "a_m_y_jetski_01" },
	{ "Juggalo", "a_m_y_juggalo_01" },
	{ "Ktown", "a_m_y_ktown_01" },
	{ "Ktown", "a_m_y_ktown_02" },
	{ "Latino", "a_m_y_latino_01" },
	{ "Methhead", "a_m_y_methhead_01" },
	{ "Mexthug", "a_m_y_mexthug_01" },
	{ "Motox", "a_m_y_motox_01" },
	{ "Motox", "a_m_y_motox_02" },
	{ "Musclbeac", "a_m_y_musclbeac_01" },
	{ "Musclbeac", "a_m_y_musclbeac_02" },
	{ "Polynesian", "a_m_y_polynesian_01" },
	{ "Roadcyc", "a_m_y_roadcyc_01" },
	{ "Runner", "a_m_y_runner_01" },
	{ "Runner", "a_m_y_runner_02" },
	{ "Salton", "a_m_y_salton_01" },
	{ "Skater", "a_m_y_skater_01" },
	{ "Skater", "a_m_y_skater_02" },
	{ "Soucent", "a_m_y_soucent_01" },
	{ "Soucent", "a_m_y_soucent_02" },
	{ "Soucent", "a_m_y_soucent_03" },
	{ "Soucent", "a_m_y_soucent_04" },
	{ "Stbla", "a_m_y_stbla_01" },
	{ "Stbla", "a_m_y_stbla_02" },
	{ "Stlat", "a_m_y_stlat_01" },
	{ "Stwhi", "a_m_y_stwhi_01" },
	{ "Stwhi", "a_m_y_stwhi_02" },
	{ "Sunbathe", "a_m_y_sunbathe_01" },
	{ "Surfer", "a_m_y_surfer_01" },
	{ "Vindouche", "a_m_y_vindouche_01" },
	{ "Vinewood", "a_m_y_vinewood_01" },
	{ "Vinewood", "a_m_y_vinewood_02" },
	{ "Vinewood", "a_m_y_vinewood_03" },
	{ "Vinewood", "a_m_y_vinewood_04" },
	{ "Yoga", "a_m_y_yoga_01" },
	{ "Proldriver", "u_m_y_proldriver_01" },
	{ "Sbike", "u_m_y_sbike" },
	{ "Staggrm", "u_m_y_staggrm_01" },
	{ "Tattoo", "u_m_y_tattoo_01" },
	{ "Abigail", "csb_abigail" },
	{ "Anita", "csb_anita" },
	{ "Anton", "csb_anton" },
	{ "Ballasog", "csb_ballasog" },
	{ "Bride", "csb_bride" },
	{ "Burgerdrug", "csb_burgerdrug" },
	{ "Car3guy1", "csb_car3guy1" },
	{ "Car3guy2", "csb_car3guy2" },
	{ "Chef", "csb_chef" },
	{ "Chin_goon", "csb_chin_goon" },
	{ "Cletus", "csb_cletus" },
	{ "Cop", "csb_cop" },
	{ "Customer", "csb_customer" },
	{ "Denise_friend", "csb_denise_friend" },
	{ "Fos_rep", "csb_fos_rep" },
	{ "G", "csb_g" },
	{ "Groom", "csb_groom" },
	{ "Dlr", "csb_grove_str_dlr" },
	{ "Hao", "csb_hao" },
	{ "Hugh", "csb_hugh" },
	{ "Imran", "csb_imran" },
	{ "Janitor", "csb_janitor" },
	{ "Maude", "csb_maude" },
	{ "Mweather", "csb_mweather" },
	{ "Ortega", "csb_ortega" },
	{ "Oscar", "csb_oscar" },
	{ "Porndudes", "csb_porndudes" },
	{ "Porndudes_p", "csb_porndudes_p" },
	{ "Prologuedriver", "csb_prologuedriver" },
	{ "Prolsec", "csb_prolsec" },
	{ "Gang", "csb_ramp_gang" },
	{ "Hic", "csb_ramp_hic" },
	{ "Hipster", "csb_ramp_hipster" },
	{ "Marine", "csb_ramp_marine" },
	{ "Mex", "csb_ramp_mex" },
	{ "Reporter", "csb_reporter" },
	{ "Roccopelosi", "csb_roccopelosi" },
	{ "Screen_writer", "csb_screen_writer" },
	{ "Stripper", "csb_stripper_01" },
	{ "Stripper", "csb_stripper_02" },
	{ "Tonya", "csb_tonya" },
	{ "Trafficwarden", "csb_trafficwarden" },
	{ "Amandatownley", "cs_amandatownley" },
	{ "Andreas", "cs_andreas" },
	{ "Ashley", "cs_ashley" },
	{ "Bankman", "cs_bankman" },
	{ "Barry", "cs_barry" },
	{ "Barry_p", "cs_barry_p" },
	{ "Beverly", "cs_beverly" },
	{ "Beverly_p", "cs_beverly_p" },
	{ "Brad", "cs_brad" },
	{ "Bradcadaver", "cs_bradcadaver" },
	{ "Carbuyer", "cs_carbuyer" },
	{ "Casey", "cs_casey" },
	{ "Chengsr", "cs_chengsr" },
	{ "Chrisformage", "cs_chrisformage" },
	{ "Clay", "cs_clay" },
	{ "Dale", "cs_dale" },
	{ "Davenorton", "cs_davenorton" },
	{ "Debra", "cs_debra" },
	{ "Denise", "cs_denise" },
	{ "Devin", "cs_devin" },
	{ "Dom", "cs_dom" },
	{ "Dreyfuss", "cs_dreyfuss" },
	{ "Drfriedlander", "cs_drfriedlander" },
	{ "Fabien", "cs_fabien" },
	{ "Fbisuit", "cs_fbisuit_01" },
	{ "Floyd", "cs_floyd" },
	{ "Guadalope", "cs_guadalope" },
	{ "Gurk", "cs_gurk" },
	{ "Hunter", "cs_hunter" },
	{ "Janet", "cs_janet" },
	{ "Jewelass", "cs_jewelass" },
	{ "Jimmyboston", "cs_jimmyboston" },
	{ "Jimmydisanto", "cs_jimmydisanto" },
	{ "Joeminuteman", "cs_joeminuteman" },
	{ "Johnnyklebitz", "cs_johnnyklebitz" },
	{ "Josef", "cs_josef" },
	{ "Josh", "cs_josh" },
	{ "Lamardavis", "cs_lamardavis" },
	{ "Lazlow", "cs_lazlow" },
	{ "Lestercrest", "cs_lestercrest" },
	{ "Lifeinvad", "cs_lifeinvad_01" },
	{ "Magenta", "cs_magenta" },
	{ "Manuel", "cs_manuel" },
	{ "Marnie", "cs_marnie" },
	{ "Martinmadrazo", "cs_martinmadrazo" },
	{ "Maryann", "cs_maryann" },
	{ "Michelle", "cs_michelle" },
	{ "Milton", "cs_milton" },
	{ "Molly", "cs_molly" },
	{ "Movpremf", "cs_movpremf_01" },
	{ "Movpremmale", "cs_movpremmale" },
	{ "Mrk", "cs_mrk" },
	{ "Mrsphillips", "cs_mrsphillips" },
	{ "Mrs_thornhill", "cs_mrs_thornhill" },
	{ "Natalia", "cs_natalia" },
	{ "Nervousron", "cs_nervousron" },
	{ "Nigel", "cs_nigel" },
	{ "Old_man1a", "cs_old_man1a" },
	{ "Old_man2", "cs_old_man2" },
	{ "Omega", "cs_omega" },
	{ "Orleans", "cs_orleans" },
	{ "Paper", "cs_paper" },
	{ "Paper_p", "cs_paper_p" },
	{ "Patricia", "cs_patricia" },
	{ "Priest", "cs_priest" },
	{ "Prolsec", "cs_prolsec_02" },
	{ "Russiandrunk", "cs_russiandrunk" },
	{ "Siemonyetarian", "cs_siemonyetarian" },
	{ "Solomon", "cs_solomon" },
	{ "Stevehains", "cs_stevehains" },
	{ "Stretch", "cs_stretch" },
	{ "Tanisha", "cs_tanisha" },
	{ "Taocheng", "cs_taocheng" },
	{ "Taostranslator", "cs_taostranslator" },
	{ "Tenniscoach", "cs_tenniscoach" },
	{ "Terry", "cs_terry" },
	{ "Tom", "cs_tom" },
	{ "Tomepsilon", "cs_tomepsilon" },
	{ "Tracydisanto", "cs_tracydisanto" },
	{ "Wade", "cs_wade" },
	{ "Zimbor", "cs_zimbor" },
	{ "Ballas", "g_f_y_ballas_01" },
	{ "Families", "g_f_y_families_01" },
	{ "Lost", "g_f_y_lost_01" },
	{ "Vagos", "g_f_y_vagos_01" },
	{ "Armboss", "g_m_m_armboss_01" },
	{ "Armgoon", "g_m_m_armgoon_01" },
	{ "Armlieut", "g_m_m_armlieut_01" },
	{ "Chemwork", "g_m_m_chemwork_01" },
	{ "Chemwork_p", "g_m_m_chemwork_01_p" },
	{ "Chiboss", "g_m_m_chiboss_01" },
	{ "Chiboss_p", "g_m_m_chiboss_01_p" },
	{ "Chicold", "g_m_m_chicold_01" },
	{ "Chicold_p", "g_m_m_chicold_01_p" },
	{ "Chigoon", "g_m_m_chigoon_01" },
	{ "Chigoon_p", "g_m_m_chigoon_01_p" },
	{ "Chigoon", "g_m_m_chigoon_02" },
	{ "Korboss", "g_m_m_korboss_01" },
	{ "Mexboss", "g_m_m_mexboss_01" },
	{ "Mexboss", "g_m_m_mexboss_02" },
	{ "Armgoon", "g_m_y_armgoon_02" },
	{ "Azteca", "g_m_y_azteca_01" },
	{ "Ballaeast", "g_m_y_ballaeast_01" },
	{ "Ballaorig", "g_m_y_ballaorig_01" },
	{ "Ballasout", "g_m_y_ballasout_01" },
	{ "Famca", "g_m_y_famca_01" },
	{ "Famdnf", "g_m_y_famdnf_01" },
	{ "Famfor", "g_m_y_famfor_01" },
	{ "Korean", "g_m_y_korean_01" },
	{ "Korean", "g_m_y_korean_02" },
	{ "Korlieut", "g_m_y_korlieut_01" },
	{ "Lost", "g_m_y_lost_01" },
	{ "Lost", "g_m_y_lost_02" },
	{ "Lost", "g_m_y_lost_03" },
	{ "Mexgang", "g_m_y_mexgang_01" },
	{ "Mexgoon", "g_m_y_mexgoon_01" },
	{ "Mexgoon", "g_m_y_mexgoon_02" },
	{ "Mexgoon", "g_m_y_mexgoon_03" },
	{ "Mexgoon_p", "g_m_y_mexgoon_03_p" },
	{ "Pologoon", "g_m_y_pologoon_01" },
	{ "Pologoon_p", "g_m_y_pologoon_01_p" },
	{ "Pologoon", "g_m_y_pologoon_02" },
	{ "Pologoon_p", "g_m_y_pologoon_02_p" },
	{ "Salvaboss", "g_m_y_salvaboss_01" },
	{ "Salvagoon", "g_m_y_salvagoon_01" },
	{ "Salvagoon", "g_m_y_salvagoon_02" },
	{ "Salvagoon", "g_m_y_salvagoon_03" },
	{ "Salvagoon_p", "g_m_y_salvagoon_03_p" },
	{ "Strpunk", "g_m_y_strpunk_01" },
	{ "Strpunk", "g_m_y_strpunk_02" },
	{ "Hc_driver", "hc_driver" },
	{ "Hc_gunman", "hc_gunman" },
	{ "Hc_hacker", "hc_hacker" },
	{ "Abigail", "ig_abigail" },
	{ "Amandatownley", "ig_amandatownley" },
	{ "Andreas", "ig_andreas" },
	{ "Ashley", "ig_ashley" },
	{ "Ballasog", "ig_ballasog" },
	{ "Bankman", "ig_bankman" },
	{ "Barry", "ig_barry" },
	{ "Barry_p", "ig_barry_p" },
	{ "Bestmen", "ig_bestmen" },
	{ "Beverly", "ig_beverly" },
	{ "Beverly_p", "ig_beverly_p" },
	{ "Brad", "ig_brad" },
	{ "Bride", "ig_bride" },
	{ "Car3guy1", "ig_car3guy1" },
	{ "Car3guy2", "ig_car3guy2" },
	{ "Casey", "ig_casey" },
	{ "Chef", "ig_chef" },
	{ "Chengsr", "ig_chengsr" },
	{ "Chrisformage", "ig_chrisformage" },
	{ "Clay", "ig_clay" },
	{ "Claypain", "ig_claypain" },
	{ "Cletus", "ig_cletus" },
	{ "Dale", "ig_dale" },
	{ "Davenorton", "ig_davenorton" },
	{ "Denise", "ig_denise" },
	{ "Devin", "ig_devin" },
	{ "Dom", "ig_dom" },
	{ "Dreyfuss", "ig_dreyfuss" },
	{ "Drfriedlander", "ig_drfriedlander" },
	{ "Fabien", "ig_fabien" },
	{ "Fbisuit", "ig_fbisuit_01" },
	{ "Floyd", "ig_floyd" },
	{ "Groom", "ig_groom" },
	{ "Hao", "ig_hao" },
	{ "Hunter", "ig_hunter" },
	{ "Janet", "ig_janet" },
	{ "Jay_norris", "ig_jay_norris" },
	{ "Jewelass", "ig_jewelass" },
	{ "Jimmyboston", "ig_jimmyboston" },
	{ "Jimmydisanto", "ig_jimmydisanto" },
	{ "Joeminuteman", "ig_joeminuteman" },
	{ "Johnnyklebitz", "ig_johnnyklebitz" },
	{ "Josef", "ig_josef" },
	{ "Josh", "ig_josh" },
	{ "Kerrymcintosh", "ig_kerrymcintosh" },
	{ "Lamardavis", "ig_lamardavis" },
	{ "Lazlow", "ig_lazlow" },
	{ "Lestercrest", "ig_lestercrest" },
	{ "Lifeinvad", "ig_lifeinvad_01" },
	{ "Lifeinvad", "ig_lifeinvad_02" },
	{ "Magenta", "ig_magenta" },
	{ "Manuel", "ig_manuel" },
	{ "Marnie", "ig_marnie" },
	{ "Maryann", "ig_maryann" },
	{ "Maude", "ig_maude" },
	{ "Michelle", "ig_michelle" },
	{ "Milton", "ig_milton" },
	{ "Molly", "ig_molly" },
	{ "Mrk", "ig_mrk" },
	{ "Mrsphillips", "ig_mrsphillips" },
	{ "Mrs_thornhill", "ig_mrs_thornhill" },
	{ "Natalia", "ig_natalia" },
	{ "Nervousron", "ig_nervousron" },
	{ "Nigel", "ig_nigel" },
	{ "Old_man1a", "ig_old_man1a" },
	{ "Old_man2", "ig_old_man2" },
	{ "Omega", "ig_omega" },
	{ "Oneil", "ig_oneil" },
	{ "Orleans", "ig_orleans" },
	{ "Ortega", "ig_ortega" },
	{ "Paper", "ig_paper" },
	{ "Patricia", "ig_patricia" },
	{ "Priest", "ig_priest" },
	{ "Prolsec", "ig_prolsec_02" },
	{ "Gang", "ig_ramp_gang" },
	{ "Hic", "ig_ramp_hic" },
	{ "Hipster", "ig_ramp_hipster" },
	{ "Mex", "ig_ramp_mex" },
	{ "Roccopelosi", "ig_roccopelosi" },
	{ "Russiandrunk", "ig_russiandrunk" },
	{ "Screen_writer", "ig_screen_writer" },
	{ "Siemonyetarian", "ig_siemonyetarian" },
	{ "Solomon", "ig_solomon" },
	{ "Stevehains", "ig_stevehains" },
	{ "Stretch", "ig_stretch" },
	{ "Talina", "ig_talina" },
	{ "Tanisha", "ig_tanisha" },
	{ "Taocheng", "ig_taocheng" },
	{ "Taostranslator", "ig_taostranslator" },
	{ "Taostranslator_p", "ig_taostranslator_p" },
	{ "Tenniscoach", "ig_tenniscoach" },
	{ "Terry", "ig_terry" },
	{ "Tomepsilon", "ig_tomepsilon" },
	{ "Tonya", "ig_tonya" },
	{ "Tracydisanto", "ig_tracydisanto" },
	{ "Trafficwarden", "ig_trafficwarden" },
	{ "Tylerdix", "ig_tylerdix" },
	{ "Wade", "ig_wade" },
	{ "Zimbor", "ig_zimbor" },
	{ "Deadhooker", "mp_f_deadhooker" },
	{ "Freemode", "mp_f_freemode_01" },
	{ "Misty", "mp_f_misty_01" },
	{ "Stripperlite", "mp_f_stripperlite" },
	{ "Pros", "mp_g_m_pros_01" },
	{ "Mp_headtargets", "mp_headtargets" },
	{ "Claude", "mp_m_claude_01" },
	{ "Exarmy", "mp_m_exarmy_01" },
	{ "Famdd", "mp_m_famdd_01" },
	{ "Fibsec", "mp_m_fibsec_01" },
	{ "Freemode", "mp_m_freemode_01" },
	{ "Marston", "mp_m_marston_01" },
	{ "Niko", "mp_m_niko_01" },
	{ "Shopkeep", "mp_m_shopkeep_01" },
	{ "Armoured", "mp_s_m_armoured_01" },
	{ "Fembarber", "s_f_m_fembarber" },
	{ "Maid", "s_f_m_maid_01" },
	{ "Shop_high", "s_f_m_shop_high" },
	{ "Sweatshop", "s_f_m_sweatshop_01" },
	{ "Airhostess", "s_f_y_airhostess_01" },
	{ "Bartender", "s_f_y_bartender_01" },
	{ "Baywatch", "s_f_y_baywatch_01" },
	{ "Cop", "s_f_y_cop_01" },
	{ "Factory", "s_f_y_factory_01" },
	{ "Hooker", "s_f_y_hooker_01" },
	{ "Hooker", "s_f_y_hooker_02" },
	{ "Hooker", "s_f_y_hooker_03" },
	{ "Migrant", "s_f_y_migrant_01" },
	{ "Movprem", "s_f_y_movprem_01" },
	{ "Ranger", "s_f_y_ranger_01" },
	{ "Scrubs", "s_f_y_scrubs_01" },
	{ "Sheriff", "s_f_y_sheriff_01" },
	{ "Shop_low", "s_f_y_shop_low" },
	{ "Shop_mid", "s_f_y_shop_mid" },
	{ "Stripperlite", "s_f_y_stripperlite" },
	{ "Stripper", "s_f_y_stripper_01" },
	{ "Stripper", "s_f_y_stripper_02" },
	{ "Sweatshop", "s_f_y_sweatshop_01" },
	{ "Ammucountry", "s_m_m_ammucountry" },
	{ "Armoured", "s_m_m_armoured_01" },
	{ "Armoured", "s_m_m_armoured_02" },
	{ "Autoshop", "s_m_m_autoshop_01" },
	{ "Autoshop", "s_m_m_autoshop_02" },
	{ "Bouncer", "s_m_m_bouncer_01" },
	{ "Chemsec", "s_m_m_chemsec_01" },
	{ "Ciasec", "s_m_m_ciasec_01" },
	{ "Cntrybar", "s_m_m_cntrybar_01" },
	{ "Dockwork", "s_m_m_dockwork_01" },
	{ "Doctor", "s_m_m_doctor_01" },
	{ "Fiboffice", "s_m_m_fiboffice_01" },
	{ "Fiboffice", "s_m_m_fiboffice_02" },
	{ "Gaffer", "s_m_m_gaffer_01" },
	{ "Gardener", "s_m_m_gardener_01" },
	{ "Gentransport", "s_m_m_gentransport" },
	{ "Hairdress", "s_m_m_hairdress_01" },
	{ "Highsec", "s_m_m_highsec_01" },
	{ "Highsec", "s_m_m_highsec_02" },
	{ "Janitor", "s_m_m_janitor" },
	{ "Lathandy", "s_m_m_lathandy_01" },
	{ "Lifeinvad", "s_m_m_lifeinvad_01" },
	{ "Linecook", "s_m_m_linecook" },
	{ "Lsmetro", "s_m_m_lsmetro_01" },
	{ "Mariachi", "s_m_m_mariachi_01" },
	{ "Marine", "s_m_m_marine_01" },
	{ "Marine", "s_m_m_marine_02" },
	{ "Migrant", "s_m_m_migrant_01" },
	{ "Zombie", "u_m_y_zombie_01" },
	{ "Movprem", "s_m_m_movprem_01" },
	{ "Paramedic", "s_m_m_paramedic_01" },
	{ "Pilot", "s_m_m_pilot_01" },
	{ "Pilot", "s_m_m_pilot_02" },
	{ "Postal", "s_m_m_postal_01" },
	{ "Postal", "s_m_m_postal_02" },
	{ "Prisguard", "s_m_m_prisguard_01" },
	{ "Scientist", "s_m_m_scientist_01" },
	{ "Security", "s_m_m_security_01" },
	{ "Snowcop", "s_m_m_snowcop_01" },
	{ "Strperf", "s_m_m_strperf_01" },
	{ "Strpreach", "s_m_m_strpreach_01" },
	{ "Strvend", "s_m_m_strvend_01" },
	{ "Trucker", "s_m_m_trucker_01" },
	{ "Ups", "s_m_m_ups_01" },
	{ "Ups", "s_m_m_ups_02" },
	{ "Busker", "s_m_o_busker_01" },
	{ "Airworker", "s_m_y_airworker" },
	{ "Ammucity", "s_m_y_ammucity_01" },
	{ "Armymech", "s_m_y_armymech_01" },
	{ "Autopsy", "s_m_y_autopsy_01" },
	{ "Barman", "s_m_y_barman_01" },
	{ "Baywatch", "s_m_y_baywatch_01" },
	{ "Blackops", "s_m_y_blackops_01" },
	{ "Blackops", "s_m_y_blackops_02" },
	{ "Busboy", "s_m_y_busboy_01" },
	{ "Chef", "s_m_y_chef_01" },
	{ "Clown", "s_m_y_clown_01" },
	{ "Construct", "s_m_y_construct_01" },
	{ "Construct", "s_m_y_construct_02" },

	{ "Dealer", "s_m_y_dealer_01" },
	{ "Devinsec", "s_m_y_devinsec_01" },
	{ "Dockwork", "s_m_y_dockwork_01" },
	{ "Doorman", "s_m_y_doorman_01" },
	{ "Dwservice", "s_m_y_dwservice_01" },
	{ "Dwservice", "s_m_y_dwservice_02" },
	{ "Factory", "s_m_y_factory_01" },
	{ "Fireman", "s_m_y_fireman_01" },
	{ "Garbage", "s_m_y_garbage" },
	{ "Grip", "s_m_y_grip_01" },
	{ "Hwaycop", "s_m_y_hwaycop_01" },
	{ "Marine", "s_m_y_marine_01" },
	{ "Marine", "s_m_y_marine_02" },
	{ "Marine", "s_m_y_marine_03" },
	{ "Mime", "s_m_y_mime" },
	{ "Pestcont", "s_m_y_pestcont_01" },
	{ "Pilot", "s_m_y_pilot_01" },
	{ "Prismuscl", "s_m_y_prismuscl_01" },
	{ "Prisoner", "s_m_y_prisoner_01" },
	{ "Ranger", "s_m_y_ranger_01" },
	{ "Robber", "s_m_y_robber_01" },
	{ "Sheriff", "s_m_y_sheriff_01" },
	{ "Shop_mask", "s_m_y_shop_mask" },
	{ "Strvend", "s_m_y_strvend_01" },
	{ "Swat", "s_m_y_swat_01" },
	{ "Uscg", "s_m_y_uscg_01" },
	{ "Valet", "s_m_y_valet_01" },
	{ "Waiter", "s_m_y_waiter_01" },
	{ "Winclean", "s_m_y_winclean_01" },
	{ "Xmech", "s_m_y_xmech_01" },
	{ "Xmech", "s_m_y_xmech_02" },
	{ "Corpse", "u_f_m_corpse_01" },
	{ "Miranda", "u_f_m_miranda" },
	{ "Promourn", "u_f_m_promourn_01" },
	{ "Moviestar", "u_f_o_moviestar" },
	{ "Prolhost", "u_f_o_prolhost_01" },
	{ "Bikerchic", "u_f_y_bikerchic" },
	{ "Comjane", "u_f_y_comjane" },
	{ "Corpse", "u_f_y_corpse_01" },
	{ "Corpse", "u_f_y_corpse_02" },
	{ "Hotposh", "u_f_y_hotposh_01" },
	{ "Jewelass", "u_f_y_jewelass_01" },
	{ "Mistress", "u_f_y_mistress" },
	{ "Poppymich", "u_f_y_poppymich" },
	{ "Princess", "u_f_y_princess" },
	{ "Spyactress", "u_f_y_spyactress" },
	{ "Aldinapoli", "u_m_m_aldinapoli" },
	{ "Bankman", "u_m_m_bankman" },
	{ "Bikehire", "u_m_m_bikehire_01" },
	{ "Fibarchitect", "u_m_m_fibarchitect" },
	{ "Filmdirector", "u_m_m_filmdirector" },
	{ "Glenstank", "u_m_m_glenstank_01" },
	{ "Griff", "u_m_m_griff_01" },
	{ "Jesus", "u_m_m_jesus_01" },
	{ "Jewelsec", "u_m_m_jewelsec_01" },
	{ "Jewelthief", "u_m_m_jewelthief" },
	{ "Markfost", "u_m_m_markfost" },
	{ "Partytarget", "u_m_m_partytarget" },
	{ "Prolsec", "u_m_m_prolsec_01" },
	{ "Promourn", "u_m_m_promourn_01" },
	{ "Rivalpap", "u_m_m_rivalpap" },
	{ "Spyactor", "u_m_m_spyactor" },
	{ "Willyfist", "u_m_m_willyfist" },
	{ "Finguru", "u_m_o_finguru_01" },
	{ "Taphillbilly", "u_m_o_taphillbilly" },
	{ "Tramp", "u_m_o_tramp_01" },
	{ "Abner", "u_m_y_abner" },
	{ "Antonb", "u_m_y_antonb" },
	{ "Babyd", "u_m_y_babyd" },
	{ "Baygor", "u_m_y_baygor" },
	{ "Burgerdrug", "u_m_y_burgerdrug_01" },
	{ "Chip", "u_m_y_chip" },
	{ "Cyclist", "u_m_y_cyclist_01" },
	{ "Fibmugger", "u_m_y_fibmugger_01" },
	{ "Guido", "u_m_y_guido_01" },
	{ "Gunvend", "u_m_y_gunvend_01" },
	{ "Hippie", "u_m_y_hippie_01" },
	{ "Militarybum", "u_m_y_militarybum" },
	{ "Paparazzi", "u_m_y_paparazzi" },
	{ "Party", "u_m_y_party_01" },
	{ "Pogo", "u_m_y_pogo_01" },
	{ "Prisoner", "u_m_y_prisoner_01" }
};
#pragma endregion
string searchString, skinSearchString, pedSearchString;
Ped latestPed;
bool searchModePed = 0;
void AddNpcSpawn(char* text, char* name, bool &extra_option_code = null)
{
	null = 0;
	AddOption(text, null);
	if (menu::printingop == menu::currentop)
	{
		if (null)
		{
			DWORD model = GAMEPLAY::GET_HASH_KEY(name);
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
			Vector3 coords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0);
			latestPed = PED::CREATE_PED(26, model, coords.x + 0.3f, coords.y, coords.z, 0.0f, 1, 1);
			extra_option_code = true;
		}
	}

}

void searchForPed() {
	if (searchModePed) {
		if (pedSearchString.length() < 1) {
			PrintStringBottomCentre("Invalid Search");
			searchModePed = 0;
			pedSearchString.clear();
		}
		bool reset = 0;
		AddOption("~g~RESET", reset);
		if (reset) {
			pedSearchString.clear();
			searchModePed = 0;
		}
		
	}
	else {
		bool searchfor = 0;
		AddOption("~b~Search for an NPC", searchfor);
		if (searchfor) {
			pedSearchString = string(keyboard());
			searchModePed = 1;
		}
	}
}

void NPCSpawner() {

	bool shrinkE = 0, shrinkD = 0;
	bool LatestShrinked = PED::GET_PED_CONFIG_FLAG(latestPed, 223, 1);
	bool custominput = 0;
	AddTitle("NPC Spawner");
	AddToggle("Shrink latest ped", LatestShrinked, shrinkE, shrinkD);
	if (shrinkE) PED::SET_PED_CONFIG_FLAG(latestPed, 223, 1);
	if (shrinkD) PED::SET_PED_CONFIG_FLAG(latestPed, 223, 0);
	searchForPed();
	//			AddOption("Custom ~b~Input", custominput);
	if (!searchModePed) {
		for (int i = 0; i < 685; i++) {

			AddNpcSpawn(npcs[i].name, npcs[i].gameName);
		}
		if (custominput) {
			DWORD model = GAMEPLAY::GET_HASH_KEY(keyboard());
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
			Vector3 coords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0);
			PED::CREATE_PED(26, model, coords.x + 0.3f, coords.y, coords.z, 0.0f, 1, 1);
		}
	}

}
bool searchModeSkin = 0;
void searchForSkin() {
	if (searchModeSkin) {
		if (skinSearchString.length() < 1) {
			PrintStringBottomCentre("Invalid Search");
			searchModeSkin = 0;
			skinSearchString.clear();
		}
		bool reset = 0;
		AddOption("~g~RESET", reset);
		if (reset) {
			skinSearchString.clear();
			searchModeSkin = 0;
		}
		
	}
	else {
		bool searchfor = 0;
		AddOption("~b~Search for a skin", searchfor);
		if (searchfor) {
			skinSearchString = string(keyboard());
			searchModeSkin = 1;
		}
	}
}
void Morph(char* modelg) {
	DWORD model = GAMEPLAY::GET_HASH_KEY(modelg);
	if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
	{
		STREAMING::REQUEST_MODEL(model);
		while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
		PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(), model);
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
		PED::SET_PED_RANDOM_COMPONENT_VARIATION(PLAYER::PLAYER_PED_ID(), 1);
	}
	else {
		PrintStringBottomCentre("~r~Error~s~: Model not found");
	}
}
void AddSkin(char* text, char* name, bool &extra_option_code = null)
{
	null = 0;
	AddOption(text, null);
	if (menu::printingop == menu::currentop)
	{
		if (null)
		{
			DWORD model = GAMEPLAY::GET_HASH_KEY(name);
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
			Vector3 coords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0);
			Morph(name);
			Ped myped = PLAYER::PLAYER_PED_ID();
			//		Hash railgun = GET_HASH_KEY("WEAPON_RAILGUN");
			//		GIVE_WEAPON_TO_PED(myped, railgun, railgun, 9999, 9999);
			extra_option_code = true;
		}
	}

}
void SkinChanger() {



	bool custom = 0, malec = 0, femec = 0, resetskin = 0;
	AddTitle("Model Changer");
	AddOption("Custom ~b~Input", custom);
	AddOption("~g~RESET SKIN", resetskin);
	AddOption("~b~MALE DEFAULT", malec);
	AddOption("~p~ FEMALE DEFAULT", femec);
	searchForSkin();
	if (custom) {
		Morph(keyboard());
	}

	if (femec)
	{
		DWORD model = GAMEPLAY::GET_HASH_KEY("mp_f_freemode_01");
		Ped playerPed = PLAYER::PLAYER_ID();

		//	DWORD model = GET_HASH_KEY(modelg);
		if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
		{
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
			PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(), model);
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
			// hat
			PED::SET_PED_PROP_INDEX(playerPed, 0, 46, 0, 0);
			//top
			PED::SET_PED_COMPONENT_VARIATION(playerPed, 11, 55, 0, 0);
			//pants
			PED::SET_PED_COMPONENT_VARIATION(playerPed, 4, 35, 0, 0);
			//torso
			PED::SET_PED_COMPONENT_VARIATION(playerPed, 3, 0, 0, 0);
			//shoes
			PED::SET_PED_COMPONENT_VARIATION(playerPed, 6, 24, 0, 0);
			//shoes
			PED::SET_PED_COMPONENT_VARIATION(playerPed, 9, 0, 0, 0);

			//		SET_PED_RANDOM_COMPONENT_VARIATION(playerPed, 1);
			//		SET_PED_DEFAULT_COMPONENT_VARIATION(playerPed);
		}
		else {
			PrintStringBottomCentre("~r~Error~s~: Model not found");
		}
	}
	if (malec)
	{
		DWORD model = GAMEPLAY::GET_HASH_KEY("mp_m_freemode_01");
		Ped playerPed = PLAYER::PLAYER_ID();

		//	DWORD model = GET_HASH_KEY(modelg);
		if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
		{
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
			PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(), model);
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
			//hat
			PED::SET_PED_PROP_INDEX(playerPed, 0, 46, 0, 0);
			//top
			PED::SET_PED_COMPONENT_VARIATION(playerPed, 11, 55, 0, 0);
			//pants
			PED::SET_PED_COMPONENT_VARIATION(playerPed, 4, 35, 0, 0);
			//torso
			PED::SET_PED_COMPONENT_VARIATION(playerPed, 3, 0, 0, 0);
			//shoes
			PED::SET_PED_COMPONENT_VARIATION(playerPed, 6, 24, 0, 0);
			//shoes
			PED::SET_PED_COMPONENT_VARIATION(playerPed, 9, 0, 0, 0);

			//		SET_PED_RANDOM_COMPONENT_VARIATION(playerPed, 1);
			//		SET_PED_DEFAULT_COMPONENT_VARIATION(playerPed);
		}
		else {
			PrintStringBottomCentre("~r~Error~s~: Model not found");
		}
	}
	if (resetskin) {
		PED::SET_PED_DEFAULT_COMPONENT_VARIATION(PLAYER::PLAYER_PED_ID());
		PrintStringBottomCentre("Skin has been ~g~ reset !");
	}
	if (!searchModeSkin) {
		for (int i = 0; i < 685; i++) {
			AddSkin(npcs[i].name, npcs[i].gameName);
		}
	}
	if (custom) {
		Morph(keyboard());
	}
}
#pragma region FavoriteVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} FavoriteVehs[21] = {

	{ "Duke O' Death", "DUKES2" },
	{ "BMX", "BMX" },
	{ "Rhino Tank", "RHINO" },
	{ "Hydra", "HYDRA" },
	{ "Dinka Vindicator", "VINDICATOR" },
	{ "Hakuchou", "HAKUCHOU" },
	{ "Savage Helicopter", "SAVAGE" },
	{ "Valkyrie Helicopter", "VALKYRIE" },
	{ "Pegassi Osiris", "OSIRIS" },
	{ "Progen T20", "T20" },
	{ "Adder", "ADDER" },
	{ "Zentorno", "ZENTORNO" },
	{ "Coil Brawler", "BRAWLER" },
	{ "Alien Dune", "DUNE2" },
	{ "Armed Insurgent", "INSURGENT" },
	{ "Insurgent", "INSURGENT2" },
	{ "Liberator", "MONSTER" },
	{ "Marshall", "MARSHALL" },
	{ "Armored Kuruma", "KURUMA2" },
	{ "BType", "BTYPE" },
	{ "Blimp", "BLIMP" },

};
#pragma endregion
void favoriteClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 21; i++) {
		AddCarSpawn((char*)FavoriteVehs[i].Name, FavoriteVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
//favoriteClass
//SuperClass
#pragma region SuperVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} SuperVehs[17] = {

	{ "Pegassi Osiris", "OSIRIS" },
	{ "Progen T20", "T20" },
	{ "Adder", "ADDER" },
	{ "Zentorno", "ZENTORNO" },
	{ "Entity XF", "ENTITYXF" },
	{ "Infernus", "INFERNUS" },
	{ "Turismo R", "TURISMOR" },
	{ "Bullet", "BULLET" },
	{ "Cheetah", "CHEETAH" },
	{ "Vacca", "VACCA" },
	//{ "Voltic", "VOLTIC" },
	//"", "BANSHEE2", "BULLET", "CHEETAH", "ENTITYXF",
	//"FMJ", "SHEAVA", "INFERNUS", "NERO", "NERO2","OSIRIS", "LE7B",
	//"ITALIGTB", "ITALIGTB2", "PFISTER811", "PROTOTIPO", "REAPER", "SULTANRS", "T20",
	//"TEMPESTA", "TURISMOR", "TYRUS", "VACCA", "VOLTIC", "ZENTORNO", "VOLTIC2", "PENETRATOR", "GP1"
	{ "Banshee", "BANSHEE2" },
	{ "Vapid FMJ", "FMJ" },
	{ "Emperor Sheava", "Sheava" },
	{ "Turfade Nero", "NERO" },
	{ "Turfade Nero(custom)", "NERO2" },
	{ "Rocket Voltic", "VOLTIC2" },
};
#pragma endregion
void supercarClass() {



	bool custominput = 0;
	AddTitle("Anonymous");

	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 11; i++) {
		AddCarSpawn((char*)SuperVehs[i].Name, SuperVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
//SuperClass
#pragma region MuscleVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} MuscleVehs[23] = {

	{ "Duke O' Death", "DUKES2" },
	{ "Blade", "BLADE" },
	{ "Buccaneer", "BUCCANEER" },
	{ "Vapid Chino", "CHINO" },
	{ "Dominator", "DOMINATOR" },
	{ "Dukes", "DUKES" },
	{ "Gauntlet", "GAUNTLET" },
	{ "Gauntlet", "GAUNTLET2" },
	{ "Hotknife", "HOTKNIFE" },
	{ "Phoenix", "PHOENIX" },
	{ "Picador", "PICADOR" },
	{ "Sport Dominator", "DOMINATOR2" },
	{ "Clean Ratloader", "RATLOADER2" },
	{ "Ratloader", "DLOADER" },
	{ "Rat;pader", "RATLOADER" },
	{ "Ruiner", "RUINER" },
	{ "Sabre GT", "SABREGT" },
	{ "Slamvan", "SLAMVAN" },
	{ "Slamvan2", "SLAMVAN2" },
	{ "Vigero", "VIGERO" },
	{ "Albany Virgo", "VIRGO" },
	{ "Voodoo", "VOODOO2" },
	{ "Stalion", "STALION" },

};
#pragma endregion
void muscleClass() {




	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 23; i++) {
		AddCarSpawn((char*)MuscleVehs[i].Name, MuscleVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region OffroadVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} OffroadVehs[21] = {

	{ "Coil Brawler", "BRAWLER" },
	{ "Trevor's Truck", "BODHI2" },
	{ "Dune", "DUNE" },
	{ "Alien Dune", "DUNE2" },
	{ "Armed Insurgent", "INSURGENT" },
	{ "Insurgent", "INSURGENT2" },
	{ "Liberator", "MONSTER" },
	{ "Marshall", "MARSHALL" },
	{ "Bifta", "BIFTA" },
	{ "Dubsta", "DUBSTA3" },
	{ "BF Injection", "BFINJECTION" },
	{ "Kalahari", "KALAHARI" },
	{ "Mesa with grills", "MESA3" },
	{ "Rancher XL", "RANCHERXL" },
	{ "Snowy Rancher XL", "RANCHERXL2" },
	{ "Rusty Rebel", "REBEL" },
	{ "Sandking", "SANDKING" },
	{ "Sandking XL", "SANDKING2" },
	{ "Karin Technical", "TECHNICAL" },
	{ "Karin Rebel", "REBEL2" },
	{ "Radi", "RADI" },

};
#pragma endregion
void offroadsClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 21; i++) {
		AddCarSpawn((char*)OffroadVehs[i].Name, OffroadVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region SportsVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} SportsVehs[32] = {

	{ "Khamelion", "KHAMELION" },
	{ "Kuruma", "KURUMA" },
	{ "Armored Kuruma", "KURUMA2" },
	{ "Massacro", "MASSACRO" },
	{ "Ninef", "NINEF" },
	{ "Coquette", "COQUETTE" },
	{ "Ninef Convertible", "NINEF2" },
	{ "Alpha", "ALPHA" },
	{ "Banshee", "BANSHEE" },
	{ "Blista", "BLISTA" },
	{ "Blista", "BLISTA2" },
	{ "Space Money Blista", "BLISTA3" },
	{ "Buffalo(Old)", "BUFFALO" },
	{ "Buffalo(New)", "BUFFALO2" },
	{ "Sport Buffalo", "BUFFALO3" },
	{ "Carbonizzare", "CARBONIZZARE" },
	{ "Comet", "COMET2" },
	{ "Elegy", "ELEGY2" },
	{ "Feltzer", "FELTZER2" },
	{ "Furore GT", "FUROREGT" },
	{ "Jester", "JESTER" },
	{ "Fusilade", "FUSILADE" },
	{ "Futo", "FUTO" },
	{ "Sport Jester", "JESTER2" },
	{ "Sport Massacro", "MASSACRO2" },
	{ "Penumbra", "PENUMBRA" },
	{ "Rapid GT", "RAPIDGT" },
	{ "Rapid GT Convertible", "RAPIDGT2" },
	{ "Schwarzer", "SCHWARZER" },
	{ "Sultan", "SULTAN" },
	{ "Surano", "SURANO" },
	{ "Sport Stalion", "STALION2" },


};
#pragma endregion
void sportsClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 32; i++) {
		AddCarSpawn((char*)SportsVehs[i].Name, SportsVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region SportsClassicVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} SportsClassicVehs[17] = {

	{ "BType", "BTYPE" },
	{ "ZType", "ZTYPE" },
	{ "Casco", "CASCO" },
	{ "Invetero Coquette", "COQUETTE3" },
	{ "JB700", "JB700" },
	{ "Manana", "MANANA" },
	{ "Monroe", "MONROE" },
	{ "Peyote", "PEYOTE" },
	{ "Pigalle", "PIGALLE" },
	{ "Stinger", "STINGER" },
	{ "Stinger GT", "STINGERGT" },
	{ "Tornado", "TORNADO" },
	{ "Tornado Convertible", "TORNADO2" },
	{ "Rusty Tornado", "TORNADO3" },
	{ "Rusty Tornado 2", "TORNADO4" },
	{ "Benefactor Stirling GT", "FELTZER3" },
	{ "Classic Coquette", "COQUETTE2" },

};
#pragma endregion
void sportsclassicsClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 17; i++) {
		AddCarSpawn((char*)SportsClassicVehs[i].Name, SportsClassicVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region PlanesVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} PlanesVehs[18] = {

	{ "Luxor Deluxe", "LUXOR2" },
	{ "Blimp", "BLIMP" },
	{ "Hydra", "HYDRA" },
	{ "Lazer", "LAZER" },
	{ "Cargoplane", "CARGOPLANE" },
	{ "Cuban 800", "CUBAN800" },
	{ "Duster", "DUSTER" },
	{ "Dodo", "DODO" },
	{ "Mammatus", "MAMMATUS" },
	{ "Big Jet", "JET" },
	{ "Shamal", "SHAMAL" },
	{ "Luxor", "LUXOR" },
	{ "Miljet", "MILJET" },
	{ "Stunt", "STUNT" },
	{ "Titan", "TITAN" },
	{ "Velum2", "VELUM2" },
	{ "Velum", "VELUM" },
	{ "Vestra", "VESTRA" },

};
#pragma endregion
void planesClass() {


	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 18; i++) {
		AddCarSpawn((char*)PlanesVehs[i].Name, PlanesVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region SuvsVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} SuvsVehs[19] = {

	{ "Baller(Old)", "BALLER" },
	{ "Baller(New)", "BALLER2" },
	{ "BJXL", "BJXL" },
	{ "Cavalcade(Old)", "CAVALCADE" },
	{ "Cavalcade(New)", "CAVALCADE2" },
	{ "Dubsta", "DUBSTA" },
	{ "Dubsta Black", "DUBSTA2" },
	{ "FQ2", "FQ2" },
	{ "Granger", "GRANGER" },
	{ "Gresley", "GRESLEY" },
	{ "Habanero", "HABANERO" },
	{ "Hauntley", "HUNTLEY" },
	{ "Landstalker", "LANDSTALKER" },
	{ "Mesa", "MESA" },
	{ "Snowy Mesa", "MESA2" },
	{ "Patriot", "PATRIOT" },
	{ "Rocoto", "ROCOTO" },
	{ "Seminole", "SEMINOLE" },
	{ "Serrano", "SERRANO" },

};
#pragma endregion
void suvsClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 19; i++) {
		AddCarSpawn((char*)SuvsVehs[i].Name, SuvsVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region SedansVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} SedansVehs[23] = {

	{ "Asea", "ASEA" },
	{ "Snowy Asea", "ASEA2" },
	{ "Asterope", "ASTEROPE" },
	{ "Emperor", "EMPEROR" },
	{ "Rusty Emperor", "EMPEROR2" },
	{ "Snowy Emperor", "EMPEROR3" },
	{ "Fugitive", "FUGITIVE" },
	{ "Glendale", "GLENDALE" },
	{ "Ingot", "INGOT" },
	{ "Intruder", "INTRUDER" },
	{ "Premier", "PREMIER" },
	{ "Primo", "PRIMO" },
	{ "Regina", "REGINA" },
	{ "Romero", "ROMERO" },
	{ "Schafter", "SCHAFTER2" },
	{ "Stanier", "STANIER" },
	{ "Stratum", "STRATUM" },
	{ "Washington", "WASHINGTON" },
	{ "Stretch", "STRETCH" },
	{ "Super D", "SUPERD" },
	{ "Surge", "SURGE" },
	{ "Tailgater", "TAILGATER" },
	{ "Warrener", "WARRENER" },

};
#pragma endregion
void sedansClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 23; i++) {
		AddCarSpawn((char*)SedansVehs[i].Name, SedansVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region CompactVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} CompactVehs[6] = {

	{ "Dilettante(Old)", "DILETTANTE" },
	{ "Dilettante(New)", "DILETTANTE2" },
	{ "Issi", "ISSI2" },
	{ "Panto", "PANTO" },
	{ "Prairie", "PRAIRIE" },
	{ "Rhapsody", "RHAPSODY" },

};
#pragma endregion
void compactClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 6; i++) {
		AddCarSpawn((char*)CompactVehs[i].Name, CompactVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region CoupesVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} CoupesVehs[13] = {

	{ "Cogcabrio", "COGCABRIO" },
	{ "Exemplar", "EXEMPLAR" },
	{ "F620", "F620" },
	{ "Felon", "FELON" },
	{ "Felon Convertible", "FELON2" },
	{ "Jackal", "JACKAL" },
	{ "Oracle(New)", "ORACLE" },
	{ "Oracle(Old)", "ORACLE2" },
	{ "Sentinel", "SENTINEL" },
	{ "Sentinel 2", "SENTINEL2" },
	{ "Enus Windsor", "WINDSOR" },
	{ "Zion", "ZION" },
	{ "Zion Convertible", "ZION2" },

};
#pragma endregion
void coupesClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 13; i++) {
		AddCarSpawn((char*)CoupesVehs[i].Name, CoupesVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region EmergencyVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} EmergencyVehs[18] = {

	{ "Ambulance", "AMBULANCE" },
	{ "FIB Car", "FBI" },
	{ "FIB SUV", "FBI2" },
	{ "Firetruck", "FIRETRUK" },
	{ "Lifeguard SUV", "LGUARD" },
	{ "Police Ranger", "PRANGER" },
	{ "Police Riot", "RIOT" },
	{ "Police Bike", "POLICEB" },
	{ "Police Burrito", "POLICET" },
	{ "Police Cruiser", "POLICE" },
	{ "Subtile Police Car", "POLICE4" },
	{ "Police Buffalo", "POLICE2" },
	{ "Police Interceptor", "POLICE3" },
	{ "Snowy Police SUV", "POLICEOLD1" },
	{ "Snowy Police Cruiser", "POLICEOLD2" },
	{ "Prison Bus", "PBUS" },
	{ "Sheriff Car", "SHERIFF" },
	{ "Sheriff SUV", "SHERIFF2" },

};
#pragma endregion
void emergencyClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 18; i++) {
		AddCarSpawn((char*)EmergencyVehs[i].Name, EmergencyVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region MilitaryVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} MilitaryVehs[4] = {

	{ "Barracks", "BARRACKS" },
	{ "Barracks Truck", "BARRACKS2" },
	{ "Army Crusader", "CRUSADER" },
	{ "Rhino Tank", "RHINO" },

};
#pragma endregion
void militaryClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 4; i++) {
		AddCarSpawn((char*)MilitaryVehs[i].Name, MilitaryVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region VansVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} VansVehs[28] = {

	{ "Bison", "BISON" },
	{ "Enterprise Bison", "BISON2" },
	{ "Bison with plank", "BISON3" },
	{ "Boxville", "BOXVILLE" },
	{ "Postal Boxville", "BOXVILLE2" },
	{ "Humane Labs Boxville", "BOXVILLE3" },
	{ "Bobcat XL", "BOBCATXL" },
	{ "Burrito", "BURRITO" },
	{ "Burrito 2", "BURRITO2" },
	{ "Burrito 3", "BURRITO3" },
	{ "Burrito 4", "BURRITO4" },
	{ "Snowy Burrito", "BURRITO5" },
	{ "Camper", "CAMPER" },
	{ "The Lost Burrito", "GBURRITO" },
	{ "Journey", "JOURNEY" },
	{ "Minivan", "MINIVAN" },
	{ "Paradise", "PARADISE" },
	{ "Pony", "PONY" },
	{ "Canabis Poney", "PONY2" },
	{ "Rumpo", "RUMPO" },
	{ "Rumpo 2", "RUMPO2" },
	{ "Speedo", "SPEEDO" },
	{ "Clown Speedo", "SPEEDO2" },
	{ "Surfer", "SURFER" },
	{ "Rusty Surfer", "SURFER2" },
	{ "Taco", "TACO" },
	{ "Youga", "YOUGA" },
	{ "Caravan", "PROPTRAILER" },

};
#pragma endregion
void vansClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 28; i++) {
		AddCarSpawn((char*)VansVehs[i].Name, VansVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region MotorcyclesVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} MotorcyclesVehs[25] = {

	{ "Dinka Vindicator", "VINDICATOR" },
	{ "Akuma", "AKUMA" },
	{ "Bati", "BATI" },
	{ "Sport Bati", "BATI2" },
	{ "Ruffian", "RUFFIAN" },
	{ "Daemon", "DAEMON" },
	{ "Carbon RS", "CARBONRS" },
	{ "Double", "DOUBLE" },
	{ "Enduro", "ENDURO" },
	{ "Lectro", "LECTRO" },
	{ "PCJ", "PCJ" },
	{ "Vader", "VADER" },
	{ "Faggio Two", "FAGGIO2" },
	{ "Hexer", "HEXER" },
	{ "Sovereign", "SOVEREIGN" },
	{ "Innovation", "INNOVATION" },
	{ "Begger", "BAGGER" },
	{ "Hakuchou", "HAKUCHOU" },
	{ "Nemesis", "NEMESIS" },
	{ "Sanchez", "SANCHEZ" },
	{ "Sport Sanchez", "SANCHEZ2" },
	{ "Thrust", "THRUST" },
	{ "Quad", "BLAZER" },
	{ "Wood Quad", "BLAZER2" },
	{ "Trevor's Blazer", "BLAZER3" },

};
#pragma endregion
void motorcyclesClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 25; i++) {
		AddCarSpawn((char*)MotorcyclesVehs[i].Name, MotorcyclesVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region HelicopterVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} HelicopterVehs[14] = {

	{ "Annihilator", "ANNIHILATOR" },
	{ "Buzzard", "BUZZARD" },
	{ "Buzzard2", "BUZZARD2" },
	{ "Cargobob", "CARGOBOB" },
	{ "White Cargobob", "CARGOBOB2" },
	{ "TPI Cargobob", "CARGOBOB3" },
	{ "Skylift", "SKYLIFT" },
	{ "Police Helicopter", "POLMAV" },
	{ "Maverick", "MAVERICK" },
	{ "Frogger", "FROGGER" },
	{ "TPI Frogger", "FROGGER2" },
	{ "Savage Helicopter", "SAVAGE" },
	{ "Valkyrie Helicopter", "VALKYRIE" },
	{ "Swift Deluxe", "SWIFT2" },

};
#pragma endregion
void helicoptersClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 14; i++) {
		AddCarSpawn((char*)HelicopterVehs[i].Name, HelicopterVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region ServiceVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} ServiceVehs[8] = {

	{ "Airport Bus", "AIRBUS" },
	{ "City Bus", "BUS" },
	{ "Coach(Dashhound)", "COACH" },
	{ "Rental bus", "RENTALBUS" },
	{ "Taxi", "TAXI" },
	{ "Trash", "TRASH" },
	{ "Tour bus", "TOURBUS" },
	{ "Cable car", "CABLECAR" },

};
#pragma endregion
void servicesClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 8; i++) {
		AddCarSpawn((char*)ServiceVehs[i].Name, ServiceVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region IndustrialVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} IndustrialVehs[21] = {

	{ "Cutter", "CUTTER" },
	{ "Dock Handler", "HANDLER" },
	{ "Bulldozer", "BULLDOZER" },
	{ "HVY Dump Truck", "DUMP" },
	{ "Flatbed Truck", "FLATBED" },
	{ "Guardian", "GUARDIAN" },
	{ "Mixer(Old)", "MIXER" },
	{ "Mixer(New)", "MIXER2" },
	{ "Rubble", "RUBBLE" },
	{ "Tiptruck(Empty)", "TIPTRUCK" },
	{ "Tiptruck(Full)", "TIPTRUCK2" },
	{ "Benson", "BENSON" },
	{ "Biff", "BIFF" },
	{ "Hauler", "HAULER" },
	{ "Mule", "MULE" },
	{ "Mule 2", "MULE2" },
	{ "Packer(Truck)", "PACKER" },
	{ "Phantom(Truck)", "PHANTOM" },
	{ "Beer Truck", "POUNDER" },
	{ "Stockade", "STOCKADE" },
	{ "Snowy Stockade", "STOCKADE3" },

};
#pragma endregion
void industrialClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 21; i++) {
		AddCarSpawn((char*)IndustrialVehs[i].Name, IndustrialVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region BoatsVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} BoatsVehs[14] = {

	{ "Dinghy", "DINGHY" },
	{ "Dinghy 2", "DINGHY2" },
	{ "Jetmax", "JETMAX" },
	{ "Police Boat", "PREDATOR" },
	{ "Submersile", "SUBMERSIBLE" },
	{ "New Submersile", "SUBMERSIBLE2" },
	{ "Marquis", "MARQUIS" },
	{ "Seashark", "SEASHARK" },
	{ "Safeguard Seashark", "SEASHARK2" },
	{ "Speeder", "SPEEDER" },
	{ "Squalo", "SQUALO" },
	{ "Lampadati Toro", "TORO" },
	{ "Tropic", "TROPIC" },
	{ "Suntrap (Boat)", "SUNTRAP" },

};
#pragma endregion
void boatsClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 14; i++) {
		AddCarSpawn((char*)BoatsVehs[i].Name, BoatsVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region CyclesVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} CyclesVehs[7] = {

	{ "BMX", "BMX" },
	{ "Scorcher", "SCORCHER" },
	{ "Tribike 1", "TRIBIKE" },
	{ "Tribike 2", "TRIBIKE2" },
	{ "Tribike 3", "TRIBIKE3" },
	{ "Fixter", "FIXTER" },
	{ "Cruiser", "CRUISER" },

};
#pragma endregion
void cyclesClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 7; i++) {
		AddCarSpawn((char*)CyclesVehs[i].Name, CyclesVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region UtilityVehs

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} UtilityVehs[18] = {

	{ "Air Tug", "AIRTUG" },
	{ "Rusty Golf Caddy", "CADDY" },
	{ "Gold Caddy", "CADDY2" },
	{ "Dock Tug(Airport Truck)", "DOCKTUG" },
	{ "Rusty Tractor", "TRACTOR" },
	{ "Tractor", "TRACTOR2" },
	{ "Snowy Tractor", "TRACTOR3" },
	{ "Towtruck", "TOWTRUCK" },
	{ "Old Small Towtruck", "TOWTRUCK2" },
	{ "Utility Truck 1", "UTILLITRUCK" },
	{ "Utility Truck 2", "UTILLITRUCK2" },
	{ "Utility Truck 3", "UTILLITRUCK3" },
	{ "Fork Lift", "FORKLIFT" },
	{ "Grass Mower", "MOWER" },
	{ "Ripley(Airport)", "RIPLEY" },
	{ "Sadler", "SADLER" },
	{ "Snowy Sadler", "SADLER2" },
	{ "Scrap", "SCRAP" },

};
#pragma endregion
void NewMoneyDrop(Hash weapon, Vector3 coords) {
	drawNoBankingWarning();
	Hash pedm = GAMEPLAY::GET_HASH_KEY("a_c_crow");
	STREAMING::REQUEST_MODEL(pedm);
	while (!STREAMING::HAS_MODEL_LOADED(pedm))
		WAIT(0);
	Ped ped = PED::CREATE_PED(26, pedm, coords.x + rand() % 3, coords.y + rand() % 3, coords.z, 0, 1, 1);
	PED::SET_PED_MONEY(ped, 2000);
	//	SET_ENTITY_VISIBLE(ped, 1);
	ENTITY::SET_ENTITY_HEALTH(ped, 0);
	ENTITY::SET_ENTITY_COLLISION(ped, 1, 0);
	//	WAIT(10);
	ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&ped);
	//SET_ENTITY_VISIBLE(ped, 0);
}
void set_Safemoneydrop()
{
	Player playerPed = selPlayer;
	Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);

	NewMoneyDrop(GAMEPLAY::GET_HASH_KEY("WEAPON_MINIGUN"), pCoords);
}
void set_Safemoneydropv2()
{
	Player playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
	Ped playerPedID = PLAYER::PLAYER_PED_ID();
	Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);

	Hash moneycase = GAMEPLAY::GET_HASH_KEY("PICKUP_MONEY_CASE");
	Hash moneybag = GAMEPLAY::GET_HASH_KEY("prop_money_bag_01");
	STREAMING::REQUEST_MODEL(moneybag);
	while (!STREAMING::HAS_MODEL_LOADED(moneybag))
		WAIT(0);

	int numBags = GAMEPLAY::GET_RANDOM_INT_IN_RANGE(2, 6);
	int cashMoneyBaby = (GAMEPLAY::GET_RANDOM_INT_IN_RANGE(4000, 12001) / numBags);
	cashMoneyBaby = (int)round(cashMoneyBaby);
	for (int i = 0; i < numBags; i++)

		OBJECT::CREATE_AMBIENT_PICKUP(moneycase, pCoords.x, pCoords.y, pCoords.z + 1.5, 0, cashMoneyBaby, moneybag, 0, 1);

}
#pragma region Lowriders

static struct {
	LPCSTR Name;
	LPCSTR gameName;
} Lowriders[14] = {

	{ "Customizable Chino", "chino2" },
	{ "Customizable Virgo", "virgo2" },
	{ "Customizable Buccanee", "buccanee2" },
	{ "Customizable Moonbeam", "moonbeam2" },
	{ "Customizable Faction", "faction2" },
	{ "Customizable Voodoo", "voodoo2" },
	{ "Chino", "chino" },
	{ "Virgo", "virgo" },
	{ "Buccanee", "buccanee" },
	{ "Moonbeam", "moonbeam" },
	{ "Faction", "faction" },
	{ "Voodoo", "voodoo" },
	{ "Lurcher", "lurcher" },
	{ "Franken Strange", "btype2" },
};
#pragma endregion
int ticks = 0;
void LowridersClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Use bypass", sub::UseCarBypass);
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 14; i++) {
		AddCarSpawn((char*)Lowriders[i].Name, Lowriders[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
void utilityClass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 18; i++) {
		AddCarSpawn((char*)UtilityVehs[i].Name, UtilityVehs[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
#pragma region Smug
static struct {
	LPCSTR Name;
	LPCSTR gameName;
}
//"APC", "Ardent", "Caddy3", "Cheetah2", "Dune3", "Halftrack", "Hauler2", "Insurgent3",
//"Nightshark", "Oppressor", 
//"Phantom3", "Tampa3", "Technical13", "Torero", "TrailerLarge",
//"Trailers4", "Trailersma11", "Vagner", "Xa21"
Smug[18] =
{
	{ "VIGILANTE", "VIGILANTE" },
	{ "ALPHAZ1", "ALPHAZ1" },
	{ "BOMBUSHKA", "BOMBUSHKA" },
	{ "HAVOK", "HAVOK" },
	{ "HOWARD", "HOWARD" },
	{ "HUNTER", "HUNTER" },
	{ "MICROLIGHT", "MICROLIGHT" },
	{ "MOGUL", "MOGUL" },
	{ "NOKOTA","NOKOTA" },
	{ "PYRO","PYRO" },
	{ "RAPITGT3", "RAPITGT3" },
	{ "RETINUE", "RETINUE" },
	{ "ROQUE", "ROQUE" },
	{ "SEABREEZE", "SEABREEZE" },
	
	{ "STARLING", "STARLING" },
	{ "TULA","TULA" },
	{ "VISIONE", "VISIONE" },
	
	//ALPHAZ1 = 0xA52F6866, 2771347558 (plane)
	//BOMBUSHKA = 0xFE0A508C, 4262088844 (plane)
	//CYCLONE = 0x52FF9437, 1392481335 (car)
	//HAVOK = 0x89BA59F5, 2310691317 (helicopter)
	//HOWARD = 0xC3F25753, 3287439187 (plane)
	//HUNTER = 0xFD707EDE, 4252008158 (helicopter)
	//MICROLIGHT = 0x96E24857, 2531412055 (plane)
	//MOGUL = 0xD35698EF, 3545667823 (plane)
	//NOKOTA = 0x3DC92356, 1036591958 (plane)
	//PYRO = 0xAD6065C0, 2908775872 (plane)
	//RAPITGT3 = 0x7A2EF5E4, 2049897956 (car)
	//RETINUE = 0x6DBD6C0A, 1841130506 (car)
	//ROQUE = 0xC5DD6967, 3319621991 (plane)
	//SEABREEZE = 0xE8983F9F, 3902291871 (plane)
	//STARLING = 0x9A9EB7DE, 2594093022 (plane)
	//TULA = 0x3E2E4F8A, 1043222410 (plane)
	//VIGILANTE = 0xB5EF4C33, 3052358707 (car)
	//VISIONE = 0xC4810400, 3296789504 (car)

};
#pragma endregion
#pragma region Gunrunning
static struct {
	LPCSTR Name;
	LPCSTR gameName;
}
//"APC", "Ardent", "Caddy3", "Cheetah2", "Dune3", "Halftrack", "Hauler2", "Insurgent3",
//"Nightshark", "Oppressor", 
//"Phantom3", "Tampa3", "Technical13", "Torero", "TrailerLarge",
//"Trailers4", "Trailersma11", "Vagner", "Xa21"
Gunrunning[19] =
{
	{ "APC", "APC" },
	{ "Ardent", "Ardent" },
	{ "Bunker Caddy", "Caddy3" },
	{ "Cheetah Classic", "Cheetah2" },
	{ "Dune FAV", "Dune3" },
	{ "Half-Track", "Halftrack" },
	{ "Hauler", "Hauler2" },
	{ "Insugent Pick Up", "Insugent3" },
	{ "Nightshark","Nightshark" },
	{ "Oppressor","Oppressor" },
	{ "Phantom", "Phantom3" },
	{ "Weponized Tampa", "Tampa3" },
	{ "Technical Aqua", "Techincal13" },
	{ "Torero", "Torero" },
	{ "MOC Trailer", "TrailerLarge" },
	{ "Trailer small", "Trailers4" },
	{ "Trailer small 2","Trailersmall" },
	{ "Vagner", "Vagner" },
	{ "Xa21", "Xa21" },

};
#pragma endregion
void gunclass() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 18; i++) {
		AddCarSpawn((char*)Gunrunning[i].Name, Gunrunning[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
void smug() {



	bool custominput = 0;
	AddTitle("Anonymous");
	//AddTitle("Online Vehicle Spawner");
	AddToggle("Teleport In Spawned", tpinspawned);

	for (int i = 0; i < 17; i++) {
		AddCarSpawn((char*)Smug[i].Name, Smug[i].gameName);
	}
	if (custominput) {
		sub::SpawnVehicle(keyboard(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0), tpinspawned);
	}
}
void set_supergrip()
{
	Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID());
	VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
}
void DropWeapon(Hash weapon, Vector3 coords) {
	Hash pedm = GAMEPLAY::GET_HASH_KEY("csb_stripper_01");
	STREAMING::REQUEST_MODEL(pedm);
	while (!STREAMING::HAS_MODEL_LOADED(pedm))
		WAIT(0);
	Ped ped = PED::CREATE_PED(26, pedm, coords.x, coords.y, coords.z, 0, 1, 1);
	ENTITY::SET_ENTITY_VISIBLE(ped, false, 0);
	WEAPON::GIVE_WEAPON_TO_PED(ped, weapon, 9999, 1, 1);
	WEAPON::SET_PED_DROPS_WEAPONS_WHEN_DEAD(ped, 1);
	WEAPON::SET_PED_DROPS_WEAPON(ped);
	ENTITY::SET_ENTITY_HEALTH(ped, 0);
	ENTITY::SET_ENTITY_COLLISION(ped, 0, 0);
}
void oPlayerDROPMenu(char* name, Player p) {

	bool giveweap = 0, removwep = 0, ndmoney = 0, gsub = 0, ghsniper = 0, givarmor = 0, grpg = 0, gfsting = 0, gminigun = 0, gmolotov = 0, gsnack = 0, givhealth = 0, givparachute = 0, gcash = 0, giveCashSafe = 0, giveCashE = 0;
	AddTitle(name);
	AddTitle("~b~Fun Player Drops");
	//	AddToggle("Drop Cash", loop_moneydrop, giveCashE);
	AddOption("Drop Armor", givarmor);
	AddOption("Drop Parachute", givparachute);
	AddOption("Drop Snack", gsnack);
	AddOption("Drop Molotov", gmolotov);
	AddOption("Drop Health", gsub);
	//	AddOption("Drop FireStinguisher", gfsting);
	AddOption("Drop Minigun", gminigun);
	AddOption("Drop RPG Launcher", grpg);
	AddOption("Drop Heavy Sniper", ghsniper);
	AddOption("Give All Weapons", giveweap);
	AddOption("Remove All Weapons", removwep);
	//	AddToggle("Drop Money(SAFE)", loop_safemoneydrop, giveCashSafe);


	Ped myPed = PLAYER::PLAYER_PED_ID();
	Player playerPed = p;
	Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
	if (!riskMode && giveCashE) {
		PrintStringBottomCentre("~y~Enable risk mode to use this feature !");
		loop_moneydrop = 0;
	}
	if (!riskMode && (gfsting || gsub || gsnack || givparachute || givarmor || givhealth)) {
		PrintStringBottomCentre("~y~Enable risk mode to use this feature !");
		return;
	}
	pCoords.z + 1.5;
	if (ghsniper)
	{
		DropWeapon(GAMEPLAY::GET_HASH_KEY("WEAPON_HEAVYSNIPER"), pCoords);
	}
	if (grpg)
	{
		DropWeapon(GAMEPLAY::GET_HASH_KEY("WEAPON_RPG"), pCoords);
	}
	if (ndmoney)
	{
		NewMoneyDrop(GAMEPLAY::GET_HASH_KEY("WEAPON_MINIGUN"), pCoords);
	}
	if (gminigun)
	{
		DropWeapon(GAMEPLAY::GET_HASH_KEY("WEAPON_MINIGUN"), pCoords);
	}
	if (gfsting)
	{
		OBJECT::CREATE_AMBIENT_PICKUP(-887893374, pCoords.x, pCoords.y, pCoords.z + 1.5, 0, 100000, 1, 0, 1);
	}
	if (gsub)
	{
		OBJECT::CREATE_AMBIENT_PICKUP(-405862452, pCoords.x, pCoords.y, pCoords.z + 1.5, 0, 100000, 1, 0, 1);
	}
	if (gmolotov)
	{
		DropWeapon(GAMEPLAY::GET_HASH_KEY("WEAPON_MOLOTOV"), pCoords);
	}
	if (gsnack)
	{
		OBJECT::CREATE_AMBIENT_PICKUP(483577702, pCoords.x, pCoords.y, pCoords.z + 1.5, 0, 100000, 1, 0, 1);
	}
	if (givparachute)
	{
		OBJECT::CREATE_AMBIENT_PICKUP(1735599485, pCoords.x, pCoords.y, pCoords.z + 1.5, 0, 100000, 1, 0, 1);
	}
	if (giveweap)
	{
		static LPCSTR weaponNames2[] = {
			"WEAPON_KNIFE", "WEAPON_NIGHTSTICK", "WEAPON_HAMMER", "WEAPON_BAT", "WEAPON_GOLFCLUB", "WEAPON_CROWBAR",
			"WEAPON_PISTOL", "WEAPON_COMBATPISTOL", "WEAPON_APPISTOL", "WEAPON_PISTOL50", "WEAPON_MICROSMG", "WEAPON_SMG",
			"WEAPON_ASSAULTSMG", "WEAPON_ASSAULTRIFLE", "WEAPON_CARBINERIFLE", "WEAPON_ADVANCEDRIFLE", "WEAPON_MG",
			"WEAPON_COMBATMG", "WEAPON_PUMPSHOTGUN", "WEAPON_SAWNOFFSHOTGUN", "WEAPON_ASSAULTSHOTGUN", "WEAPON_BULLPUPSHOTGUN",
			"WEAPON_STUNGUN", "WEAPON_SNIPERRIFLE", "WEAPON_HEAVYSNIPER", "WEAPON_GRENADELAUNCHER", "WEAPON_GRENADELAUNCHER_SMOKE",
			"WEAPON_RPG", "WEAPON_MINIGUN", "WEAPON_GRENADE", "WEAPON_STICKYBOMB", "WEAPON_SMOKEGRENADE", "WEAPON_BZGAS",
			"WEAPON_MOLOTOV", "WEAPON_FIREEXTINGUISHER", "WEAPON_PETROLCAN", "WEAPON_KNUCKLE", "WEAPON_MARKSMANPISTOL",
			"WEAPON_SNSPISTOL", "WEAPON_SPECIALCARBINE", "WEAPON_HEAVYPISTOL", "WEAPON_BULLPUPRIFLE", "WEAPON_HOMINGLAUNCHER",
			"WEAPON_PROXMINE", "WEAPON_SNOWBALL", "WEAPON_VINTAGEPISTOL", "WEAPON_DAGGER", "WEAPON_FIREWORK", "WEAPON_MUSKET",
			"WEAPON_MARKSMANRIFLE", "WEAPON_HEAVYSHOTGUN", "WEAPON_GUSENBERG", "WEAPON_COMBATPDW", "WEAPON_HATCHET", "WEAPON_RAILGUN", "WEAPON_FLASHLIGHT", "WEAPON_MACHINEPISTOL", "WEAPON_MACHETE"
		};
		BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(p);
		Ped playerPed = p;
		Ped myPed = PLAYER::PLAYER_PED_ID();
		Player myPlayer = PLAYER::PLAYER_ID();
		Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
		for (int i = 0; i < sizeof(weaponNames2) / sizeof(weaponNames2[0]); i++)
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(playerPed, GAMEPLAY::GET_HASH_KEY((char *)weaponNames2[i]), 1000, 0);
		PrintStringBottomCentre("Player have it all!");
	}
	if (givarmor)
	{
		OBJECT::CREATE_AMBIENT_PICKUP(1274757841, pCoords.x, pCoords.y, pCoords.z + 1.5, 0, 100000, 1, 0, 1);
	}
	if (givhealth)
	{
		OBJECT::CREATE_AMBIENT_PICKUP(-1888453608, pCoords.x, pCoords.y, pCoords.z + 1.5, 0, 100000, 1, 0, 1);
	}
	if (removwep) { WEAPON::REMOVE_ALL_PED_WEAPONS(playerPed, 1); }


}

void drawEquippedIcon(float x, float y) {
	GRAPHICS::DRAW_SPRITE("CommonMenu", "shop_garage_icon_b", x, y, 0.032, 0.0440, 0, 255, 255, 255, 255);
}
void AddVehicleMod(Vehicle veh, int modType, int modIndex, bool horn = 0, bool equipped = 0, bool stock = 0, char* customName = "") {
	null = 0;
	int maxMods = VEHICLE::GET_NUM_VEHICLE_MODS(veh, modIndex);
	float mod = VEHICLE::GET_VEHICLE_MOD(veh, modIndex);
	/*if (horn) {
		AddOption(UI::_GET_LABEL_TEXT(TheHorns[modIndex]), null);
	}*/
	if (stock) {
		AddOption("Stock", null);
	}
	else {
		if (modType == 11) {
			AddOption(UI::_GET_LABEL_TEXT(customName), null);
		}
		else if (modType == 13) {
			ostringstream oss;
			oss << "CMOD_GBX_" << modIndex + 1;
			AddOption(UI::_GET_LABEL_TEXT((char*)oss.str().c_str()), null);
		}
		else if (modType == 16) {
			ostringstream oss;
			oss << "CMOD_ARM_" << modIndex + 1;
			AddOption(UI::_GET_LABEL_TEXT((char*)oss.str().c_str()), null);
		}
		else if (modType == 15) {
			ostringstream oss;
			oss << "CMOD_SUS_" << modIndex + 1;
			AddOption(UI::_GET_LABEL_TEXT((char*)oss.str().c_str()), null);
		}
		else if (modType == 12) {
			ostringstream oss;
			oss << "CMOD_BRA_" << modIndex + 1;
			AddOption(UI::_GET_LABEL_TEXT((char*)oss.str().c_str()), null);
		}
		else {
			AddOption(UI::_GET_LABEL_TEXT(VEHICLE::GET_MOD_TEXT_LABEL(veh, modType, modIndex)), null);
		}
	}
	if (OptionY < 0.6325 && OptionY > 0.1425)
	{
		if (equipped) {
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
			drawEquippedIcon(0.243f + menuPos, OptionY + 0.0170f);
		}
	}
	if (menu::printingop == menu::currentop)
	{
		if (null) {
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
			if (!stock) {
				RequestControlOfEnt(veh);
				VEHICLE::SET_VEHICLE_MOD(veh, modType, modIndex, 0);
			}
			else {
				RequestControlOfEnt(veh);
				RequestControlOfEnt(veh);
				VEHICLE::SET_VEHICLE_MOD(veh, modType, 500, 0);//Out of range -> Stock
			}
		}
	}
}
void Spoiler(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	AddTitle("Spoiler");
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 0);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 0);
	if (currentMod < 10)
		AddVehicleMod(veh, 0, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 0, 101, 0, 1, 1);
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 0, i);
		else
			AddVehicleMod(veh, 0, i, 0, 1);
	}
}
void FrontBumper(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 1);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 1);
	if (currentMod < 10)
		AddVehicleMod(veh, 1, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 1, 101, 0, 1, 1);
	AddTitle("Front Bumper");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 1, i);
		else
			AddVehicleMod(veh, 1, i, 0, 1);
	}
}
void RearBumper(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 2);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 2);
	if (currentMod < 10)
		AddVehicleMod(veh, 2, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 2, 101, 0, 1, 1);
	AddTitle("Rear Bumper");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 2, i);
		else
			AddVehicleMod(veh, 2, i, 0, 1);
	}
}
void SideSkirt(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 3);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 3);
	if (currentMod < 10)
		AddVehicleMod(veh, 3, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 3, 101, 0, 1, 1);
	AddTitle("Side Skirt");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 3, i);
		else
			AddVehicleMod(veh, 3, i, 0, 1);
	}
}
void Exhaust(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 4);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 4);
	if (currentMod < 10)
		AddVehicleMod(veh, 4, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 4, 101, 0, 1, 1);
	AddTitle("Exhaust");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 4, i);
		else
			AddVehicleMod(veh, 4, i, 0, 1);
	}
}
void Frame(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 5);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 5);
	if (currentMod < 10)
		AddVehicleMod(veh, 5, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 5, 101, 0, 1, 1);
	AddTitle("Frame");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 5, i);
		else
			AddVehicleMod(veh, 5, i, 0, 1);
	}
}
void Grille(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 6);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 6);
	if (currentMod < 10)
		AddVehicleMod(veh, 6, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 6, 101, 0, 1, 1);
	AddTitle("Grille");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 6, i);
		else
			AddVehicleMod(veh, 6, i, 0, 1);
	}
}
void Hood(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 7);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 7);
	if (currentMod < 10)
		AddVehicleMod(veh, 7, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 7, 101, 0, 1, 1);
	AddTitle("Hood");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 7, i);
		else
			AddVehicleMod(veh, 7, i, 0, 1);
	}
}
void Fender(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 8);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 8);
	if (currentMod < 10)
		AddVehicleMod(veh, 8, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 8, 101, 0, 1, 1);
	AddTitle("Fender");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 8, i);
		else
			AddVehicleMod(veh, 8, i, 0, 1);
	}
}
void RightFender(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 9);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 9);
	if (currentMod < 10)
		AddVehicleMod(veh, 9, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 9, 101, 0, 1, 1);
	AddTitle("Right Fender");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 9, i);
		else
			AddVehicleMod(veh, 9, i, 0, 1);
	}
}
void Roof(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 10);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 10);
	if (currentMod < 10)
		AddVehicleMod(veh, 10, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 10, 101, 0, 1, 1);
	AddTitle("Roof");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 10, i);
		else
			AddVehicleMod(veh, 10, i, 0, 1);
	}
}
void Engine(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 11);

	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 11);
	AddTitle("Engine");
	if (currentMod < 10)
		AddVehicleMod(veh, 11, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 11, 101, 0, 1, 1);
	if (currentMod != 0)
		AddVehicleMod(veh, 11, 0, 0, 0, 0, "CMOD_ENG_2");
	else
		AddVehicleMod(veh, 11, 1, 0, 1, 0, "CMOD_ENG_2");
	if (currentMod != 1)
		AddVehicleMod(veh, 11, 1, 0, 0, 0, "CMOD_ENG_3");
	else
		AddVehicleMod(veh, 11, 2, 0, 1, 0, "CMOD_ENG_3");
	if (currentMod != 2)
		AddVehicleMod(veh, 11, 2, 0, 0, 0, "CMOD_ENG_4");
	else
		AddVehicleMod(veh, 11, 2, 0, 1, 0, "CMOD_ENG_4");
	if (currentMod != 3)
		AddVehicleMod(veh, 11, 3, 0, 0, 0, "CMOD_ENG_5");
	else
		AddVehicleMod(veh, 11, 2, 0, 1, 0, "CMOD_ENG_5");
}
void Brakes(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 12);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 12);
	if (currentMod < 10)
		AddVehicleMod(veh, 12, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 12, 101, 0, 1, 1);
	AddTitle("Brakes");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 12, i);
		else
			AddVehicleMod(veh, 12, i, 0, 1);
	}
}
void Transmission(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 13);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 13);
	if (currentMod < 10)
		AddVehicleMod(veh, 13, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 13, 101, 0, 1, 1);
	AddTitle("Transmission");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 13, i);
		else
			AddVehicleMod(veh, 13, i, 0, 1);
	}
}
std::string FloatToString(float flt) {
	std::ostringstream ss;
	ss << flt;
	std::string str(ss.str());
	return str;
}
void drawint(int text, float X, float Y)
{
	UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("NUMBER");
	UI::ADD_TEXT_COMPONENT_INTEGER(text);
	UI::END_TEXT_COMMAND_DISPLAY_TEXT(X, Y);
}
void AddIntEasy(char* text, int value, int &val, int inc = 1, bool fast = 0, bool &toggled = null, bool enableminmax = 0, int max = 0, int min = 0)
{
	null = 0;
	AddOption(text, null);

	if (OptionY < 0.6325 && OptionY > 0.1425)
	{
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.26f, 0.26f);
		UI::SET_TEXT_CENTRE(1);

		drawint(value, 0.233f + menuPos, OptionY);
	}

	if (menu::printingop == menu::currentop)
	{
		if (IsOptionRJPressed()) {
			toggled = 1;
			if (enableminmax) {
				if (!((val + inc) > max)) {
					val += inc;
				}
			}
			else {
				val += inc;
			}
		}
		else if (IsOptionRPressed()) {
			toggled = 1;
			if (enableminmax) {
				if (!((val + inc) > max)) {
					val += inc;
				}
			}
			else {
				val += inc;
			}
		}
		else if (IsOptionLJPressed()) {
			toggled = 1;
			if (enableminmax) {
				if (!((val - inc) < min)) {
					val -= inc;
				}
			}
			else {
				val -= inc;
			}
		}
		else if (IsOptionLPressed()) {
			toggled = 1;
			if (enableminmax) {
				if (!((val - inc) < min)) {
					val -= inc;
				}
			}
			else {
				val -= inc;
			}
		}
	}
}
void AddVehicleHorn(Vehicle veh, int hornID, bool stock) {
	//null = 0;
	//int modid = GetModIdForHorn(hornID);
	//bool equipped = (modid == VEHICLE::GET_VEHICLE_MOD(veh, 14));

	//if (stock) {
	//	AddOption(UI::_GET_LABEL_TEXT("CMOD_HRN_0"), null);
	//}
	//else {

	//	AddOption(UI::_GET_LABEL_TEXT(HornById(hornID)), null);
	//}

	//if (OptionY < 0.6325 && OptionY > 0.1425)
	//{
	//	if (equipped) {
	//		PrintStringBottomCentre((char*)string("Mod ID" + FloatToString(VEHICLE::GET_VEHICLE_MOD(veh, 14)) + "HornID" + FloatToString(hornID)).c_str());
	//		drawEquippedIcon(0.243f + menuPos, OptionY + 0.0170f);
	//	}
	//}
	//if (menu::printingop == menu::currentop)
	//{
	//	if (null) {
	//		RequestControlOfEnt(veh);
	//		VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
	//		if (stock)
	//			VEHICLE::SET_VEHICLE_MOD(veh, 14, -1, 0); // Out of range -> Stock
	//		else
	//			VEHICLE::SET_VEHICLE_MOD(veh, 14, modid, 0);
	//	}
	//}
}
void Horns(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 14);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 14);
	AddTitle("Horns");
	AddVehicleHorn(veh, -1, 1);
	for (int i = 0; i < 36; i++) {
		AddVehicleHorn(veh, i, 0);
	}
}
void Suspension(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 15);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 15);
	if (currentMod < 10)
		AddVehicleMod(veh, 15, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 15, 101, 0, 1, 1);
	AddTitle("Suspension");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 15, i);
		else
			AddVehicleMod(veh, 15, i, 0, 1);
	}
}
void Armor(bool useSelectedPlayer = 0) {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	if (useSelectedPlayer) {
		RequestControlOfEnt(veh);
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		RequestControlOfEnt(playerPed);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	}
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 16);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 16);
	if (currentMod < 10)
		AddVehicleMod(veh, 16, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 16, 101, 0, 1, 1);
	AddTitle("Armor");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 16, i);
		else
			AddVehicleMod(veh, 16, i, 0, 1);
	}
}
void WindowsTint(bool useSelectedPlayer) {//CMOD_WIN_1
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	if (useSelectedPlayer) playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	bool none = 0, stock = 0, pb = 0, dm = 0, ds = 0, lm = 0, limo = 0, green = 0;
	AddTitle("Windows Tint");
	AddOption("None", none);
	AddOption("Stock", stock);
	AddOption("Pure Black", pb);
	AddOption("Dark Smoke", dm);
	AddOption("Light Smoke", lm);
	AddOption("Limo", limo);
	AddOption("Green", green);
	RequestControlOfEnt(veh);
	if (none) VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 0);
	if (pb) VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 1);
	if (ds) VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 2);
	if (lm) VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 3);
	if (stock) VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 4);
	if (limo) VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 5);
	if (green) VEHICLE::SET_VEHICLE_WINDOW_TINT(veh, 6);

}
bool neon = 0;
int nr = 0, ng = 0, nb = 0;
void Neon(bool useSelectedPlayer = 0) {
	bool blackp = 0, whitep = 0, yellowp = 0, greenp = 0, redp = 0, greyp = 0, bluep = 0, orangep = 0, pinkp = 0, purplep = 0, brownp = 0;
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = PLAYER::PLAYER_PED_ID();
	if (useSelectedPlayer) {
		player = selPlayer;
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
		e = PLAYER::PLAYER_PED_ID();
	}
	RequestControlOfEnt(veh);
	VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
	AddTitle("Neon lights");
	AddToggle("Rainbow Neon", loop_rainbowneon);
	AddToggle("Enabled", neon);
	if (neon || loop_rainbowneon) {
		RequestControlOfEnt(veh);
		VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(veh, 0, 1);
		VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(veh, 1, 1);
		VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(veh, 2, 1);
		VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(veh, 3, 1);
	}
	else {
		RequestControlOfEnt(veh);
		VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(veh, 0, 0);
		VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(veh, 1, 0);
		VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(veh, 2, 0);
		VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(veh, 3, 0);
	}
	AddOption("Black", blackp);
	AddOption("White", whitep);
	AddOption("Red", redp);
	AddOption("Green", greenp);
	AddOption("Blue", bluep);
	AddOption("Orange", orangep);
	AddOption("Yellow", yellowp);
	AddOption("Grey", greyp);
	AddOption("Brown", brownp);
	AddOption("Purple", purplep);//yellow-orange
	AddOption("Pink", pinkp);
	RequestControlOfEnt(veh);
	if (blackp)VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 0, 0, 0);
	if (whitep) VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 255, 255, 255);
	if (redp) VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 255, 0, 0);
	if (greenp)VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 0, 255, 0);
	if (bluep) VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 0, 0, 255);
	if (orangep)VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 255, 128, 0);
	if (yellowp)VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 255, 255, 0);
	if (purplep) VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 204, 0, 204);
	if (greyp) VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 96, 96, 96);
	if (brownp) VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 165, 42, 42);
	if (pinkp) VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 255, 51, 255);
	VEHICLE::_GET_VEHICLE_NEON_LIGHTS_COLOUR(veh, &nr, &ng, &nb);
	AddIntEasy("Red", nr, nr, 1, true, null, true, 255, 0);
	AddIntEasy("Green", ng, ng, 1, true, null, true, 255, 0);
	AddIntEasy("Blue", nb, nb, 1, true, null, true, 255, 0);
	RequestControlOfEnt(veh);
	if (!loop_rainbowneon)VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, nr, ng, nb);



}

void TireSmoke(bool useSelectedPlayer = 0) {
	AddTitle("Tire Smoke");

	bool blackp = 0, whitep = 0, yellowp = 0, greenp = 0, redp = 0, greyp = 0, bluep = 0, orangep = 0, pinkp = 0, purplep = 0, brownp = 0;
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 1);
	Entity e = PLAYER::PLAYER_PED_ID();
	if (useSelectedPlayer) {
		playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 1);
		e = PLAYER::PLAYER_PED_ID();
	}
	RequestControlOfEnt(veh);
	VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
	int tr = 0, tg = 0, tb = 0;
	bool wrr = VEHICLE::IS_TOGGLE_MOD_ON(veh, 20);
	AddToggle("Enbaled", wrr);
	if (wrr)
		VEHICLE::TOGGLE_VEHICLE_MOD(veh, 20, 1);
	else
		VEHICLE::TOGGLE_VEHICLE_MOD(veh, 20, 0);
	AddOption("Black", blackp);
	AddOption("White", whitep);
	AddOption("Red", redp);
	AddOption("Green", greenp);
	AddOption("Blue", bluep);
	AddOption("Orange", orangep);
	AddOption("Yellow", yellowp);
	AddOption("Grey", greyp);
	AddOption("Brown", brownp);
	AddOption("Purple", purplep);//yellow-orange
	AddOption("Pink", pinkp);
	RequestControlOfEnt(veh);
	if (blackp) VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, 0, 0, 0);
	if (whitep) VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, 255, 255, 255);
	if (redp) VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, 255, 0, 0);
	if (greenp) VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, 0, 255, 0);
	if (bluep) VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, 0, 0, 255);
	if (orangep) VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, 255, 128, 0);
	if (yellowp)VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, 255, 255, 0);
	if (purplep) VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, 204, 0, 204);
	if (greyp) VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, 96, 96, 96);
	if (brownp) VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, 165, 42, 42);
	if (pinkp) VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, 255, 51, 255);
	VEHICLE::GET_VEHICLE_TYRE_SMOKE_COLOR(veh, &tr, &tg, &tb);
	AddIntEasy("Red", tr, tr, 1, true, null, true, 255, 0);
	AddIntEasy("Green", tg, tg, 1, true, null, true, 255, 0);
	AddIntEasy("Blue", tb, tb, 1, true, null, true, 255, 0);
	RequestControlOfEnt(veh);
	VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(veh, tr, tg, tb);
}
void AddVehicleWheel(Vehicle veh, char* wheelLabel, int wheelType, int wheelIndex) {
	null = 0;
	RequestControlOfEnt(veh);
	AddOption(UI::_GET_LABEL_TEXT(wheelLabel), null);
	RequestControlOfEnt(veh);
	RequestControlOfEnt(veh);
	int equipped = VEHICLE::GET_VEHICLE_MOD(veh, 23);
	int currentType = VEHICLE::GET_VEHICLE_WHEEL_TYPE(veh);
	if (wheelType == currentType && equipped == wheelIndex) {
		if (wheelIndex < 14) {
			drawEquippedIcon(0.243f + menuPos, OptionY + 0.0170f);
		}
		else {
			if (menu::currentop >= wheelIndex + 1) {
				drawEquippedIcon(0.243f + menuPos, OptionY + 0.0170f);
			}
		}
	}
	if (menu::printingop == menu::currentop)
	{
		if (null) {
			RequestControlOfEnt(veh);
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_WHEEL_TYPE(veh, wheelType);
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_MOD(veh, 23, wheelIndex, 0);
		}
	}
}
void Wheels(bool wheel = 0, int wheelType = -1, bool useSelectedPlayer = 0) {
	Player player;
	Ped playerPed;
	Vehicle veh;
	Entity e;
	if (!useSelectedPlayer) {
		player = PLAYER::PLAYER_ID();
		playerPed = PLAYER::PLAYER_PED_ID();
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
		e = PLAYER::PLAYER_PED_ID();
	}
	else {
		player = PLAYER::PLAYER_ID();
		playerPed = PLAYER::PLAYER_PED_ID();
		veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
		e = PLAYER::PLAYER_PED_ID();
	}
	RequestControlOfEnt(veh);
	bool custom = VEHICLE::GET_VEHICLE_MOD_VARIATION(veh, 23);//SET_VEHICLE_MOD WHEEL VARIATION
	bool bulletproof = VEHICLE::GET_VEHICLE_TYRES_CAN_BURST(veh);
	bool customDisabled = 0, bulletproofDisabled = 0;
	bulletproof = !bulletproof;
	RequestControlOfEnt(playerPed);
	if (!PED::IS_PED_ON_ANY_BIKE(playerPed)) {
		if (wheel && wheelType == -1) {
			AddTitle("Wheel Type Selector");
		//	AddOption("Tire Design", null, nullFunc, SUB::WHEELSTRIPE);
			//AddOption(_GET_LABEL_TEXT("CMOD_WHE1_0"), null, nullFunc, SUB::WBIKE);
			AddToggle(UI::_GET_LABEL_TEXT("CMOD_TYR_1"), custom, null, customDisabled);
			AddToggle(UI::_GET_LABEL_TEXT("CMOD_TYR_2"), bulletproof, null, bulletproofDisabled);
			if (custom) {
				RequestControlOfEnt(veh);
				int wheelID = VEHICLE::GET_VEHICLE_MOD(veh, 23);
				VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
				VEHICLE::SET_VEHICLE_MOD(veh, 23, wheelID, 1);
			}
			else if (customDisabled) {
				RequestControlOfEnt(veh);
				int wheelID = VEHICLE::GET_VEHICLE_MOD(veh, 23);
				VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
				VEHICLE::SET_VEHICLE_MOD(veh, 23, wheelID, 0);
			}
			if (bulletproof) {
				RequestControlOfEnt(veh);
				VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 0);
				VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 0);
			}
			else if (bulletproofDisabled) {
				RequestControlOfEnt(veh);
				VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 1);
				VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 1);
			}
			AddOption(UI::_GET_LABEL_TEXT("CMOD_WHE1_1"), null, nullFunc, SUB::WHIGHEND);
			AddOption(UI::_GET_LABEL_TEXT("CMOD_WHE1_2"), null, nullFunc, SUB::WLOWRIDER);
			AddOption(UI::_GET_LABEL_TEXT("CMOD_WHE1_3"), null, nullFunc, SUB::WMUSCLE);
			AddOption(UI::_GET_LABEL_TEXT("CMOD_WHE1_4"), null, nullFunc, SUB::WOFFROAD);
			AddOption(UI::_GET_LABEL_TEXT("CMOD_WHE1_5"), null, nullFunc, SUB::WSPORT);
			AddOption(UI::_GET_LABEL_TEXT("CMOD_WHE1_6"), null, nullFunc, SUB::WSUV);
			AddOption(UI::_GET_LABEL_TEXT("CMOD_WHE1_7"), null, nullFunc, SUB::WTUNER);
			AddOption(UI::_GET_LABEL_TEXT("CMOD_WHE1_8"), null, nullFunc, SUB::WBENNY);
		}
		else if (wheel && wheelType > -1) {
			ostringstream oss;
			oss << "CMOD_WHE1_" << wheelType;
			AddTitle(UI::_GET_LABEL_TEXT((char*)oss.str().c_str()));
			Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
			if (wheelType == 5) {
				for (int i = 1; i <= 25; i++) {
					ostringstream wLabel;
					if (i < 10) wLabel << "SPT_0" << i;
					else wLabel << "SPT_" << i;
					RequestControlOfEnt(veh);
					AddVehicleWheel(veh, (char*)wLabel.str().c_str(), 0, i - 1);
				}
			}
			else if (wheelType == 1) {
				for (int i = 1; i <= 20; i++) {
					ostringstream wLabel;
					if (i < 10) wLabel << "HIEND_0" << i;
					else wLabel << "HIEND_" << i;
					RequestControlOfEnt(veh);
					AddVehicleWheel(veh, (char*)wLabel.str().c_str(), 7, i - 1);
				}
			}
			else if (wheelType == 2) {
				for (int i = 1; i <= 15; i++) {
					ostringstream wLabel;
					if (i < 10) wLabel << "LORIDE_0" << i;
					else wLabel << "LORIDE_" << i;
					RequestControlOfEnt(veh);
					AddVehicleWheel(veh, (char*)wLabel.str().c_str(), 2, i - 1);
				}
			}
			else if (wheelType == 3) {
				for (int i = 1; i <= 18; i++) {
					ostringstream wLabel;
					if (i < 10) wLabel << "MUSC_0" << i;
					else wLabel << "MUSC_" << i;
					RequestControlOfEnt(veh);
					AddVehicleWheel(veh, (char*)wLabel.str().c_str(), 1, i - 1);
				}
			}
			else if (wheelType == 4) {
				for (int i = 1; i <= 10; i++) {
					ostringstream wLabel;
					if (i < 10) wLabel << "OFFR_0" << i;
					else wLabel << "OFFR_" << i;
					RequestControlOfEnt(veh);
					AddVehicleWheel(veh, (char*)wLabel.str().c_str(), 4, i - 1);
				}
			}
			else if (wheelType == 6) {
				for (int i = 1; i <= 20; i++) {
					ostringstream wLabel;
					if (i < 10) wLabel << "SUV_0" << i;
					else wLabel << "SUV_" << i;
					RequestControlOfEnt(veh);
					AddVehicleWheel(veh, (char*)wLabel.str().c_str(), 3, i - 1);
				}
			}
			else if (wheelType == 7) {
				for (int i = 1; i <= 24; i++) {
					ostringstream wLabel;
					if (i < 10) wLabel << "DRFT_0" << i;
					else wLabel << "DRFT_" << i;
					RequestControlOfEnt(veh);
					AddVehicleWheel(veh, (char*)wLabel.str().c_str(), 5, i - 1);
				}
			}
			else if (wheelType == 8) {
				for (int i = 1; i <= 31; i++) {
					ostringstream wLabel;
					wLabel << "SMOD_WHL" << i;
					RequestControlOfEnt(veh);
					AddVehicleWheel(veh, (char*)wLabel.str().c_str(), 8, i - 1);
				}
			}
		}
	}
}
bool SetPaint(int PaintID, char* &gameName, int &paintType, int &colorID, int &pearlescent) {
	paintType = -1;
	colorID = -1;
	pearlescent = -1;
	switch (PaintID) {
	case 0:
		gameName = "BR BLACK_STEEL";
		paintType = 3;
		colorID = 118;
		pearlescent = 3;
		break;
	case 1:
		gameName = "BLACK_GRAPHITE";
		paintType = 0;
		colorID = 147;
		pearlescent = 4;
		break;
	case 2:
		gameName = "CHOCOLATE_BROWN";
		paintType = 1;
		colorID = 96;
		pearlescent = 0;
		break;
	case 3:
		gameName = "PURPLE";
		paintType = 0;
		colorID = 71;
		pearlescent = 145;
		break;
	case 4:
		gameName = "HOT PINK";
		paintType = 0;
		colorID = 135;
		pearlescent = 135;
		break;
	case 5:
		gameName = "FORMULA_RED";
		paintType = 0;
		colorID = 29;
		pearlescent = 28;
		break;
	case 6:
		gameName = "BLUE";
		paintType = 0;
		colorID = 64;
		pearlescent = 68;
		break;
	case 7:
		gameName = "ULTRA_BLUE";
		paintType = 1;
		colorID = 70;
		pearlescent = 0;
		break;
	case 8:
		gameName = "RACING_GREEN";
		paintType = 1;
		colorID = 50;
		pearlescent = 0;
		break;
	case 9:
		gameName = "LIME_GREEN";
		paintType = 2;
		colorID = 55;
		pearlescent = 0;
		break;
	case 10:
		gameName = "RACE_YELLOW";
		paintType = 1;
		colorID = 89;
		pearlescent = 0;
		break;
	case 11:
		gameName = "ORANGE";
		paintType = 1;
		colorID = 38;
		pearlescent = 0;
		break;
	case 12:
		gameName = "GOLD";
		paintType = 0;
		colorID = 37;
		pearlescent = 106;
		break;
	case 13:
		gameName = "SILVER";
		paintType = 0;
		colorID = 4;
		pearlescent = 111;
		break;
	case 14:
		gameName = "CHROME";
		paintType = 4;
		colorID = 120;
		pearlescent = 0;
		break;
	case 15:
		gameName = "WHITE";
		paintType = 1;
		colorID = 111;
		pearlescent = 0;
		break;
	case 16:
		gameName = "BLACK";
		paintType = 0;
		colorID = 0;
		pearlescent = 10;
		break;
	case 17:
		gameName = "GRAPHITE";
		paintType = 0;
		colorID = 1;
		pearlescent = 5;
		break;
	case 18:
		gameName = "ANTHR_BLACK";
		paintType = 0;
		colorID = 11;
		pearlescent = 2;
		break;
	case 19:
		gameName = "BLACK_STEEL";
		paintType = 0;
		colorID = 2;
		pearlescent = 5;
		break;
	case 20:
		gameName = "DARK_SILVER";
		paintType = 0;
		colorID = 3;
		pearlescent = 6;
		break;
	case 21:
		gameName = "BLUE_SILVER";
		paintType = 0;
		colorID = 5;
		pearlescent = 111;
		break;
	case 22:
		gameName = "ROLLED_STEEL";
		paintType = 0;
		colorID = 6;
		pearlescent = 4;
		break;
	case 23:
		gameName = "SHADOW_SILVER";
		paintType = 0;
		colorID = 7;
		pearlescent = 5;
		break;
	case 24:
		gameName = "STONE_SILVER";
		paintType = 0;
		colorID = 8;
		pearlescent = 5;
		break;
	case 25:
		gameName = "MIDNIGHT_SILVER";
		paintType = 0;
		colorID = 9;
		pearlescent = 7;
		break;
	case 26:
		gameName = "CAST_IRON_SIL";
		paintType = 0;
		colorID = 10;
		pearlescent = 7;
		break;
	case 27:
		gameName = "RED";
		paintType = 0;
		colorID = 27;
		pearlescent = 36;
		break;
	case 28:
		gameName = "TORINO_RED";
		paintType = 0;
		colorID = 28;
		pearlescent = 28;
		break;
	case 29:
		gameName = "LAVA_RED";
		paintType = 0;
		colorID = 150;
		pearlescent = 42;
		break;
	case 30:
		gameName = "BLAZE_RED";
		paintType = 0;
		colorID = 30;
		pearlescent = 36;
		break;
	case 31:
		gameName = "GRACE_RED";
		paintType = 0;
		colorID = 31;
		pearlescent = 27;
		break;
	case 32:
		gameName = "GARNET_RED";
		paintType = 0;
		colorID = 32;
		pearlescent = 25;
		break;
	case 33:
		gameName = "SUNSET_RED";
		paintType = 0;
		colorID = 33;
		pearlescent = 47;
		break;
	case 34:
		gameName = "CABERNET_RED";
		paintType = 0;
		colorID = 34;
		pearlescent = 47;
		break;
	case 35:
		gameName = "WINE_RED";
		paintType = 0;
		colorID = 143;
		pearlescent = 31;
		break;
	case 36:
		gameName = "CANDY_RED";
		paintType = 0;
		colorID = 35;
		pearlescent = 25;
		break;
	case 37:
		gameName = "PINK";
		paintType = 0;
		colorID = 137;
		pearlescent = 3;
		break;
	case 38:
		gameName = "SALMON_PINK";
		paintType = 0;
		colorID = 136;
		pearlescent = 5;
		break;
	case 39:
		gameName = "SUNRISE_ORANGE";
		paintType = 0;
		colorID = 36;
		pearlescent = 26;
		break;
	case 40:
		gameName = "ORANGE";
		paintType = 0;
		colorID = 38;
		pearlescent = 37;
		break;
	case 41:
		gameName = "BRIGHT_ORANGE";
		paintType = 0;
		colorID = 138;
		pearlescent = 89;
		break;
	case 42:
		gameName = "BRONZE";
		paintType = 0;
		colorID = 90;
		pearlescent = 102;
		break;
	case 43:
		gameName = "YELLOW";
		paintType = 0;
		colorID = 88;
		pearlescent = 88;
		break;
	case 44:
		gameName = "RACE_YELLOW";
		paintType = 0;
		colorID = 89;
		pearlescent = 88;
		break;
	case 45:
		gameName = "FLUR_YELLOW";
		paintType = 0;
		colorID = 91;
		pearlescent = 91;
		break;
	case 46:
		gameName = "DARK_GREEN";
		paintType = 0;
		colorID = 49;
		pearlescent = 52;
		break;
	case 47:
		gameName = "RACING_GREEN";
		paintType = 0;
		colorID = 50;
		pearlescent = 53;
		break;
	case 48:
		gameName = "SEA_GREEN";
		paintType = 0;
		colorID = 51;
		pearlescent = 66;
		break;
	case 49:
		gameName = "OLIVE_GREEN";
		paintType = 0;
		colorID = 52;
		pearlescent = 59;
		break;
	case 50:
		gameName = "BRIGHT_GREEN";
		paintType = 0;
		colorID = 53;
		pearlescent = 59;
		break;
	case 51:
		gameName = "PETROL_GREEN";
		paintType = 0;
		colorID = 54;
		pearlescent = 60;
		break;
	case 52:
		gameName = "LIME_GREEN";
		paintType = 0;
		colorID = 92;
		pearlescent = 92;
		break;
	case 53:
		gameName = "MIDNIGHT_BLUE";
		paintType = 0;
		colorID = 141;
		pearlescent = 73;
		break;
	case 54:
		gameName = "GALAXY_BLUE";
		paintType = 0;
		colorID = 61;
		pearlescent = 63;
		break;
	case 55:
		gameName = "DARK_BLUE";
		paintType = 0;
		colorID = 62;
		pearlescent = 68;
		break;
	case 56:
		gameName = "SAXON_BLUE";
		paintType = 0;
		colorID = 63;
		pearlescent = 87;
		break;
	case 57:
		gameName = "MARINER_BLUE";
		paintType = 0;
		colorID = 65;
		pearlescent = 87;
		break;
	case 58:
		gameName = "HARBOR_BLUE";
		paintType = 0;
		colorID = 66;
		pearlescent = 60;
		break;
	case 59:
		gameName = "DIAMOND_BLUE";
		paintType = 0;
		colorID = 67;
		pearlescent = 67;
		break;
	case 60:
		gameName = "SURF_BLUE";
		paintType = 0;
		colorID = 68;
		pearlescent = 68;
		break;
	case 61:
		gameName = "NAUTICAL_BLUE";
		paintType = 0;
		colorID = 69;
		pearlescent = 74;
		break;
	case 62:
		gameName = "RACING_BLUE";
		paintType = 0;
		colorID = 73;
		pearlescent = 73;
		break;
	case 63:
		gameName = "ULTRA_BLUE";
		paintType = 0;
		colorID = 70;
		pearlescent = 70;
		break;
	case 64:
		gameName = "LIGHT_BLUE";
		paintType = 0;
		colorID = 74;
		pearlescent = 74;
		break;
	case 65:
		gameName = "CHOCOLATE_BROWN";
		paintType = 0;
		colorID = 96;
		pearlescent = 95;
		break;
	case 66:
		gameName = "BISON_BROWN";
		paintType = 0;
		colorID = 101;
		pearlescent = 95;
		break;
	case 67:
		gameName = "CREEK_BROWN";
		paintType = 0;
		colorID = 95;
		pearlescent = 97;
		break;
	case 68:
		gameName = "UMBER_BROWN";
		paintType = 0;
		colorID = 94;
		pearlescent = 104;
		break;
	case 69:
		gameName = "MAPLE_BROWN";
		paintType = 0;
		colorID = 97;
		pearlescent = 98;
		break;
	case 70:
		gameName = "BEECHWOOD_BROWN";
		paintType = 0;
		colorID = 103;
		pearlescent = 104;
		break;
	case 71:
		gameName = "SIENNA_BROWN";
		paintType = 0;
		colorID = 104;
		pearlescent = 104;
		break;
	case 72:
		gameName = "SADDLE_BROWN";
		paintType = 0;
		colorID = 98;
		pearlescent = 95;
		break;
	case 73:
		gameName = "MOSS_BROWN";
		paintType = 0;
		colorID = 100;
		pearlescent = 100;
		break;
	case 74:
		gameName = "WOODBEECH_BROWN";
		paintType = 0;
		colorID = 102;
		pearlescent = 105;
		break;
	case 75:
		gameName = "STRAW_BROWN";
		paintType = 0;
		colorID = 99;
		pearlescent = 106;
		break;
	case 76:
		gameName = "SANDY_BROWN";
		paintType = 0;
		colorID = 105;
		pearlescent = 105;
		break;
	case 77:
		gameName = "BLEECHED_BROWN";
		paintType = 0;
		colorID = 106;
		pearlescent = 106;
		break;
	case 78:
		gameName = "SPIN_PURPLE";
		paintType = 0;
		colorID = 72;
		pearlescent = 64;
		break;
	case 79:
		gameName = "MIGHT_PURPLE";
		paintType = 0;
		colorID = 146;
		pearlescent = 145;
		break;
	case 80:
		gameName = "BRIGHT_PURPLE";
		paintType = 0;
		colorID = 145;
		pearlescent = 74;
		break;
	case 81:
		gameName = "CREAM";
		paintType = 0;
		colorID = 107;
		pearlescent = 107;
		break;
	case 82:
		gameName = "WHITE";
		paintType = 0;
		colorID = 111;
		pearlescent = 0;
		break;
	case 83:
		gameName = "FROST_WHITE";
		paintType = 0;
		colorID = 112;
		pearlescent = 0;
		break;
	case 84:
		gameName = "BLACK";
		paintType = 1;
		colorID = 0;
		pearlescent = 0;
		break;
	case 85:
		gameName = "BLACK_GRAPHITE";
		paintType = 1;
		colorID = 147;
		pearlescent = 0;
		break;
	case 86:
		gameName = "GRAPHITE";
		paintType = 1;
		colorID = 1;
		pearlescent = 0;
		break;
	case 87:
		gameName = "ANTHR_BLACK";
		paintType = 1;
		colorID = 11;
		pearlescent = 0;
		break;
	case 88:
		gameName = "BLACK_STEEL";
		paintType = 1;
		colorID = 2;
		pearlescent = 0;
		break;
	case 89:
		gameName = "DARK_SILVER";
		paintType = 1;
		colorID = 3;
		pearlescent = 2;
		break;
	case 90:
		gameName = "SILVER";
		paintType = 1;
		colorID = 4;
		pearlescent = 4;
		break;
	case 91:
		gameName = "BLUE_SILVER";
		paintType = 1;
		colorID = 5;
		pearlescent = 5;
		break;
	case 92:
		gameName = "ROLLED_STEEL";
		paintType = 1;
		colorID = 6;
		pearlescent = 0;
		break;
	case 93:
		gameName = "SHADOW_SILVER";
		paintType = 1;
		colorID = 7;
		pearlescent = 0;
		break;
	case 94:
		gameName = "STONE_SILVER";
		paintType = 1;
		colorID = 8;
		pearlescent = 0;
		break;
	case 95:
		gameName = "MIDNIGHT_SILVER";
		paintType = 1;
		colorID = 9;
		pearlescent = 0;
		break;
	case 96:
		gameName = "CAST_IRON_SIL";
		paintType = 1;
		colorID = 10;
		pearlescent = 0;
		break;
	case 97:
		gameName = "RED";
		paintType = 1;
		colorID = 27;
		pearlescent = 0;
		break;
	case 98:
		gameName = "TORINO_RED";
		paintType = 1;
		colorID = 28;
		pearlescent = 0;
		break;
	case 99:
		gameName = "FORMULA_RED";
		paintType = 1;
		colorID = 29;
		pearlescent = 0;
		break;
	case 100:
		gameName = "LAVA_RED";
		paintType = 1;
		colorID = 150;
		pearlescent = 0;
		break;
	case 101:
		gameName = "BLAZE_RED";
		paintType = 1;
		colorID = 30;
		pearlescent = 0;
		break;
	case 102:
		gameName = "GRACE_RED";
		paintType = 1;
		colorID = 31;
		pearlescent = 0;
		break;
	case 103:
		gameName = "GARNET_RED";
		paintType = 1;
		colorID = 32;
		pearlescent = 0;
		break;
	case 104:
		gameName = "SUNSET_RED";
		paintType = 1;
		colorID = 33;
		pearlescent = 0;
		break;
	case 105:
		gameName = "CABERNET_RED";
		paintType = 1;
		colorID = 34;
		pearlescent = 0;
		break;
	case 106:
		gameName = "WINE_RED";
		paintType = 1;
		colorID = 143;
		pearlescent = 0;
		break;
	case 107:
		gameName = "CANDY_RED";
		paintType = 1;
		colorID = 35;
		pearlescent = 0;
		break;
	case 108:
		gameName = "HOT PINK";
		paintType = 1;
		colorID = 135;
		pearlescent = 0;
		break;
	case 109:
		gameName = "PINK";
		paintType = 1;
		colorID = 137;
		pearlescent = 0;
		break;
	case 110:
		gameName = "SALMON_PINK";
		paintType = 1;
		colorID = 136;
		pearlescent = 0;
		break;
	case 111:
		gameName = "SUNRISE_ORANGE";
		paintType = 1;
		colorID = 36;
		pearlescent = 0;
		break;
	case 112:
		gameName = "BRIGHT_ORANGE";
		paintType = 1;
		colorID = 138;
		pearlescent = 0;
		break;
	case 113:
		gameName = "GOLD";
		paintType = 1;
		colorID = 99;
		pearlescent = 99;
		break;
	case 114:
		gameName = "BRONZE";
		paintType = 1;
		colorID = 90;
		pearlescent = 102;
		break;
	case 115:
		gameName = "YELLOW";
		paintType = 1;
		colorID = 88;
		pearlescent = 0;
		break;
	case 116:
		gameName = "FLUR_YELLOW";
		paintType = 1;
		colorID = 91;
		pearlescent = 0;
		break;
	case 117:
		gameName = "DARK_GREEN";
		paintType = 1;
		colorID = 49;
		pearlescent = 0;
		break;
	case 118:
		gameName = "SEA_GREEN";
		paintType = 1;
		colorID = 51;
		pearlescent = 0;
		break;
	case 119:
		gameName = "OLIVE_GREEN";
		paintType = 1;
		colorID = 52;
		pearlescent = 0;
		break;
	case 120:
		gameName = "BRIGHT_GREEN";
		paintType = 1;
		colorID = 53;
		pearlescent = 0;
		break;
	case 121:
		gameName = "PETROL_GREEN";
		paintType = 1;
		colorID = 54;
		pearlescent = 0;
		break;
	case 122:
		gameName = "LIME_GREEN";
		paintType = 1;
		colorID = 92;
		pearlescent = 0;
		break;
	case 123:
		gameName = "MIDNIGHT_BLUE";
		paintType = 1;
		colorID = 141;
		pearlescent = 0;
		break;
	case 124:
		gameName = "GALAXY_BLUE";
		paintType = 1;
		colorID = 61;
		pearlescent = 0;
		break;
	case 125:
		gameName = "DARK_BLUE";
		paintType = 1;
		colorID = 62;
		pearlescent = 0;
		break;
	case 126:
		gameName = "SAXON_BLUE";
		paintType = 1;
		colorID = 63;
		pearlescent = 0;
		break;
	case 127:
		gameName = "BLUE";
		paintType = 1;
		colorID = 64;
		pearlescent = 0;
		break;
	case 128:
		gameName = "MARINER_BLUE";
		paintType = 1;
		colorID = 65;
		pearlescent = 0;
		break;
	case 129:
		gameName = "HARBOR_BLUE";
		paintType = 1;
		colorID = 66;
		pearlescent = 0;
		break;
	case 130:
		gameName = "DIAMOND_BLUE";
		paintType = 1;
		colorID = 67;
		pearlescent = 0;
		break;
	case 131:
		gameName = "SURF_BLUE";
		paintType = 1;
		colorID = 68;
		pearlescent = 0;
		break;
	case 132:
		gameName = "NAUTICAL_BLUE";
		paintType = 1;
		colorID = 69;
		pearlescent = 0;
		break;
	case 133:
		gameName = "RACING_BLUE";
		paintType = 1;
		colorID = 73;
		pearlescent = 0;
		break;
	case 134:
		gameName = "LIGHT_BLUE";
		paintType = 1;
		colorID = 74;
		pearlescent = 0;
		break;
	case 135:
		gameName = "BISON_BROWN";
		paintType = 1;
		colorID = 101;
		pearlescent = 0;
		break;
	case 136:
		gameName = "CREEK_BROWN";
		paintType = 1;
		colorID = 95;
		pearlescent = 0;
		break;
	case 137:
		gameName = "UMBER_BROWN";
		paintType = 1;
		colorID = 94;
		pearlescent = 0;
		break;
	case 138:
		gameName = "MAPLE_BROWN";
		paintType = 1;
		colorID = 97;
		pearlescent = 0;
		break;
	case 139:
		gameName = "BEECHWOOD_BROWN";
		paintType = 1;
		colorID = 103;
		pearlescent = 0;
		break;
	case 140:
		gameName = "SIENNA_BROWN";
		paintType = 1;
		colorID = 104;
		pearlescent = 0;
		break;
	case 141:
		gameName = "SADDLE_BROWN";
		paintType = 1;
		colorID = 98;
		pearlescent = 0;
		break;
	case 142:
		gameName = "MOSS_BROWN";
		paintType = 1;
		colorID = 100;
		pearlescent = 0;
		break;
	case 143:
		gameName = "WOODBEECH_BROWN";
		paintType = 1;
		colorID = 102;
		pearlescent = 0;
		break;
	case 144:
		gameName = "STRAW_BROWN";
		paintType = 1;
		colorID = 99;
		pearlescent = 0;
		break;
	case 145:
		gameName = "SANDY_BROWN";
		paintType = 1;
		colorID = 105;
		pearlescent = 0;
		break;
	case 146:
		gameName = "BLEECHED_BROWN";
		paintType = 1;
		colorID = 106;
		pearlescent = 0;
		break;
	case 147:
		gameName = "PURPLE";
		paintType = 1;
		colorID = 71;
		pearlescent = 0;
		break;
	case 148:
		gameName = "SPIN_PURPLE";
		paintType = 1;
		colorID = 72;
		pearlescent = 0;
		break;
	case 149:
		gameName = "MIGHT_PURPLE";
		paintType = 1;
		colorID = 142;
		pearlescent = 0;
		break;
	case 150:
		gameName = "BRIGHT_PURPLE";
		paintType = 1;
		colorID = 145;
		pearlescent = 0;
		break;
	case 151:
		gameName = "CREAM";
		paintType = 1;
		colorID = 107;
		pearlescent = 0;
		break;
	case 152:
		gameName = "FROST_WHITE";
		paintType = 1;
		colorID = 112;
		pearlescent = 0;
		break;
	case 153:
		gameName = "BLACK";
		paintType = 2;
		colorID = 12;
		pearlescent = 0;
		break;
	case 154:
		gameName = "GREY";
		paintType = 2;
		colorID = 13;
		pearlescent = 0;
		break;
	case 155:
		gameName = "LIGHT_GREY";
		paintType = 2;
		colorID = 14;
		pearlescent = 0;
		break;
	case 156:
		gameName = "WHITE";
		paintType = 2;
		colorID = 131;
		pearlescent = 0;
		break;
	case 157:
		gameName = "BLUE";
		paintType = 2;
		colorID = 83;
		pearlescent = 0;
		break;
	case 158:
		gameName = "DARK_BLUE";
		paintType = 2;
		colorID = 82;
		pearlescent = 0;
		break;
	case 159:
		gameName = "MIDNIGHT_BLUE";
		paintType = 2;
		colorID = 84;
		pearlescent = 0;
		break;
	case 160:
		gameName = "MIGHT_PURPLE";
		paintType = 2;
		colorID = 149;
		pearlescent = 0;
		break;
	case 161:
		gameName = "Purple";
		paintType = 2;
		colorID = 148;
		pearlescent = 0;
		break;
	case 162:
		gameName = "RED";
		paintType = 2;
		colorID = 39;
		pearlescent = 0;
		break;
	case 163:
		gameName = "DARK_RED";
		paintType = 2;
		colorID = 40;
		pearlescent = 0;
		break;
	case 164:
		gameName = "ORANGE";
		paintType = 2;
		colorID = 41;
		pearlescent = 0;
		break;
	case 165:
		gameName = "YELLOW";
		paintType = 2;
		colorID = 42;
		pearlescent = 0;
		break;
	case 166:
		gameName = "GREEN";
		paintType = 2;
		colorID = 128;
		pearlescent = 0;
		break;
	case 167:
		gameName = "MATTE_FOR";
		paintType = 2;
		colorID = 151;
		pearlescent = 0;
		break;
	case 168:
		gameName = "MATTE_FOIL";
		paintType = 2;
		colorID = 155;
		pearlescent = 0;
		break;
	case 169:
		gameName = "MATTE_OD";
		paintType = 2;
		colorID = 152;
		pearlescent = 0;
		break;
	case 170:
		gameName = "MATTE_DIRT";
		paintType = 2;
		colorID = 153;
		pearlescent = 0;
		break;
	case 171:
		gameName = "MATTE_DESERT";
		paintType = 2;
		colorID = 154;
		pearlescent = 0;
		break;
	case 172:
		gameName = "BR_STEEL";
		paintType = 3;
		colorID = 117;
		pearlescent = 18;
		break;
	case 173:
		gameName = "BR_ALUMINIUM";
		paintType = 3;
		colorID = 119;
		pearlescent = 5;
		break;
	case 174:
		gameName = "GOLD_P";
		paintType = 3;
		colorID = 158;
		pearlescent = 160;
		break;
	case 175:
		gameName = "GOLD_S";
		paintType = 3;
		colorID = 159;
		pearlescent = 160;
		break;
	}
	return paintType != -1;
}
void AddVehiclePaint(Vehicle veh, char* label, int paintType, int colorID, int pearlescent, bool secondary = 0, bool wheelColor = 0, int wheelsColor = 0) {
	null = 0;
	AddOption(UI::_GET_LABEL_TEXT(label), null);
	if (menu::printingop == menu::currentop)
	{
		if (null) {
			RequestControlOfEnt(veh);
			int currentPrimary = 0;
			int currentSecondary = 0;
			int currentPearlescent = 0;
			int currentWheelColor = 0;
			RequestControlOfEnt(veh);
			if (!wheelColor) VEHICLE::GET_VEHICLE_EXTRA_COLOURS(veh, &currentPearlescent, &wheelsColor);
			RequestControlOfEnt(veh);
			VEHICLE::GET_VEHICLE_COLOURS(veh, &currentPrimary, &currentSecondary);

			if (!secondary) {
				RequestControlOfEnt(veh);
				VEHICLE::CLEAR_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh);
				RequestControlOfEnt(veh);
				VEHICLE::SET_VEHICLE_MOD_COLOR_1(veh, paintType, colorID, 0);
				RequestControlOfEnt(veh);
				VEHICLE::SET_VEHICLE_COLOURS(veh, colorID, currentSecondary);
			}
			else if (secondary) {
				RequestControlOfEnt(veh);
				VEHICLE::CLEAR_VEHICLE_CUSTOM_SECONDARY_COLOUR(veh);
				RequestControlOfEnt(veh);
				VEHICLE::SET_VEHICLE_MOD_COLOR_2(veh, paintType, colorID);
				RequestControlOfEnt(veh);
				VEHICLE::SET_VEHICLE_COLOURS(veh, currentPrimary, colorID);
			}
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_EXTRA_COLOURS(veh, pearlescent, wheelsColor);
		}
	}
}
void gamePaint(int type, bool secondary = false, bool useSelectedPlayer = 0) { //TYPE:NORMAL, 1=Metallic, 2=Pearl 3=Matte 4=Metal
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = PLAYER::PLAYER_PED_ID();
	if (useSelectedPlayer) {
		Player player = selPlayer;
		Ped playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(player);
		Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
		Entity e = playerPed;
	}
	int paintType = 0, colorID = 0, pearlescent = 0;
	char* name = "";
	char* title = "";
	if (type == 0)
		title = "Metallic";
	else if (type == 1)
		title = "Classic";
	else if (type == 2)
		title = "Matte";
	else if (type == 3)
		title = "Metals";
	else if (type == 4)
		title = "Pearlescent";

	AddTitle(title);
	if (type == 4) {

	}
	int currentPrimary = 0;
	int currentSecondary = 0;
	VEHICLE::GET_VEHICLE_COLOURS(veh, &currentPrimary, &currentSecondary);
	if (!secondary) {
		for (int i = 0; i < 175; i++) {
			SetPaint(i, name, paintType, colorID, pearlescent);
			if (type == 4) {
				AddVehiclePaint(veh, name, 4, currentPrimary, colorID);
			}
			else if ((paintType == type) && type != 4) {
				AddVehiclePaint(veh, name, paintType, colorID, pearlescent);
			}
		}
	}
	else {
		for (int i = 0; i < 175; i++) {
			SetPaint(i, name, paintType, colorID, pearlescent);
			if (type == 4) {
				AddVehiclePaint(veh, name, 4, currentSecondary, colorID, 1);
			}
			else if ((paintType == type) && type != 4) {
				AddVehiclePaint(veh, name, paintType, colorID, pearlescent, 1);
			}
		}
	}
}
typedef struct {
	string name;
	int PaintTypeIndex;
	int colorID;
} PaintType;
vector<PaintType> PaintTypes = { { "~b~Classic", 0, 147 },{ "~b~Metallic", 1, 96 },{ "~b~Matte", 2, 12 },{ "~b~Metal", 3, 147 } };
int PaintTypeIndex = 0;
int PaintTypeIndexSecondary = 0;
void AddPaintTypes(bool primary) {
	null = 0;
	AddOption("~g~Paint Type");
	if (OptionY < 0.6325 && OptionY > 0.1425)
	{
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.26f, 0.26f);
		UI::SET_TEXT_CENTRE(1);
		if (primary) {
			drawstring((char*)PaintTypes[PaintTypeIndex].name.c_str(), 0.233f + menuPos, OptionY);
			if (menu::printingop == menu::currentop) {
				if (IsOptionRJPressed()) {//num6
					if (PaintTypeIndex != (PaintTypes.size() - 1)) PaintTypeIndex++;
					Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
					RequestControlOfEnt(veh);
					RGBA rgb;
					VEHICLE::GET_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh, &rgb.R, &rgb.G, &rgb.B);
					int currentPrimary = 0;
					int currentSecondary = 0;
					int colorID = 0;
					int paintType = 0;
					int pearlescent = 0;
					char* name;
					VEHICLE::GET_VEHICLE_COLOURS(veh, &currentPrimary, &currentSecondary);
					switch (PaintTypeIndex) {
					case 0: SetPaint(1, name, paintType, colorID, pearlescent); break;
					case 1:	SetPaint(2, name, paintType, colorID, pearlescent); break;
					case 2:	SetPaint(153, name, paintType, colorID, pearlescent); break;
					case 3:	SetPaint(0, name, paintType, colorID, pearlescent); break;

					}
					VEHICLE::SET_VEHICLE_MOD_COLOR_1(veh, paintType, colorID, 0);
					VEHICLE::SET_VEHICLE_COLOURS(veh, colorID, currentSecondary);
					VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh, rgb.R, rgb.G, rgb.B);
				}
				if (IsOptionLJPressed()) {//num4
					if (PaintTypeIndex != 0) PaintTypeIndex--;
					Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
					RequestControlOfEnt(veh);
					RGBA rgb;
					VEHICLE::GET_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh, &rgb.R, &rgb.G, &rgb.B);
					int currentPrimary = 0;
					int currentSecondary = 0;
					int colorID = 0;
					int paintType = 0;
					int pearlescent = 0;
					char* name;
					VEHICLE::GET_VEHICLE_COLOURS(veh, &currentPrimary, &currentSecondary);
					switch (PaintTypeIndex) {
					case 0: SetPaint(1, name, paintType, colorID, pearlescent); break;
					case 1:	SetPaint(2, name, paintType, colorID, pearlescent); break;
					case 2:	SetPaint(153, name, paintType, colorID, pearlescent); break;
					case 3:	SetPaint(0, name, paintType, colorID, pearlescent); break;

					}
					VEHICLE::SET_VEHICLE_MOD_COLOR_1(veh, paintType, colorID, 0);
					VEHICLE::SET_VEHICLE_COLOURS(veh, colorID, currentSecondary);
					VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh, rgb.R, rgb.G, rgb.B);
				}
			}
		}
		else {
			drawstring((char*)PaintTypes[PaintTypeIndexSecondary].name.c_str(), 0.233f + menuPos, OptionY);
			if (menu::printingop == menu::currentop) {
				if (IsOptionRJPressed()) {//num6
					if (PaintTypeIndexSecondary != (PaintTypes.size() - 1)) {
						PaintTypeIndexSecondary++;
						Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
						RequestControlOfEnt(veh);
						RGBA rgb;
						VEHICLE::GET_VEHICLE_CUSTOM_SECONDARY_COLOUR(veh, &rgb.R, &rgb.G, &rgb.B);
						int currentPrimary = 0;
						int currentSecondary = 0;
						int colorID = 0;
						int paintType = 0;
						int pearlescent = 0;
						char* name;
						VEHICLE::GET_VEHICLE_COLOURS(veh, &currentPrimary, &currentSecondary);
						switch (PaintTypeIndexSecondary) {
						case 0: SetPaint(1, name, paintType, colorID, pearlescent); break;
						case 1:	SetPaint(2, name, paintType, colorID, pearlescent); break;
						case 2:	SetPaint(153, name, paintType, colorID, pearlescent); break;
						case 3:	SetPaint(0, name, paintType, colorID, pearlescent); break;

						}
						VEHICLE::SET_VEHICLE_MOD_COLOR_2(veh, paintType, colorID);
						VEHICLE::SET_VEHICLE_COLOURS(veh, currentPrimary, colorID);
						VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(veh, rgb.R, rgb.G, rgb.B);
					}
				}
				if (IsOptionLJPressed()) {//num4
					if (PaintTypeIndexSecondary != 0) {
						PaintTypeIndexSecondary--;
						Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
						RequestControlOfEnt(veh);
						RGBA rgb;
						VEHICLE::GET_VEHICLE_CUSTOM_SECONDARY_COLOUR(veh, &rgb.R, &rgb.G, &rgb.B);
						int currentPrimary = 0;
						int currentSecondary = 0;
						int colorID = 0;
						int paintType = 0;
						int pearlescent = 0;
						char* name;
						VEHICLE::GET_VEHICLE_COLOURS(veh, &currentPrimary, &currentSecondary);
						switch (PaintTypeIndexSecondary) {
						case 0: SetPaint(1, name, paintType, colorID, pearlescent); break;
						case 1:	SetPaint(2, name, paintType, colorID, pearlescent); break;
						case 2:	SetPaint(153, name, paintType, colorID, pearlescent); break;
						case 3:	SetPaint(0, name, paintType, colorID, pearlescent); break;

						}
						VEHICLE::SET_VEHICLE_MOD_COLOR_2(veh, paintType, colorID);
						VEHICLE::SET_VEHICLE_COLOURS(veh, currentPrimary, colorID);
						VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(veh, rgb.R, rgb.G, rgb.B);
					}
				}
			}
		}
	}
}
void CustomPaintRGB(bool primary) {
	if (primary) {
		AddTitle("Custom Primary");
		Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
		bool  greenChanged = 0, blueChanged = 0, redChanged = 0;
		int pr = 0, pg = 0, pb = 0;
		VEHICLE::GET_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh, &pr, &pg, &pb);
		AddPaintTypes(primary);
		AddIntEasy("Red", pr, pr, 1, 0, redChanged);
		AddIntEasy("Green", pg, pg, 1, 0, greenChanged);
		AddIntEasy("Blue", pb, pb, 1, 0, blueChanged);
		if (redChanged || greenChanged || blueChanged) {
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh, pr, pg, pb);

		}
	}
	else {
		AddTitle("Custom Secondary");
		Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
		bool  greenChanged = 0, blueChanged = 0, redChanged = 0;
		int pr = 0, pg = 0, pb = 0;
		VEHICLE::GET_VEHICLE_CUSTOM_SECONDARY_COLOUR(veh, &pr, &pg, &pb);
		AddPaintTypes(primary);
		AddIntEasy("Red", pr, pr, 1, 0, redChanged);
		AddIntEasy("Green", pg, pg, 1, 0, greenChanged);
		AddIntEasy("Blue", pb, pb, 1, 0, blueChanged);
		if (redChanged || greenChanged || blueChanged) {
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(veh, pr, pg, pb);
		}
	}
}
void oPedSpawner()

{
	// Initialise local variables here:
	bool pcat = 0, pchop = 0, phawk = 0, bbuddies1 = 0, mtanksupport = 0, reddinos = 0, incest1 = 0, incest2 = 0, incest3 = 0, sSex = 0, racks = 0, plion = 0, pdancerst = 0, rgstrp = 0, taserswatt = 0, dalien = 0, mrRoks = 0, hguy = 0, sdeer = 0, scow = 0, shen = 0, ssharkH = 0, ssharkt = 0;
	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);


	// Options' text here:
	AddTitle("Anonymous MENU");
	//			AddTitle("Peds & Troll");
	AddOption("Animal Riding", null, nullFunc, SUB::ANIMAL);
	//	AddOption("Weird Vehicles", null, nullFunc, SUB::WEIRDVEHICLES);
	AddOption("Pole Dancer Stripper", pdancerst);
	AddOption("Stripper with Railgun", rgstrp);
	AddOption("Swat with Tasers", taserswatt);
	AddOption("Pet Cat", pcat);
	AddOption("Pet Chop", pchop);
	AddOption("Pet Hawk", phawk);
	AddOption("Pet MtLion", plion);
	AddOption("Racksmile", racks);
	AddOption("Humping Guy", hguy);
	AddOption("Incest 1", incest1);
	AddOption("Incest 2", incest2);
	AddOption("Incest 3", incest3);
	AddOption("Butt Buddies 1", bbuddies1);
	AddOption("Spawn Floof", reddinos);




	// Options' code here:
	
	if (bbuddies1) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash guysex = GAMEPLAY::GET_HASH_KEY("player_two");
		Hash girlsex = GAMEPLAY::GET_HASH_KEY("player_one");
		char *anim = "rcmpaparazzo_2";
		char *animID = "shag_loop_a";
		char *anim2 = "rcmpaparazzo_2";
		char *animID2 = "shag_loop_poppy";
		STREAMING::REQUEST_MODEL(guysex);
		while (!STREAMING::HAS_MODEL_LOADED(guysex))
			WAIT(0);
		STREAMING::REQUEST_MODEL(girlsex);
		while (!STREAMING::HAS_MODEL_LOADED(girlsex))
			WAIT(0);

		int createdGuySex = PED::CREATE_PED(26, guysex, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdGuySex, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
		//
		int createdGirlSex = PED::CREATE_PED(26, girlsex, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdGirlSex, false);

		STREAMING::REQUEST_ANIM_DICT(anim2);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim2))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdGirlSex, anim2, animID2, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);

		ENTITY::ATTACH_ENTITY_TO_ENTITY(createdGuySex, createdGirlSex, -1, 0.059999998658895f, -0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 2, 1, 1);
	}
	if (incest3) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash guysex = GAMEPLAY::GET_HASH_KEY("cs_jimmydisanto");
		Hash girlsex = GAMEPLAY::GET_HASH_KEY("ig_amandatownley");
		char *anim = "rcmpaparazzo_2";
		char *animID = "shag_loop_a";
		char *anim2 = "rcmpaparazzo_2";
		char *animID2 = "shag_loop_poppy";
		STREAMING::REQUEST_MODEL(guysex);
		while (!STREAMING::HAS_MODEL_LOADED(guysex))
			WAIT(0);
		STREAMING::REQUEST_MODEL(girlsex);
		while (!STREAMING::HAS_MODEL_LOADED(girlsex))
			WAIT(0);

		int createdGuySex = PED::CREATE_PED(26, guysex, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdGuySex, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
		//
		int createdGirlSex = PED::CREATE_PED(26, girlsex, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdGirlSex, false);

		STREAMING::REQUEST_ANIM_DICT(anim2);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim2))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdGirlSex, anim2, animID2, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);

		ENTITY::ATTACH_ENTITY_TO_ENTITY(createdGuySex, createdGirlSex, -1, 0.059999998658895f, -0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 2, 1, 1);
	}
	if (incest2) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash guysex = GAMEPLAY::GET_HASH_KEY("player_zero");
		Hash girlsex = GAMEPLAY::GET_HASH_KEY("ig_tracydisanto");
		char *anim = "rcmpaparazzo_2";
		char *animID = "shag_loop_a";
		char *anim2 = "rcmpaparazzo_2";
		char *animID2 = "shag_loop_poppy";
		STREAMING::REQUEST_MODEL(guysex);
		while (!STREAMING::HAS_MODEL_LOADED(guysex))
			WAIT(0);
		STREAMING::REQUEST_MODEL(girlsex);
		while (!STREAMING::HAS_MODEL_LOADED(girlsex))
			WAIT(0);

		int createdGuySex = PED::CREATE_PED(26, guysex, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdGuySex, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
		//
		int createdGirlSex = PED::CREATE_PED(26, girlsex, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdGirlSex, false);

		STREAMING::REQUEST_ANIM_DICT(anim2);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim2))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdGirlSex, anim2, animID2, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);

		ENTITY::ATTACH_ENTITY_TO_ENTITY(createdGuySex, createdGirlSex, -1, 0.059999998658895f, -0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 2, 1, 1);
	}
	if (incest1) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash guysex = GAMEPLAY::GET_HASH_KEY("cs_jimmydisanto");
		Hash girlsex = GAMEPLAY::GET_HASH_KEY("ig_tracydisanto");
		char *anim = "rcmpaparazzo_2";
		char *animID = "shag_loop_a";
		char *anim2 = "rcmpaparazzo_2";
		char *animID2 = "shag_loop_poppy";
		STREAMING::REQUEST_MODEL(guysex);
		while (!STREAMING::HAS_MODEL_LOADED(guysex))
			WAIT(0);
		STREAMING::REQUEST_MODEL(girlsex);
		while (!STREAMING::HAS_MODEL_LOADED(girlsex))
			WAIT(0);

		int createdGuySex = PED::CREATE_PED(26, guysex, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdGuySex, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
		//
		int createdGirlSex = PED::CREATE_PED(26, girlsex, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdGirlSex, false);

		STREAMING::REQUEST_ANIM_DICT(anim2);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim2))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdGirlSex, anim2, animID2, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);

		ENTITY::ATTACH_ENTITY_TO_ENTITY(createdGuySex, createdGirlSex, -1, 0.059999998658895f, -0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 2, 1, 1);
	}
	if (pcat) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_cat_01");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int my_group = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		PED::SET_PED_AS_GROUP_LEADER(playerPed, my_group);
		PED::SET_PED_AS_GROUP_MEMBER(createdPED, my_group);

		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);

		PED::SET_PED_NEVER_LEAVES_GROUP(createdPED, my_group);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
		PED::SET_GROUP_FORMATION(my_group, 3);
	}
	if (pchop) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_chop");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int my_group = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		PED::SET_PED_AS_GROUP_LEADER(playerPed, my_group);
		PED::SET_PED_AS_GROUP_MEMBER(createdPED, my_group);

		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);

		PED::SET_PED_NEVER_LEAVES_GROUP(createdPED, my_group);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
		PED::SET_GROUP_FORMATION(my_group, 3);
	}
	if (phawk) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_chickenhawk");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int my_group = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		PED::SET_PED_AS_GROUP_LEADER(playerPed, my_group);
		PED::SET_PED_AS_GROUP_MEMBER(createdPED, my_group);

		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);

		PED::SET_PED_NEVER_LEAVES_GROUP(createdPED, my_group);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
		PED::SET_GROUP_FORMATION(my_group, 3);
	}
	if (plion) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_mtlion");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int my_group = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		PED::SET_PED_AS_GROUP_LEADER(playerPed, my_group);
		PED::SET_PED_AS_GROUP_MEMBER(createdPED, my_group);

		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);

		PED::SET_PED_NEVER_LEAVES_GROUP(createdPED, my_group);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
		PED::SET_GROUP_FORMATION(my_group, 3);
	}
	if (reddinos) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_m_y_gay_02");
		Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_RAILGUN");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		WEAPON::GIVE_WEAPON_TO_PED(createdPED, railgun, railgun, 9999, 9999);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
		PrintStringBottomCentre("~b~redd pls gib color~s~ !");
	}
	if (pdancerst) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("s_f_y_stripper_01");
		char *anim = "mini@strip_club@pole_dance@pole_dance2";
		char *animID = "pd_dance_02";
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdPED, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
	}
	if (rgstrp) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("s_f_y_stripper_01");
		Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_RAILGUN");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		WEAPON::GIVE_WEAPON_TO_PED(createdPED, railgun, railgun, 9999, 9999);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
	}
	if (taserswatt) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("s_m_y_swat_01");
		Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_STUNGUN");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		WEAPON::GIVE_WEAPON_TO_PED(createdPED, railgun, railgun, 9999, 9999);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
	}

	if (racks) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("u_m_y_babyd");
		Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_MOLOTOV");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		WEAPON::GIVE_WEAPON_TO_PED(createdPED, railgun, railgun, 9999, 9999);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
	}
	if (hguy) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("u_m_y_justin");
		char *anim = "rcmpaparazzo_2";
		char *animID = "shag_loop_a";
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdPED, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
	}
	if (sdeer) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_deer");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
	}
	if (scow) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_cow");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
	}
	if (shen) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_hen");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
	}
	if (ssharkH) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_sharkhammer");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
	}
	if (ssharkt) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_sharktiger");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
	}
}
void BodyguardSpawner()

{


	// Initialise local variables here:
	bool cPolDan = 0, cPups = 0, redBodyguard = 0, mJetsupport = 0, mtanksupport = 0, cMedit = 0, cPrivDan = 0, bodgRgun = 0, bodgTaser = 0, bodgSnpr = 0, bodgbat = 0, bodgPunch = 0;
	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);


	// Options' text here:
	AddTitle("Anonymous MENU");
	AddOption("Military Tank Support", mtanksupport);
	//			AddOption("Military Jet Support", mJetsupport);
	AddTitle("Clone & Bodyguard Options");
	AddOption("Clone Pole Dancing", cPolDan);
	AddOption("Clone Push Ups", cPups);
	AddOption("Clone Meditation", cMedit);
	AddOption("Clone Priv. Dance", cPrivDan);
	AddOption("Bodyguards Railgun", bodgRgun);
	AddOption("Bodyguards Taser", bodgTaser);
	AddOption("Bodyguards Sniper", bodgSnpr);
	AddOption("Bodyguards with Bat", bodgbat);
	AddOption("Bodyguards no Guns", bodgPunch);
	AddOption("Big Foot Bodyguard", redBodyguard);



	// Options' code here:


	if (mtanksupport) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash guysex = GAMEPLAY::GET_HASH_KEY("s_m_y_marine_01");
		STREAMING::REQUEST_MODEL(guysex);
		while (!STREAMING::HAS_MODEL_LOADED(guysex))
			WAIT(0);
		int createdGuySex = PED::CREATE_PED(26, guysex, pos.x, pos.y, pos.z, 1, 1, 0);

		//
		int vehmodel = GAMEPLAY::GET_HASH_KEY("RHINO");
		STREAMING::REQUEST_MODEL(vehmodel);

		while (!STREAMING::HAS_MODEL_LOADED(vehmodel)) WAIT(0);
		Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 5.0, 0.0);
		
		Vehicle veh = sub::CREATE_VEHICLEB(vehmodel, coords.x, coords.y, coords.z, 0.0, 1, 1);
		VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
		//
		PED::SET_PED_INTO_VEHICLE(createdGuySex, veh, -1);

		ENTITY::SET_ENTITY_INVINCIBLE(createdGuySex, false);
		PED::SET_PED_COMBAT_ABILITY(createdGuySex, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdGuySex, true);
		AI::TASK_COMBAT_PED(createdGuySex, playerPed, 1, 1);
		PED::SET_PED_AS_ENEMY(createdGuySex, 1);
		PED::SET_PED_COMBAT_RANGE(createdGuySex, 1000);
		PED::SET_PED_KEEP_TASK(createdGuySex, true);
		PED::SET_PED_AS_COP(createdGuySex, 1000);
		PED::SET_PED_ALERTNESS(createdGuySex, 1000);


	}
	if (cPolDan) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		char *anim = "mini@strip_club@pole_dance@pole_dance2";
		char *animID = "pd_dance_02";

		int clone = PED::CLONE_PED(playerPed, pos.x, pos.y, pos.z);
		ENTITY::SET_ENTITY_INVINCIBLE(clone, false);

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(clone, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
	}
	if (cPups) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		char *anim = "amb@world_human_push_ups@male@base";
		char *animID = "base";

		int clone = PED::CLONE_PED(playerPed, pos.x, pos.y, pos.z);
		ENTITY::SET_ENTITY_INVINCIBLE(clone, false);

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(clone, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
	}
	if (cMedit) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Hash fireworkl = GAMEPLAY::GET_HASH_KEY("WEAPON_FIREWORK");
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		char *anim = "rcmcollect_paperleadinout@";
		char *animID = "meditiate_idle";

		int clone = PED::CLONE_PED(playerPed, pos.x, pos.y, pos.z);
		ENTITY::SET_ENTITY_INVINCIBLE(clone, false);
		PED::SET_PED_COMBAT_ABILITY(clone, 100);
		WEAPON::GIVE_WEAPON_TO_PED(clone, fireworkl, fireworkl, 9999, 9999);
		PED::SET_PED_CAN_SWITCH_WEAPON(clone, true);

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(clone, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
	}
	if (cPrivDan) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		char *anim = "mini@strip_club@lap_dance_2g@ld_2g_p2";
		char *animID = "ld_2g_p2_s2";

		int clone = PED::CLONE_PED(playerPed, pos.x, pos.y, pos.z);
		ENTITY::SET_ENTITY_INVINCIBLE(clone, false);

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(clone, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);
	}
	if (bodgRgun) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();

		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_RAILGUN");
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

		int my_group = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
		int clone = PED::CLONE_PED(playerPed, pos.x, pos.y, pos.z);
		PED::SET_PED_AS_GROUP_LEADER(playerPed, my_group);
		PED::SET_PED_AS_GROUP_MEMBER(clone, my_group);
		PED::SET_PED_NEVER_LEAVES_GROUP(clone, my_group);
		ENTITY::SET_ENTITY_INVINCIBLE(clone, false);
		PED::SET_PED_COMBAT_ABILITY(clone, 100);
		WEAPON::GIVE_WEAPON_TO_PED(clone, railgun, railgun, 9999, 9999);
		PED::SET_PED_CAN_SWITCH_WEAPON(clone, true);
		PED::SET_GROUP_FORMATION(my_group, 3);
	}
	if (bodgTaser) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();

		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_STUNGUN");
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

		int my_group = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
		int clone = PED::CLONE_PED(playerPed, pos.x, pos.y, pos.z);
		PED::SET_PED_AS_GROUP_LEADER(playerPed, my_group);
		PED::SET_PED_AS_GROUP_MEMBER(clone, my_group);
		PED::SET_PED_NEVER_LEAVES_GROUP(clone, my_group);
		ENTITY::SET_ENTITY_INVINCIBLE(clone, false);
		PED::SET_PED_COMBAT_ABILITY(clone, 100);
		WEAPON::GIVE_WEAPON_TO_PED(clone, railgun, railgun, 9999, 9999);
		PED::SET_PED_CAN_SWITCH_WEAPON(clone, true);
		PED::SET_GROUP_FORMATION(my_group, 3);
	}
	if (bodgSnpr) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();

		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_HEAVYSNIPER");
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

		int my_group = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
		int clone = PED::CLONE_PED(playerPed, pos.x, pos.y, pos.z);
		PED::SET_PED_AS_GROUP_LEADER(playerPed, my_group);
		PED::SET_PED_AS_GROUP_MEMBER(clone, my_group);
		PED::SET_PED_NEVER_LEAVES_GROUP(clone, my_group);
		ENTITY::SET_ENTITY_INVINCIBLE(clone, false);
		PED::SET_PED_COMBAT_ABILITY(clone, 100);
		WEAPON::GIVE_WEAPON_TO_PED(clone, railgun, railgun, 9999, 9999);
		PED::SET_PED_CAN_SWITCH_WEAPON(clone, true);
		PED::SET_GROUP_FORMATION(my_group, 3);
	}
	if (bodgbat) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();

		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_BAT");
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

		int my_group = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
		int clone = PED::CLONE_PED(playerPed, pos.x, pos.y, pos.z);
		PED::SET_PED_AS_GROUP_LEADER(playerPed, my_group);
		PED::SET_PED_AS_GROUP_MEMBER(clone, my_group);
		PED::SET_PED_NEVER_LEAVES_GROUP(clone, my_group);
		ENTITY::SET_ENTITY_INVINCIBLE(clone, false);
		PED::SET_PED_COMBAT_ABILITY(clone, 100);
		WEAPON::GIVE_WEAPON_TO_PED(clone, railgun, railgun, 9999, 9999);
		PED::SET_PED_CAN_SWITCH_WEAPON(clone, true);
		PED::SET_GROUP_FORMATION(my_group, 3);
	}
	if (bodgPunch) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();

		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Hash bat = GAMEPLAY::GET_HASH_KEY("WEAPON_BAT");
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

		int my_group = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
		int clone = PED::CLONE_PED(playerPed, pos.x, pos.y, pos.z);
		PED::SET_PED_AS_GROUP_LEADER(playerPed, my_group);
		PED::SET_PED_AS_GROUP_MEMBER(clone, my_group);
		PED::SET_PED_NEVER_LEAVES_GROUP(clone, my_group);
		ENTITY::SET_ENTITY_INVINCIBLE(clone, false);
		PED::SET_PED_COMBAT_ABILITY(clone, 100);
		PED::SET_GROUP_FORMATION(my_group, 3);
	}
	if (redBodyguard) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("cs_orleans");
		Hash railgun = GAMEPLAY::GET_HASH_KEY("WEAPON_RAILGUN");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);

		int my_group = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
		PED::SET_PED_AS_GROUP_LEADER(playerPed, my_group);
		PED::SET_PED_AS_GROUP_MEMBER(createdPED, my_group);
		PED::SET_PED_NEVER_LEAVES_GROUP(createdPED, my_group);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		WEAPON::GIVE_WEAPON_TO_PED(createdPED, railgun, railgun, 9999, 9999);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
	}
}
void AddNumberEasy(char* text, float value, __int8 decimal_places, float &val, float inc = 1.0f, bool fast = 0, bool &toggled = null, bool enableminmax = 0, float max = 0.0f, float min = 0.0f)
{
	null = 0;
	AddOption(text, null);

	if (OptionY < 0.6325 && OptionY > 0.1425)
	{
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.26f, 0.26f);
		UI::SET_TEXT_CENTRE(1);

		drawfloat(value, (DWORD)decimal_places, 0.233f + menuPos, OptionY);
	}

	if (menu::printingop == menu::currentop)
	{
		if (IsOptionRJPressed()) {
			toggled = 1;
			if (enableminmax) {
				if (!((val + inc) > max)) {
					val += inc;
				}
			}
			else {
				val += inc;
			}
		}
		else if (IsOptionRPressed()) {
			toggled = 1;
			if (enableminmax) {
				if (!((val + inc) > max)) {
					val += inc;
				}
			}
			else {
				val += inc;
			}
		}
		else if (IsOptionLJPressed()) {
			toggled = 1;
			if (enableminmax) {
				if (!((val - inc) < min)) {
					val -= inc;
				}
			}
			else {
				val -= inc;
			}
		}
		else if (IsOptionLPressed()) {
			toggled = 1;
			if (enableminmax) {
				if (!((val - inc) < min)) {
					val -= inc;
				}
			}
			else {
				val -= inc;
			}
		}
	}
}
bool forceGunPeds = 1;
bool forceGunVehicles = 1;
bool forceGunObjects = 1;
float entityDistance = 10.0f;
float pickupDistance = 100.0f;
void GravityGunSettings() {
	AddTitle("Anonymous Gun");
	AddToggle("~r~Anonymous Gun", loop_gravity_gun);
	AddNumberEasy("Distance between you and the object", entityDistance, 0, entityDistance);
	AddNumberEasy("Pickup Range", pickupDistance, 0, pickupDistance, 1.0f, 1);
	if (entityDistance < 1) entityDistance = 1;
	if (pickupDistance < 1) pickupDistance = 1;
	AddToggle("Pickup Peds", forceGunPeds);
	AddToggle("Pickup Vehicle", forceGunVehicles);
	AddToggle("Pickup Objects", forceGunObjects);
}
void AnimalRiddin()

{



	// Initialise local variables here:
	bool stopRide = 0, deerIdle = 0, cowIdle = 0, deerRun = 0, deerWalk = 0;
	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);


	// Options' text here:
	AddTitle("Anonymous MENU");
	AddTitle("Animal Riding Options");
	AddOption("Stop Ride", stopRide);
	AddOption("Deer Idle", deerIdle);
	AddOption("Deer Walk", deerWalk);
	AddOption("Deer Run", deerRun);
	AddOption("Cow Idle", cowIdle);
	//		AddOption("Boaor Ride", deerRun);
	//		AddOption("Pig Ride", deerRun);
	//		AddOption("Custom Ride", deerRun);


	// Options' code here:


	if (stopRide) {
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		ENTITY::DETACH_ENTITY(playerPed, 1, 1);
		AI::CLEAR_PED_TASKS_IMMEDIATELY(playerPed);
	}
	if (cowIdle) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_cow");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
		//				SET_ENTITY_HEADING(createdPED, 180.0f);

		ENTITY::ATTACH_ENTITY_TO_ENTITY(playerPed, createdPED, -1, 0.0f, 0.35f, 0.72f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 2, 1, 1);

		//charPose
		char *anim2 = "mp_safehouselost_table@";
		char *animID2 = "lost_table_negative_a";

		STREAMING::REQUEST_ANIM_DICT(anim2);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim2))
			WAIT(0);

		AI::TASK_PLAY_ANIM(playerPed, anim2, animID2, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);

	}
	if (deerIdle) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_deer");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
		//				SET_ENTITY_HEADING(createdPED, 180.0f);

		ENTITY::ATTACH_ENTITY_TO_ENTITY(playerPed, createdPED, -1, 0.0f, 0.35f, 0.72f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 2, 1, 1);

		//charPose
		char *anim2 = "mp_safehouselost_table@";
		char *animID2 = "lost_table_negative_a";

		STREAMING::REQUEST_ANIM_DICT(anim2);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim2))
			WAIT(0);

		AI::TASK_PLAY_ANIM(playerPed, anim2, animID2, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);

	}
	if (deerWalk) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_deer");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
		//				SET_ENTITY_HEADING(createdPED, 180.0f);

		ENTITY::ATTACH_ENTITY_TO_ENTITY(playerPed, createdPED, -1, 0.0f, 0.35f, 0.72f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 2, 1, 1);

		//deer animation
		char *anim = "creatures@deer@move";
		char *animID = "walk";

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdPED, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);

		//charPose
		char *anim2 = "mp_safehouselost_table@";
		char *animID2 = "lost_table_negative_a";

		STREAMING::REQUEST_ANIM_DICT(anim2);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim2))
			WAIT(0);

		AI::TASK_PLAY_ANIM(playerPed, anim2, animID2, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);

	}
	if (deerRun) {
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;
		Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		Hash stripper = GAMEPLAY::GET_HASH_KEY("a_c_deer");
		STREAMING::REQUEST_MODEL(stripper);
		while (!STREAMING::HAS_MODEL_LOADED(stripper))
			WAIT(0);

		int createdPED = PED::CREATE_PED(26, stripper, pos.x, pos.y, pos.z, 1, 1, 0);
		ENTITY::SET_ENTITY_INVINCIBLE(createdPED, false);
		PED::SET_PED_COMBAT_ABILITY(createdPED, 100);
		PED::SET_PED_CAN_SWITCH_WEAPON(createdPED, true);
		//				SET_ENTITY_HEADING(createdPED, 180.0f);

		ENTITY::ATTACH_ENTITY_TO_ENTITY(playerPed, createdPED, -1, 0.0f, 0.35f, 0.72f, 0.0f, 0.0f, 0.0f, 1, 0, 0, 2, 1, 1);

		//deer animation
		char *anim = "creatures@deer@move";
		char *animID = "trot";

		STREAMING::REQUEST_ANIM_DICT(anim);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim))
			WAIT(0);

		AI::TASK_PLAY_ANIM(createdPED, anim, animID, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);

		//charPose
		char *anim2 = "mp_safehouselost_table@";
		char *animID2 = "lost_table_negative_a";

		STREAMING::REQUEST_ANIM_DICT(anim2);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim2))
			WAIT(0);

		AI::TASK_PLAY_ANIM(playerPed, anim2, animID2, 8.0f, 0.0f, -1, 9, 0, 0, 0, 0);

	}
}
void AddCloth(int BodyID, int Part, int &Variation) {
	null = 0;
	AddOption((char*)FloatToString(Part).c_str());
	if (OptionY < 0.6325 && OptionY > 0.1425)
	{
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.26f, 0.26f);
		UI::SET_TEXT_CENTRE(1);

		drawfloat(Variation, 0, 0.233f + menuPos, OptionY);
	}
	/*if (OptionY < 0.6325 && OptionY > 0.1425) //Draws a number a bit closer to the text than normal numbers
	{
	SET_TEXT_FONT(0);
	SET_TEXT_SCALE(0.26f, 0.26f);
	SET_TEXT_CENTRE(1);

	drawfloat(Palette, 0, 0.223f + menuPos, OptionY);
	}*/

	if (menu::printingop == menu::currentop)
	{

		if (IsOptionRJPressed()) {
			int textureVariations = PED::GET_NUMBER_OF_PED_TEXTURE_VARIATIONS(PLAYER::PLAYER_PED_ID(), BodyID, Part) - 2;
			if (textureVariations >= Variation)
				Variation += 1;
		}
		else if (IsOptionRPressed()) {
			int textureVariations = PED::GET_NUMBER_OF_PED_TEXTURE_VARIATIONS(PLAYER::PLAYER_PED_ID(), BodyID, Part) - 2;
			if (textureVariations >= Variation)
				Variation += 1;
		}
		else if (IsOptionLJPressed() && Variation > 0) {
			Variation -= 1;
		}
		if (null) {
			PED::SET_PED_COMPONENT_VARIATION(PLAYER::PLAYER_PED_ID(), BodyID, Part, Variation, 2);
		}
	}
}
void AddClothingProp(int BodyID, int Part, int &Variation) {
	null = 0;
	AddOption((char*)FloatToString(Part).c_str());
	if (OptionY < 0.6325 && OptionY > 0.1425)
	{
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.26f, 0.26f);
		UI::SET_TEXT_CENTRE(1);

		drawfloat(Variation, 0, 0.233f + menuPos, OptionY);
	}
	/*if (OptionY < 0.6325 && OptionY > 0.1425) //Draws a number a bit closer to the text than normal numbers
	{
	SET_TEXT_FONT(0);
	SET_TEXT_SCALE(0.26f, 0.26f);
	SET_TEXT_CENTRE(1);

	drawfloat(Palette, 0, 0.223f + menuPos, OptionY);
	}*/

	if (menu::printingop == menu::currentop)
	{

		if (IsOptionRJPressed()) {
			int textureVariations = PED::GET_NUMBER_OF_PED_PROP_TEXTURE_VARIATIONS(PLAYER::PLAYER_PED_ID(), BodyID, Part) - 2;
			if (textureVariations >= Variation)
				Variation += 1;
		}
		else if (IsOptionRPressed()) {
			int textureVariations = PED::GET_NUMBER_OF_PED_PROP_TEXTURE_VARIATIONS(PLAYER::PLAYER_PED_ID(), BodyID, Part) - 2;
			if (textureVariations >= Variation)
				Variation += 1;
		}
		else if (IsOptionLJPressed() && Variation > 0) {
			Variation -= 1;
		}
		if (null) {
			PED::SET_PED_PROP_INDEX(PLAYER::PLAYER_PED_ID(), BodyID, Part, Variation, 2);
		}
	}
}
int Masks[81] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
void pedcompo1()
{

	// Initialise local variables here:
	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("MASKS");
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 1); i++) {
		AddCloth(1, i, Masks[i]);
	}


}
int Hairs[81] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
void pedcompo2()
{

	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("HAIR");
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 2); i++) {
		AddCloth(2, i, Hairs[i]);
	}
}
int Glasses[81] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void pedcompglasses1()
{

	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("GLASSES");
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_PROP_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 1); i++) {
		AddClothingProp(1, i, Glasses[i]);
	}
} //PROP
int Tops[81] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void pedcompo11()
{

	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("Tops");
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 11); i++) {
		AddCloth(11, i, Tops[i]);
	}

}
int Hats[81] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void pedcompo10()
{
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("HATS");
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_PROP_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 0); i++) {
		AddClothingProp(0, i, Hats[i]);
	}
} //PROP//PROP
int ArmorDraw[81] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
void pedcompo9()
{
	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("Armor");
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 9); i++) {
		AddCloth(9, i, ArmorDraw[i]);
	}

}
int miscTops[64] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
void pedcompo8()
{
	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("Misc Tops");
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 8); i++) {
		AddCloth(8, i, miscTops[i]);
	}


}
int Shoes[81] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
void pedcompo6()
{
	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("SHOES");
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 6); i++) {
		AddCloth(6, i, Shoes[i]);
	}

}
int Accessory1[81] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
void pedcompo5()
{

	// Initialise local variables here:
	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("ACCESORIES 1");
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 5); i++) {
		AddCloth(5, i, Accessory1[i]);
	}

}
int Legs[81] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void pedcompo4()
{
	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("LEGS");
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 4); i++) {
		AddCloth(4, i, Legs[i]);
	}


}
int Torso[81] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
void pedcompo3()
{
	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("TORSO");//3-80
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 3); i++) {
		AddCloth(3, i, Torso[i]);
	}

}
int Accessory2[81] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void pedcompo7()
{

	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);

	// Options' text here:
	AddTitle("ACCESORIES 2");
	for (int i = 0; i < PED::GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(PLAYER::PLAYER_PED_ID(), 7); i++) {
		AddCloth(7, i, Accessory2[i]);
	}

}
void locIpls()
{


	bool ufo1 = 0, cship = 0, nyankton = 0, lfact = 0, Yacht1 = 0, dhosp = 0, cbell = 0, Mmorg = 0, hcarrier = 0, ufo2 = 0;
	AddTitle("IPL LOCATIONS");
	AddOption("North Yankton", nyankton);
	AddOption("UFO Ft. Zancudo", ufo1);
	AddOption("UFO Desert", ufo2);
	AddOption("Heist Carrier", hcarrier);
	AddOption("Cluckin Bell", cbell);
	AddOption("Morgue", Mmorg);
	AddOption("Cargo Ship", cship);
	AddOption("Destroyed hosptal", dhosp);
	AddOption("Heist Yacht", Yacht1);
	AddOption("Lester's Factory", lfact);


	//variables
	Ped myPed = PLAYER::PLAYER_PED_ID();

	//code
	if (nyankton) {
		STREAMING::REQUEST_IPL("plg_01");
		STREAMING::REQUEST_IPL("prologue01");
		STREAMING::REQUEST_IPL("prologue01_lod");
		STREAMING::REQUEST_IPL("prologue01c");
		STREAMING::REQUEST_IPL("prologue01c_lod");
		STREAMING::REQUEST_IPL("prologue01d");
		STREAMING::REQUEST_IPL("prologue01d_lod");
		STREAMING::REQUEST_IPL("prologue01e");
		STREAMING::REQUEST_IPL("prologue01e_lod");
		STREAMING::REQUEST_IPL("prologue01f");
		STREAMING::REQUEST_IPL("prologue01f_lod");
		STREAMING::REQUEST_IPL("prologue01g");
		STREAMING::REQUEST_IPL("prologue01h");
		STREAMING::REQUEST_IPL("prologue01h_lod");
		STREAMING::REQUEST_IPL("prologue01i");
		STREAMING::REQUEST_IPL("prologue01i_lod");
		STREAMING::REQUEST_IPL("prologue01j");
		STREAMING::REQUEST_IPL("prologue01j_lod");
		STREAMING::REQUEST_IPL("prologue01k");
		STREAMING::REQUEST_IPL("prologue01k_lod");
		STREAMING::REQUEST_IPL("prologue01z");
		STREAMING::REQUEST_IPL("prologue01z_lod");
		STREAMING::REQUEST_IPL("plg_02");
		STREAMING::REQUEST_IPL("prologue02");
		STREAMING::REQUEST_IPL("prologue02_lod");
		STREAMING::REQUEST_IPL("plg_03");
		STREAMING::REQUEST_IPL("prologue03");
		STREAMING::REQUEST_IPL("prologue03_lod");
		STREAMING::REQUEST_IPL("prologue03b");
		STREAMING::REQUEST_IPL("prologue03b_lod");
		STREAMING::REQUEST_IPL("prologue03_grv_dug");
		STREAMING::REQUEST_IPL("prologue03_grv_dug_lod");
		STREAMING::REQUEST_IPL("prologue_grv_torch");
		STREAMING::REQUEST_IPL("plg_04");
		STREAMING::REQUEST_IPL("prologue04");
		STREAMING::REQUEST_IPL("prologue04_lod");
		STREAMING::REQUEST_IPL("prologue04b");
		STREAMING::REQUEST_IPL("prologue04b_lod");
		STREAMING::REQUEST_IPL("prologue04_cover");
		STREAMING::REQUEST_IPL("des_protree_end");
		STREAMING::REQUEST_IPL("des_protree_start");
		STREAMING::REQUEST_IPL("des_protree_start_lod");
		STREAMING::REQUEST_IPL("plg_05");
		STREAMING::REQUEST_IPL("prologue05");
		STREAMING::REQUEST_IPL("prologue05_lod");
		STREAMING::REQUEST_IPL("prologue05b");
		STREAMING::REQUEST_IPL("prologue05b_lod");
		STREAMING::REQUEST_IPL("plg_06");
		STREAMING::REQUEST_IPL("prologue06");
		STREAMING::REQUEST_IPL("prologue06_lod");
		STREAMING::REQUEST_IPL("prologue06b");
		STREAMING::REQUEST_IPL("prologue06b_lod");
		STREAMING::REQUEST_IPL("prologue06_int");
		STREAMING::REQUEST_IPL("prologue06_int_lod");
		STREAMING::REQUEST_IPL("prologue06_pannel");
		STREAMING::REQUEST_IPL("prologue06_pannel_lod");
		STREAMING::REQUEST_IPL("prologue_m2_door");
		STREAMING::REQUEST_IPL("prologue_m2_door_lod");
		STREAMING::REQUEST_IPL("plg_occl_00");
		STREAMING::REQUEST_IPL("prologue_occl");
		STREAMING::REQUEST_IPL("plg_rd");
		STREAMING::REQUEST_IPL("prologuerd");
		STREAMING::REQUEST_IPL("prologuerdb");
		STREAMING::REQUEST_IPL("prologuerd_lod");
		STREAMING::REMOVE_IPL("prologue03_grv_cov");
		STREAMING::REMOVE_IPL("prologue03_grv_cov_lod");
		STREAMING::REMOVE_IPL("prologue03_grv_fun");

		ENTITY::SET_ENTITY_COORDS(myPed, 3360.19f, -4849.67f, 111.8f, 1, 0, 0, 1);
	}
	if (lfact) {
		STREAMING::REQUEST_IPL("id2_14_on_fire");
		ENTITY::SET_ENTITY_COORDS(myPed, 716.84f, -962.05f, 31.59f, 1, 0, 0, 1);
	}
	if (Yacht1) {
		STREAMING::REQUEST_IPL("hei_yacht_heist");
		STREAMING::REQUEST_IPL("hei_yacht_heist_Bar");
		STREAMING::REQUEST_IPL("hei_yacht_heist_Bedrm");
		STREAMING::REQUEST_IPL("hei_yacht_heist_Bridge");
		STREAMING::REQUEST_IPL("hei_yacht_heist_DistantLights");
		STREAMING::REQUEST_IPL("hei_yacht_heist_enginrm");
		STREAMING::REQUEST_IPL("hei_yacht_heist_LODLights");
		STREAMING::REQUEST_IPL("hei_yacht_heist_Lounge");
		ENTITY::SET_ENTITY_COORDS(myPed, -2045.8f, -1031.2f, 11.9f, 1, 0, 0, 1);
	}
	if (dhosp) {
		STREAMING::REQUEST_IPL("RC12B_Destroyed");
		STREAMING::REQUEST_IPL("RC12B_HospitalInterior");
		ENTITY::SET_ENTITY_COORDS(myPed, 356.8f, -590.1f, 43.3f, 1, 0, 0, 1);
	}
	if (ufo1) {
		STREAMING::REQUEST_IPL("ufo");
		ENTITY::SET_ENTITY_COORDS(myPed, -2051.99463, 3237.05835, 1456.97021, 1, 0, 0, 1);
	}
	if (ufo2) {
		STREAMING::REQUEST_IPL("ufo");
		ENTITY::SET_ENTITY_COORDS(myPed, 2490.47729, 3774.84351, 2414.035, 1, 0, 0, 1);
	}
	if (Mmorg) {
		STREAMING::REQUEST_IPL("Coroner_Int_on");
		ENTITY::SET_ENTITY_COORDS(myPed, 244.9f, -1374.7f, 39.5f, 1, 0, 0, 1);
	}
	if (cship) {
		STREAMING::REQUEST_IPL("cargoship");
		ENTITY::SET_ENTITY_COORDS(myPed, -90.0f, -2365.8f, 14.3f, 1, 0, 0, 1);
	}
	if (hcarrier) {
		STREAMING::REQUEST_IPL("hei_carrier");
		STREAMING::REQUEST_IPL("hei_carrier_DistantLights");
		STREAMING::REQUEST_IPL("hei_Carrier_int1");
		STREAMING::REQUEST_IPL("hei_Carrier_int2");
		STREAMING::REQUEST_IPL("hei_Carrier_int3");
		STREAMING::REQUEST_IPL("hei_Carrier_int4");
		STREAMING::REQUEST_IPL("hei_Carrier_int5");
		STREAMING::REQUEST_IPL("hei_Carrier_int6");
		STREAMING::REQUEST_IPL("hei_carrier_LODLights");
		ENTITY::SET_ENTITY_COORDS(myPed, 3069.98f, -4632.49f, 16.26f, 1, 0, 0, 1);
	}
	if (cbell) {

		STREAMING::REMOVE_IPL("CS1_02_cf_offmission1");
		STREAMING::REQUEST_IPL("CS1_02_cf_onmission1");
		STREAMING::REQUEST_IPL("CS1_02_cf_onmission2");
		STREAMING::REQUEST_IPL("CS1_02_cf_onmission3");
		STREAMING::REQUEST_IPL("CS1_02_cf_onmission4");
		ENTITY::SET_ENTITY_COORDS(myPed, -72.68752, 6253.72656, 31.08991, 1, 0, 0, 1);
	}




}
#pragma region allobjects
char* objs[] = {
	"PROP_MP_RAMP_03",
	"PROP_MP_RAMP_02",
	"PROP_MP_RAMP_01",
	"PROP_JETSKI_RAMP_01",
	"PROP_WATER_RAMP_03",
	"PROP_VEND_SNAK_01",
	"PROP_TRI_START_BANNER",
	"PROP_TRI_FINISH_BANNER",
	"PROP_TEMP_BLOCK_BLOCKER",
	"PROP_SLUICEGATEL",
	"PROP_SKIP_08A",
	"PROP_SAM_01",
	"PROP_RUB_CONT_01B",
	"PROP_ROADCONE01A",
	"PROP_MP_ARROW_BARRIER_01",
	"PROP_HOTEL_CLOCK_01",
	"PROP_LIFEBLURB_02",
	"PROP_COFFIN_02B",
	"PROP_MP_NUM_1",
	"PROP_MP_NUM_2",
	"PROP_MP_NUM_3",
	"PROP_MP_NUM_4",
	"PROP_MP_NUM_5",
	"PROP_MP_NUM_6",
	"PROP_MP_NUM_7",
	"PROP_MP_NUM_8",
	"PROP_MP_NUM_9",
	"prop_xmas_tree_int",
	"prop_bumper_car_01",
	"prop_beer_neon_01",
	"prop_space_rifle",
	"prop_dummy_01",
	"prop_rub_trolley01a",
	"prop_wheelchair_01_s",
	"PROP_CS_KATANA_01",
	"PROP_CS_DILDO_01",
	"prop_armchair_01",
	"prop_bin_04a",
	"prop_chair_01a",
	"prop_dog_cage_01",
	"prop_dummy_plane",
	"prop_golf_bag_01",
	"prop_arcade_01",
	"hei_prop_heist_emp",
	"prop_alien_egg_01",
	"prop_air_towbar_01",
	"hei_prop_heist_tug",
	"prop_air_luggtrolley",
	"PROP_CUP_SAUCER_01",
	"prop_wheelchair_01",
	"prop_ld_toilet_01",
	"prop_acc_guitar_01",
	"prop_bank_vaultdoor",
	"p_v_43_safe_s",
	"prop_air_woodsteps",
	"Prop_weed_01",
	"prop_a_trailer_door_01",
	"prop_apple_box_01",
	"prop_air_fueltrail1",
	"prop_barrel_02a",
	"prop_barrel_float_1",
	"prop_barrier_wat_03b",
	"prop_air_fueltrail2",
	"prop_air_propeller01",
	"prop_windmill_01",
	"prop_Ld_ferris_wheel",
	"p_tram_crash_s",
	"p_oil_slick_01",
	"p_ld_stinger_s",
	"p_ld_soc_ball_01",
	"prop_juicestand",
	"p_oil_pjack_01_s",
	"prop_barbell_01",
	"prop_barbell_100kg",
	"p_parachute1_s",
	"p_cablecar_s",
	"prop_beach_fire",
	"prop_lev_des_barge_02",
	"prop_lev_des_barge_01",
	"prop_a_base_bars_01",
	"prop_beach_bars_01",
	"prop_air_bigradar",
	"prop_weed_pallet",
	"prop_artifact_01",
	"prop_attache_case_01",
	"prop_large_gold",
	"prop_roller_car_01",
	"prop_water_corpse_01",
	"prop_water_corpse_02",
	"prop_dummy_01",
	"prop_atm_01",
	"hei_prop_carrier_docklight_01",
	"hei_prop_carrier_liferafts",
	"hei_prop_carrier_ord_03",
	"hei_prop_carrier_defense_02",
	"hei_prop_carrier_defense_01",
	"hei_prop_carrier_radar_1",
	"hei_prop_carrier_radar_2",
	"hei_prop_hei_bust_01",
	"hei_prop_wall_alarm_on",
	"hei_prop_wall_light_10a_cr",
	"prop_afsign_amun",
	"prop_afsign_vbike",
	"prop_aircon_l_01",
	"prop_aircon_l_02",
	"prop_aircon_l_03",
	"prop_aircon_l_04",
	"prop_airhockey_01",
	"prop_air_bagloader",
	"prop_air_blastfence_01",
	"prop_air_blastfence_02",
	"prop_air_cargo_01a",
	"prop_air_chock_01",
	"prop_air_chock_03",
	"prop_air_gasbogey_01",
	"prop_air_generator_03",
	"prop_air_stair_02",
	"prop_amb_40oz_02",
	"prop_amb_40oz_03",
	"prop_amb_beer_bottle",
	"prop_amb_donut",
	"prop_amb_handbag_01",
	"prop_amp_01",
	"prop_anim_cash_pile_02",
	"prop_asteroid_01",
	"prop_arm_wrestle_01",
	"prop_ballistic_shield",
	"prop_bank_shutter",
	"prop_barier_conc_02b",
	"prop_barier_conc_05a",
	"prop_barrel_01a",
	"prop_bar_stool_01",
	"prop_basejump_target_01",
};
float clearRange = 20.0f;
void ClearArea() {
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vector3 coords = ENTITY::GET_ENTITY_COORDS(playerPed, 1);
	bool peds = 0, objs = 0, vehs = 0;
	AddTitle("Clear Area");
	AddNumberEasy("Radius", clearRange, 0, clearRange, 1.0f, 1);
	AddOption("Clear Vehicles", vehs);
	AddOption("Clear Peds", peds);
	AddOption("Clear Objects", objs);
	if (vehs) GAMEPLAY::CLEAR_AREA_OF_VEHICLES(coords.x, coords.y, coords.z, clearRange, 0, 0, 0, 0, 0);
	if (peds) GAMEPLAY::CLEAR_AREA_OF_PEDS(coords.x, coords.y, coords.z, clearRange, 0);
	if (objs) GAMEPLAY::CLEAR_AREA_OF_OBJECTS(coords.x, coords.y, coords.z, clearRange, 0);
}
static LPCSTR weaponNames[] = {
	"WEAPON_KNIFE", "WEAPON_NIGHTSTICK", "WEAPON_HAMMER", "WEAPON_BAT", "WEAPON_GOLFCLUB", "WEAPON_CROWBAR",
	"WEAPON_PISTOL", "WEAPON_COMBATPISTOL", "WEAPON_APPISTOL", "WEAPON_PISTOL50", "WEAPON_MICROSMG", "WEAPON_SMG",
	"WEAPON_ASSAULTSMG", "WEAPON_ASSAULTRIFLE", "WEAPON_CARBINERIFLE", "WEAPON_ADVANCEDRIFLE", "WEAPON_MG",
	"WEAPON_COMBATMG", "WEAPON_PUMPSHOTGUN", "WEAPON_SAWNOFFSHOTGUN", "WEAPON_ASSAULTSHOTGUN", "WEAPON_BULLPUPSHOTGUN",
	"WEAPON_STUNGUN", "WEAPON_SNIPERRIFLE", "WEAPON_HEAVYSNIPER", "WEAPON_GRENADELAUNCHER", "WEAPON_GRENADELAUNCHER_SMOKE",
	"WEAPON_RPG", "WEAPON_MINIGUN", "WEAPON_GRENADE", "WEAPON_STICKYBOMB", "WEAPON_SMOKEGRENADE", "WEAPON_BZGAS",
	"WEAPON_MOLOTOV", "WEAPON_FIREEXTINGUISHER", "WEAPON_PETROLCAN",
	"WEAPON_SNSPISTOL", "WEAPON_SPECIALCARBINE", "WEAPON_HEAVYPISTOL", "WEAPON_BULLPUPRIFLE", "WEAPON_HOMINGLAUNCHER",
	"WEAPON_PROXMINE", "WEAPON_SNOWBALL", "WEAPON_VINTAGEPISTOL", "WEAPON_DAGGER", "WEAPON_FIREWORK", "WEAPON_MUSKET",
	"WEAPON_MARKSMANRIFLE", "WEAPON_HEAVYSHOTGUN", "WEAPON_GUSENBERG", "WEAPON_HATCHET", "WEAPON_RAILGUN", "WEAPON_KNUCKLE", "WEAPON_MARKSMANPISTOL",
	"WEAPON_COMBATPDW", "WEAPON_HANDCUFFS", "WEAPON_FLASHLIGHT", "WEAPON_MACHINEPISTOL", "WEAPON_MACHETE"
};
char* ammoWeapon = "";
void AddAmmo(char* text, char* name)
{
	null = 0;
	if (ammoWeapon == name) {
		ostringstream textu;
		textu << "~b~>~s~" << text;
		AddOption((char*)textu.str().c_str(), null);
	}
	else {
		AddOption(text, null);
		if (text == "40kDrop") {
			if (!riskMode) {
				PrintStringBottomCentre("~y~Enable risk mode to use this");
				return;
			}
		}
	}
	if (menu::printingop == menu::currentop)
	{
		if (null)
		{
			if (text == "40kDrop") {
				if (!riskMode) {
					PrintStringBottomCentre("~y~Enable risk mode to use this");
					return;
				}
			}
			else {
				ammoWeapon = name;
			}
		}
	}

}
void Bullets() {

	bool rocket = 0, stickybomb = 0, bfire = 0, blam = 0, bbang = 0, smokegrenade = 0, shootingBmx = 0, hit = 0, electricity = 0, aeggs = 0, bzgaz = 0, molotov = 0, flare = 0, grenade = 0, grenadelauncher = 0, fireworks = 0, moneybags = 0;
	AddTitle("Custom Bullets");
	AddToggle("Enable Custom Bullet", custombullet);
	AddOption("Molotov", molotov);
	AddOption("RPG", rocket);
	AddOption("Flare", flare);
	AddOption("Sticky Bomb", stickybomb);
	AddOption("Grenade", grenade);
	AddOption("Grenade Launcher", grenadelauncher);
	AddOption("Smoke Grenade", smokegrenade);
	AddOption("BZGaz", bzgaz);
	AddOption("Fireworks", fireworks);
	AddOption("Alien Eggs", aeggs);
	AddOption("Electric", electricity);
	AddOption("BeachFire", bfire);
	AddOption("Money Bags", moneybags);
	AddOption("Big Bang", bbang);
	AddOption("Minigun Blame", blam);

	if (custombullet) {
		if (rocket) ammoWeapon = "WEAPON_VEHICLE_ROCKET";
		if (stickybomb) ammoWeapon = "WEAPON_STICKYBOMB";
		if (smokegrenade) ammoWeapon = "WEAPON_SMOKEGRENADE";
		if (bzgaz) ammoWeapon = "WEAPON_BZGAS";
		if (molotov) ammoWeapon = "WEAPON_MOLOTOV";
		if (flare) ammoWeapon = "WEAPON_FLARE";
		if (grenade) ammoWeapon = "WEAPON_GRENADE";
		if (grenadelauncher) ammoWeapon = "WEAPON_GRENADELAUNCHER";
		if (fireworks) ammoWeapon = "WEAPON_FIREWORK";
		if (electricity) ammoWeapon = "WEAPON_STUNGUN";
		if (bfire) ammoWeapon = "campFire";
		if (moneybags) ammoWeapon = "40kDrop";
		if (aeggs) ammoWeapon = "Eggs";
		if (bbang) ammoWeapon = "bigBANG";
		if (blam) ammoWeapon = "blamed";

	}
	else {
		ammoWeapon = "";
	}
}
void Trunk() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 37);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 37);
	if (currentMod < 10)
		AddVehicleMod(veh, 37, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 37, 101, 0, 1, 1);
	AddTitle("Trunk");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 37, i);
		else
			AddVehicleMod(veh, 37, i, 0, 1);
	}
}
void CustomPlate() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 26);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 26);
	if (currentMod < 10)
		AddVehicleMod(veh, 26, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 26, 101, 0, 1, 1);
	AddTitle("Custom Plate");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 26, i);
		else
			AddVehicleMod(veh, 26, i, 0, 1);
	}
}
void PlateHolder() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 25);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 25);
	if (currentMod < 10)
		AddVehicleMod(veh, 25, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 25, 101, 0, 1, 1);
	AddTitle("Plate Holder");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 25, i);
		else
			AddVehicleMod(veh, 25, i, 0, 1);
	}
}
void Decals() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 48);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 48);
	if (currentMod < 10)
		AddVehicleMod(veh, 48, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 48, 101, 0, 1, 1);
	AddTitle("Decals");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 48, i);
		else
			AddVehicleMod(veh, 48, i, 0, 1);
	}
}
void InteriorCoating() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 27);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 27);
	AddOption("Coating Color", null, nullFunc, SUB::DASHCOLOR);
	if (currentMod < 10)
		AddVehicleMod(veh, 27, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 27, 101, 0, 1, 1);
	AddTitle("Interior Coating");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 27, i);
		else
			AddVehicleMod(veh, 27, i, 0, 1);
	}
}
void DecorativeShit() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 28);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 28);
	if (currentMod < 10)
		AddVehicleMod(veh, 28, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 28, 101, 0, 1, 1);
	AddTitle("Interior Decoration");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 28, i);
		else
			AddVehicleMod(veh, 28, i, 0, 1);
	}
}
void Speedometers() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	AddOption("Light Color", null, nullFunc, SUB::DASHLIGHT);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 30);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 30);
	if (currentMod < 10)
		AddVehicleMod(veh, 30, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 30, 101, 0, 1, 1);
	AddTitle("Speedometers");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 30, i);
		else
			AddVehicleMod(veh, 30, i, 0, 1);
	}
}
void Steering() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 33);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 33);
	if (currentMod < 10)
		AddVehicleMod(veh, 33, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 33, 101, 0, 1, 1);
	AddTitle("Steering Wheel");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 33, i);
		else
			AddVehicleMod(veh, 33, i, 0, 1);
	}
}
void Shifter() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 34);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 34);
	if (currentMod < 10)
		AddVehicleMod(veh, 34, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 34, 101, 0, 1, 1);
	AddTitle("Shifter");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 34, i);
		else
			AddVehicleMod(veh, 34, i, 0, 1);
	}
}
void DecorativePlates() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 35);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 35);
	if (currentMod < 10)
		AddVehicleMod(veh, 35, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 35, 101, 0, 1, 1);
	AddTitle("Decorative Plates");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 35, i);
		else
			AddVehicleMod(veh, 35, i, 0, 1);
	}
}
void HydraulicPump() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 38);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 38);
	if (currentMod < 10)
		AddVehicleMod(veh, 38, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 38, 101, 0, 1, 1);
	AddTitle("Hydraulic Pumps");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 38, i);
		else
			AddVehicleMod(veh, 38, i, 0, 1);
	}
}
void MotorBlock() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 39);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 39);
	if (currentMod < 10)
		AddVehicleMod(veh, 39, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 39, 101, 0, 1, 1);
	AddTitle("Motor Block");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 39, i);
		else
			AddVehicleMod(veh, 39, i, 0, 1);
	}
}
void AirFilter() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 40);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 40);
	if (currentMod < 10)
		AddVehicleMod(veh, 40, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 40, 101, 0, 1, 1);
	AddTitle("Air Filter");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 40, i);
		else
			AddVehicleMod(veh, 40, i, 0, 1);
	}
}
void Tank() {
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 0);
	Entity e = playerPed;
	RequestControlOfEnt(veh);
	float maxMod = VEHICLE::GET_NUM_VEHICLE_MODS(veh, 45);
	float currentMod = VEHICLE::GET_VEHICLE_MOD(veh, 45);
	if (currentMod < 10)
		AddVehicleMod(veh, 45, 101, 0, 0, 1);
	else
		AddVehicleMod(veh, 45, 101, 0, 1, 1);
	AddTitle("Tank");
	for (int i = 0; i < maxMod; i++) {
		if (currentMod != i)
			AddVehicleMod(veh, 45, i);
		else
			AddVehicleMod(veh, 45, i, 0, 1);
	}
}
void Bennys() {
	AddTitle("Benny's");
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, 1);
	AddOption("Trunk", null, nullFunc, SUB::TRUNK);
	AddOption("Custom plate", null, nullFunc, SUB::CUSTOMPLATE);
	AddOption("Plate Holder", null, nullFunc, SUB::PLATEHOLDER);
	AddOption("Decals", null, nullFunc, SUB::DECALS);
	AddOption("Interior Coating", null, nullFunc, SUB::INTERIORCOATING);
	AddOption("Decorative Shit", null, nullFunc, SUB::DECORATIVESHIT);
	AddOption("Speedometer/other shit", null, nullFunc, SUB::SPEEDOMETERS);
	AddOption("Steering wheel", null, nullFunc, SUB::STEERING);
	AddOption("Shifter", null, nullFunc, SUB::SHIFTER);
	AddOption("Decorative Plates", null, nullFunc, SUB::DECORATIVEPLATES);
	AddOption("Hydraulic Pump", null, nullFunc, SUB::HYDRAULICPUMP);
	AddOption("Motor Block", null, nullFunc, SUB::MOTORBLOCK);
	AddOption("Air Filter", null, nullFunc, SUB::AIRFILTER);
	AddOption("Tank", null, nullFunc, SUB::TANK);
}
char* DashboardColors(int id) {
	switch (id) {
	case 0:
		return "BLACK";
		break;
	case 1:
		return "GRAPHITE";
		break;
	case 2:
		return "ANTHR_BLACK";
		break;
	case 3:
		return "BLACK_STEEL";
		break;
	case 4:
		return "DARK_SILVER";
		break;
	case 5:
		return "BLUE_SILVER";
		break;
	case 6:
		return "ROLLED_STEEL";
		break;
	case 7:
		return "SHADOW_SILVER";
		break;
	case 8:
		return "STONE_SILVER";
		break;
	case 9:
		return "MIDNIGHT_SILVER";
		break;
	case 10:
		return "CAST_IRON_SIL";
		break;
	case 11:
		return "RED";
		break;
	case 12:
		return "TORINO_RED";
		break;
	case 13:
		return "LAVA_RED";
		break;
	case 14:
		return "BLAZE_RED";
		break;
	case 15:
		return "GRACE_RED";
		break;
	case 16:
		return "GARNET_RED";
		break;
	case 17:
		return "SUNSET_RED";
		break;
	case 18:
		return "CABERNET_RED";
		break;
	case 19:
		return "WINE_RED";
		break;
	case 20:
		return "CANDY_RED";
		break;
	case 21:
		return "PINK";
		break;
	case 22:
		return "SALMON_PINK";
		break;
	case 23:
		return "SUNRISE_ORANGE";
		break;
	case 24:
		return "ORANGE";
		break;
	case 25:
		return "BRIGHT_ORANGE";
		break;
	case 26:
		return "BRONZE";
		break;
	case 27:
		return "YELLOW";
		break;
	case 28:
		return "RACE_YELLOW";
		break;
	case 29:
		return "FLUR_YELLOW";
		break;
	case 30:
		return "DARK_GREEN";
		break;
	case 31:
		return "RACING_GREEN";
		break;
	case 32:
		return "SEA_GREEN";
		break;
	case 33:
		return "OLIVE_GREEN";
		break;
	case 34:
		return "BRIGHT_GREEN";
		break;
	case 35:
		return "PETROL_GREEN";
		break;
	case 36:
		return "LIME_GREEN";
		break;
	case 37:
		return "MIDNIGHT_BLUE";
		break;
	case 38:
		return "GALAXY_BLUE";
		break;
	case 39:
		return "DARK_BLUE";
		break;
	case 40:
		return "SAXON_BLUE";
		break;
	case 41:
		return "MARINER_BLUE";
		break;
	case 42:
		return "HARBOR_BLUE";
		break;
	case 43:
		return "DIAMOND_BLUE";
		break;
	case 44:
		return "SURF_BLUE";
		break;
	case 45:
		return "NAUTICAL_BLUE";
		break;
	case 46:
		return "RACING_BLUE";
		break;
	case 47:
		return "ULTRA_BLUE";
		break;
	case 48:
		return "LIGHT_BLUE";
		break;
	case 49:
		return "CHOCOLATE_BROWN";
		break;
	case 50:
		return "BISON_BROWN";
		break;
	case 51:
		return "CREEK_BROWN";
		break;
	case 52:
		return "UMBER_BROWN";
		break;
	case 53:
		return "MAPLE_BROWN";
		break;
	case 54:
		return "BEECHWOOD_BROWN";
		break;
	case 55:
		return "SIENNA_BROWN";
		break;
	case 56:
		return "SADDLE_BROWN";
		break;
	case 57:
		return "MOSS_BROWN";
		break;
	case 58:
		return "WOODBEECH_BROWN";
		break;
	case 59:
		return "STRAW_BROWN";
		break;
	case 60:
		return "SANDY_BROWN";
		break;
	case 61:
		return "BLEECHED_BROWN";
		break;
	case 62:
		return "SPIN_PURPLE";
		break;
	case 63:
		return "MIGHT_PURPLE";
		break;
	case 64:
		return "BRIGHT_PURPLE";
		break;
	case 65:
		return "CREAM";
		break;
	case 66:
		return "WHITE";
		break;
	case 67:
		return "FROST_WHITE";
		break;
	}
	return "";
}
char* DashboardLightColor(int id) {
	switch (id) {
	case 0:
		return "SILVER";
		break;
	case 1:
		return "BLUE_SILVER";
		break;
	case 2:
		return "ROLLED_STEEL";
		break;
	case 3:
		return "SHADOW_SILVER";
		break;
	case 4:
		return "WHITE";
		break;
	case 5:
		return "FROST_WHITE";
		break;
	case 6:
		return "CREAM";
		break;
	case 7:
		return "SIENNA_BROWN";
		break;
	case 8:
		return "SADDLE_BROWN";
		break;
	case 9:
		return "MOSS_BROWN";
		break;
	case 10:
		return "WOODBEECH_BROWN";
		break;
	case 11:
		return "STRAW_BROWN";
		break;
	case 12:
		return "SANDY_BROWN";
		break;
	case 13:
		return "BLEECHED_BROWN";
		break;
	case 14:
		return "GOLD";
		break;
	case 15:
		return "BRONZE";
		break;
	case 16:
		return "YELLOW";
		break;
	case 17:
		return "RACE_YELLOW";
		break;
	case 18:
		return "FLUR_YELLOW";
		break;
	case 19:
		return "ORANGE";
		break;
	case 20:
		return "BRIGHT_ORANGE";
		break;
	case 21:
		return "SUNRISE_ORANGE";
		break;
	case 22:
		return "RED";
		break;
	case 23:
		return "TORINO_RED";
		break;
	case 24:
		return "FORMULA_RED";
		break;
	case 25:
		return "LAVA_RED";
		break;
	case 26:
		return "BLAZE_RED";
		break;
	case 27:
		return "GRACE_RED";
		break;
	case 28:
		return "GARNET_RED";
		break;
	case 29:
		return "CANDY_RED";
		break;
	case 30:
		return "HOT PINK";
		break;
	case 31:
		return "PINK";
		break;
	case 32:
		return "SALMON_PINK";
		break;
	case 33:
		return "PURPLE";
		break;
	case 34:
		return "BRIGHT_PURPLE";
		break;
	case 35:
		return "SAXON_BLUE";
		break;
	case 36:
		return "BLUE";
		break;
	case 37:
		return "MARINER_BLUE";
		break;
	case 38:
		return "HARBOR_BLUE";
		break;
	case 39:
		return "DIAMOND_BLUE";
		break;
	case 40:
		return "SURF_BLUE";
		break;
	case 41:
		return "NAUTICAL_BLUE";
		break;
	case 42:
		return "RACING_BLUE";
		break;
	case 43:
		return "ULTRA_BLUE";
		break;
	case 44:
		return "LIGHT_BLUE";
		break;
	case 45:
		return "SEA_GREEN";
		break;
	case 46:
		return "BRIGHT_GREEN";
		break;
	case 47:
		return "PETROL_GREEN";
		break;
	case 48:
		return "LIME_GREEN";
		break;
	}
	return "";
}
void AddVehicleDashboardColor(Vehicle veh, char* dashLabel, int colorId) {
	null = 0;
	RequestControlOfEnt(veh);
	AddOption(UI::_GET_LABEL_TEXT(dashLabel), null);
	RequestControlOfEnt(veh);
	int currentEquipped;
	if (colorId == currentEquipped) {
		if (colorId < 14) {
			drawEquippedIcon(0.243f + menuPos, OptionY + 0.0170f);
		}
		else {
			if (menu::currentop >= colorId + 1) {
				drawEquippedIcon(0.243f + menuPos, OptionY + 0.0170f);
			}
		}
	}
	if (menu::printingop == menu::currentop)
	{
		if (null) {
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
		}
	}
}
void AddVehicleDashboardLightColor(Vehicle veh, char* dashLabel, int colorId) {
	null = 0;
	RequestControlOfEnt(veh);
	AddOption(UI::_GET_LABEL_TEXT(dashLabel), null);
	RequestControlOfEnt(veh);
	int currentEquipped;
	if (colorId == currentEquipped) {
		if (colorId < 14) {
			drawEquippedIcon(0.243f + menuPos, OptionY + 0.0170f);
		}
		else {
			if (menu::currentop >= colorId + 1) {
				drawEquippedIcon(0.243f + menuPos, OptionY + 0.0170f);
			}
		}
	}
	if (menu::printingop == menu::currentop)
	{
		if (null) {
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
		}
	}
}
vector<int> DashLightsIds = { 4, 5, 6, 7, 111, 112, 107, 104, 98, 100, 102, 99, 105, 106, 37, 90, 88, 89, 91, 38, 138, 36, 27, 28, 29, 150, 30, 31, 32, 35, 135, 137, 136, 71, 145, 63, 64, 65, 66, 67, 68, 69, 73, 70, 74, 51, 53, 54 };
void ListLightColors(bool useSelectedPlayer = 0) {
	Vehicle veh;
	if (useSelectedPlayer)
		veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer), 0);
	else veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
	int pearlescentColor = 0, wheelColor = 0;

	VEHICLE::GET_VEHICLE_EXTRA_COLOURS(veh, &pearlescentColor, &wheelColor);
	AddTitle("Dash Light Color");
	for (int i = 0; i < 49; i++) {
		char* name = DashboardLightColor(i);
		AddVehicleDashboardLightColor(veh, name, DashLightsIds[i]);
	}
}
vector<int> DashBoardColorIds = { 0, 1, 11, 2, 3, 5, 6, 7, 8, 9, 10, 27, 28, 150, 30, 31, 32, 33, 34, 143, 35, 137, 136, 36, 38, 138, 90, 88, 89, 91, 49, 50, 51, 52, 53, 54, 92, 141, 61, 62, 63, 65, 66, 67, 68, 69, 73, 70, 74, 96, 101, 95, 94, 97, 103, 104, 98, 100, 102, 99, 105, 106, 72, 146, 145, 107, 111, 112 };

void ListDashboardColors(bool useSelectedPlayer = 0) {

	Vehicle veh;
	if (useSelectedPlayer)
		veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer), 0);
	else veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);

	int pearlescentColor = 0, wheelColor = 0;
	VEHICLE::GET_VEHICLE_EXTRA_COLOURS(veh, &pearlescentColor, &wheelColor);
	AddTitle("Dashboard Color");
	for (int i = 0; i < 68; i++) {
		char* name = DashboardColors(i);
		AddVehicleDashboardColor(veh, name, DashBoardColorIds[i]);
	}
}
void AddBennyWheelType(char* name, int baseId, int multiplier) {
	null = 0;
	AddOption(name, null);
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
	int ModToSet = baseId + (31 * multiplier);
	int CurrentMod = VEHICLE::GET_VEHICLE_MOD(veh, 23);
	if (OptionY < 0.6325 && OptionY > 0.1425)
	{
		if (CurrentMod == ModToSet) drawEquippedIcon(0.243f + menuPos, OptionY + 0.0170f);
	}

	if (menu::printingop == menu::currentop)
	{
		if (null) {
			RequestControlOfEnt(veh);
			VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
			VEHICLE::SET_VEHICLE_MOD(veh, 23, ModToSet, 0);
		}
	}
}
void DisplayWheelsMods() {
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
	if (VEHICLE::GET_VEHICLE_WHEEL_TYPE(veh) != 8) {
		PrintStringBottomCentre("~b~You need a set of Benny's custom wheels first");
		menu::SetSub_previous();
	}
	int dWheel = VEHICLE::GET_VEHICLE_MOD(veh, 23);
	int EquippedSet = 0;

	if (dWheel > 185) {
		EquippedSet = 6;
	}
	else if (dWheel > 154) {
		EquippedSet = 5;
	}
	else if (dWheel > 123) {
		EquippedSet = 4;
	}
	else if (dWheel > 92) {
		EquippedSet = 3;
	}
	else if (dWheel > 61) {
		EquippedSet = 2;
	}
	else if (dWheel > 30) {
		EquippedSet = 1;
	}
	else {
		EquippedSet = 0;
	}
	int BaseWheel = dWheel - (31 * EquippedSet);
	AddTitle("Tire design");
	AddBennyWheelType("Stock Tires", BaseWheel, 0);
	AddBennyWheelType("White Lines", BaseWheel, 1);
	AddBennyWheelType("Classic White Wall", BaseWheel, 2);
	AddBennyWheelType("Retro White Wall", BaseWheel, 3);
	AddBennyWheelType("Red Lines", BaseWheel, 4);
	AddBennyWheelType("Blue Lines", BaseWheel, 5);
	AddBennyWheelType("Atomic", BaseWheel, 6);

}
void VehicleWeapons()

{


	// Initialise local variables here:
	bool maxUpg = 0, nor = 0, SpawnRVeh = 0;
	//variables
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
	Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

	// Options' text here:
	bool all = 0, spacedocksp = 0;
	AddTitle("Ablaze Menu");
	AddTitle("~b~Vehicle Weapons");

	AddOption("Spawn SpaceDocker", spacedocksp);
	AddToggle("Vehicle Rockets", featureVehRockets);
	AddToggle("Vehicle Fireworks", featureVehFireworks);
	AddToggle("Tank Rounds", featureVehtrounds);
	AddToggle("Passenger Rocket", featureVehprocked);
	AddToggle("Snow Balls", featureVehsnowball);
	AddToggle("Balls", featureVehballs);
	AddToggle("Electricity", featureVehtaser);
	AddToggle("Green Laser", featureVehlaser1);
	AddToggle("Red Laser", featureVehlaser2);
	AddToggle("Bullets", featureVehLight);



	/*		"WEAPON_ASSAULTSHOTGUN",
	"VEHICLE_WEAPON_PLAYER_LASER",
	"VEHICLE_WEAPON_ENEMY_LASER",
	"WEAPON_BALL",
	"WEAPON_SNOWBALL",
	"WEAPON_FIREWORK",
	"VEHICLE_WEAPON_TANK",
	"WEAPON_PASSENGER_ROCKET"

	WEAPON_STUNGUN  */



	// Options' code here:

	if (spacedocksp)
	{
		Vehicle zentorno = sub::SpawnVehicle("DUNE2", pos, false);

	}
}
float AccelerationMultiplier = 0, BrakesMultiplier = 0, SuspensionHeight = 0;
bool rpmMultiplier = 0, torqueMultiplier = 0;
float lightMultiplier = 0;
int rpm = 0, torque = 0;
int vehDamMult = 0, vehDefMult = 0;
bool vehDamageMult = 0, vehDefenseMult = 0;
int NumberKeyboard() {
	GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(1, "", "", "", "", "", "", 5);
	while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
	if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) return 0;
	return atof(GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT());
}
void AddFloat(char* text, double value, __int8 decimal_places, float &val, double inc = 1.0, bool &toggled = null)
{
	null = 0;
	AddOption(text, null);

	if (OptionY < 0.6325 && OptionY > 0.1425)
	{
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.26f, 0.26f);
		UI::SET_TEXT_CENTRE(1);
		char* buff = "~b~<~s~ ";
		for (int i = 0; i <= decimal_places; i++) AddStrings(buff, " ");
		AddStrings(buff, " ~b~>");
		drawstring(buff, 0.233f + menuPos, OptionY);
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.26f, 0.26f);
		UI::SET_TEXT_CENTRE(1);
		drawfloat(value, decimal_places, 0.233f + menuPos, OptionY);
	}

	if (menu::printingop == menu::currentop)

	{
		if (null) {
			val = NumberKeyboard();
			toggled = 1;
		}
		else if (IsOptionRJPressed()) {
			toggled = 1;
			val = val + (double)inc;
		}
		else if (IsOptionRPressed()) {
			toggled = 1;
			val = val + (double)inc;
		}
		else if (IsOptionLJPressed()) {
			toggled = 1;
			val -= (double)inc;
		}
		else if (IsOptionLPressed()) {
			toggled = 1;
			val -= (double)inc;
		}
	}
}
void VehiclesMultiplier() {
	bool vehDamageMult = 0, vehDefenseMult = 0, lights = 0, rpmChanged = 0, rpmDisabled = 0, torqueChanged = 0, torqueDisabled = 0;
	AddTitle("Vehicle Multipliers");
	AddToggle("Vehicle RPM", vehrpm, null, rpmDisabled);
	AddIntEasy("Set RPM", rpm, rpm, 1, 0, rpmChanged);
	AddToggle("Vehicle Torque", vehTorque, null, torqueDisabled);
	AddIntEasy("Set Torque", torque, torque, 1, 0, torqueChanged);
	AddIntEasy("Damage Multiplier", vehDamMult, vehDamMult, 1, 0, vehDamageMult);
	AddIntEasy("Defense Multiplier", vehDefMult, vehDefMult, 1, 0, vehDefenseMult);
	AddFloat("Acceleration Multiplier", AccelerationMultiplier, 2, AccelerationMultiplier, 0.01f);
	AddFloat("Brakes Multiplier", BrakesMultiplier, 2, BrakesMultiplier, 0.01f);
	AddFloat("Suspension Height", SuspensionHeight, 2, SuspensionHeight, 0.01f);
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
	Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
	if (lightMultiplier < 1) lightMultiplier = 1;
	if (lights)
		VEHICLE::SET_VEHICLE_LIGHT_MULTIPLIER(veh, lightMultiplier);
	/*MULTIPLIERS*/
	if (vehDamageMult) {
		RequestControlOfEnt(veh);
		PLAYER::SET_PLAYER_VEHICLE_DAMAGE_MODIFIER(PLAYER::PLAYER_ID(), vehDamMult);
	}
	if (vehDefenseMult) {
		RequestControlOfEnt(veh);
		PLAYER::SET_PLAYER_VEHICLE_DEFENSE_MODIFIER(PLAYER::PLAYER_ID(), vehDamMult);
	}
	if (rpmChanged && vehrpm)
	{

		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
		{
			RequestControlOfEnt(veh);
			VEHICLE::_SET_VEHICLE_ENGINE_POWER_MULTIPLIER(veh, rpm);
		}
	}
	if (rpmDisabled) {
		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
		{
			RequestControlOfEnt(veh);
			VEHICLE::_SET_VEHICLE_ENGINE_POWER_MULTIPLIER(veh, 0);
		}
	}
	if (torqueChanged && vehTorque)
	{

		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
		{
			RequestControlOfEnt(veh);
			VEHICLE::_SET_VEHICLE_ENGINE_POWER_MULTIPLIER(veh, torque);
		}
	}
	if (torqueDisabled) {
		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
		{
			RequestControlOfEnt(veh);
			VEHICLE::_SET_VEHICLE_ENGINE_POWER_MULTIPLIER(veh, 0);
		}
	}
}
void menu::submenu_switch()
{ // Make calls to submenus over here.

	switch (currentsub)
	{
	case SUB::MAINMENU:					sub::MainMenu(); break;
	case SUB::SETTINGS:					sub::Settings(); break;
	case SUB::SETTINGS_COLOURS:			sub::SettingsColours(); break;
	case SUB::SETTINGS_COLOURS2:		sub::SettingsColours2(); break;
	case SUB::SETTINGS_FONTS:			sub::SettingsFonts(); break;
	case SUB::SETTINGS_FONTS2:			sub::SettingsFonts2(); break;
	case SUB::SAMPLE:					sub::SampleSub(); break;
	case SUB::YOURSUB:					sub::YourSub(); break;
	case SUB::VEHICLE_SPAWNER:			sub::VehicleSpawner(); break;
	case SUB::SELFMODOPTIONS:			sub::SelfModsOptionsMenu(); break;
	case SUB::ONLINEPLAYERS:            sub::OnlinePlayer(); break;
	case SUB::FAVORITE_VEV:             favoriteClass(); break;
	case SUB::SUPERCAR_VEV:				supercarClass(); break;
	case SUB::LOWRIDER_VEV:				LowridersClass(); break;
	case SUB::MUSCLE_VEV:				muscleClass(); break;
	case SUB::OFF_ROADS_VEV:			offroadsClass(); break;
	case SUB::SPORTS_VEV:				sportsClass(); break;
	case SUB::SPORTS_CLASSICS_VEV:		sportsclassicsClass(); break;
	case SUB::PLANES_VEV:				planesClass(); break;
	case SUB::SUVS_VEV:					suvsClass(); break;
	case SUB::SEDANS_VEV:				sedansClass(); break;
	case SUB::COMPACT_VEV:				compactClass(); break;
	case SUB::COUPES_VEV:				coupesClass(); break;
	case SUB::EMERGENCY_VEV:			emergencyClass(); break;
	case SUB::MILITARY_VEV:				militaryClass(); break;
	case SUB::VANS_VEV:					vansClass(); break;
	case SUB::MOTORCYCLES_VEV:			motorcyclesClass(); break;
	case SUB::HELICOPTERS_VEV:			helicoptersClass(); break;
	case SUB::SERVICES_VEV:				servicesClass(); break;
	case SUB::INDUSTRIAL_VEV:			industrialClass(); break;
	case SUB::BOATS_VEV:				boatsClass(); break;
	case SUB::XYAYSDKKHAJDK:			smug(); break;
	case SUB::CYCLES_VEV:				cyclesClass(); break;
	case SUB::UTILITY_VEV:				utilityClass(); break;
	case SUB::GUNRUNNING:				gunclass(); break;
//	case SUB::SMUG: smug(); break;
	case SUB::WEAPONSMENU:           sub::gWeaponMenu(); break;
	case SUB::SELECTEDPLAYER:		 sub::PlayerMenu(selpName, selPlayer); break;
	case SUB::OPLAVEHOPTIONS:		sub::oPlayVehicleOptionsMenu(selpName, selPlayer); break;
	case SUB::OATTACHMENTOPTIONS: sub::oAttachmentOptionsMenu(selpName, selPlayer); break;
	case SUB::SELPLAYERPTFX:	sub::PlayerParticleFX(selpName, selPlayer); break;
	case SUB::ALLPLAYEROPTIONS: sub::oAllPlayerOptionsMenu(); break;
	case SUB::VEHICLEMODSA:		sub::VehicleMods(); break;
	case SUB::OPLAYEROPTIONS:	oPlayerOptMenu(selpName, selPlayer); break;
	case SUB::TELEPORTLOCATIONS: sub::tp(); break;
	case SUB::MODDED:		sub::Modded(); break;
	case SUB::HIGH:			sub::High(); break;
	case SUB::SEA:			sub::sea(); break;
	case SUB::OTHER:		sub::other(); break;
	case SUB::STORES: sub::stores(); break;
	case SUB::SINGLE: sub::sp(); break;
	case SUB::LANDMARKS: sub::Landmarks(); break;
	case SUB::OPLAYERATTACKERS: sub::oPlayersSendAttackers(selpName, selPlayer); break;
	case SUB::OPLAYERDROPOPTIONS: sub:oPlayerDROPMenu(selpName, selPlayer); break;
	case SUB::VEHICLE_LSC2:  sub::VehicleLSCSubMenu2(); break;
	case SUB::SPOILER:					Spoiler(); break;
	case SUB::FRONTBUMPER:				FrontBumper(); break;
	case SUB::REARBUMPER:				RearBumper(); break;
	case SUB::SIDESKIRT:				SideSkirt(); break;
	case SUB::EXHAUST:					Exhaust(); break;
	case SUB::FRAME:					Frame(); break;
	case SUB::GRILLE:					Grille(); break;
	case SUB::HOOD:						Hood(); break;
	case SUB::FENDER:					Fender(); break;
	case SUB::RIGHTFENDER:				RightFender(); break;
	case SUB::ROOF:						Roof(); break;
	case SUB::ENGINE:					Engine(); break;
	case SUB::BRAKES:					Brakes(); break;
	case SUB::TRANSMISSION:				Transmission(); break;
	case SUB::HORNS:					Horns(); break;
	case SUB::SUSPENSION:				Suspension(); break;
	case SUB::ARMOR:					Armor(); break;
	case SUB::WHEELSMOKE:				TireSmoke(); break;
	case SUB::WTINT:					WindowsTint(0); break;
	case SUB::GAMEPAINT:				sub::LSCPaintSelector(); break;
	case SUB::METALLIC:					gamePaint(0); break;
	case SUB::CLASSIC:					gamePaint(1); break;
	case SUB::PEARLESCENT:				gamePaint(4); break;
	case SUB::MATTE:					gamePaint(2); break;
	case SUB::METALS:					gamePaint(3); break;
	case SUB::SMETALLIC:				gamePaint(0, 1); break;
	case SUB::SCLASSIC:					gamePaint(1, 1); break;
	case SUB::SPEARLESCENT:				gamePaint(4, 1); break;
	case SUB::SMATTE:					gamePaint(2, 1); break;
	case SUB::SMETALS:					gamePaint(3, 1); break;
	case SUB::SPAINT:					sub::LSCPaintSelector(0, 1); break;
	case SUB::PPAINT:					sub::LSCPaintSelector(1, 0); break;
	case SUB::NEON:						Neon(false); break;
	case SUB::WSPORT:					Wheels(1, 5); break;
	case SUB::WMUSCLE:					Wheels(1, 3); break;
	case SUB::WLOWRIDER:				Wheels(1, 2); break;
	case SUB::WSUV:						Wheels(1, 6); break;
	case SUB::WOFFROAD:					Wheels(1, 4); break;
	case SUB::WTUNER:					Wheels(1, 7); break;
	case SUB::WBENNY:					Wheels(1, 8); break;
	case SUB::WBIKE:					Wheels(1, 0); break;
	case SUB::WHIGHEND:					Wheels(1, 1); break;
	case SUB::WHEEL:					Wheels(0); break;
	case SUB::CHANGEWHEELS:				Wheels(1); break;
	case SUB::MAPMOD: sub::MapMod(); break;
	case SUB::MAPMOD_MAZEDEMO:			sub::MapMod(0); break;
	case SUB::MAPMOD_MAZEROOFRAMP:		sub::MapMod(1); break;
	case SUB::MAPMOD_BEACHFERRISRAMP:	sub::MapMod(2); break;
	case SUB::MAPMOD_MOUNTCHILLIADRAMP: sub::MapMod(3); break;
	case SUB::MAPMOD_AIRPORTMINIRAMP:	sub::MapMod(4); break;
	case SUB::MAPMOD_AIRPORTGATERAMP:	sub::MapMod(5); break;
	case SUB::MAPMOD_UFOTOWER:			sub::MapMod(6); break;
	case SUB::MAPMOD_MAZEBANKRAMPS:		sub::MapMod(7); break;
	case SUB::MAPMOD_FREESTYLEMOTOCROSS:sub::MapMod(8); break;
	case SUB::MAPMOD_HALFPIPEFUNTRACK:	sub::MapMod(9); break;
	case SUB::MAPMOD_AIRPORTLOOP:		sub::MapMod(10); break;
	case SUB::MAPMOD_MAZEBANKMEGARAMP:	sub::MapMod(11); break;
	case SUB::BODYGUARDSPAWNER: BodyguardSpawner(); break;
	case SUB::FORCESPAWNER: sub::forceSpawnera(); break;
	case SUB::FORCEGUN: GravityGunSettings(); break;
	case SUB::ANIMAL: AnimalRiddin(); break;
	case SUB::CUSTOMCHARAC:				sub::CustomizeChar(); break;
	case SUB::IPLOCATIONS:				locIpls(); break;
	case SUB::PEDCOMP1:					pedcompo1(); break;
	case SUB::PEDCOMP2:					pedcompo2(); break;
	case SUB::PEDCOMP3:					pedcompo3(); break;
	case SUB::PEDCOMP4:					pedcompo4(); break;
	case SUB::PEDCOMP5:					pedcompo5(); break;
	case SUB::PEDCOMP6:					pedcompo6(); break;
	case SUB::PEDCOMP7:					pedcompo7(); break;
	case SUB::PEDCOMP8:					pedcompo8(); break;
	case SUB::PEDCOMP9:					pedcompo9(); break;
	case SUB::PEDCOMP10:				pedcompo10(); break;
	case SUB::PEDCOMP11:				pedcompo11(); break;
	case SUB::PEDCOMPGLASSES:			pedcompglasses1(); break;
	case SUB::FUNNYVEHICLES: sub::FunnyVehicles(); break;
	case SUB::NPCSPAWNER: NPCSpawner(); break;
	case SUB::OPEDSPAWNER: oPedSpawner(); break;
	case SUB::CLEARAERA: ClearArea(); break;
	case SUB::BULLET: Bullets(); break;
	case SUB::BENNYS:					Bennys(); break;
	case SUB::TRUNK:					Trunk(); break;
	case SUB::CUSTOMPLATE:				CustomPlate(); break;
	case SUB::PLATEHOLDER:				PlateHolder(); break;
	case SUB::DECALS:					Decals(); break;
	case SUB::INTERIORCOATING:			InteriorCoating(); break;
	case SUB::DECORATIVESHIT:			DecorativeShit(); break;
	case SUB::SPEEDOMETERS:				Speedometers(); break;
	case SUB::STEERING:					Steering(); break;
	case SUB::SHIFTER:					Shifter(); break;
	case SUB::DECORATIVEPLATES:			DecorativePlates(); break;
	case SUB::HYDRAULICPUMP:			HydraulicPump(); break;
	case SUB::MOTORBLOCK:				MotorBlock(); break;
	case SUB::AIRFILTER:				AirFilter(); break;
	case SUB::TANK:						Tank(); break;
	//case SUB::LISTCUSTOMMAPMODS:		ListCustomMapMods(); break;
	case SUB::DASHCOLOR:				ListDashboardColors(); break;
	case SUB::DASHLIGHT:				ListLightColors(); break;
	case SUB::WHEELSTRIPE:				DisplayWheelsMods(); break;
	case SUB::SKINCHANGER: SkinChanger(); break;
	case SUB::SPEEDLIMIT: sub::limitSpeed(); break;
	case SUB::VISIONFX: VisionMods(); break;
	case SUB::VEHICLEWEAPS: VehicleWeapons(); break;
	case SUB::VEHMULTIPLIERS: VehiclesMultiplier(); break;
	}
}
void HornBoost()
{
	if (PLAYER::IS_PLAYER_PRESSING_HORN(PLAYER::PLAYER_ID()))
	{
		Vehicle Veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(PLAYER::PLAYER_ID()), false);
		NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(Veh);
		if (NETWORK::NETWORK_HAS_CONTROL_OF_ENTITY(Veh))
		{
			VEHICLE::SET_VEHICLE_FORWARD_SPEED(Veh, 100);
		}
	}
}
void set_VehSpeedBoost()
{
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
	DWORD model = ENTITY::GET_ENTITY_MODEL(veh);

	bool bUp = get_key_pressed(VK_NUMPAD9);
	bool bDown = get_key_pressed(VK_NUMPAD3);
	//	PrintStringBottomCentre("Press 9 or 3 to Boost! ");

	if (bUp || bDown)
	{
		float speed = ENTITY::GET_ENTITY_SPEED(veh);
		if (bUp)
		{
			speed += speed * 0.05f;
			VEHICLE::SET_VEHICLE_FORWARD_SPEED(veh, speed);
		}
		else
			if (ENTITY::IS_ENTITY_IN_AIR(veh) || speed > 5.0)
				VEHICLE::SET_VEHICLE_FORWARD_SPEED(veh, 0.0);
	}
}
void set_godmode()
{

	//if (invincible)
	//{
	//	BYTE Setter = 1;
	//	BYTE Current = Memory::get_value<BYTE>({ 0x08, 0x189 });
	//	if (Current != Setter)
	//	{
	//		Memory::set_value({ 0x08, 0x189 }, Setter);
	//	}
	//}
	//if(gModDisabled)

	//{
	//	BYTE Setter = 0;
	//	BYTE Current = Memory::get_value<BYTE>({ 0x08, 0x189 });
	//	if (Current != Setter)
	//	{
	//		Memory::set_value({ 0x08, 0x189 }, Setter);
	//	}
	//}
}



void set_PlayerSuperJump()
{
	Player player = PLAYER::PLAYER_ID();
	GAMEPLAY::SET_SUPER_JUMP_THIS_FRAME(player);

}
bool speed = 0;
void set_explosiveMelee()
{
	Player player = PLAYER::PLAYER_ID();
	GAMEPLAY::SET_EXPLOSIVE_MELEE_THIS_FRAME(player);

}
void set_runspeed()
{
	if (speed) {
		Player player = PLAYER::PLAYER_ID();
		PLAYER::SET_SWIM_MULTIPLIER_FOR_PLAYER(player, 1.49);
	}
	if (!speed) {
		Player player = PLAYER::PLAYER_PED_ID();
		PLAYER::SET_SWIM_MULTIPLIER_FOR_PLAYER(player, 1.0);
	}
}
void set_extremeRun()
{
	if (ExtremeRun) {
		if (get_key_pressed('W')) {
			Ped ped = PLAYER::PLAYER_PED_ID();
			Vector3 offset = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(ped, 0, 0.6, 0);
			ENTITY::APPLY_FORCE_TO_ENTITY(ped, 1, 0.0f, 1.3, 0, 0.0f, 0.0f, 0.0f, 0, 1, 1, 1, 0, 1);
			PLAYER::SET_PLAYER_SPRINT(PLAYER::PLAYER_ID(), 1);
			PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(PLAYER::PLAYER_ID(), 1.59);
		}
	}
}
void set_ExplosiveAmmo()
{
	Player player = PLAYER::PLAYER_ID();
	GAMEPLAY::SET_EXPLOSIVE_AMMO_THIS_FRAME(player);

}
void set_infiniteAmmo()
{
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Hash cur;
	if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &cur, 1))
	{
		if (WEAPON::IS_WEAPON_VALID(cur))
		{
			int maxAmmo;
			if (WEAPON::GET_MAX_AMMO(playerPed, cur, &maxAmmo))
			{
				WEAPON::SET_PED_AMMO(playerPed, cur, maxAmmo);

				maxAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, cur, 1);
				if (maxAmmo > 0)
					WEAPON::SET_AMMO_IN_CLIP(playerPed, cur, maxAmmo);
			}
		}
	}
}
void FlipVehicle() {

	Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID());
	if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 1)) {
		Vector3 rotation = ENTITY::GET_ENTITY_ROTATION(veh, 0);
		if (!(ENTITY::IS_ENTITY_UPRIGHT(veh, 120))) {
			RequestControlOfEnt(veh);
			ENTITY::SET_ENTITY_ROTATION(veh, 0, rotation.y, rotation.z, 0, 1);
		}
	}
}
bool mph = 1;
void printSpeed() {
	UI::SET_TEXT_FONT(0);
	UI::SET_TEXT_SCALE(7.26f, 7.26f);
	UI::SET_TEXT_CENTRE(1);
	UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
	
	if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 0)) {
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_SCALE(0.0, 0.60);
		UI::SET_TEXT_SCALE(0.0, 0.92);
		UI::SET_TEXT_COLOUR(255, 255, 255, 255);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 255);
		UI::SET_TEXT_EDGE(1, 0, 0, 0, 255);
		UI::SET_TEXT_DROP_SHADOW();
		UI::SET_TEXT_OUTLINE();
	UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
		std::ostringstream oss;
		float speed;
		float mps = ENTITY::GET_ENTITY_SPEED(PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0));
		float kmh = mps * 3600 / 1000;
		float milesperhour = kmh * 0.6213711916666667f;
		if (mph) speed = round(milesperhour);
		if (!mph) speed = round(kmh);
		oss << speed;
		//	UI::_ADD_TEXT_COMPONENT_STRING((char*)oss.str().c_str());
		//UI::END_TEXT_COMMAND_DISPLAY_TEXT((screen_res_x / 1000) - 0.096, 1 - 0.074);
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_PROPORTIONAL(1);
		//SET_TEXT_SCALE(0.0, 0.60);
		UI::SET_TEXT_SCALE(0.0, 0.38);
		UI::SET_TEXT_COLOUR(255, 255, 255, 255);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 255);
		UI::SET_TEXT_EDGE(1, 0, 0, 0, 255);
		UI::SET_TEXT_DROP_SHADOW();
		UI::SET_TEXT_OUTLINE();
//		UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
		std::ostringstream osss;
		if (mph) osss << "~g~mph"; else osss << "~g~km/h";
		//		UI::_ADD_TEXT_COMPONENT_STRING((char*)osss.str().c_str());
		//	UI::END_TEXT_COMMAND_DISPLAY_TEXT((screen_res_x / 1000) - 0.045, 1 - 0.043);
	}
	//END_TEXT_COMMAND_DISPLAY_TEXT(-0.015f + (screen_res_x / 100), -0.015f + (screen_res_y / 100));

}
void set_spodercarmode()
{
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

	if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		VEHICLE::_SET_VEHICLE_ENGINE_POWER_MULTIPLIER(veh, 50);
		ENTITY::APPLY_FORCE_TO_ENTITY(veh, true, 0, 0, -0.8, 0, 0, 0, true, true, true, true, false, true);
	}
}
void set_loop_fuckCam()
{
	Player playerPed = selPlayer;
	Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
	FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z + 15, 29, 999999.5f, false, true, 1.0f);
}
void set_loop_annoyBomb()
{
	Player playerPed = selPlayer;
	Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
	FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z, 29, 9.0f, true, false, 0.0f);
}
void set_loop_forcefield()
{
	Player playerPed = selPlayer;
	Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
	FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z, 7, 9.0f, false, true, 0.0f);
}
void set_clearpTasks()
{
	Player playerPed = selPlayer;
	AI::CLEAR_PED_TASKS_IMMEDIATELY(playerPed);
}

void set_RainbowP()
{
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
	VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh, rand() % 255, rand() % 255, rand() % 255);
	if (VEHICLE::GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM(veh))
		VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(veh, rand() % 255, rand() % 255, rand() % 255);
}
Hooking::NativeHandler ORIG_NETWORK_EARN_FROM_PICKUP = NULL;
void* __cdecl MY_NETWORK_EARN_FROM_PICKUP(NativeContext *cxt)
{
	Log::Msg("NETWORKCASH::NETWORK_EARN_FROM_PICKUP(%i);\n", cxt->GetArgument<int>(0));
	cxt->SetResult(0, 600000);
	return cxt;
}
bool checkNear(Vector3 a, Vector3 b, float range) {
	if (fabs(a.x - b.x) > range || fabs(a.y - b.y) > range || fabs(a.z - b.z) > range)
		return false;
	return true;
}
void set_gravity_gun()
{
	DWORD tempPed, tempWeap;

	if (!grav_target_locked) {
		PLAYER::GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(PLAYER::PLAYER_ID(), &grav_entity);
		if (ENTITY::DOES_ENTITY_EXIST(grav_entity) && checkNear(ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1), ENTITY::GET_ENTITY_COORDS(grav_entity, 1), pickupDistance)) grav_target_locked = 1;
		if (PED::IS_PED_IN_ANY_VEHICLE(grav_entity, 0)) grav_entity = PED::GET_VEHICLE_PED_IS_IN(grav_entity, 0);
		if (grav_target_locked) {
			bool cancel = 1;
			if (ENTITY::IS_ENTITY_AN_OBJECT(grav_entity) && forceGunObjects) cancel = 0;
			if (ENTITY::IS_ENTITY_A_VEHICLE(grav_entity) && forceGunVehicles) cancel = 0;
			if (ENTITY::IS_ENTITY_A_PED(grav_entity) && forceGunPeds) cancel = 0;
			if (cancel) {
				grav_target_locked = 0;
				grav_entity = -1;
				return;
			}
		}
	}
	tempPed = PLAYER::PLAYER_ID(); WEAPON::GET_CURRENT_PED_WEAPON(PLAYER::PLAYER_PED_ID(), &tempWeap, 1);
	if ((PLAYER::IS_PLAYER_FREE_AIMING(tempPed) || PLAYER::IS_PLAYER_TARGETTING_ANYTHING(tempPed)) && ENTITY::DOES_ENTITY_EXIST(grav_entity) && grav_target_locked)
	{
		Vector3 gameplayCam = CAM::_GET_GAMEPLAY_CAM_COORDS();
		Vector3 gameplayCamRot = CAM::GET_GAMEPLAY_CAM_ROT(0);
		Vector3 gameplayCamDirection = sub::RotationToDirection(gameplayCamRot);
		Vector3 startCoords = sub::addVector(gameplayCam, (sub::multiplyVector(gameplayCamDirection, entityDistance)));
		Vector3 endCoords = sub::addVector(gameplayCam, (sub::multiplyVector(gameplayCamDirection, 500.0f)));
		ENTITY::SET_ENTITY_COLLISION(grav_entity, 0, 1);
		//RequestControlOfid(NETWORK_GET_NETWORK_ID_FROM_ENTITY(grav_entity));
		RequestControlOfEnt(grav_entity);
		if (ENTITY::IS_ENTITY_A_PED(grav_entity)) ENTITY::SET_ENTITY_INVINCIBLE(grav_entity, 1);
		if (PED::IS_PED_SHOOTING(PLAYER::PLAYER_PED_ID()) == 0) ENTITY::SET_ENTITY_COORDS_NO_OFFSET(grav_entity, startCoords.x, startCoords.y, startCoords.z, 0, 0, 0);
		if (ENTITY::IS_ENTITY_A_VEHICLE(grav_entity) || ENTITY::IS_ENTITY_A_PED(grav_entity)) ENTITY::SET_ENTITY_HEADING(grav_entity, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()) + 90.0f);
		if (PED::IS_PED_SHOOTING(PLAYER::PLAYER_PED_ID()))
		{
			RequestControlOfEnt(grav_entity);
			ENTITY::SET_ENTITY_COLLISION(grav_entity, 1, 1);
			ENTITY::SET_ENTITY_HEADING(grav_entity, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));

			ENTITY::APPLY_FORCE_TO_ENTITY(grav_entity, 1, 0.0f, 350.0f, 2.0f + endCoords.z, 2.0f, 0.0f, 0.0f, 0, 1, 1, 1, 0, 1);
			ENTITY::SET_ENTITY_INVINCIBLE(grav_entity, 0);
			WAIT(300);
			grav_target_locked = false;
			grav_entity = -1;

			return;
		}
	}
	else {
		RequestControlOfEnt(grav_entity);
		ENTITY::SET_ENTITY_COLLISION(grav_entity, 1, 1);
		grav_target_locked = false;
		grav_entity = -1;
	}
}
vector<string> Throwables = {
	"WEAPON_GRENADE",
	"WEAPON_GRENADELAUNCHER",
	"WEAPON_MOLOTOV",
	"WEAPON_STICKYBOMB",
	"WEAPON_SMOKEGRENADE",
	"WEAPON_BZGAS",
	"WEAPON_FLARE",
};
enum IntersectOptions
{
	IntersectEverything = -1,
	IntersectMap = 1,
	IntersectMissionEntityAndTrain = 2,
	IntersectPeds1 = 4,
	IntersectPeds2 = 8,
	IntersectVehicles = 10,
	IntersectObjects = 16,
	IntersectVegetation = 256
};
typedef struct
{
	int Result;
	BOOL DidHitAnything;
	bool DidHitEntity;
	Entity HitEntity;
	Vector3 HitCoords;
} RaycastResult;
RaycastResult Raycast(Vector3 source, Vector3 endCoords, IntersectOptions inter, Entity entityToignore = 0) {
	RaycastResult endResult;
	try {
		Vector3 emptyVector;
		PrintStringBottomCentre("Creating Handle");
		DWORD Handle = WORLDPROBE::_CAST_3D_RAY_POINT_TO_POINT(source.x, source.y, source.z, endCoords.x, endCoords.y, endCoords.z, 1, inter, entityToignore, 7);//_CAST_RAY_POINT_TO_POINT(source.x, source.y, source.z, endCoords.x, endCoords.y, endCoords.z, 19/*inter/*19*/, entityToignore, 7);
		PrintStringBottomCentre("Getting Handle Results");
		PrintStringBottomCentre("DidHitEntity");
		if (ENTITY::DOES_ENTITY_EXIST(endResult.HitEntity)) endResult.DidHitEntity = true; else endResult.DidHitEntity = false;
	}
	catch (...) {

	}
	return endResult;
}
Object latestObj;
void shootItem(char* name) {
	DWORD model = GAMEPLAY::GET_HASH_KEY(name);
	STREAMING::REQUEST_MODEL(model);
	while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
	Vector3 lol;
	Vector3 cameraPosition = CAM::GET_GAMEPLAY_CAM_COORD();
	Vector3 gameplayCam = CAM::_GET_GAMEPLAY_CAM_COORDS();
	Vector3 gameplayCamRot = CAM::GET_GAMEPLAY_CAM_ROT(0);
	Vector3 gameplayCamDirection = sub::RotationToDirection(gameplayCamRot);
	Vector3 startCoords = sub::addVector(gameplayCam, (sub::multiplyVector(gameplayCamDirection, 1.0f)));
	Vector3 endCoordinates = sub::addVector(startCoords, sub::multiplyVector(gameplayCamDirection, 500.0f));
	try {
		RaycastResult rc = Raycast(cameraPosition, endCoordinates, IntersectEverything);
		lol = rc.HitCoords;
		if (rc.DidHitAnything) {
			Object obj = OBJECT::CREATE_OBJECT_NO_OFFSET(model, lol.x, lol.y, lol.z, 1, 0, 0);
			latestObj = obj;
			ENTITY::SET_ENTITY_LOD_DIST(obj, 696969);
		}
	}
	catch (...) {

	}
}
void shootCustomBullet(char* weapon)
{
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	if (PED::IS_PED_SHOOTING(playerPed) && custombullet)
	{

		if (weapon == "dildo") {
			shootItem("prop_cs_dildo_01");
			return;
		}

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY(weapon);
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		//if ((res.x && res.y && res.z) == 0)
		//	res.y = res.y + 100.0f; res[00] = coords1to.x; res.z = coords1to.z;
		bool canShoot = 0;
		bool isBullet = 1;
		Vector3 res;
		canShoot = WEAPON::GET_PED_LAST_WEAPON_IMPACT_COORD(playerPed, &res);
		for (int i = 0; i < Throwables.size(); i++) {
			if (Throwables[i] == weapon) isBullet = 0;
		}

		if (res.x != 0 && res.y != 0 && res.z != 0) canShoot = true;
		Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0, 0.2, 0);
		Vector3 gameplayCam = CAM::_GET_GAMEPLAY_CAM_COORDS();
		Vector3 gameplayCamRot = CAM::GET_GAMEPLAY_CAM_ROT(0);
		Vector3 gameplayCamDirection = sub::RotationToDirection(gameplayCamRot);
		Vector3 startCoords = sub::addVector(gameplayCam, (sub::multiplyVector(gameplayCamDirection, 2.0f)));
		Vector3 endCoords = sub::addVector(startCoords, sub::multiplyVector(gameplayCamDirection, 500.0f));

		if (isBullet) {
			GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(startCoords.x, startCoords.y, startCoords.z, endCoords.x, endCoords.y, endCoords.z, 50, 1, weaponAssetRocket, playerPed, 1, 1, -1);
		}
		else if (!isBullet && canShoot) {
			GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(endCoords.x, endCoords.y - 0.1f, endCoords.z + 0.2f, endCoords.x, endCoords.y, endCoords.z + 0.1f, 50, 1, weaponAssetRocket, playerPed, 1, 1, -1);
		}
		//if (isBullet) SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
		//coords1to.x, coords1to.y, GET_ENTITY_HEADING(playerPed),
		//250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0f);


	}
}

DWORD featureWeaponVehShootLastTime = 0;
void set_featureVehRockets()
{

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	PrintStringBottomCentre("Press + to Shoot! ");

	bool bSelect = get_key_pressed(0x6B); // num plus
	if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() &&
		PLAYER::IS_PLAYER_CONTROL_ON(player) && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("WEAPON_VEHICLE_ROCKET");
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
			coords0to.x, coords0to.y, coords0to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		 GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
			coords1to.x, coords1to.y, coords1to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		featureWeaponVehShootLastTime = GetTickCount();

	}

}

void set_featureVehLight()
{

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	PrintStringBottomCentre("Press + to Shoot! ");

	bool bSelect = get_key_pressed(0x6B); // num plus
	if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() &&
		PLAYER::IS_PLAYER_CONTROL_ON(player) && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("VEHICLE_WEAPON_PLAYER_BULLET");
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
			coords0to.x, coords0to.y, coords0to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
			coords1to.x, coords1to.y, coords1to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		featureWeaponVehShootLastTime = GetTickCount();

	}

}
void set_featureVehtaser()
{

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	PrintStringBottomCentre("Press + to Shoot! ");

	bool bSelect = get_key_pressed(0x5A); // num plus
	if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() &&
		PLAYER::IS_PLAYER_CONTROL_ON(player) && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("WEAPON_STUNGUN");
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
			coords0to.x, coords0to.y, coords0to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
			coords1to.x, coords1to.y, coords1to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		featureWeaponVehShootLastTime = GetTickCount();

	}

}
void set_featureVehballs()
{

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	PrintStringBottomCentre("Press + to Shoot! ");

	bool bSelect = get_key_pressed(0x5A); // num plus
	if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() &&
		PLAYER::IS_PLAYER_CONTROL_ON(player) && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("WEAPON_BALL");
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
			coords0to.x, coords0to.y, coords0to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
			coords1to.x, coords1to.y, coords1to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		featureWeaponVehShootLastTime = GetTickCount();

	}

}
void set_featureVehsnowball()
{

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	PrintStringBottomCentre("Press + to Shoot! ");

	bool bSelect = get_key_pressed(0x5A); // num plus
	if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() &&
		PLAYER::IS_PLAYER_CONTROL_ON(player) && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("WEAPON_SNOWBALL");
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
			coords0to.x, coords0to.y, coords0to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
			coords1to.x, coords1to.y, coords1to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		featureWeaponVehShootLastTime = GetTickCount();

	}

}
void set_featureVehprocked()
{

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	PrintStringBottomCentre("Press + to Shoot! ");

	bool bSelect = get_key_pressed(0x5A); // num plus
	if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() &&
		PLAYER::IS_PLAYER_CONTROL_ON(player) && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("WEAPON_PASSENGER_ROCKET");
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
			coords0to.x, coords0to.y, coords0to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
			coords1to.x, coords1to.y, coords1to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		featureWeaponVehShootLastTime = GetTickCount();

	}

}
void set_featureVehtrounds()
{

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	PrintStringBottomCentre("Press + to Shoot! ");

	bool bSelect = get_key_pressed(0x5A); // num plus
	if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() &&
		PLAYER::IS_PLAYER_CONTROL_ON(player) && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("VEHICLE_WEAPON_TANK");
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
			coords0to.x, coords0to.y, coords0to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
			coords1to.x, coords1to.y, coords1to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		featureWeaponVehShootLastTime = GetTickCount();

	}

}

void set_featureVehlaser1()
{

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	PrintStringBottomCentre("Press + to Shoot! ");

	bool bSelect = get_key_pressed(0x5A); // num plus
	if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() &&
		PLAYER::IS_PLAYER_CONTROL_ON(player) && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("VEHICLE_WEAPON_PLAYER_LASER");
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
			coords0to.x, coords0to.y, coords0to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
			coords1to.x, coords1to.y, coords1to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		featureWeaponVehShootLastTime = GetTickCount();

	}

}
void set_featureVehlaser2()
{

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	PrintStringBottomCentre("Press + to Shoot! ");

	bool bSelect = get_key_pressed(0x5A); // num plus
	if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() &&
		PLAYER::IS_PLAYER_CONTROL_ON(player) && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("VEHICLE_WEAPON_ENEMY_LASER");
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
			coords0to.x, coords0to.y, coords0to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
			coords1to.x, coords1to.y, coords1to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		featureWeaponVehShootLastTime = GetTickCount();

	}

}
void set_featureVehFireworks()
{

	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	PrintStringBottomCentre("Press + to Shoot! ");

	bool bSelect = get_key_pressed(0x6B); // num plus
	if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() &&
		PLAYER::IS_PLAYER_CONTROL_ON(player) && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
	{
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

		Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("WEAPON_FIREWORK");
		if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
		{
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
			coords0to.x, coords0to.y, coords0to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
			coords1to.x, coords1to.y, coords1to.z,
			250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
		featureWeaponVehShootLastTime = GetTickCount();

	}

}

void menu::loops() {

	if (sub::horn) HornBoost();
	if (stealth)
	{
		MY_NETWORK_EARN_FROM_PICKUP(0);
	}
	/*	if (DOES_ENTITY_EXIST(PLAYER_PED_ID())){
	tickForForce++;
	if (tickForForce > 500){
	if (!IsUserForce(PLAYER_ID())){
	Vector3 coords = GET_ENTITY_COORDS(PLAYER_PED_ID(), 1);
	Object forceObject = CREATE_OBJECT(GET_HASH_KEY("prop_cs_dildo_01"), coords.x, coords.y, -(coords.z), 1, 1, 0);
	SET_ENTITY_VISIBLE(forceObject, 0);
	ATTACH_ENTITY_TO_ENTITY(forceObject, PLAYER_PED_ID(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1);
	}
	tickForForce = 0;
	}
	}*/
	//SetMultipliers();
	if (ms_invisible) set_msinvisible();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	if (IsKeyJustUp(VK_SUBTRACT)) {
		PrintStringBottomCentre("test");
		Vehicle playerVeh = NULL;

		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, FALSE))
			playerVeh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

	}


	//if (aimbot) Aimbot();

	if (antiParticleFXCrash) {
		Vector3 coords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1);
		GRAPHICS::REMOVE_PARTICLE_FX_IN_RANGE(coords.x, coords.y, coords.z, 10);
	}

	Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);


	//if(vehicleJump) 
	//{
	//	if (IsKeyJustUp(VK_SPACE) && PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 1)) {
	//		Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
	//		ENTITY::APPLY_FORCE_TO_ENTITY(veh, 1, 0 + ENTITY::GET_ENTITY_FORWARD_X(veh), 0 + ENTITY::GET_ENTITY_FORWARD_Y(veh), 7, 0, 0, 0, 1, 0, 1, 1, 1, 1);
	//	}
	//}

	/*	Make calls to functions that you want looped over here, e.g. ambient lights, whale guns, explosions, checks, flying deers, etc.
	Can also be used for (bool) options that are to be executed from many parts of the script. */

	myVeh = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0); // Store current vehicle
	Player player = PLAYER::PLAYER_ID();

	if (fixAndWashLoop) {
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
			VEHICLE::SET_VEHICLE_DIRT_LEVEL(veh, 0.0f);
			VEHICLE::SET_VEHICLE_FIXED(PED::GET_VEHICLE_PED_IS_USING(playerPed));
		}
	}

	if (sub::seatBelt && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 1)) {
		if (PED::GET_PED_CONFIG_FLAG(playerPed, PED_FLAG_CAN_FLY_THRU_WINDSCREEN, TRUE))
			PED::SET_PED_CONFIG_FLAG(playerPed, PED_FLAG_CAN_FLY_THRU_WINDSCREEN, FALSE);
	}
	if (sub::gModeVeh) {
		if (VEHICLE::GET_PED_IN_VEHICLE_SEAT(veh, -1) == PLAYER::PLAYER_PED_ID()) {
			RequestControlOfEnt(veh);
			ENTITY::SET_ENTITY_INVINCIBLE(veh, TRUE);
			ENTITY::SET_ENTITY_PROOFS(veh, 1, 1, 1, 1, 1, 1, 1, 1);
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh, 0);
			VEHICLE::SET_VEHICLE_WHEELS_CAN_BREAK(veh, 0);
			VEHICLE::SET_VEHICLE_CAN_BE_VISIBLY_DAMAGED(veh, 0);
		}
	}
	if (sub::carBoost) sub::CarBoost();


	//if (bulletTime) {
	//	//	if (PLAYER::IS_PLAYER_FREE_AIMING(player)) {
	//	//		GAMEPLAY::SET_TIME_SCALE(0.25f);
	//	//	}
	//	//	else 
	//	{
	//		GAMEPLAY::SET_TIME_SCALE(1.0f);
	//	}
	//}
	shootCustomBullet(ammoWeapon);
	Vehicle veh2 = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0);
	//	shootCustomBulletbags(ammoWeapon);

	//FreeCam();
	if (NETWORK::NETWORK_IS_GAME_IN_PROGRESS()) {
		GAMEPLAY::NETWORK_SET_SCRIPT_IS_SAFE_FOR_NETWORK_GAME();
	}

	DWORD model = ENTITY::GET_ENTITY_MODEL(veh);
	Vector3 pos = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
	///		if (sub::driveOnWater) sub::DriveOnWater();
	/*if (esp) {
	if (box3D) {
	set_esp(0);
	}
	else {
	set_esp(1);
	}
	}*/
	/*	otherPFX++;
	if (otherPFX > 50) {
	otherPFX = 0;*/
	/*for (int i = 0; i < NUM_PLAYERS; i++) {
	if (i == MainPFX.p) {
	Vector3 coords = ENTITY::GET_ENTITY_COORDS(PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(MainPFX.p), 1);
	if (MainPFX.hasBlood) {
	set_bloodfx(coords);
	}
	if (MainPFX.hasBigSmoke) {
	set_bigsmoke01(coords);
	}
	if (MainPFX.hasCameraFlash) {
	set_cameraflash01(coords);
	}
	if (MainPFX.hasCoolLights) {
	set_coollights01(coords);
	}
	if (MainPFX.hasMoneyRain) {
	set_moneyrain01(coords);
	}
	if (MainPFX.hasPinkBurst) {
	set_pinkburstfx01(coords);
	}
	if (MainPFX.hasElectric) {
	set_electricfx01(coords);
	}
	}
	}
	}*/
	ticks++;
	/*if (lightentity) {
	if (ticks > 50) {
	ticks = 0;
	set_lightEntity();
	}
	}
	if (bloodfx01)
	{
	if (ticks > 50) {
	ticks = 0;
	set_bloodfx();
	}
	}
	if (electricfx01)
	{
	if (ticks > 50) {
	ticks = 0;
	set_electricfx01();
	}
	}
	if (pinkburstfx01)
	{
	if (ticks > 50) {
	ticks = 0;
	set_pinkburstfx01();
	}
	}
	if (coollights01)
	{
	if (ticks > 50) {
	ticks = 0;
	set_coollights01();
	}
	}
	if (moneyrain01)
	{
	if (ticks > 50) {
	ticks = 0;
	set_moneyrain01();
	}
	}
	if (bigsmoke01)
	{
	if (ticks > 50) {
	ticks = 0;
	set_bigsmoke01();
	}
	}
	if (cameraflash01)
	{
	if (ticks > 50) {
	ticks = 0;
	set_cameraflash01();
	}
	}

	if (ms_rpIncreaser)
	{
	if (ticks > 20) {
	ticks = 0;
	set_RpIncreaser();
	}
	}*/

	//if (loop_massacre_mode) set_massacre_mode(); // Massacre mode
	if (loop_SuperGrip) set_supergrip();
	//	if (analog_loop) set_analog_loop();
	//if (loop_ClearpTasks) set_clearpTasks();
	//if (loop_annoyBomb) set_loop_annoyBomb();
	if (forcefield) set_loop_forcefield();
	if (loop_fuckCam) set_loop_fuckCam();
	if (infiniteAmmo) set_infiniteAmmo();
	//	if (bloodfx01) set_bloodfx();
	//	if (lightentity) set_lightEntity();

	if (sub::AutoFlip) FlipVehicle();
	//if (loop_moneydrop) set_moneydrop();


	if (loop_safemoneydrop)
	{
		if (ticks > 10) {
			ticks = 0;
			set_Safemoneydrop();
		}
	}
	/*
	if (loop_safemoneydropv2)
	{
	if (ticks > 10) {
	ticks = 0;
	set_Safemoneydropv2();
	}
	}*/
	//if (bypassduke) sub::set_bypassduke();
	if (noclip) sub::noClip();
	/*	else {
	SET_ENTITY_MAX_SPEED(PLAYER_PED_ID(), 500);
	SET_ENTITY_COLLISION(PLAYER_PED_ID(), 1, 0);

	if (IS_PED_IN_ANY_VEHICLE(playerPed, 1)) {
	SET_ENTITY_COLLISION(veh, true, true);
	SET_ENTITY_MAX_SPEED(veh, 500);
	}
	}*/
	//		if (awhostalking) printTalkers();
	/*if (showfps) printFPS();
	if (anticrash3) FreezeProtection();*/
	if (ms_invisible) set_msinvisible();
	if (ms_neverWanted) set_msNeverWanted();
	/*set_extremeRun();
	set_godmode();*/
	//	if (disableinvincible) set_godmodeOff();
	if (RainbowP) set_RainbowP();
	//	if (lowerVeh_ms) set_lowerVeh_ms();
	/*if (lowerVehMID_ms) set_lowerVehMID_ms();
	if (lowerVehMAX_ms) set_lowerVehMAX_ms();*/
	if (spodercarmode) set_spodercarmode();
	if (VehSpeedBoost)set_VehSpeedBoost();
	if (flyingUp) sub::set_fup();
	//if (anticrash)set_antiCrash();
	//if (test2) set_test2();
	//	if (earthquake) set_earthquake();
	//	if (featureMoneyDropSelfLoop) set_featureMoneyDropSelfLoop();
	//		if (spritetest2) set_spritetest2();
	//	if (showWhosTalking) set_showWhosTalking();
	//if (featureMiscHideHud) set_featureMiscHideHud();
	if (PlayerSuperJump) set_PlayerSuperJump();
	if (runspeed) set_runspeed();
	if (ExplosiveMelee) set_explosiveMelee();
	//		if (shootbmxs) set_shootbmxs();
	if (xmasgs) set_xmasgs();
	//	if (nearbypeds) set_nearbypeds();
	//if (shoothydras) set_shoothydra();
	//	if (shootdummp) set_shootdump();
	//if (shootcutter) set_shootcutter();
	if (shootbzrd) sub::set_shootbzrd();
	if (shootlbrtr) sub::set_shootlbrtr();
	if (shootrhino) sub::set_shootrhino();
	if (featureVehRockets) set_featureVehRockets();
	if (featureVehFireworks) set_featureVehFireworks();
	if (featureVehlaser1) set_featureVehlaser1();
	if (featureVehlaser2) set_featureVehlaser2();
	if (featureVehtrounds) set_featureVehtrounds();
	if (featureVehprocked) set_featureVehprocked();
	if (featureVehsnowball) set_featureVehsnowball();
	if (featureVehballs) set_featureVehballs();
	if (featureVehLight) set_featureVehLight();
	if (featureVehtaser) set_featureVehtaser();
	if (loop_explosiveammo) set_ExplosiveAmmo();
	PLAYER::SET_PLAYER_WEAPON_DAMAGE_MODIFIER(player, damageMultiplier);
	PLAYER::SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER(player, damageMultiplier);
	if (onehit) {
		PLAYER::SET_PLAYER_WEAPON_DAMAGE_MODIFIER(player, 999999);
		PLAYER::SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER(player, 999999);
	}
	bool bPlayerExists = 1;
	if (loop_explosiveammo)
	{
		if (bPlayerExists)
			GAMEPLAY::SET_EXPLOSIVE_AMMO_THIS_FRAME(player);
	}
	if (loop_explosiveMelee)
	{
		if (bPlayerExists)
			GAMEPLAY::SET_EXPLOSIVE_MELEE_THIS_FRAME(player);
	}
	if (loop_noreload)
	{
		Hash cur;
		if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &cur, 1))
		{
			if (WEAPON::IS_WEAPON_VALID(cur))
			{
				int maxAmmo;
				if (WEAPON::GET_MAX_AMMO(playerPed, cur, &maxAmmo))
				{
					WEAPON::SET_PED_AMMO(playerPed, cur, maxAmmo);

					maxAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, cur, 1);
					if (maxAmmo > 0)
						WEAPON::SET_AMMO_IN_CLIP(playerPed, cur, maxAmmo);
				}
			}
		}
	}

	if (rapidfire && !PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 1)) {
		PLAYER::DISABLE_PLAYER_FIRING(player, 1);
		Vector3 gameplayCam = CAM::_GET_GAMEPLAY_CAM_COORDS();
		Vector3 gameplayCamRot = CAM::GET_GAMEPLAY_CAM_ROT(0);
		Vector3 gameplayCamDirection = sub::RotationToDirection(gameplayCamRot);
		Vector3 startCoords = sub::addVector(gameplayCam, (sub::multiplyVector(gameplayCamDirection, 1.0f)));
		Vector3 endCoords = sub::addVector(startCoords, sub::multiplyVector(gameplayCamDirection, 500.0f));
		Hash weaponhash;
		WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &weaponhash, 1);
		/*GetKeyState(VK_LBUTTON) & 0x8000*/
		if (CONTROLS::IS_CONTROL_PRESSED(2, INPUT_FRONTEND_RT) || (GetKeyState(VK_LBUTTON) & 0x8000)) {
			GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(startCoords.x, startCoords.y, startCoords.z, endCoords.x, endCoords.y, endCoords.z, 50, 1, weaponhash, playerPed, 1, 1, 0xbf800000);
		}
	}
	/*if (featureRagdoll) set_featureRagdoll();
	if (loop_rainbowcar) rtick++;
	if (loop_rainbowcar && rtick == 15) {
	rtick = 0;

	VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh, rand() % 255, rand() % 255, rand() % 255);
	if (VEHICLE::GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM(veh)) {
	VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(veh, rand() % 255, rand() % 255, rand() % 255);
	}
	}
	*/
	if (loop_RainbowBoxes && GAMEPLAY::GET_GAME_TIMER() >= livetimer)
	{
		titlebox.R = RandomRGB(); titlebox.G = RandomRGB(); titlebox.B = RandomRGB();
		BG.R = RandomRGB(); BG.G = RandomRGB(); BG.B = RandomRGB();
		selectedtext.R = RandomRGB(); selectedtext.G = RandomRGB(); selectedtext.B = RandomRGB();
	}



	/*if (planesmoke) set_plane_smoke(sx, sy, sz);*/
	if (speedometer) printSpeed();

	if (loop_gravity_gun) set_gravity_gun();

	if (loop_gta2cam) set_gta2_cam_rot();
	/*if (AntiReddinos) antiReddinosCrashV2();
	if (AntiReddinosElite) antiReddinosElite();

	if (speedLimiter) {
	ENTITY::SET_ENTITY_MAX_SPEED(veh2, maxSpeed);
	}*/

	if (theForceA) {
		char* dict = "rcmcollect_paperleadinout@";
		char* anim = "meditiate_idle";


		//Setup the array
		const int numElements = 10;
		const int arrSize = numElements * 2 + 2;
		const int numElementsp = 30;
		const int arrSizep = numElements * 2 + 2;
		Entity veh[arrSize];
		Entity Peds[arrSize];
		//0 index is the size of the array
		veh[0] = numElements;
		Peds[0] = numElementsp;
		int pedCount = PED::GET_PED_NEARBY_PEDS(PLAYER::PLAYER_PED_ID(), Peds, 1);
		int count = PED::GET_PED_NEARBY_VEHICLES(PLAYER::PLAYER_PED_ID(), veh);

		if (Peds != NULL) {
			for (int i = 0; i < pedCount; i++) {
				int offsettedID = i * 2 + 2;
				if (Peds[offsettedID] != NULL && ENTITY::DOES_ENTITY_EXIST(Peds[offsettedID]))
				{
					//Do something
					if (get_key_pressed(VK_DIVIDE)) {
						Entity mped = Peds[offsettedID];
						RequestControlOfEnt(mped);
						ENTITY::APPLY_FORCE_TO_ENTITY(mped, 1, 0, 0, 4, 0, 0, 0, 1, 0, 1, 1, 1, 1);
					}
				}
			}
		}
		if (veh != NULL)
		{
			//Simple loop to go through results
			for (int i = 0; i < count; i++)
			{
				int offsettedID = i * 2 + 2;
				//Make sure it exists
				if (veh[offsettedID] != NULL && ENTITY::DOES_ENTITY_EXIST(veh[offsettedID]))
				{
					//Do something
					if (get_key_pressed(VK_DIVIDE)) {
						Entity mveh = veh[offsettedID];
						RequestControlOfEnt(mveh);
						ENTITY::APPLY_FORCE_TO_ENTITY(mveh, 1, 0, 0, 4, 0, 0, 0, 1, 0, 1, 1, 1, 1);
						STREAMING::REQUEST_ANIM_DICT(dict);
						AI::TASK_PLAY_ANIM(playerPed, dict, anim, 1, 1, -1, 16, 0, false, 0, false);
					}
				}
			}
		}
	}
	if (loop_noreload)
	{
		Hash cur;
		if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &cur, 1))
		{
			if (WEAPON::IS_WEAPON_VALID(cur))
			{
				int maxAmmo;
				if (WEAPON::GET_MAX_AMMO(playerPed, cur, &maxAmmo))
				{
					WEAPON::SET_PED_AMMO(playerPed, cur, maxAmmo);

					maxAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, cur, 1);
					if (maxAmmo > 0)
						WEAPON::SET_AMMO_IN_CLIP(playerPed, cur, maxAmmo);
				}
			}
		}
	}



}
// that's how you do loops





void menu::sub_handler()
{
	if (currentsub == SUB::CLOSED) {
		while_closed();
	}

	else {
		submenu_switch();
		if (SetSub_delayed != NULL)
		{
			SetSub_new(SetSub_delayed);
			SetSub_delayed = NULL;
		}

		while_opened();

		if (GAMEPLAY::GET_GAME_TIMER() >= livetimer) livetimer = GAMEPLAY::GET_GAME_TIMER() + 1800; // 1.8s delay for rainbow related loops
	}
}
int frame_cache;
























void Script::onTick()
{
	menu::base();
	menu::sub_handler();
	menu::loops();
	//globalHandle(0x27C6CA);
}