#pragma once

#include <regamedll_api.h>

//#include <vector.h>
//#include <progdefs.h>

// Make them extern here so that if
// they are needed elsewhere then
// just this header needs to be included.
extern IReGameApi* g_ReGameApi;
extern const ReGameFuncs_t* g_ReGameFuncs;
extern IReGameHookchains* g_ReGameHookchains;
extern CGameRules* g_pGameRules;

extern bool meta_init_regamedll_api();
extern bool meta_stop_regamedll_api();

CGameRules* InstallGameRules(IReGameHook_InstallGameRules*);

Vector& HookFireBullets3(IReGameHook_CBaseEntity_FireBullets3*, CBaseEntity*, Vector&, Vector&, float, float, int, int, int, float, entvars_t*, bool, int);

void HookKickBack(IReGameHook_CBasePlayerWeapon_KickBack* chain, CBasePlayerWeapon* pEntity, float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change);