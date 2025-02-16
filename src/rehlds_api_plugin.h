#pragma once

#include <rehlds_api.h> // IRehldsApi, RehldsFuncs_t, IRehldsHookchains;
#include "rehlds_interfaces.h" // IRehldsServerStatic, IRehldsServerData;
#include "IMessageManager.h" // IMessageManager;

extern IRehldsApi* g_RehldsApi;
extern const RehldsFuncs_t* g_RehldsFuncs;
extern IRehldsHookchains* g_RehldsHookchains;
extern IRehldsServerStatic* g_RehldsSvs;
extern IRehldsServerData* g_RehldsData;
extern IMessageManager* g_RehldsMessageManager;

// Will be called form Metamod API @ Meta_Attach.
bool meta_init_rehlds_api();