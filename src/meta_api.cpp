#include "wrapper_meta_api.h"
#include "meta_globals.h"
#include "rehlds_api_plugin.h"
#include "regame_api_plugin.h"
#include "spread.h"

// Messages: https://wiki.alliedmods.net/Half-Life_1_Game_Events

#ifdef DO_DEBUG

#ifndef DEBUG_CONSOLE
#define DEBUG_CONSOLE(...) LOG_CONSOLE(PLID, __VA_ARGS__)
#endif

std::ofstream logFile;
void LogToFile(const char* fmt, ...)
{
	char buffer[1024]; // Buffer to hold the formatted string

	// Initialize variable argument list
	va_list args;
	va_start(args, fmt);

	// Format the string into buffer
	vsnprintf(buffer, sizeof(buffer), fmt, args);

	// Clean up variable argument list
	va_end(args);


	if (logFile.is_open())
	{
		logFile << std::string(buffer) << std::endl;
		logFile.flush();
	}
	else
	{
		logFile.open("messages_log.txt", std::ios::app);
		if (logFile.is_open())
		{
			logFile << std::string(buffer) << std::endl;
			logFile.flush();
		}
		else
			DEBUG_CONSOLE("ERROR OPENING MESSAGES LOG FILE");
	}
}

#define DEBUG_FILE(msg, ...) LogToFile(msg, __VA_ARGS__);

#else
#define DEBUG_CONSOLE(...)
#define DEBUG_FILE(msg, ...)
#endif

enginefuncs_t g_engfuncs;
globalvars_t* gpGlobals;
meta_globals_t* gpMetaGlobals;
mutil_funcs_t* gpMetaUtilFuncs;
gamedll_funcs_t* gpGamedllFuncs;

plugin_info_t Plugin_info =
{
	META_INTERFACE_VERSION,		// ifvers
	"Spread Control Plugin",	// name
	"1.0",						// version
	__DATE__,					// date
	"DevOak",					// author
	"https://github.com/wizzar/spread",	// url
	"SPREAD CTRL",				// logtag
	PT_ANYTIME,					// (when) loadable
	PT_ANYTIME,					// (when) unloadable
};

// Whatever we don't use, just set it to NULL.
// Must provide at least one function returning a standard HL function table,
// from either the following standard HLSDK functions.
// These are defined in the engine_api.cpp and dllapi.cpp files.
META_FUNCTIONS gMetaFunctionTable =
{
	NULL,						//            pfnGetEntityAPI	HL SDK; called before game DLL;
	NULL,						//       pfnGetEntityAPI_Post	META; called after game DLL;
	GetEntityAPI2,				//           pfnGetEntityAPI2	HL SDK2; called before game DLL;
	GetEntityAPI2_Post,			//      pfnGetEntityAPI2_Post	META; called after game DLL;
	NULL,						//      pfnGetNewDLLFunctions	HL SDK2; called before game DLL;
	NULL,						// pfnGetNewDLLFunctions_Post	META; called after game DLL;
	GetEngineFunctions,			//      pfnGetEngineFunctions	META; called before HL engine;
	NULL,						// pfnGetEngineFunctions_Post	META; called after HL engine;
};

DLL_FUNCTIONS g_DllFunctionTable =
{
	NULL,					// pfnGameInit
	NULL,					// pfnSpawn
	NULL,					// pfnThink
	NULL,					// pfnUse
	NULL,					// pfnTouch
	NULL,					// pfnBlocked
	NULL,					// pfnKeyValue
	NULL,					// pfnSave
	NULL,					// pfnRestore
	NULL,					// pfnSetAbsBox
	NULL,					// pfnSaveWriteFields
	NULL,					// pfnSaveReadFields
	NULL,					// pfnSaveGlobalState
	NULL,					// pfnRestoreGlobalState
	NULL,					// pfnResetGlobalState
	NULL,					// pfnClientConnect
	NULL,					// pfnClientDisconnect
	NULL,					// pfnClientKill
	NULL,					// pfnClientPutInServer
	NULL,					// pfnClientCommand
	NULL,					// pfnClientUserInfoChanged
	NULL,					// pfnServerActivate
	NULL,					// pfnServerDeactivate
	NULL,					// pfnPlayerPreThink
	NULL,					// pfnPlayerPostThink
	NULL,					// pfnStartFrame
	NULL,					// pfnParmsNewLevel
	NULL,					// pfnParmsChangeLevel
	NULL,					// pfnGetGameDescription
	NULL,					// pfnPlayerCustomization
	NULL,					// pfnSpectatorConnect
	NULL,					// pfnSpectatorDisconnect
	NULL,					// pfnSpectatorThink
	NULL,					// pfnSys_Error
	NULL,					// pfnPM_Move
	NULL,					// pfnPM_Init
	NULL,					// pfnPM_FindTextureType
	NULL,					// pfnSetupVisibility
	NULL,					// pfnUpdateClientData
	NULL,					// pfnAddToFullPack
	NULL,					// pfnCreateBaseline
	NULL,					// pfnRegisterEncoders
	NULL,					// pfnGetWeaponData
	NULL,					// pfnCmdStart
	NULL,					// pfnCmdEnd
	NULL,					// pfnConnectionlessPacket
	NULL,					// pfnGetHullBounds
	NULL,					// pfnCreateInstancedBaselines
	NULL,					// pfnInconsistentFile
	NULL,					// pfnAllowLagCompensation
};

