#include "spread.h"
#include "regame_api_plugin.h"
#include <chrono>
#include <ctime>

CSpread gSpread;

#define DO_DEBUG

#ifndef DO_DEBUG

#ifdef _MSC_VER
#define FORCEDINLINE __forceinline
#else
#define FORCEDINLINE __attribute__((always_inline))
#endif

#else

#define FORCEDINLINE

#endif

#ifdef DO_DEBUG

// Persist these throughout the whole server life to measure statistics.
int sc_DeadCenter = 0;
int sc_Airborne = 0;
int sc_StillStanding = 0;
int sc_StillDucking = 0;
int sc_MovingStanding = 0;
int sc_MovingDucking = 0;
int sc_Default = 0;
auto last = std::chrono::steady_clock::now();
#define DEBUG_CONSOLE(...) LOG_CONSOLE(PLID, __VA_ARGS__)
#define LOG_FILE(msg) (this->LogToFile(msg))

void CSpread::LogToFile(const std::string& message) {

	if (this->m_logFile.is_open())
		this->m_logFile << message << std::endl;
}

void CSpread::SetupLog()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
	std::string nowStr(std::ctime(&now_time_t));

	if (!this->m_logFile.is_open())
	{
		this->m_logFile.open("cstrike/addons/spread/spread_log.txt", std::ios::app);

		if (!this->m_logFile) {
			LOG_ERROR(PLID, "ERROR OPENING SPREAD LOG FILE");
			LOG_CONSOLE(PLID, "ERROR OPENING SPREAD LOG FILE");
		}

		LOG_FILE("Log Started - " + nowStr);
	}
	else
	{
		LOG_FILE("Server Reloaded - " + nowStr);
	}
}

#else

#define DEBUG_CONSOLE(...)
#define LOG_FILE(msg)

#endif

#define IS_STANDING(flags) (!((flags) & FL_DUCKING))
#define IS_ON_GROUND(flags) ((flags) & FL_ONGROUND)
#define IS_AIRBORNE(flags) (!((flags) & FL_ONGROUND))

FORCEDINLINE bool ShouldForceDeadCenterShot(CBasePlayer*, bool);

bool CSpread::RegisterCvar()
{
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
			this->m_pDeadCenterFirstShotCvar = cvarPtr;
		}
		else {
			LOG_ERROR(PLID, "Failed to register \"%s\" cvar: an error occurred!", cvarName);
			LOG_CONSOLE(PLID, "Failed to register \"%s\" cvar: an error occurred!", cvarName);
			this->m_pDeadCenterFirstShotCvar = NULL;
			return false;
		}
	}
	else {
		LOG_ERROR(PLID, "Failed to register \"%s\" cvar: already exists!", cvarName);
		LOG_CONSOLE(PLID, "Failed to register \"%s\" cvar: already exists!", cvarName);
		return false;
	}

	return true;
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
	this->m_rgWeaponsCfg[WeaponIndex] =
	WEAPON_SPREAD_CFG {
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

#ifdef DO_DEBUG
	// Log every 10 seconds.
	if (std::chrono::duration<double>(std::chrono::steady_clock::now() - last).count() > 30)
	{
		auto now = std::chrono::system_clock::now();
		std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
		std::tm* tmPtr = std::localtime(&currentTime);
		std::ostringstream oss;
		oss << std::put_time(tmPtr, "%H:%M:%S");
		LOG_FILE(oss.str());

		LOG_FILE("AIRBORNE " + std::to_string(sc_Airborne));
		LOG_FILE("DEADCENTER " + std::to_string(sc_DeadCenter));
		LOG_FILE("STILL STANDING " + std::to_string(sc_StillStanding));
		LOG_FILE("STILL DUCKING " + std::to_string(sc_StillDucking));
		LOG_FILE("MOVING STANDING " + std::to_string(sc_MovingStanding));
		LOG_FILE("MOVING DUCKING " + std::to_string(sc_MovingDucking));
		LOG_FILE("DEFAULT " + std::to_string(sc_Default));

		if (this->m_logFile.is_open())
			this->m_logFile.flush();

		last = std::chrono::steady_clock::now();
	}
#endif

	CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEntity);
	if (!(pPlayer && pPlayer->m_pActiveItem))
	{
#ifdef DO_DEBUG
		if (!pPlayer)
			DEBUG_CONSOLE("[%s] !Player", __FUNCTION__);
		else if (!pPlayer->m_pActiveItem)
			DEBUG_CONSOLE("[%s] !Player->m_pActiveItem", __FUNCTION__);
#endif
		return vecSpread;
	}

	// See if current player weapon is configured.
	const int wepId = pPlayer->m_pActiveItem->m_iId;
	WEAPON_SPREAD_CFG weaponCfg = this->m_rgWeaponsCfg[wepId];
	
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
#ifdef DO_DEBUG			
			sc_Airborne += 1;
#endif
			return vecSpread * weaponCfg.InAir;
		}
	}
	else
	{
		if (ShouldForceDeadCenterShot(pPlayer, this->m_pDeadCenterFirstShotCvar->value > 0.0f))
		{
			DEBUG_CONSOLE("[%s] (first shot dead center) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, 0.0f);
#ifdef DO_DEBUG
			sc_DeadCenter += 1;
#endif
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
#ifdef DO_DEBUG
					sc_MovingStanding += 1;
#endif
					return vecSpread * weaponCfg.MovingStanding;
				}
			}
			else
			{
				// Player is ducking.
				if (weaponCfg.MovingDucking >= 0.0f)
				{
					DEBUG_CONSOLE("[%s] (moving, ducking) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.MovingDucking);
#ifdef DO_DEBUG
					sc_MovingDucking += 1;
#endif
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
#ifdef DO_DEBUG
					sc_StillStanding += 1;
#endif
					return vecSpread * weaponCfg.StandingStill;
				}

			}
			else
			{
				// Player is ducking.
				if (weaponCfg.DuckingStill >= 0.0f)
				{
					DEBUG_CONSOLE("[%s] (still, ducking) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.DuckingStill);
#ifdef DO_DEBUG
					sc_StillDucking += 1;
#endif
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
#ifdef DO_DEBUG
		sc_Default += 1;
#endif
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