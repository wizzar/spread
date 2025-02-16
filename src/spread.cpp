#include <chrono>
#include <extdll.h> // Needed by meta_api;
#include <meta_api.h> // Need access to global variables;
#include "spread.h"
#include "regame_api_plugin.h"
#include "progdefs.h" // entvars_t;
#include "player.h" // CBasePlayer;
#include "cbase.h" // CBaseEntity;
#include "qstring.h" // gpGlobals;

enum PLAYER_SITUATION
{						// Rough % estimates (relevant to optimize the order
						// of the GetPlayerSituation and the switch in CalcSpread):
	STANDING_STILL,		// 24.70%
	STANDING_MOVING,	// 32.20%
	DUCKING_STILL,		// 12.51%
	DUCKING_MOVING,		//	1.41%
	AIRBORNE			//	1.29%
};

CSpread gSpread;

#ifdef DO_DEBUG

#define FORCEDINLINE
#define LOG_FILE(msg) (this->logger.Log(msg))

#ifdef LOCAL_DEBUG
#define MIN_PLAYERS_TO_LOG 1
#define LOG_INTERVAL_SEC 5
#define DEBUG_CONSOLE(...) LOG_CONSOLE(PLID, __VA_ARGS__)
#else
#define MIN_PLAYERS_TO_LOG 7
#define LOG_INTERVAL_SEC 30
#define DEBUG_CONSOLE(...)
#endif

// Persist these throughout the whole server life to measure statistics.
int sc_DeadCenter = 0;
int sc_Airborne = 0;
int sc_StillStanding = 0;
int sc_StillDucking = 0;
int sc_MovingStanding = 0;
int sc_MovingDucking = 0;
int sc_Default = 0;
auto last = std::chrono::steady_clock::now();

#else

#ifdef _MSC_VER
#define FORCEDINLINE __forceinline
#else
#define FORCEDINLINE __attribute__((always_inline))
#endif

#define DEBUG_CONSOLE(...)
#define LOG_FILE(msg)

#endif

#define IS_STANDING(flags) (!((flags) & FL_DUCKING))
#define IS_ON_GROUND(flags) ((flags) & FL_ONGROUND)
#define IS_AIRBORNE(flags) (!((flags) & FL_ONGROUND))

FORCEDINLINE PLAYER_SITUATION GetPlayerSituation(int, entvars_t*);
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

	// Log every 30 seconds.
	if (std::chrono::duration<double>(std::chrono::steady_clock::now() - last).count() > LOG_INTERVAL_SEC)
	{
		int numPl = 0;
		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(i);

			if (pEdict && !pEdict->free && pEdict->v.flags & FL_CLIENT)
				numPl++;
		}

		if (numPl >= MIN_PLAYERS_TO_LOG)
		{
			LOG_FILE("AIRBORNE " + std::to_string(sc_Airborne));
			LOG_FILE("DEADCENTER " + std::to_string(sc_DeadCenter));
			LOG_FILE("STILL STANDING " + std::to_string(sc_StillStanding));
			LOG_FILE("STILL DUCKING " + std::to_string(sc_StillDucking));
			LOG_FILE("MOVING STANDING " + std::to_string(sc_MovingStanding));
			LOG_FILE("MOVING DUCKING " + std::to_string(sc_MovingDucking));
			LOG_FILE("DEFAULT " + std::to_string(sc_Default));
		}

		last = std::chrono::steady_clock::now();
	}