DLL_FUNCTIONS g_DllFunctionTable_Post =
{
	NULL,					// pfnGameInit
	NULL,					// pfnSpawn
	NULL,					// pfnThink
	NULL,					// pfnUse
	NULL,					// pfnTouch
	NULL,					// pfnBlocked
	NULL,					// pfnKeyValue
	NULL,					// pfnSave
	NULL,					// pfnRestore
	NULL,					// pfnSetAbsBox
	NULL,					// pfnSaveWriteFields
	NULL,					// pfnSaveReadFields
	NULL,					// pfnSaveGlobalState
	NULL,					// pfnRestoreGlobalState
	NULL,					// pfnResetGlobalState
	NULL,					// pfnClientConnect
	NULL,					// pfnClientDisconnect
	NULL,					// pfnClientKill
	NULL,					// pfnClientPutInServer
	NULL,					// pfnClientCommand
	NULL,					// pfnClientUserInfoChanged
	NULL,					// pfnServerActivate
	NULL,					// pfnServerDeactivate
	NULL,					// pfnPlayerPreThink
	NULL,					// pfnPlayerPostThink
	NULL,					// pfnStartFrame
	NULL,					// pfnParmsNewLevel
	NULL,					// pfnParmsChangeLevel
	NULL,					// pfnGetGameDescription
	NULL,					// pfnPlayerCustomization
	NULL,					// pfnSpectatorConnect
	NULL,					// pfnSpectatorDisconnect
	NULL,					// pfnSpectatorThink
	NULL,					// pfnSys_Error
	NULL,					// pfnPM_Move
	NULL,					// pfnPM_Init
	NULL,					// pfnPM_FindTextureType
	NULL,					// pfnSetupVisibility
	NULL,					// pfnUpdateClientData
	NULL,					// pfnAddToFullPack
	NULL,					// pfnCreateBaseline
	NULL,					// pfnRegisterEncoders
	NULL,					// pfnGetWeaponData
	NULL,					// pfnCmdStart
	NULL,					// pfnCmdEnd
	NULL,					// pfnConnectionlessPacket
	NULL,					// pfnGetHullBounds
	NULL,					// pfnCreateInstancedBaselines
	NULL,					// pfnInconsistentFile
	NULL,					// pfnAllowLagCompensation
};

