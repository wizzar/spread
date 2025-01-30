#pragma once

class CSpread
{
public:
    void ServerActivate();
    float GetSpread(CBasePlayer *Player, float vecSpread);
    
    cvar_t* m_Spread_Divisor = nullptr;
};

extern CSpread gSpread;
