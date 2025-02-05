#include "precompiled.h"

CSpread gSpread;

#ifndef DEBUG

#ifdef _MSC_VER
#define FORCEDINLINE __forceinline
#else
#define FORCEDINLINE __attribute__((always_inline))
#endif

#else

#define FORCEDINLINE

#endif

#ifdef DEBUG

int sc_DeadCenter = 0;
int sc_Airborne = 0;
int sc_StillStanding = 0;
int sc_StillDucking = 0;
int sc_MovingStanding = 0;
int sc_MovingDucking = 0;
int sc_Default = 0;

#define DEBUG_CONSOLE(...) LOG_CONSOLE(PLID, __VA_ARGS__)
#define LOG_SHOT(msg) logToFile(msg)
#else
#define DEBUG_CONSOLE(...)
#define LOG_SHOT(msg)
#endif

#define IS_STANDING(flags) (!((flags) & FL_DUCKING))
#define IS_ON_GROUND(flags) ((flags) & FL_ONGROUND)
#define IS_AIRBORNE(flags) (!((flags) & FL_ONGROUND))

FORCEDINLINE bool ShouldForceDeadCenterShot(CBasePlayer*, bool);

void CSpread::logToFile(const std::string& message) {

	if (this->logFile.is_open())
		this->logFile << message << std::endl;
}

void CSpread::ServerActivate()
{
#ifdef DEBUG
	if (!this->logFile.is_open())
	{
		this->logFile.open("cstrike/addons/spread/spread_log.txt", std::ios::app);

		if (!this->logFile) {
			LOG_ERROR(PLID, "ERROR OPENING SPREAD LOG FILE");
			LOG_CONSOLE(PLID, "ERROR OPENING SPREAD LOG FILE");
		}

		this->logToFile("Log Started");
	}
	else
	{
		this->logToFile("Server reloaded");
	}
#endif

	memset(this->m_WeaponsCfg, 0, sizeof(this->m_WeaponsCfg));

	char cmd_name[] = "spread_wpn";
	g_engfuncs.pfnAddServerCommand(cmd_name, this->SetWeapon);

	const char* cvarName = "spread_deadCenterFirstShot";
	cvar_t* cvarPtr = CVAR_GET_POINTER(cvarName);
	if (!cvarPtr)
	{
		// Defines whether the first shot is always dead center when standing or ducking, but not moving.
		cvar_t CvarData;
		char str_val[] = "0";
		CvarData = { cvarName, str_val, 0, 0.0f, NULL };
		CVAR_REGISTER(&CvarData);
		cvarPtr = CVAR_GET_POINTER(cvarName);

		if (cvarPtr) {
			this->m_deadCenterFirstShotCvar = cvarPtr;
		}
		else {
			LOG_ERROR(PLID, "Failed to register \"%s\" cvar: an error occurred!", cvarName);
			LOG_CONSOLE(PLID, "Failed to register \"%s\" cvar: an error occurred!", cvarName);
			this->m_deadCenterFirstShotCvar = NULL;
		}
	}
	else {
		LOG_ERROR(PLID, "Failed to register \"%s\" cvar: already exists!", cvarName);
		LOG_CONSOLE(PLID, "Failed to register \"%s\" cvar: already exists!", cvarName);
	}

	char execCfgCmd[] = "exec addons/spread/spread.cfg\n";
	g_engfuncs.pfnServerCommand(execCfgCmd);
}