enginefuncs_t g_EngineFunctionsTable =
{
	NULL,		// pfnPrecacheModel()
	NULL,		// pfnPrecacheSound()
	NULL,		// pfnSetModel()
	NULL,		// pfnModelIndex()
	NULL,		// pfnModelFrames()
	NULL,		// pfnSetSize()
	NULL,		// pfnChangeLevel()
	NULL,		// pfnGetSpawnParms()
	NULL,		// pfnSaveSpawnParms()
	NULL,		// pfnVecToYaw()
	NULL,		// pfnVecToAngles()
	NULL,		// pfnMoveToOrigin()
	NULL,		// pfnChangeYaw()
	NULL,		// pfnChangePitch()
	NULL,		// pfnFindEntityByString()
	NULL,		// pfnGetEntityIllum()
	NULL,		// pfnFindEntityInSphere()
	NULL,		// pfnFindClientInPVS()
	NULL,		// pfnEntitiesInPVS()
	NULL,		// pfnMakeVectors()
	NULL,		// pfnAngleVectors()
	NULL,		// pfnCreateEntity()
	NULL,		// pfnRemoveEntity()
	NULL,		// pfnCreateNamedEntity()
	NULL,		// pfnMakeStatic()
	NULL,		// pfnEntIsOnFloor()
	NULL,		// pfnDropToFloor()
	NULL,		// pfnWalkMove()
	NULL,		// pfnSetOrigin()
	NULL,		// pfnEmitSound()
	NULL,		// pfnEmitAmbientSound()
	NULL,		// pfnTraceLine()
	NULL,		// pfnTraceToss()
	NULL,		// pfnTraceMonsterHull()
	NULL,		// pfnTraceHull()
	NULL,		// pfnTraceModel()
	NULL,		// pfnTraceTexture()
	NULL,		// pfnTraceSphere()
	NULL,		// pfnGetAimVector()
	NULL,		// pfnServerCommand()
	NULL,		// pfnServerExecute()
	NULL,		// pfnClientCommand()
	NULL,		// pfnParticleEffect()
	NULL,		// pfnLightStyle()
	NULL,		// pfnDecalIndex()
	NULL,		// pfnPointContents()
	NULL,		// pfnMessageBegin()
	NULL,		// pfnMessageEnd()
	NULL,		// pfnWriteByte()
	NULL,		// pfnWriteChar()
	NULL,		// pfnWriteShort()
	NULL,		// pfnWriteLong()
	NULL,		// pfnWriteAngle()
	NULL,		// pfnWriteCoord()
	NULL,		// pfnWriteString()
	NULL,		// pfnWriteEntity()
	NULL,		// pfnCVarRegister()
	NULL,		// pfnCVarGetFloat()
	NULL,		// pfnCVarGetString()
	NULL,		// pfnCVarSetFloat()
	NULL,		// pfnCVarSetString()
	NULL,		// pfnAlertMessage()
	NULL,		// pfnEngineFprintf()
	NULL,		// pfnPvAllocEntPrivateData()
	NULL,		// pfnPvEntPrivateData()
	NULL,		// pfnFreeEntPrivateData()
	NULL,		// pfnSzFromIndex()
	NULL,		// pfnAllocString()
	NULL,		// pfnGetVarsOfEnt()
	NULL,		// pfnPEntityOfEntOffset()
	NULL,		// pfnEntOffsetOfPEntity()
	NULL,		// pfnIndexOfEdict()
	NULL,		// pfnPEntityOfEntIndex()
	NULL,		// pfnFindEntityByVars()
	NULL,		// pfnGetModelPtr()
	NULL,		// pfnRegUserMsg()
	NULL,		// pfnAnimationAutomove()
	NULL,		// pfnGetBonePosition()
	NULL,		// pfnFunctionFromName()
	NULL,		// pfnNameForFunction()
	NULL,		// pfnClientPrintf()
	NULL,		// pfnServerPrint()
	NULL,		// pfnCmd_Args()
	NULL,		// pfnCmd_Argv()
	NULL,		// pfnCmd_Argc()
	NULL,		// pfnGetAttachment()
	NULL,		// pfnCRC32_Init()
	NULL,		// pfnCRC32_ProcessBuffer()
	NULL,		// pfnCRC32_ProcessByte()
	NULL,		// pfnCRC32_Final()
	NULL,		// pfnRandomLong()
	NULL,		// pfnRandomFloat()
	NULL,		// pfnSetView()
	NULL,		// pfnTime()
	NULL,		// pfnCrosshairAngle()
	NULL,		// pfnLoadFileForMe()
	NULL,		// pfnFreeFile()
	NULL,		// pfnEndSection()
	NULL,		// pfnCompareFileTime()
	NULL,		// pfnGetGameDir()
	NULL,		// pfnCvar_RegisterVariable()
	NULL,		// pfnFadeClientVolume()
	NULL,		// pfnSetClientMaxspeed()
	NULL,		// pfnCreateFakeClient()
	NULL,		// pfnRunPlayerMove()
	NULL,		// pfnNumberOfEntities()
	NULL,		// pfnGetInfoKeyBuffer()
	NULL,		// pfnInfoKeyValue()
	NULL,		// pfnSetKeyValue()
	NULL,		// pfnSetClientKeyValue()
	NULL,		// pfnIsMapValid()
	NULL,		// pfnStaticDecal()
	NULL,		// pfnPrecacheGeneric()
	NULL,		// pfnGetPlayerUserId()
	NULL,		// pfnBuildSoundMsg()
	NULL,		// pfnIsDedicatedServer()
	NULL,		// pfnCVarGetPointer()
	NULL,		// pfnGetPlayerWONId()
	NULL,		// pfnInfo_RemoveKey()
	NULL,		// pfnGetPhysicsKeyValue()
	NULL,		// pfnSetPhysicsKeyValue()
	NULL,		// pfnGetPhysicsInfoString()
	NULL,		// pfnPrecacheEvent()
	NULL,		// pfnPlaybackEvent()
	NULL,		// pfnSetFatPVS()
	NULL,		// pfnSetFatPAS()
	NULL,		// pfnCheckVisibility()
	NULL,		// pfnDeltaSetField()
	NULL,		// pfnDeltaUnsetField()
	NULL,		// pfnDeltaAddEncoder()
	NULL,		// pfnGetCurrentPlayer()
	NULL,		// pfnCanSkipPlayer()
	NULL,		// pfnDeltaFindField()
	NULL,		// pfnDeltaSetFieldByIndex()
	NULL,		// pfnDeltaUnsetFieldByIndex()
	NULL,		// pfnSetGroupMask()
	NULL,		// pfnCreateInstancedBaseline()
	NULL,		// pfnCvar_DirectSet()
	NULL,		// pfnForceUnmodified()
	NULL,		// pfnGetPlayerStats()
	NULL,		// pfnAddServerCommand()

	// Added in SDK 2.2:
	NULL,		// pfnVoice_GetClientListening()
	NULL,		// pfnVoice_SetClientListening()

	// Added for HL 1109 (no SDK update):
	NULL,		// pfnGetPlayerAuthId()

	// Added 2003/11/10 (no SDK update):
	NULL,		// pfnSequenceGet()
	NULL,		// pfnSequencePickSentence()
	NULL,		// pfnGetFileSize()
	NULL,		// pfnGetApproxWavePlayLen()
	NULL,		// pfnIsCareerMatch()
	NULL,		// pfnGetLocalizedStringLength()
	NULL,		// pfnRegisterTutorMessageShown()
	NULL,		// pfnGetTimesTutorMessageShown()
	NULL,		// pfnProcessTutorMessageDecayBuffer()
	NULL,		// pfnConstructTutorMessageDecayBuffer()
	NULL,		// pfnResetTutorMessageDecayData()

	// Added Added 2005-08-11 (no SDK update)
	NULL,		// pfnQueryClientCvarValue()
	// Added Added 2005-11-22 (no SDK update)
	NULL,		// pfnQueryClientCvarValue2()
	// Added 2009-06-17 (no SDK update)
	// NULL,	// pfnEngCheckParm()			// s1lent: comment out this, for compatibility with older versions metamod
};

