#include <osconfig.h>
#include "wrapper_meta_api.h"
#include "meta_globals.h"
#include "rehlds_api_plugin.h"
#include "regame_api_plugin.h"

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
	NULL,						// pfnGetEntityAPI		HL SDK; called before game DLL
	NULL,						// pfnGetEntityAPI_Post		META; called after game DLL
	GetEntityAPI2,				// pfnGetEntityAPI2		HL SDK2; called before game DLL
	GetEntityAPI2_Post,			// pfnGetEntityAPI2_Post	META; called after game DLL
	GetNewDLLFunctions,			// pfnGetNewDLLFunctions	HL SDK2; called before game DLL
	GetNewDLLFunctions_Post,	// pfnGetNewDLLFunctions_Post	META; called after game DLL
	GetEngineFunctions,			// pfnGetEngineFunctions	META; called before HL engine
	GetEngineFunctions_Post,	// pfnGetEngineFunctions_Post	META; called after HL engine
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

NEW_DLL_FUNCTIONS g_NewDllFunctionTable =
{
	NULL,					//! pfnOnFreeEntPrivateData()	Called right before the object's memory is freed.  Calls its destructor.
	NULL,					//! pfnGameShutdown()
	NULL,					//! pfnShouldCollide()
	NULL,					//! pfnCvarValue()
	NULL,					//! pfnCvarValue2()
};

NEW_DLL_FUNCTIONS g_NewDllFunctionTable_Post =
{
	NULL,					//! pfnOnFreeEntPrivateData()	Called right before the object's memory is freed.  Calls its destructor.
	NULL,					//! pfnGameShutdown()
	NULL,					//! pfnShouldCollide()
	NULL,					//! pfnCvarValue()
	NULL,					//! pfnCvarValue2()
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

enginefuncs_t g_EngineFunctionsTable_Post =
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
	char buffer[128];
	std::sprintf(buffer, "\n\n#########################\n# Interface version: %s #\n#########################\n\n", interfaceVersion);
	SERVER_PRINT(buffer);
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

	if (pGamedllFuncs) {
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

C_DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
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

	memcpy(pFunctionTable, &g_DllFunctionTable_Post, sizeof(DLL_FUNCTIONS));

	return TRUE;
}

C_DLLEXPORT int GetNewDLLFunctions(NEW_DLL_FUNCTIONS* pNewFunctionTable, int* interfaceVersion)
{
	if (!pNewFunctionTable) {
		ALERT(at_logged, "%s called with null pNewFunctionTable", __FUNCTION__);
		return FALSE;
	}
	if (*interfaceVersion != NEW_DLL_FUNCTIONS_VERSION) {
		ALERT(at_logged, "%s version mismatch; requested=%d ours=%d", __FUNCTION__, *interfaceVersion, NEW_DLL_FUNCTIONS_VERSION);
		*interfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
		return FALSE;
	}

	memcpy(pNewFunctionTable, &g_NewDllFunctionTable, sizeof(NEW_DLL_FUNCTIONS));
	return TRUE;
}

C_DLLEXPORT int GetNewDLLFunctions_Post(NEW_DLL_FUNCTIONS* pNewFunctionTable, int* interfaceVersion)
{
	if (!pNewFunctionTable) {
		ALERT(at_logged, "%s called with null pNewFunctionTable", __FUNCTION__);
		return FALSE;
	}
	if (*interfaceVersion != NEW_DLL_FUNCTIONS_VERSION) {
		ALERT(at_logged, "%s version mismatch; requested=%d ours=%d", __FUNCTION__, *interfaceVersion, NEW_DLL_FUNCTIONS_VERSION);
		*interfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
		return FALSE;
	}

	memcpy(pNewFunctionTable, &g_NewDllFunctionTable_Post, sizeof(NEW_DLL_FUNCTIONS));
	return TRUE;
}

C_DLLEXPORT int GetEngineFunctions(enginefuncs_t* pengfuncsFromEngine, int* interfaceVersion)
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

	memcpy(pengfuncsFromEngine, &g_EngineFunctionsTable, sizeof(enginefuncs_t));
	return TRUE;
}

C_DLLEXPORT int GetEngineFunctions_Post(enginefuncs_t* pengfuncsFromEngine, int* interfaceVersion)
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

	memcpy(pengfuncsFromEngine, &g_EngineFunctionsTable_Post, sizeof(enginefuncs_t));
	return TRUE;
}