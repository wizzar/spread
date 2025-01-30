#include "precompiled.h"

CSpread gSpread;

void CSpread::ServerActivate()
{
    static cvar_t Spread_Divisor = { "sc_divisor", strdup("2.0"), FCVAR_SERVER | FCVAR_PROTECTED, 2.0f, NULL };

	g_engfuncs.pfnCVarRegister(&Spread_Divisor);

	this->m_Spread_Divisor = g_engfuncs.pfnCVarGetPointer(Spread_Divisor.name);
}

float CSpread::GetSpread(CBasePlayer *Player, float vecSpread)
{
    if (this->m_Spread_Divisor)
    {
        if (this->m_Spread_Divisor->value > 0.0f)
        {
            if (Player->m_pActiveItem)
            {
                if (Player->pev->flags & FL_ONGROUND)
                {
                    if (Player->pev->punchangle.IsZero())
                    {
                        if (Player->pev->velocity.Length2D() < (Player->m_pActiveItem->GetMaxSpeed() / this->m_Spread_Divisor->value))
                        {
                            vecSpread = 0.0f;
                        }
                    }
                }
            }
        }
    }

    return vecSpread;
}