void GameInit();
void HookClientCommand(edict_t* pEntity);
void HookMsgBegin(int msg_dest, int msg_type, const float* pOrigin, edict_t* ed);
void HookMsgEnd();
void HookWriteByte(int iValue);
void HookWriteChar(int iValue);
void HookWriteShort(int iValue);
void HookWriteLong(int iValue);
void HookWriteAngle(float flValue);
void HookWriteCoord(float flValue);
void HookWriteString(const char* sz);
void HookWriteEntity(int iValue);

#ifdef DO_DEBUG
void ServerActivate_Post(edict_t*, int, int);
#endif

// Receive engine function table and globals from the engine.
// This appears to be the _first_ DLL routine called by the engine,
// so we do some setup operations here.
//
// This function is called by GoldSrc engine
// when it is loading the game/mod dll (mp.dll for Counter-Strike).
// It is used to send engine functions to the game so that
// it can do what it wants with them, such as order a render,
// load a file, play a sound, whatever the game engine provides.
// GoldSrc was not made public by Valve, but you can get an
// idea of how it works here https://github.com/headcrab-junkyard/OGS/.
//
// Metamod will receive this call AS IF IT WAS the mod to be loaded,
// and will then sit between the engine and the actual game,
// allowing it to modify function calls etc.
// Metamod itself calls this function for each metamod plugin it loads as if it was the engine (https://github.com/rehlds/Metamod-R/blob/273618c684807a4b3809b23d2fd7a3353b46a5da/metamod/src/mplugin.cpp#L795).
//
// After this, we go on to the meta_api file.
C_DLLEXPORT void WINAPI GiveFnptrsToDll(enginefuncs_t* pengfuncsFromEngine, globalvars_t* pGlobals)
{
	memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
	gpGlobals = pGlobals;
}

