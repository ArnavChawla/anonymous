#pragma once

class Script
{
public:
	static bool isInit();
	static void onTick();
}; extern uint RequestedModel;
enum RequestState
{
	loaded,
	requesting
}; 
extern RequestState model_state;
extern void(*modelFunction)();
#pragma once

class EnableCars
{
public:
	static void Run(HINSTANCE hinstDLL);
private:
	static void FindPatterns();
	static void FindScriptAddresses();
	static void EnableCarsGlobal();
	static void PatchCheatController();


	//not essential
	static void FindAffectedCars(int scriptSwitchOffset);
	static void FindSwitch(int funcCallOffset);
	static char* GetDisplayName(int modelHash);

};
