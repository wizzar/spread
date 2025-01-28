#pragma once

typedef struct S_WEAPON_CTRL
{
    float OnAir;        // Player is not in the ground
    float Velocity;     // Velocity length for moving check
    float Moving;       // When Player is moving
    float Ducking;      // When Ducking
    float Default;      // Normal value
} P_WEAPON_CTRL, *LP_WEAPON_CTRL;

class CSpread
{
public:
    void ServerActivate();
    static void SetWeapon();
    void AddWeapon(int WeaponIndex, float OnAir, float Moving, float Velocity, float Ducking, float Default);
    float CalcSpread(CBaseEntity *pEntity, float vecSpread);

private:
    std::map<int, P_WEAPON_CTRL> m_Weapon = {};
};

extern CSpread gSpread;