// Metamod requesting info about this plugin:
// 
// interfaceVersion		(given)		interface version (META_INTERFACE_VERSION) Metamod is using;
//           plinfo		(requested)	struct with info about plugin that we must provide to Metamod;
//   pMetaUtilFuncs		(given)		table of utility functions provided by Metamod;
C_DLLEXPORT int Meta_Query(char *interfaceVersion, plugin_info_t **plinfo, mutil_funcs_t *pMetaUtilFuncs)
{
	*plinfo = &Plugin_info;
	gpMetaUtilFuncs = pMetaUtilFuncs;
	return TRUE;
}

// Metamod attaching plugin to the server.
//
//             now		(given)		current phase, i.e., during map, during changelevel, or at startup;
//  pFunctionTable		(requested)	table of function tables this plugin catches;
//       pMGlobals		(given)		global vars from Metamod;
//   pGamedllFuncs		(given)		copy of function tables from game dll;
// https://github.com/rehlds/Metamod-R/blob/273618c684807a4b3809b23d2fd7a3353b46a5da/metamod/src/mplugin.cpp#L969
C_DLLEXPORT int Meta_Attach(PLUG_LOADTIME now, META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs)
{
	// Store global vars from metamod.
	if (!pMGlobals) {
		LOG_ERROR(PLID, "Meta_Attach called with null pMGlobals");
		return(FALSE);
	}
	gpMetaGlobals = pMGlobals;

	// Give metamod function tables this plugin catches.
	if (!pFunctionTable) {
		LOG_ERROR(PLID, "Meta_Attach called with null pFunctionTable");
		return(FALSE);
	}
	memcpy(pFunctionTable, &gMetaFunctionTable, sizeof(META_FUNCTIONS));

	if (!pGamedllFuncs) {
		LOG_ERROR(PLID, "Meta_Attach called with null pGamedllFuncs");
		return(FALSE);
	}
	gpGamedllFuncs = pGamedllFuncs;

	char buffer[128];
	std::sprintf(buffer, "\n\n#########################\n# Meta_Attach from plugin %s #\n#########################\n\n", Plugin_info.name);
	SERVER_PRINT(buffer);

	// This is not plain metamod only.
	// Whole nother universe here.
	if (meta_init_rehlds_api())
		SERVER_PRINT("ReHLDS API successfully initialized.\n");

	if (meta_init_regamedll_api())
		SERVER_PRINT("ReGameDLL API successfully initialized.\n");

	return TRUE;
}

// Metamod detaching plugin from the server.
// 
//    now	(given)	current phase, i.e., during map, during changelevel, or at startup;
// reason	(given)	why detaching (refresh, console unload, forced unload, etc);
C_DLLEXPORT int Meta_Detach(PLUG_LOADTIME now, PL_UNLOAD_REASON reason)
{
	meta_stop_regamedll_api();
	return TRUE;
}

C_DLLEXPORT	int	GetEngineFunctions(enginefuncs_t* pengfuncsFromEngine, int* interfaceVersion)
{
	if (!pengfuncsFromEngine) {
		ALERT(at_logged, "%s called with null pengfuncsFromEngine", __FUNCTION__);
		return FALSE;
	}

	if (*interfaceVersion != ENGINE_INTERFACE_VERSION) {
		ALERT(at_logged, "%s version mismatch; requested=%d ours=%d", __FUNCTION__, *interfaceVersion, ENGINE_INTERFACE_VERSION);
		*interfaceVersion = ENGINE_INTERFACE_VERSION;
		return FALSE;
	}

	/*g_EngineFunctionsTable.pfnMessageBegin = HookMsgBegin;
	g_EngineFunctionsTable.pfnMessageEnd = HookMsgEnd;
	g_EngineFunctionsTable.pfnWriteByte = HookWriteByte;
	g_EngineFunctionsTable.pfnWriteChar = HookWriteChar;
	g_EngineFunctionsTable.pfnWriteShort = HookWriteShort;
	g_EngineFunctionsTable.pfnWriteLong = HookWriteLong;
	g_EngineFunctionsTable.pfnWriteAngle = HookWriteAngle;
	g_EngineFunctionsTable.pfnWriteCoord = HookWriteCoord;
	g_EngineFunctionsTable.pfnWriteString = HookWriteString;
	g_EngineFunctionsTable.pfnWriteEntity = HookWriteEntity;*/

	memcpy(pengfuncsFromEngine, &g_EngineFunctionsTable, sizeof(enginefuncs_t));
	return TRUE;
}

