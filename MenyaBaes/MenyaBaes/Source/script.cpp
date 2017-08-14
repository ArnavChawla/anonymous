#include "stdafx.h"
#include "keyboard.h"
#include<xstring>
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
		SELECTEDPLAYER
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
			/*if (timeGetTime() <= WakeAt1) {
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
			}*/
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
		//UI::_SET_TEXT_ENTRY_2("STRING");
		//UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text);
		//UI::_DRAW_SUBTITLE_TIMED(2000, 1);
	}
	void PrintFloatBottomCentre(float text, __int8 decimal_places)
	{/*
		UI::_SET_TEXT_ENTRY_2("NUMBER");
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

	}; unsigned __int16 menu::currentopb = 0;  unsigned __int16 menu::printingopb = 0; unsigned __int16 menu::currentsub = 0; unsigned __int16 menu::currentop = 0; unsigned __int16 menu::currentop_w_breaks = 0; unsigned __int16 menu::totalop = 0; unsigned __int16 menu::printingop = 0; unsigned __int16 menu::breakcount = 0; unsigned __int16 menu::totalbreaks = 0; unsigned __int8 menu::breakscroll = 0; __int16 menu::currentsub_ar_index = 0; int menu::currentsub_ar[20] = {}; int menu::currentop_ar[20] = {}; int menu::SetSub_delayed = 0; unsigned long int menu::livetimer; bool menu::bit_centre_title = 1, menu::bit_centre_options = 0, menu::bit_centre_breaks = 1;
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
	
	bool Shrink = 0;
	// Define submenus here.
	void MainMenu()
	{
		//Add To Auth Premium/ VIP
		bool ShrinkE = 0, ShrinkD = 0;
		AddTitle("Anonymous");
		
		//ddOption("~g~Name Changer", null, nullFunc, SUB::NAMECHANGER);
		AddOption("~g~Teleport Locations", null, nullFunc, SUB::TELEPORTLOCATIONS);
		AddOption("~b~Online Players", null, nullFunc, SUB::ONLINEPLAYERS);
		AddOption("All Players", null, nullFunc, SUB::ALLPLAYEROPTIONS);
		AddOption("~b~Self Mods", null, nullFunc, SUB::SELFMODOPTIONS);
		AddOption("Weapon Mods", null, nullFunc, SUB::WEAPONSMENU);
		AddOption("~b~Vehicle Spawner", null, nullFunc, SUB::VEHICLE_SPAWNER);
		AddOption("~b~Vehicle Mods", null, nullFunc, SUB::VEHICLEMODSA);
		//AddOption("~y~Obj/Ped/Bodyguard Spawner", null, nullFunc, SUB::FORCESPAWNER);
	//	AddOption("~y~Particle Effects", null, nullFunc, SUB::PARTICLEFX);
		//AddOption("~y~Gun", null, nullFunc, SUB::FORCEGUN);
		AddToggle("Shrink", Shrink, ShrinkE, ShrinkD);
		if (ShrinkE) PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 223, 1);
		if (ShrinkD) PED::SET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 223, 0);
		//	AddToggle("Freecam", freecam, freecamEnabled, freeCamDisabled);
		AddOption("~y~Misc Options", null, nullFunc, SUB::SAMPLE);
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
		AddTitle("~y~ROBBERY SPREE");
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
	//	if (NoClipControls) {
		//	DrawNotif("Ctrl:Up\nShift:Down\nNumpad 7/1:CSpeed\nW-A-S-D: Move", ICONS::CHAR_MILSITE, ICONTYPE::ICON_NOTHING);
		//}
		AddToggle("Off the Radar",offTheRadar);
		AddOption("~b~Character Customizer", null, nullFunc, SUB::CUSTOMCHARAC);
		AddOption("Model Changer", null, nullFunc, SUB::SKINCHANGER);
		AddOption("~b~Animations", null, nullFunc, SUB::PANIMATIONMENU);
		AddOption("Animation Scenarios", null, nullFunc, SUB::GTASCENIC);
		AddOption("Movement Clipsets", null, nullFunc, SUB::CLIPSETS);
		AddOption("~y~Detach All Objects V2", detachAllObjects);
		//	AddToggle("God Mode", invincible, null, invincibleDisabled);
		AddToggle("God Mode", invincible, null, gModDisabled);
		if (invincible) ENTITY::SET_ENTITY_INVINCIBLE(PLAYER::PLAYER_PED_ID(), true);
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
	/*void DriveOnWater() {
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
		//AddOption("Vehicle ModShop (LSC)", null, nullFunc, SUB::VEHICLE_LSC2);
		//AddOption("Vehicle Weapons", null, nullFunc, SUB::VEHICLEWEAPS);
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
	void SampleSub()
	{
		
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
		//AddOption("~b~Bullet Selector", null, nullFunc, SUB::BULLET);
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
		Ped playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(p);
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
		int clientHandle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(p);

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
		Player playerPed = selPlayer;
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
	
	void VehicleSpawner() {


		bool custominput = 0;
		AddTitle("Anonymous ");
		
		AddToggle("Use bypass", UseCarBypass);
		//AddToggle("Bypass car spawning limit", carspawnlimit);
		AddToggle("Spawn Fully Upgraded", automax);
		AddOption("Custom ~b~Input", custominput);
		AddOption("~HUD_COLOUR_GOLD~My Vehicles", null, nullFunc, SUB::SAVEDVEHICLES);
		AddOption("~b~Favorites", null, nullFunc, SUB::FAVORITE_VEV);
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
	{"Banshee", "BANSHEE2" },
	{"Vapid FMJ", "FMJ"},
	{"Emperor Sheava", "Sheava"},
	{"Turfade Nero", "NERO"},
	{"Turfade Nero(custom)", "NERO2"},
	{"Rocket Voltic", "VOLTIC2"},
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
	{"APC", "APC"},
	{"Ardent", "Ardent"},
	{"Bunker Caddy", "Caddy3"},
	{"Cheetah Classic", "Cheetah2"},
	{ "Dune FAV", "Dune3" },
	{"Half-Track", "Halftrack"},
	{"Hauler", "Hauler2"},
	{"Insugent Pick Up", "Insugent3"},
	{"Nightshark","Nightshark"},
	{"Oppressor","Oppressor"},
	{"Phantom", "Phantom3"},
	{"Weponized Tampa", "Tampa3"},
	{"Technical Aqua", "Techincal13"},
	{"Torero", "Torero"},
	{"MOC Trailer", "TrailerLarge"},
	{"Trailer small", "Trailers4"},
	{"Trailer small 2","Trailersmall"},
	{"Vagner", "Vagner"},
	{"Xa21", "Xa21"},

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
void set_supergrip()
{
	Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID());
	VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
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
	case SUB::CYCLES_VEV:				cyclesClass(); break;
	case SUB::UTILITY_VEV:				utilityClass(); break;
	case SUB::GUNRUNNING:				gunclass(); break;
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
	if (invincible) {
		ENTITY::SET_ENTITY_INVINCIBLE(PLAYER::PLAYER_PED_ID(), true);

	}
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
bool mph =1;
void printSpeed() {
	/*SET_TEXT_FONT(0);
	SET_TEXT_SCALE(7.26f, 7.26f);
	SET_TEXT_CENTRE(1);speedometertypesize
	_SET_TEXT_ENTRY("STRING");
	_ADD_TEXT_COMPONENT_STRING(name);*/
	if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 0)) {
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_PROPORTIONAL(1);
		//SET_TEXT_SCALE(0.0, 0.60);
		UI::SET_TEXT_SCALE(0.0, 0.92);
		UI::SET_TEXT_COLOUR(255, 255, 255, 255);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 255);
		UI::SET_TEXT_EDGE(1, 0, 0, 0, 255);
		UI::SET_TEXT_DROP_SHADOW();
		UI::SET_TEXT_OUTLINE();
		UI::_SET_TEXT_ENTRY("STRING");
		std::ostringstream oss;
		float speed;
		float mps = ENTITY::GET_ENTITY_SPEED(PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 0));
		float kmh = mps * 3600 / 1000;
		float milesperhour = kmh * 0.6213711916666667f;
		if (mph) speed = round(milesperhour);
		if (!mph) speed = round(kmh);
		oss << speed;
	//	UI::_ADD_TEXT_COMPONENT_STRING((char*)oss.str().c_str());
		//UI::_DRAW_TEXT((screen_res_x / 1000) - 0.096, 1 - 0.074);
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_PROPORTIONAL(1);
		//SET_TEXT_SCALE(0.0, 0.60);
		UI::SET_TEXT_SCALE(0.0, 0.38);
		UI::SET_TEXT_COLOUR(255, 255, 255, 255);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 255);
		UI::SET_TEXT_EDGE(1, 0, 0, 0, 255);
		UI::SET_TEXT_DROP_SHADOW();
		UI::SET_TEXT_OUTLINE();
		UI::_SET_TEXT_ENTRY("STRING");
		std::ostringstream osss;
		if (mph) osss << "~g~mph"; else osss << "~g~km/h";
