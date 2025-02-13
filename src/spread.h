#pragma once

#include "wrapper_meta_api.h"
#include <regamedll_api.h>
#include "dbg_logger.h"

typedef struct tagWEAPON_SPREAD_CFG
{
    bool  IsValid;

    float InAir;            // Mitigation while the player is in the air.
    float MovingStanding;
    float MovingDucking;
    float StandingStill;
    float DuckingStill;
    float Default;          // Default value used when a setting is not defined or the conditions are not met. Example: player is moving above MovingOnGround speed, or player is airborne but InAir is not set, etc.

} WEAPON_SPREAD_CFG;

class CSpread
{
public:
	static void SetWeapon();
	void AddWeapon(int WeaponIndex, float InAir, float MovingStanding, float MovingDucking, float StandingStill, float DuckingStill, float Default);
	float CalcSpread(CBaseEntity* pEntity, float vecSpread);
	bool RegisterCvar();

#ifdef DO_DEBUG
	DbgLogger logger;

	CSpread()
		: logger("cstrike/addons/spread/spread_log.txt")
	{
		if (!this->logger.IsOpen())
		{
			LOG_ERROR(PLID, "ERROR OPENING SPREAD LOG FILE");
			LOG_CONSOLE(PLID, "ERROR OPENING SPREAD LOG FILE");
		}
	}
#endif

private:
	WEAPON_SPREAD_CFG m_rgWeaponsCfg[MAX_WEAPONS] = {};
	cvar_t* m_pDeadCenterFirstShotCvar = NULL;
};

extern CSpread gSpread;
