#include "precompiled.h"

CSpread gSpread;

#ifdef _MSC_VER
#define FORCEDINLINE __forceinline
#else
#define FORCEDINLINE __attribute__((always_inline))
#endif

#define HAS_ACTIVE_ZOOM(pPlayer) ( \
    ((pPlayer)->m_pActiveItem->m_iId == WEAPON_AWP || \
     (pPlayer)->m_pActiveItem->m_iId == WEAPON_SCOUT) ? \
        ((pPlayer)->m_bResumeZoom == true) : \
    ((pPlayer)->m_pActiveItem->m_iId == WEAPON_SG550 || \
     (pPlayer)->m_pActiveItem->m_iId == WEAPON_G3SG1 || \
     (pPlayer)->m_pActiveItem->m_iId == WEAPON_SG552 || \
     (pPlayer)->m_pActiveItem->m_iId == WEAPON_AUG) ? \
        ((pPlayer)->pev->fov != DEFAULT_FOV) : false)

#define IS_STANDING(pPlayer) (!((pPlayer)->pev->flags & FL_DUCKING))

#define IS_ON_GROUND(pPlayer) ((pPlayer)->pev->flags & FL_ONGROUND)

#define IS_AIRBORNE(pPlayer) (!((pPlayer)->pev->flags & FL_ONGROUND))

bool ShouldForceDeadCenterShot(CBasePlayer*, bool);

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
			LOG_CONSOLE(PLID, "ERROR OPENING SPREAD LOG FILE");
		}

		this->logToFile("Log Started");
	}	

	// TODO -> add situational counters and log on dettach.
#endif

	this->m_Weapon.clear();

	g_engfuncs.pfnAddServerCommand("spread_wpn", this->SetWeapon);

	const char* cvarName = "spread_deadCenterFirstShot";
	cvar_t* cvarPtr = CVAR_GET_POINTER(cvarName);
	if (!cvarPtr)
	{
		// Defines whether the first shot is always dead center when standing or ducking, but not moving.
		cvar_t CvarData;
		CvarData = { cvarName, "0", 0, 0.0f, NULL };
		CVAR_REGISTER(&CvarData);
		cvarPtr = CVAR_GET_POINTER(cvarName);

		if (cvarPtr) {
			this->m_deadCenterFirstShotCvar = cvarPtr;
		}
		else {
			gpMetaUtilFuncs->pfnLogError(PLID, "Failed to register \"%s\" cvar: an error occurred!", cvarName);
			gpMetaUtilFuncs->pfnLogConsole(PLID, "Failed to register \"%s\" cvar: an error occurred!", cvarName);
			this->m_deadCenterFirstShotCvar = NULL;
		}
	}
	else {
		gpMetaUtilFuncs->pfnLogMessage(PLID, "Failed to register \"%s\" cvar: already exists!", cvarName);
		gpMetaUtilFuncs->pfnLogConsole(PLID, "Failed to register \"%s\" cvar: already exists!", cvarName);
	}

	g_engfuncs.pfnServerCommand("exec addons/spread/spread.cfg\n");
}

void CSpread::SetWeapon()
{
	if (g_engfuncs.pfnCmd_Argc() >= 8)
	{
		std::string WeaponName = g_engfuncs.pfnCmd_Argv(1);

		if (WeaponName.find("weapon_") == std::string::npos)
		{
			WeaponName = "weapon_" + WeaponName;
		}

		if (!WeaponName.empty())
		{
			auto Slot = g_ReGameApi->GetWeaponSlot(WeaponName.c_str());

			if (Slot)
			{
				gSpread.AddWeapon(Slot->id,
					std::stof(g_engfuncs.pfnCmd_Argv(2)),  // InAir
					std::stof(g_engfuncs.pfnCmd_Argv(3)),  // MovingStanding
					std::stof(g_engfuncs.pfnCmd_Argv(4)),  // MovingDucking
					std::stof(g_engfuncs.pfnCmd_Argv(5)),  // StandingStill
					std::stof(g_engfuncs.pfnCmd_Argv(6)),  // DuckingStill
					std::stof(g_engfuncs.pfnCmd_Argv(7))); // Default
			}
		}
#ifdef DEBUG
		LOG_CONSOLE(PLID, "Spread control for \"%s\" set successfully", g_engfuncs.pfnCmd_Argv(1));
#else
		LOG_CONSOLE(PLID, "Spread config loaded successfully");
#endif
	}
	else
	{
		LOG_CONSOLE(PLID, "[%s] Usage: %s <weapon_name> <in_air> <moving_standing> <moving_ducking> <standing_still> <ducking_still> <default>. Example: \"%s ak47 -1.0 -1.0 0.8 0.75 0.70 -1.01\".", Plugin_info.logtag, g_engfuncs.pfnCmd_Argv(0), g_engfuncs.pfnCmd_Argv(0));
	}
}