//		UI::_ADD_TEXT_COMPONENT_STRING((char*)osss.str().c_str());
	//	UI::_DRAW_TEXT((screen_res_x / 1000) - 0.045, 1 - 0.043);
	}
	//_DRAW_TEXT(-0.015f + (screen_res_x / 100), -0.015f + (screen_res_y / 100));

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
	Player playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
	Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
	FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z + 15, 29, 999999.5f, false, true, 1.0f);
}
void set_loop_annoyBomb()
{
	Player playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
	Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
	FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z, 29, 9.0f, true, false, 0.0f);
}
void set_loop_forcefield()
{
	Player playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
	Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
	FIRE::ADD_EXPLOSION(pCoords.x, pCoords.y, pCoords.z, 7, 9.0f, false, true, 0.0f);
}
void set_clearpTasks()
{
	Player playerPed = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(selPlayer);
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
void menu::loops() {

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
		

		//	if (aimbot) Aimbot();

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
		//shootCustomBullet(ammoWeapon);
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
		//	if (noclip) noClip();
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
		//if (flyingUp) set_fup();
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
		//if (featureVehRockets) set_featureVehRockets();
		//if (featureVehFireworks) set_featureVehFireworks();
		//if (featureVehlaser1) set_featureVehlaser1();
		//if (featureVehlaser2) set_featureVehlaser2();
		//if (featureVehtrounds) set_featureVehtrounds();
		//if (featureVehprocked) set_featureVehprocked();
		//if (featureVehsnowball) set_featureVehsnowball();
		//if (featureVehballs) set_featureVehballs();
		//if (featureVehLight) set_featureVehLight();
		//if (featureVehtaser) set_featureVehtaser();
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

		//	if (loop_gravity_gun) set_gravity_gun();

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
}