void CSpread::SetWeapon()
{
	if (g_engfuncs.pfnCmd_Argc() >= 8)
	{
		std::string weaponName = g_engfuncs.pfnCmd_Argv(1);

		if (weaponName.find("weapon_") == std::string::npos)
		{
			weaponName = "weapon_" + weaponName;
		}

		if (!weaponName.empty())
		{
			auto slot = g_ReGameApi->GetWeaponSlot(weaponName.c_str());

			if (slot->slot == PRIMARY_WEAPON_SLOT ||
				slot->slot == PISTOL_SLOT)
			{
				gSpread.AddWeapon(slot->id,
					std::stof(g_engfuncs.pfnCmd_Argv(2)),  // InAir
					std::stof(g_engfuncs.pfnCmd_Argv(3)),  // MovingStanding
					std::stof(g_engfuncs.pfnCmd_Argv(4)),  // MovingDucking
					std::stof(g_engfuncs.pfnCmd_Argv(5)),  // StandingStill
					std::stof(g_engfuncs.pfnCmd_Argv(6)),  // DuckingStill
					std::stof(g_engfuncs.pfnCmd_Argv(7))); // Default

				LOG_CONSOLE(PLID, "Spread control for \"%s\" set successfully", g_engfuncs.pfnCmd_Argv(1));
			}
			else
			{
				LOG_CONSOLE(PLID, "Unknown weapon \"%s\"", g_engfuncs.pfnCmd_Argv(1));
			}
		}
	}
	else
	{
		LOG_CONSOLE(PLID, "[%s] Usage: %s <weapon_name> <in_air> <moving_standing> <moving_ducking> <standing_still> <ducking_still> <default>. Example: \"%s ak47 -1.0 -1.0 0.8 0.75 0.70 -1.01\".", Plugin_info.logtag, g_engfuncs.pfnCmd_Argv(0), g_engfuncs.pfnCmd_Argv(0));
	}
}

void CSpread::AddWeapon(int WeaponIndex, float InAir, float MovingStanding, float MovingDucking, float StandingStill, float DuckingStill, float Default)
{
	this->m_WeaponsCfg[WeaponIndex] =
	P_WEAPON_CTRL {
		true,
		InAir,
		MovingStanding,
		MovingDucking,
		StandingStill,
		DuckingStill,
		Default
	};
}

