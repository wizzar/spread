#include "precompiled.h"

CSpread gSpread;

void CSpread::ServerActivate()
{
    this->m_Weapon.clear();

    g_engfuncs.pfnAddServerCommand(strdup("spread_weapon_set"), this->SetWeapon);

    g_engfuncs.pfnServerCommand(strdup("exec addons/spread/spread.cfg\n"));
}

void CSpread::SetWeapon()
{
    if (CMD_ARGC() >= 7)
    {
        auto Slot = g_ReGameApi->GetWeaponSlot(CMD_ARGV(1));

        if (Slot)
        {
            gSpread.AddWeapon(Slot->id, std::stof(CMD_ARGV(2)), std::stof(CMD_ARGV(3)), std::stof(CMD_ARGV(4)), std::stof(CMD_ARGV(5)), std::stof(CMD_ARGV(6)));
        }  
    }
    else
    {
        gpMetaUtilFuncs->pfnLogConsole(PLID, "[%s] Usage: %s <weapon_name> <on_air> <velocity> <moving> <ducking> <default>", Plugin_info.logtag, g_engfuncs.pfnCmd_Argv(0));
    }
}

void CSpread::AddWeapon(int WeaponIndex, float OnAir, float Velocity, float Moving, float Ducking, float Default)
{
    this->m_Weapon[WeaponIndex].OnAir = OnAir;

    this->m_Weapon[WeaponIndex].Velocity = Velocity;
    
    this->m_Weapon[WeaponIndex].Moving = Moving;

    this->m_Weapon[WeaponIndex].Ducking = Ducking;

    this->m_Weapon[WeaponIndex].Default = Default;
}

float CSpread::CalcSpread(CBaseEntity *pEntity, float vecSpread)
{
    if (pEntity)
    {
        CBasePlayer* Player = static_cast<CBasePlayer*>(pEntity);

        if (Player)
        {
            if (Player->m_pActiveItem)
            {
                if (this->m_Weapon.find(Player->m_pActiveItem->m_iId) != this->m_Weapon.end())
                {
                    P_WEAPON_CTRL Control = this->m_Weapon.at(Player->m_pActiveItem->m_iId);

                    if (!(pEntity->pev->flags & FL_ONGROUND))
                    {
                        if (Control.OnAir >= 0.0f)
                        {
                            vecSpread = (vecSpread * Control.OnAir);
                        }
                    }
                    else if (pEntity->pev->velocity.Length2D() > Control.Velocity)
                    {
                        if (Control.Moving >= 0.0f)
                        {
                            vecSpread = (vecSpread * Control.Moving);
                        }
                    }
                    else if (pEntity->pev->flags & FL_DUCKING)
                    {
                        if (Control.Ducking >= 0.0f)
                        {
                            vecSpread = (vecSpread * Control.Ducking);
                        }
                    }
                    else
                    {
                        if (Control.Default >= 0.0f)
                        {
                            vecSpread = (vecSpread * Control.Default);
                        }
                    }
                }
            }
        }
    }

    return vecSpread;
}
