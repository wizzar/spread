#pragma once

extern IReGameApi *g_ReGameApi;
extern const ReGameFuncs_t *g_ReGameFuncs;
extern IReGameHookchains *g_ReGameHookchains;
extern CGameRules *g_pGameRules;

extern bool ReGameDLL_Init();
extern bool ReGameDLL_Stop();

CGameRules *ReGameDLL_InstallGameRules(IReGameHook_InstallGameRules *chain);
Vector &ReGameDLL_CBaseEntity_FireBullets3(IReGameHook_CBaseEntity_FireBullets3 *chain, CBaseEntity *pEntity, Vector &vecSrc, Vector &vecDirShooting, float vecSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t *pevAttacker, bool bPistol, int shared_rand);
