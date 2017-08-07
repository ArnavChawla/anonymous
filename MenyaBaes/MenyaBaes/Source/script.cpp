#include "stdafx.h"
#include "keyboard.h"

typedef struct { int R, G, B, A; } RGBA;

/* Models */
uint RequestedModel = 0ul;
RequestState model_state = loaded;
void(*modelFunction)() = nullptr;
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
	int *settings_font, inull;
	RGBA *settings_rgba;
	RGBA titlebox = { 0, 0, 0, 255 };
	RGBA BG = { 20, 20, 20, 200 };
	RGBA titletext = { 255, 255, 255, 255 };
	RGBA optiontext = { 128, 128, 128, 255 };
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
			if (timeGetTime() <= WakeAt1) {
				UI::SET_TEXT_FONT(0);
				UI::SET_TEXT_SCALE(0.50, 0.50);
				UI::SET_TEXT_COLOUR(255, 255, 255, 255);
				UI::SET_TEXT_WRAP(0.0, 1.0);
				UI::SET_TEXT_CENTRE(1);
				UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
				UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
				UI::_SET_TEXT_ENTRY("STRING");
				UI::_ADD_TEXT_COMPONENT_STRING("~r~IF YOU BANK THE CASH GET FUCKED BY TRUMP :)");
				UI::_DRAW_TEXT(0.5, 0.15);
			}
			else DrawnWarningOnce = 0;
		}
	}



	// Booleans for loops go here:
	DWORD VehicleShootLastTime = 0;
	bool loop_massacre_mode = 0, showfps = 0, loop_gravity_gun = 0, xmasgs = 0, coollights01 = 0, moneyrain01 = 0, bigsmoke01 = 0, cameraflash01 = 0,
		automax = 0, noclip = 0, laghimout = 0, theleecher = 0, rapidfire = 0, speedLimiter = 0, analog_loop = 0, loop_SuperGrip = 0, msCAR_invisible = 0,
		theForceA = 0, awhostalking = 1, amiHOSTl = 0, anticrash3 = 0, vehrpm = 0, crashLobby = 0, loop_RainbowBoxes = 0, oneshotkillv2 = 0, invincible = 0,
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

	Object mapMods[250];
	int mapModsIndex = 0; }
	bool ExtremeRun = 0;
	bool stealth = 0;

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
		


	int PlaceObject(DWORD Hash, float X, float Y, float Z, float Pitch, float Roll, float Yaw)
	{
		RequestModel(Hash);
		int object = OBJECT::CREATE_OBJECT(Hash, X, Y, Z, 1, 1, 0);
		ENTITY::SET_ENTITY_ROTATION(object, Pitch, Roll, Yaw, 2, 1);
		ENTITY::FREEZE_ENTITY_POSITION(object, 1);
		ENTITY::SET_ENTITY_LOD_DIST(object, 696969);
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(Hash);
		ENTITY::SET_OBJECT_AS_NO_LONGER_NEEDED(&object);

		return object;
	}
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
		UI::_SET_TEXT_ENTRY("STRING");
		UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text);
		UI::_DRAW_TEXT(X, Y);
	}
	void drawinteger(int text, float X, float Y)
	{
		UI::_SET_TEXT_ENTRY("NUMBER");
		UI::ADD_TEXT_COMPONENT_INTEGER(text);
		UI::_DRAW_TEXT(X, Y);
	}
	void drawfloat(float text, DWORD decimal_places, float X, float Y)
	{
		UI::_SET_TEXT_ENTRY("NUMBER");
		UI::ADD_TEXT_COMPONENT_FLOAT(text, decimal_places);
		UI::_DRAW_TEXT(X, Y);
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
	void DrawNotif(char* message, char* Icon, ICONTYPE icoType, char* sender = "DANK HAX", char* Subject = "Information", ICONTYPE secondIcon = ICON_NOTHING, char* ClanTag = "___DANKKKKK") {
		UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");//202709F4C58A0424
		UI::_ADD_TEXT_COMPONENT_STRING(message); //6C188BE134E074AA
		UI::_SET_NOTIFICATION_MESSAGE_CLAN_TAG_2(Icon, Icon, 1, icoType, sender, Subject, 1, ClanTag, ICON_NOTHING, 0);//531B84E7DA981FB6
		UI::_DRAW_NOTIFICATION(FALSE, FALSE);//2ED7843F8F801023
	}

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
		UI::_SET_TEXT_ENTRY_2("STRING");
		UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text);
		UI::_DRAW_SUBTITLE_TIMED(2000, 1);
	}
	void PrintFloatBottomCentre(float text, __int8 decimal_places)
	{
		UI::_SET_TEXT_ENTRY_2("NUMBER");
		UI::ADD_TEXT_COMPONENT_FLOAT(text, (DWORD)decimal_places);
		UI::_DRAW_SUBTITLE_TIMED(2000, 1);
	}
	void PrintBottomLeft(char* text)
	{
		UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
		UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text);
		UI::_DRAW_NOTIFICATION_2(0, 1); // Not sure if right native
	}
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

			UI::_SET_TEXT_ENTRY("CM_ITEM_COUNT");
			UI::ADD_TEXT_COMPONENT_INTEGER(currentop); // ! currentop_w_breaks
			UI::ADD_TEXT_COMPONENT_INTEGER(totalop); // ! totalop - totalbreaks
			UI::_DRAW_TEXT(0.2205f + menuPos, temp);
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

	}; unsigned __int16 menu::currentsub = 0; unsigned __int16 menu::currentop = 0; unsigned __int16 menu::currentop_w_breaks = 0; unsigned __int16 menu::totalop = 0; unsigned __int16 menu::printingop = 0; unsigned __int16 menu::breakcount = 0; unsigned __int16 menu::totalbreaks = 0; unsigned __int8 menu::breakscroll = 0; __int16 menu::currentsub_ar_index = 0; int menu::currentsub_ar[20] = {}; int menu::currentop_ar[20] = {}; int menu::SetSub_delayed = 0; unsigned long int menu::livetimer; bool menu::bit_centre_title = 1, menu::bit_centre_options = 0, menu::bit_centre_breaks = 1;
	bool CheckAJPressed()
	{
		if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RDOWN) || IsKeyJustUp(VK_RETURN)) return true; else return false;
	}
	bool CheckRPressed()
	{
		if (CONTROLS::IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_RIGHT) || IsKeyDown(VK_NUMPAD6)) return true; else return false;
	}
	bool CheckRJPressed()
	{
		if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_RIGHT) || IsKeyJustUp(VK_NUMPAD6)) return true; else return false;
	}
	bool CheckLPressed()
	{
		if (CONTROLS::IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_LEFT) || IsKeyDown(VK_NUMPAD4)) return true; else return false;
	}
	bool CheckLJPressed()
	{
		if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_LEFT) || IsKeyJustUp(VK_NUMPAD4)) return true; else return false;
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
				tempChar = "☐";
			}
			else if (status == 1) {
				UI::SET_TEXT_COLOUR(102, 255, 102, 250); //GREEN
				tempChar = "☑";
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
	bool Shrink = 0;
	// Define submenus here.
	void MainMenu()
	{
		//Add To Auth Premium/ VIP
		bool ShrinkE = 0, ShrinkD = 0;
		AddTitle("Anonymous Menu");
		
		AddOption("~g~Name Changer", null, nullFunc, SUB::NAMECHANGER);
		AddOption("~g~Teleport Locations", null, nullFunc, SUB::TELEPORTLOCATIONS);
		AddOption("~b~Online Players", null, nullFunc, SUB::ONLINEPLAYERS);
		AddOption("All Players", null, nullFunc, SUB::ALLPLAYEROPTIONS);
		AddOption("~b~Self Mods", null, nullFunc, SUB::SELFMODOPTIONS);
		AddOption("Weapon Mods", null, nullFunc, SUB::WEAPONSMENU);
		AddOption("~b~Vehicle Spawner", null, nullFunc, SUB::VEHICLE_SPAWNER);
		AddOption("~b~Vehicle Mods", null, nullFunc, SUB::VEHICLEMODSA);
		AddOption("~y~Obj/Ped/Bodyguard Spawner", null, nullFunc, SUB::FORCESPAWNER);
		AddOption("~y~Particle Effects", null, nullFunc, SUB::PARTICLEFX);
		AddOption("~y~Gun", null, nullFunc, SUB::FORCEGUN);
		AddToggle("Shrink", Shrink, ShrinkE, ShrinkD);
		if (ShrinkE) PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 223, 1);
		if (ShrinkD) PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 223, 0);
		//	AddToggle("Freecam", freecam, freecamEnabled, freeCamDisabled);
		AddOption("~y~Misc Options", null, nullFunc, SUB::SAMPLE);
	}

	void BypassOnlineVehicleKick(Vehicle vehicle)
	{
		Player player = PLAYER::PLAYER_ID();
		int var;
		DECORATOR::DECOR_REGISTER("Player_Vehicle", 3);
		DECORATOR::DECOR_REGISTER("Veh_Modded_By_Player", 3);
		DECORATOR::DECOR_SET_INT(vehicle, "Player_Vehicle", NETWORK::_0xBC1D768F2F5D6C05(player));
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

	void SelfModsOptionsMenu()
	{

		//	draw_force();
		// Initialise local variables here:
		bool fUp = get_key_pressed(VK_SUBTRACT), fastswimUpdated = 0, invisibleDisabled = 0, invincibleDisabled = 0,
			fixPlayer = 0, noclipSafety = 0, spritetest = 0, fastrunUpdated = 0, sample_gta2cam = 0, forceforward = 0, srun = 0, forceOne = 0, nCops = 0,
			enableGmode = 0, disableGmode = 0, sjump = 0, frun = 0, fswim = 0, nRagdoll = 0, fastRun_ON = 0, fastRun_OFF = 0, detachAllObjects = 0, increaserEnabled = 0, extremeRunOff = 0;
		bool fastrune = 0;
		//variables
		bool NoClipControls = 0;
		bool offTheRadar = 0;
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);

		bool gModeEnabled = 0, gModDisabled = 0;
		// Options' text here:
		AddTitle("Self Mods");
		//	AddOption("- - -Self Options- - -");
		AddToggle("Noclip", noclip, NoClipControls, noclipSafety);
		if (NoClipControls) {
			DrawNotif("Ctrl:Up\nShift:Down\nNumpad 7/1:CSpeed\nW-A-S-D: Move", ICONS::CHAR_MILSITE, ICONTYPE::ICON_NOTHING);
		}
		AddToggle("Off the Radar",offTheRadar);
		AddOption("~b~Character Customizer", null, nullFunc, SUB::CUSTOMCHARAC);
		AddOption("Model Changer", null, nullFunc, SUB::SKINCHANGER);
		AddOption("~b~Animations", null, nullFunc, SUB::PANIMATIONMENU);
		AddOption("Animation Scenarios", null, nullFunc, SUB::GTASCENIC);
		AddOption("Movement Clipsets", null, nullFunc, SUB::CLIPSETS);
		AddOption("~y~Detach All Objects V2", detachAllObjects);
		//	AddToggle("God Mode", invincible, null, invincibleDisabled);
		AddToggle("God Mode", invincible, null, gModDisabled);
		if (gModDisabled) ENTITY::SET_ENTITY_INVINCIBLE(PLAYER::PLAYER_PED_ID(), false);
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
			globalHandle(2390201).At(PLAYER::PLAYER_ID(), 358).At(203).As<int>() = 1;
			globalHandle(2433125).At(69).As<int>() = NETWORK::GET_NETWORK_TIME();
		}
		if (extremeRunOff) {
			PLAYER::SET_PLAYER_SPRINT(PLAYER::PLAYER_ID(), 0);
			PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(PLAYER::PLAYER_ID(), 1.0);
		}

		// Options' code here:
		if (detachAllObjects)
		{


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
	void SampleSub()
	{
		
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
	case SUB::SELFMODOPTIONS:			sub::SelfModsOptionsMenu(); break;
	}
}
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
	
}