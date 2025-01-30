#include "precompiled.h"

CSpread gSpread;

void CSpread::ServerActivate()
{
	this->m_Weapon.clear();

	g_engfuncs.pfnAddServerCommand(strdup("spread_wpn"), this->SetWeapon);

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

	g_engfuncs.pfnServerCommand(strdup("exec addons/spread/spread.cfg\n"));
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
	}
	else
	{
		gpMetaUtilFuncs->pfnLogConsole(PLID, "[%s] Usage: %s <weapon_name> <in_air> <moving_standing> <moving_ducking> <standing_still> <ducking_still> <default>. Example: \"%s ak47 -1.0 -1.0 0.8 0.75 0.70 -1.01\".", Plugin_info.logtag, g_engfuncs.pfnCmd_Argv(0), g_engfuncs.pfnCmd_Argv(0));
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
	if (!pEntity)
		return vecSpread;

	CBasePlayer* Player = static_cast<CBasePlayer*>(pEntity);
	if (!(Player && Player->m_pActiveItem))
		return vecSpread;

	// See if current player weapon is configured.
	auto theWeapon = this->m_Weapon.find(Player->m_pActiveItem->m_iId);
	if (theWeapon == this->m_Weapon.end())
		return vecSpread;

	P_WEAPON_CTRL Control = theWeapon->second;

	// Player is in the air.
	if (!(pEntity->pev->flags & FL_ONGROUND))
	{
		if (Control.InAir >= 0.0f)
		{
#ifdef DEBUG
			LOG_CONSOLE(PLID, "[%s][OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * Control.InAir);
#endif
			return vecSpread * Control.InAir;
		}
	}
	else
	{
		// If "first shot should be dead center"...
		if (this->m_deadCenterFirstShotCvar->value > 0.0f
			&& Player->pev->punchangle.IsZero()
			&& Player->pev->velocity.Length2D() < (Player->m_pActiveItem->GetMaxSpeed() / 2))
		{
#ifdef DEBUG
			LOG_CONSOLE(PLID, "[%s][OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, 0.0f);
#endif
			return 0.0f;
		}
		else {

			// If the player has any *horizontal* movement at all...
			if (!pEntity->pev->velocity.IsZero())
			{
				// Player is standing.
				if (!(pEntity->pev->flags & FL_DUCKING))
				{
					if (Control.MovingStanding >= 0.0f)
					{
#ifdef DEBUG
						LOG_CONSOLE(PLID, "[%s][OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * Control.MovingStanding);
#endif
						return vecSpread * Control.MovingStanding;
					}
				}
				else
				{
					// Player is ducking.
					if (Control.MovingDucking >= 0.0f)
					{
#ifdef DEBUG
						LOG_CONSOLE(PLID, "[%s][OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * Control.MovingDucking);
#endif
						return vecSpread * Control.MovingDucking;
					}
				}
			}
			else
			{
				// Player is still/stationary/motionless.

				// Player is standing.
				if (!(pEntity->pev->flags & FL_DUCKING))
				{
					if (Control.StandingStill >= 0.0f)
					{
#ifdef DEBUG
						LOG_CONSOLE(PLID, "[%s][OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * Control.StandingStill);
#endif
						return vecSpread * Control.StandingStill;
					}

				}
				else
				{
					// Player is ducking.
					if (Control.DuckingStill >= 0.0f)
					{
#ifdef DEBUG
						LOG_CONSOLE(PLID, "[%s][OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * Control.DuckingStill);
#endif
						return vecSpread * Control.DuckingStill;
					}
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
	if (Control.Default >= 0.0f)
	{
#ifdef DEBUG
		LOG_CONSOLE(PLID, "[%s][OLD SP: %f] [NEW SP: %f]", __FUNCTION__, vecSpread, vecSpread * Control.Default);
#endif
		return vecSpread * Control.Default;
	}

	return vecSpread;
}
