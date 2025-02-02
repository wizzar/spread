#include "precompiled.h"
//#include <chrono>

IReGameApi *g_ReGameApi;
const ReGameFuncs_t *g_ReGameFuncs;
IReGameHookchains *g_ReGameHookchains;
CGameRules *g_pGameRules;

bool ReGameDLL_Init()
{
	auto szGameDLLModule = gpMetaUtilFuncs->pfnGetGameInfo(PLID, GINFO_DLL_FULLPATH);

	if (szGameDLLModule)
	{
		auto GameModule = Sys_GetModuleHandle(szGameDLLModule);

		if (GameModule)
		{
			auto ifaceFactory = Sys_GetFactory(GameModule);

			if (ifaceFactory)
			{
				int retCode = 0;

				g_ReGameApi = (IReGameApi *)ifaceFactory(VRE_GAMEDLL_API_VERSION, &retCode);

				if (g_ReGameApi)
				{
					if (g_ReGameApi->GetMajorVersion() == REGAMEDLL_API_VERSION_MAJOR)
					{
						if (g_ReGameApi->GetMinorVersion() >= REGAMEDLL_API_VERSION_MINOR)
						{
							if (g_ReGameApi->BGetICSEntity(CSENTITY_API_INTERFACE_VERSION))
							{
								if (g_ReGameApi->BGetIGameRules(GAMERULES_API_INTERFACE_VERSION))
								{
									g_ReGameFuncs = g_ReGameApi->GetFuncs();

									g_ReGameHookchains = g_ReGameApi->GetHookchains();

									if (g_ReGameHookchains)
									{
										g_ReGameHookchains->InstallGameRules()->registerHook(ReGameDLL_InstallGameRules);

										g_ReGameHookchains->CBaseEntity_FireBullets3()->registerHook(ReGameDLL_CBaseEntity_FireBullets3);
									}

									gpMetaUtilFuncs->pfnLogConsole(PLID, "[%s] ReGameDLL API Loaded: %d.%d", Plugin_info.logtag, REGAMEDLL_API_VERSION_MAJOR, REGAMEDLL_API_VERSION_MINOR);

									return true;
								}
							}
						}
					}
				}
			}
		}
	}

	gpMetaUtilFuncs->pfnLogConsole(PLID, "[%s] ReGameDLL API failed to load.", Plugin_info.logtag);

	return false;
}

bool ReGameDLL_Stop()
{
	if (g_ReGameHookchains)
	{
		g_ReGameHookchains->InstallGameRules()->unregisterHook(ReGameDLL_InstallGameRules);

		g_ReGameHookchains->CBaseEntity_FireBullets3()->unregisterHook(ReGameDLL_CBaseEntity_FireBullets3);

	}

	return true;
}

CGameRules *ReGameDLL_InstallGameRules(IReGameHook_InstallGameRules *chain)
{
	auto Result = chain->callNext();

	if (g_ReGameApi)
	{
		g_pGameRules = Result;
	}

	return Result;
}

Vector &ReGameDLL_CBaseEntity_FireBullets3(IReGameHook_CBaseEntity_FireBullets3 *chain, CBaseEntity *pEntity, Vector &vecSrc, Vector &vecDirShooting, float vecSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t *pevAttacker, bool bPistol, int shared_rand)
{
	//using std::chrono::high_resolution_clock;
	//using std::chrono::duration_cast;
	//using std::chrono::duration;
	//using std::chrono::milliseconds;

	//auto t1 = high_resolution_clock::now();
	vecSpread = gSpread.CalcSpread(pEntity, vecSpread);
	//auto t2 = high_resolution_clock::now();

	//auto ms_int = duration_cast<milliseconds>(t2 - t1);
	//duration<double, std::milli> ms_double = t2 - t1;
	//LOG_CONSOLE(PLID, "TIME: %g", ms_double.count());

	return chain->callNext(pEntity, vecSrc, vecDirShooting, vecSpread, flDistance, iPenetration, iBulletType, iDamage, flRangeModifier, pevAttacker, bPistol, shared_rand);
}