float CSpread::CalcSpread(CBaseEntity* pEntity, float vecSpread)
{
	CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEntity);
	if (!(pPlayer && pPlayer->m_pActiveItem))
	{
#ifdef DEBUG
		if (!pPlayer)
			DEBUG_CONSOLE("[%s] !Player", __FUNCTION__);
		else if (!pPlayer->m_pActiveItem)
			DEBUG_CONSOLE("[%s] !Player->m_pActiveItem", __FUNCTION__);
#endif
		return vecSpread;
	}

	// See if current player weapon is configured.
	const int wepId = pPlayer->m_pActiveItem->m_iId;
	P_WEAPON_CTRL weaponCfg = this->m_WeaponsCfg[wepId];
	
	if (!weaponCfg.IsValid)
		return vecSpread;

	const int flags = pPlayer->pev->flags;
	const auto pev = pPlayer->pev;

	// Player is in the air.
	if (IS_AIRBORNE(flags))
	{
		if (weaponCfg.InAir >= 0.0f)
		{
			DEBUG_CONSOLE("[%s] (airborne) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.InAir);
			LOG_SHOT("AIRBORNE " + std::to_string(++sc_Airborne));
			return vecSpread * weaponCfg.InAir;
		}
	}
	else
	{
		if (ShouldForceDeadCenterShot(pPlayer, this->m_deadCenterFirstShotCvar->value > 0.0f))
		{
			DEBUG_CONSOLE("[%s] (first shot dead center) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, 0.0f);
			LOG_SHOT("DEADCENTER " + std::to_string(++sc_DeadCenter));
			return 0.0f;
		}

		// If the player has any *horizontal* movement at all...
		if (!pev->velocity.IsZero())
		{
			// Player is standing.
			if (IS_STANDING(flags))
			{
				if (weaponCfg.MovingStanding >= 0.0f)
				{
					DEBUG_CONSOLE("[%s] (moving, standing) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.MovingStanding);
					LOG_SHOT("STILL STANDING " + std::to_string(++sc_StillStanding));
					return vecSpread * weaponCfg.MovingStanding;
				}
			}
			else
			{
				// Player is ducking.
				if (weaponCfg.MovingDucking >= 0.0f)
				{
					DEBUG_CONSOLE("[%s] (moving, ducking) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.MovingDucking);
					LOG_SHOT("STILL DUCKING " + std::to_string(++sc_StillDucking));
					return vecSpread * weaponCfg.MovingDucking;
				}
			}
		}
		else
		{
			// Player is still/stationary/motionless.

			// Player is standing.
			if (IS_STANDING(flags))
			{
				if (weaponCfg.StandingStill >= 0.0f)
				{
					DEBUG_CONSOLE("[%s] (still, standing) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.StandingStill);
					LOG_SHOT("MOVING STANDING " + std::to_string(++sc_MovingStanding));
					return vecSpread * weaponCfg.StandingStill;
				}

			}
			else
			{
				// Player is ducking.
				if (weaponCfg.DuckingStill >= 0.0f)
				{
					DEBUG_CONSOLE("[%s] (still, ducking) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.DuckingStill);
					LOG_SHOT("MOVING DUCKING " + std::to_string(++sc_MovingDucking));
					return vecSpread * weaponCfg.DuckingStill;
				}
			}
		}

	}

	// At this point the player may be airborne,
	// moving horizontally, or some settings
	// may not be set to override the spread.
	// However, we must be careful as to not
	// override the spread already mitigated
	// by previous conditions.
	if (weaponCfg.Default >= 0.0f)
	{
		DEBUG_CONSOLE("[%s] (default) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.Default);
		LOG_SHOT("DEFAULT " + std::to_string(++sc_Default));
		return vecSpread * weaponCfg.Default;
	}

	return vecSpread;
}

FORCEDINLINE bool ShouldForceDeadCenterShot(CBasePlayer* pPlayer, bool firstShotDeadCenter)
{
	// SCOPE WEAPONS CONSIDERATIONS:
	// AWP, SCOUT
	// SG550 -> CT tectec
	// G3SG1 -> TR tectec
	// SG552 -> TR krieg (B-4-4)
	// AUG   -> CT bullpup (B-4-4)
	//
	//   AWP & SCOUT WITH ZOOM      -> always dead center, if cvar set, and still.
	//   AWP & SCOUT WITHOUT ZOOM   -> never force dead center, just mitigate the resulting spread, if configured.
	// For both of these, punchangle is always zero, even if holding mouse button.
	// 
	// SG550 & G3SG1 WITH ZOOM      -> dead center on first shot, if cvar set.
	// SG550 & G3SG1 WITHOUT ZOOM   -> never force dead center, just mitigate the resulting spread, if configured.
	// For both of these, punchangle is also always zero, even if holding mouse button.
	// 
	//   AUG & SG552 WITH ZOOM      -> dead center on first shot, if cvar set.
	//   AUG & SG552 WITHOUT ZOOM   -> never force dead center, just mitigate the resulting spread, if configured,
	//                                 even though the original game does not change spread for these weapons
	//                                 when not zooming, only the time between shots.

	const auto pev = pPlayer->pev;

	if (firstShotDeadCenter &&
		pev->punchangle.IsZero())
	{
		bool hasActiveZoom = false;

		switch (pPlayer->m_pActiveItem->m_iId)
		{
		case WEAPON_AWP:
		case WEAPON_SCOUT:
			hasActiveZoom = pPlayer->m_bResumeZoom;
			break;

		case WEAPON_SG550:
		case WEAPON_G3SG1:
			hasActiveZoom = pev->fov != DEFAULT_FOV;
			break;

		default:

			if (pev->velocity.Length2D() < (pPlayer->m_pActiveItem->GetMaxSpeed() / 2))
			{
				DEBUG_CONSOLE( "[%s] (no scope) Dead center first shot!", __FUNCTION__);
				return true;
			}

			return false;
		}

		if (hasActiveZoom && pev->velocity.IsZero())
		{
			DEBUG_CONSOLE("[%s] (scoped) Dead center first shot!", __FUNCTION__);
			return true;
		}

		return false;				
	}

	return false;
}