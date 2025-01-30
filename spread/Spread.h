#pragma once

typedef struct S_WEAPON_CTRL
{
    float InAir;            // Mitigation while the player is in the air.
    float MovingStanding;
    float MovingDucking;
    float StandingStill;
    float DuckingStill;
    float Default;          // Default value used when a setting is not defined or the conditions are not met. Example: player is moving above MovingOnGround speed, or player is airborne but InAir is not set, etc.

} P_WEAPON_CTRL, *LP_WEAPON_CTRL;

class CSpread
{
public:
    void ServerActivate();
    static void SetWeapon();
    void AddWeapon(int WeaponIndex, float InAir, float MovingStanding, float MovingDucking, float StandingStill, float DuckingStill, float Default);
    float CalcSpread(CBaseEntity *pEntity, float vecSpread);
    cvar_t* m_deadCenterFirstShotCvar;

private:
    std::map<int, P_WEAPON_CTRL> m_Weapon = {};
};

extern CSpread gSpread;