C_DLLEXPORT	int	GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
{
	if (!pFunctionTable) {
		ALERT(at_logged, "%s called with null pFunctionTable", __FUNCTION__);
		return FALSE;
	}

	if (*interfaceVersion != INTERFACE_VERSION) {
		ALERT(at_logged, "%s version mismatch; requested=%d ours=%d", __FUNCTION__, *interfaceVersion, INTERFACE_VERSION);
		*interfaceVersion = INTERFACE_VERSION;
		return FALSE;
	}

	//g_DllFunctionTable.pfnClientCommand = HookClientCommand;

	memcpy(pFunctionTable, &g_DllFunctionTable, sizeof(DLL_FUNCTIONS));

	return TRUE;
}

// META; called after game DLL.
C_DLLEXPORT int GetEntityAPI2_Post(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
{
	if (!pFunctionTable) {
		ALERT(at_logged, "%s called with null pFunctionTable", __FUNCTION__);
		return FALSE;
	}

	if (*interfaceVersion != INTERFACE_VERSION) {
		ALERT(at_logged, "%s version mismatch; requested=%d ours=%d", __FUNCTION__, *interfaceVersion, INTERFACE_VERSION);
		*interfaceVersion = INTERFACE_VERSION;
		return FALSE;
	}

	g_DllFunctionTable_Post.pfnGameInit = GameInit;

#ifdef DO_DEBUG
	g_DllFunctionTable_Post.pfnServerActivate = ServerActivate_Post;
#endif

	memcpy(pFunctionTable, &g_DllFunctionTable_Post, sizeof(DLL_FUNCTIONS));

	return TRUE;
}

void GameInit()
{
	if (gSpread.RegisterCvar())
	{
		LOG_CONSOLE(PLID, "CVAR & SERVER COMMAND REGISTERED");
		LOG_CONSOLE(PLID, "Executing addons/spread/spread.cfg...");
		char execCfgCmd[] = "exec addons/spread/spread.cfg\n";
		g_engfuncs.pfnServerCommand(execCfgCmd);
	}
	else {
		LOG_CONSOLE(PLID, "CVAR & SERVER COMMAND FAILED TO REGISTER. HALTING PLUGIN LOAD.");
	}

	RETURN_META(MRES_IGNORED);
}

void HookClientCommand(edict_t* pEntity)
{
	//CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEntity->pvPrivateData);

	// Arguments for the command, not including the command.
	//CMD_ARGS();
	// 
	// Returns the nth argument, being 0 the command string itself.
	// CMD_ARGV(0);
	//
	// Returns number of arguments.
	// CMD_ARGC();

	const char* cmd = CMD_ARGV(0);

	DEBUG_CONSOLE("CLCMD HOOK: %s with", cmd);
	auto c = CMD_ARGC();
	for (int i = 1; i < c; i++)
		DEBUG_CONSOLE("CLCMD HOOK: %s", CMD_ARGV(1));

	// Prevent people from changing team.
	// This won't even bring up the menu.
	// Handles key "m".
	if (Q_strcmp(cmd, "chooseteam") == 0)
	{
		DEBUG_CONSOLE("chooseteam halted");
		RETURN_META(MRES_SUPERCEDE);
	}

	// Handle joining via console jointeam command.
	if (Q_strcmp(cmd, "jointeam") == 0)
	{
		DEBUG_CONSOLE("jointeam halted");
		RETURN_META(MRES_SUPERCEDE);
	}
}

//void HookMsgBegin(int msg_dest, int msg_type, const float* pOrigin, edict_t* ed)
//{
//	char buffer[19];
//
//	switch (msg_dest)
//	{
//		case MSG_BROADCAST:              // unreliable to all
//			Q_snprintf(buffer, sizeof(buffer), "MSG_BROADCAST");
//			break;
//		case MSG_ONE:                    // reliable to one (msg_entity)
//			Q_snprintf(buffer, sizeof(buffer), "MSG_ONE");
//			break;
//		case MSG_ALL:                    // reliable to all
//			Q_snprintf(buffer, sizeof(buffer), "MSG_ALL");
//			break;
//		case MSG_INIT:                   // write to the init string
//			Q_snprintf(buffer, sizeof(buffer), "MSG_INIT");
//			break;
//		case MSG_PVS:                    // Ents in PVS of org
//			Q_snprintf(buffer, sizeof(buffer), "MSG_PVS");
//			break;
//		case MSG_PAS:                    // Ents in PAS of org
//			Q_snprintf(buffer, sizeof(buffer), "MSG_PAS");
//			break;
//		case MSG_PVS_R:                  // Reliable to PVS
//			Q_snprintf(buffer, sizeof(buffer), "MSG_PVS_R");
//			break;
//		case MSG_PAS_R:                  // Reliable to PAS
//			Q_snprintf(buffer, sizeof(buffer), "MSG_PAS_R");
//			break;
//		case MSG_ONE_UNRELIABLE:         // Send to one client, but don't put in reliable stream, put in unreliable datagram ( could be dropped )
//			Q_snprintf(buffer, sizeof(buffer), "MSG_ONE_UNRELIABLE");
//			break;
//		case MSG_SPEC:                   // Sends to all spectator proxies
//			Q_snprintf(buffer, sizeof(buffer), "MSG_SPEC");
//			break;
//	default:
//		Q_snprintf(buffer, sizeof(buffer), "UNKNOWN MSG");
//		break;
//	}
//	
//	int msgSize;
//	std::string origin = pOrigin ? std::to_string(*pOrigin) : "null";
//
//	DEBUG_CONSOLE("[%s] [msg_dest: %d (%s)] [msg_tpye: %d (%s)] [pOrigin: %f] [edict class: %s]", __FUNCTION__, msg_dest, buffer, msg_type, GET_USER_MSG_NAME(PLID, msg_type, &msgSize), origin.c_str(), ed ? STRING(ed->v.classname) : "null");
//	DEBUG_FILE("[%s] [msg_dest: %d (%s)] [msg_tpye: %d (%s)] [pOrigin: %f] [edict class: %s]", __FUNCTION__, msg_dest, buffer, msg_type, GET_USER_MSG_NAME(PLID, msg_type, &msgSize), origin.c_str(), ed ? STRING(ed->v.classname) : "null");
//}
//
//void HookMsgEnd()
//{
//	DEBUG_CONSOLE("[%s] done\n", __FUNCTION__);
//	DEBUG_FILE("[%s] done\n", __FUNCTION__);
//}
//
//void HookWriteByte(int iValue)
//{
//	DEBUG_CONSOLE("[%s] %d", __FUNCTION__, iValue);
//	DEBUG_FILE("[%s] %d", __FUNCTION__, iValue);
//}
//
//void HookWriteChar(int iValue)
//{
//	DEBUG_CONSOLE("[%s] %d", __FUNCTION__, iValue);
//	DEBUG_FILE("[%s] %d", __FUNCTION__, iValue);
//}
//
//void HookWriteShort(int iValue)
//{
//	DEBUG_CONSOLE("[%s] %d", __FUNCTION__, iValue);
//	DEBUG_FILE("[%s] %d", __FUNCTION__, iValue);
//}
//
//void HookWriteLong(int iValue)
//{
//	DEBUG_CONSOLE("[%s] %d", __FUNCTION__, iValue);
//	DEBUG_FILE("[%s] %d", __FUNCTION__, iValue);
//}
//
//void HookWriteAngle(float flValue)
//{
//	DEBUG_CONSOLE("[%s] %f", __FUNCTION__, flValue);
//	DEBUG_FILE("[%s] %f", __FUNCTION__, flValue);
//}
//
//void HookWriteCoord(float flValue)
//{
//	DEBUG_CONSOLE("[%s] %f", __FUNCTION__, flValue);
//	DEBUG_FILE("[%s] %f", __FUNCTION__, flValue);
//}
//
//void HookWriteString(const char* sz)
//{
//	DEBUG_CONSOLE("[%s] %s", __FUNCTION__, sz);
//	DEBUG_FILE("[%s] %s", __FUNCTION__, sz);
//}
//
//void HookWriteEntity(int iValue)
//{
//	DEBUG_CONSOLE("[%s] %d", __FUNCTION__, iValue);
//	DEBUG_FILE("[%s] %d", __FUNCTION__, iValue);
//}

#ifdef DO_DEBUG
void ServerActivate_Post(edict_t* pEdictList, int edictCount, int clientMax)
{
	gSpread.SetupLog();
	RETURN_META(MRES_IGNORED);
}
#endif