void CSpread::AddWeapon(int WeaponIndex, float InAir, float MovingStanding, float MovingDucking, float StandingStill, float DuckingStill, float Default)
{
	this->m_Weapon[WeaponIndex].InAir = InAir;

	this->m_Weapon[WeaponIndex].MovingStanding = MovingStanding;

	this->m_Weapon[WeaponIndex].MovingDucking = MovingDucking;

	this->m_Weapon[WeaponIndex].StandingStill = StandingStill;

	this->m_Weapon[WeaponIndex].DuckingStill = DuckingStill;

	this->m_Weapon[WeaponIndex].Default = Default;
}

float CSpread::CalcSpread(CBaseEntity* pEntity, float vecSpread)
{
	CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEntity);
	if (!(pPlayer && pPlayer->m_pActiveItem))
	{
#ifdef DEBUG
		if (!pPlayer)
			LOG_CONSOLE(PLID, "[%s] !Player", __FUNCTION__);
		else if (!pPlayer->m_pActiveItem)
			LOG_CONSOLE(PLID, "[%s] !Player->m_pActiveItem", __FUNCTION__);
#endif
		return vecSpread;
	}
	
	// See if current player weapon is configured.
	auto theWeapon = this->m_Weapon.find(pPlayer->m_pActiveItem->m_iId);
	if (theWeapon == this->m_Weapon.end())
		return vecSpread;

	P_WEAPON_CTRL weaponCfg = theWeapon->second;

	// Player is in the air.
	if (IS_AIRBORNE(pPlayer))
	{
		if (weaponCfg.InAir >= 0.0f)
		{
#ifdef DEBUG
			LOG_CONSOLE(PLID, "[%s] (airborne) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.InAir);
#endif
			return vecSpread * weaponCfg.InAir;
		}
	}
	else
	{
		if (ShouldForceDeadCenterShot(pPlayer, this->m_deadCenterFirstShotCvar->value > 0.0f))
		{
#ifdef DEBUG
			LOG_CONSOLE(PLID, "[%s] (first shot dead center) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, 0.0f);
#endif
			return 0.0f;
		}
		

		// If the player has any *horizontal* movement at all...
		if (!pEntity->pev->velocity.IsZero())
		{
			// Player is standing.
			if (IS_STANDING(pPlayer))
			{
				if (weaponCfg.MovingStanding >= 0.0f)
				{
#ifdef DEBUG
					LOG_CONSOLE(PLID, "[%s] (moving, standing) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.MovingStanding);
#endif
					return vecSpread * weaponCfg.MovingStanding;
				}
			}
			else
			{
				// Player is ducking.
				if (weaponCfg.MovingDucking >= 0.0f)
				{
#ifdef DEBUG
					LOG_CONSOLE(PLID, "[%s] (moving, ducking) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.MovingDucking);
#endif
					return vecSpread * weaponCfg.MovingDucking;
				}
			}
		}
		else
		{
			// Player is still/stationary/motionless.

			// Player is standing.
			if (IS_STANDING(pPlayer))
			{
				if (weaponCfg.StandingStill >= 0.0f)
				{
#ifdef DEBUG
					LOG_CONSOLE(PLID, "[%s] (still, standing) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.StandingStill);
#endif
					return vecSpread * weaponCfg.StandingStill;
				}

			}
			else
			{
				// Player is ducking.
				if (weaponCfg.DuckingStill >= 0.0f)
				{
#ifdef DEBUG
					LOG_CONSOLE(PLID, "[%s] (still, ducking) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.DuckingStill);
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
#ifdef DEBUG
		LOG_CONSOLE(PLID, "[%s] (default) [OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * weaponCfg.Default);
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

	if (firstShotDeadCenter &&
		pPlayer->pev->punchangle.IsZero())
	{
		bool hasActiveZoom = false;
		bool zoomSet = false;

		switch (pPlayer->m_pActiveItem->m_iId)
		{
		case WEAPON_AWP:
		case WEAPON_SCOUT:

			hasActiveZoom = pPlayer->m_bResumeZoom;
			zoomSet = true;

		case WEAPON_SG550:
		case WEAPON_G3SG1:
		case WEAPON_SG552:
		case WEAPON_AUG:

			if (!zoomSet) // This check is necessary so that we don't overwrite the zoom in case of AWP or SCOUT.
				hasActiveZoom = pPlayer->pev->fov != DEFAULT_FOV;

			if (hasActiveZoom &&
				pPlayer->pev->velocity.IsZero())
			{
#ifdef DEBUG
				LOG_CONSOLE(PLID, "[%s] (scoped) Dead center first shot!", __FUNCTION__);
#endif
				return true;
			}

			return false;

		default:
			
			if (pPlayer->pev->velocity.Length2D() < (pPlayer->m_pActiveItem->GetMaxSpeed() / 2))
			{
#ifdef DEBUG
				LOG_CONSOLE(PLID, "[%s] Dead center first shot!", __FUNCTION__);
#endif
				return true;
			}

			return false;
		}
				
	}

	return false;
}