#endif

	CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEntity);
	if (!(pPlayer && pPlayer->m_pActiveItem))
	{
#ifdef DO_DEBUG
		if (!pPlayer)
			LOG_FILE("!Player");
		else if (!pPlayer->m_pActiveItem)
			LOG_FILE("!Player->m_pActiveItem");
#endif
		return vecSpread;
	}

	// See if current player weapon is configured.
	const int wepId = pPlayer->m_pActiveItem->m_iId;
	WEAPON_SPREAD_CFG weaponCfg = this->m_rgWeaponsCfg[wepId];

	if (!weaponCfg.IsValid)
		return vecSpread;

	const PLAYER_SITUATION situation = GetPlayerSituation(pPlayer->pev->flags, pPlayer->pev);

	if (ShouldForceDeadCenterShot(pPlayer, this->m_pDeadCenterFirstShotCvar->value > 0.0f))
	{
#ifdef DO_DEBUG
		sc_DeadCenter += 1;
#endif
		DEBUG_CONSOLE("(dead center) [OLD SP: %f] [NEW SP: %f]", vecSpread, 0.0f);
		return 0.0f;
	}

	// Ordered according to server statistics.
	switch (situation)
	{

	case STANDING_MOVING:

#ifdef DO_DEBUG
		sc_MovingStanding += 1;
#endif
		if (weaponCfg.MovingStanding >= 0.0f)
		{
			DEBUG_CONSOLE("(moving, standing) [OLD SP: %f] [NEW SP: %f]", vecSpread, vecSpread * weaponCfg.MovingStanding);
			return vecSpread * weaponCfg.MovingStanding;
		}
#ifdef DO_DEBUG	
		else
		{
			DEBUG_CONSOLE("(moving, standing) not configured");
		}
#endif

		break;

	case STANDING_STILL:

#ifdef DO_DEBUG
		sc_StillStanding += 1;
#endif
		if (weaponCfg.StandingStill >= 0.0f)
		{
			DEBUG_CONSOLE("(still, standing) [OLD SP: %f] [NEW SP: %f]", vecSpread, vecSpread * weaponCfg.StandingStill);
			return vecSpread * weaponCfg.StandingStill;
		}
#ifdef DO_DEBUG	
		else
		{
			DEBUG_CONSOLE("(still, standing) not configured");
		}
#endif

		break;

	case DUCKING_STILL:

#ifdef DO_DEBUG
		sc_StillDucking += 1;
#endif
		// Player is ducking.
		if (weaponCfg.DuckingStill >= 0.0f)
		{
			DEBUG_CONSOLE("(still, ducking) [OLD SP: %f] [NEW SP: %f]", vecSpread, vecSpread * weaponCfg.DuckingStill);
			return vecSpread * weaponCfg.DuckingStill;
		}
#ifdef DO_DEBUG	
		else
		{
			DEBUG_CONSOLE("(still, ducking) not configured");
		}
#endif

		break;

	case AIRBORNE:

#ifdef DO_DEBUG			
		sc_Airborne += 1;
#endif
		if (weaponCfg.InAir >= 0.0f)
		{
			DEBUG_CONSOLE("(airborne) [OLD SP: %f] [NEW SP: %f]", vecSpread, vecSpread * weaponCfg.InAir);
			return vecSpread * weaponCfg.InAir;
		}
#ifdef DO_DEBUG	
		else
		{
			DEBUG_CONSOLE("(airborne) not configured");
		}
#endif

		break;

	case DUCKING_MOVING:

#ifdef DO_DEBUG
		sc_MovingDucking += 1;
#endif
		// Player is ducking.
		if (weaponCfg.MovingDucking >= 0.0f)
		{
			DEBUG_CONSOLE("(moving, ducking) [OLD SP: %f] [NEW SP: %f]", vecSpread, vecSpread * weaponCfg.MovingDucking);
			return vecSpread * weaponCfg.MovingDucking;
		}
#ifdef DO_DEBUG	
		else
		{
			DEBUG_CONSOLE("(moving, ducking) not configured");
		}
#endif
		break;

#ifdef DO_DEBUG
	default:
		DEBUG_CONSOLE("SWITCH(situation) DEFAULTED: SHOULD NEVER HAPPEN!");
		LOG_FILE("SWITCH(situation) DEFAULTED: SHOULD NEVER HAPPEN!");
		break;
#endif

	}

	// At this point the player may be
	// in any situation, but the specific
	// configuration was not set (-1.0),
	// so we check for default.
	if (weaponCfg.Default >= 0.0f)
	{
#ifdef DO_DEBUG
		sc_Default += 1;
#endif
		DEBUG_CONSOLE("(default) [OLD SP: %f] [NEW SP: %f]", vecSpread, vecSpread * weaponCfg.Default);
		return vecSpread * weaponCfg.Default;
	}
#ifdef DO_DEBUG
	else 
	{
		DEBUG_CONSOLE("(default) not configured");
	}
#endif

	return vecSpread;
}

FORCEDINLINE PLAYER_SITUATION GetPlayerSituation(int flags, entvars_t* pev)
{
	// Player is on the ground, not in the air, not jumping.
	if (IS_ON_GROUND(flags))
	{
		// Standing.
		if (IS_STANDING(flags))
		{
			return !pev->velocity.IsZero() ? STANDING_MOVING : STANDING_STILL;
		}

		// Crouching.
		return pev->velocity.IsZero() ? DUCKING_STILL : DUCKING_MOVING;
	}

	// Player is in the air.
	return AIRBORNE;
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

			if (IS_ON_GROUND(pev->flags))
			{
				if (IS_STANDING(pev->flags))
				{
					if (pev->velocity.Length2D() < (pPlayer->m_pActiveItem->GetMaxSpeed() / 2))
					{
						DEBUG_CONSOLE("(dead center) no scope, standing (vel: %f, max: %f )",
							pev->velocity.Length2D(),
							pPlayer->m_pActiveItem->GetMaxSpeed());

						return true;
					}
				}
				else 
				{
					// Velocity threshold must be different for ducking,
					// otherwise you can move while ducking and hit dead on every time.
					if (pev->velocity.Length2D() < (pPlayer->m_pActiveItem->GetMaxSpeed() / 3.5))
					{
						DEBUG_CONSOLE("(dead center) no scope, ducking (vel: %f, max: %f )",
							pev->velocity.Length2D(),
							pPlayer->m_pActiveItem->GetMaxSpeed());

						return true;
					}
				}
			}

			return false;
		}

		if (IS_ON_GROUND(pev->flags)
			&& hasActiveZoom && pev->velocity.IsZero())
		{
			DEBUG_CONSOLE("(dead center) scoped");
			return true;
		}				
	}

	return false;
}