<h3 align="center">ReGameDLL_CS Spread Control Plugin</h3>

<p align="center">Control the FireBullets3 function vecSpread parameter</p>

<p align="center">
    <a href="https://github.com/SmileYzn/spread/issues"><img alt="GitHub Issues" src="https://img.shields.io/github/issues-raw/smileyzn/spread?style=flat-square"></a>
    <a href="https://github.com/SmileYzn/spread/actions"><img alt="GitHub Workflow Status" src="https://img.shields.io/github/actions/workflow/status/SmileYzn/spread/build.yml?branch=main&label=Build&style=flat-square"></a>
</p>

<hr>

<p align="center">This plugin control the vecSpread variable of FireBullets3 on <a href="https://github.com/rehlds/ReGameDLL_CS/blob/master/regamedll/dlls/cbase.cpp#L1268" target="_blank">cbase.cpp</a></p>
<p align="center">The plugin multiply the vecSpread with specified factor on some situations.</p>

<br>

<p align="center">The plugin optimizes shot spread.</p>
<p align="center">It does not affect recoil, nor does it change the default behavior of aim, it just adjusts the insane pattern of shots in the game.</p>
<p align="center">You can use this plugin as replacement for my <a href="https://github.com/smileYzn/accuracyfix" target="_blank">Accuracy Fix Plugin</a>.</p>
<p>Since it is more optmized than accuracy fix plugin.</p>

<hr>

<details>
  <summary align="center">Spread Control Settings (Click to expand)</summary><br>
    
```
// Spread Control Plugin
// This plugin control the vecSpread variable of FireBullets3
// https://github.com/rehlds/ReGameDLL_CS/blob/master/regamedll/dlls/cbase.cpp#L1268
//
// The plugin multiply the vecSpread with specified factor on some situations.
//
// 1. The default factor is for any weapon is 1.0, any value bellow 0.0 will be ignored
// 2. To not use some situation variable (Maintain as original), just pass -1.0 (minus -1.0) the plugin will ignore it
// 3. To completly remove spread of weapon shots, pass 0.0 to an variable
// 4. The weapon name must not include weapon_prefixes (Ie. ak47)
//
// weapon	Name of weapon WITHOUT prefix (Example: ak47)
// on_air	Multiply factor when player is not on ground
// speed	Max player velocity to dettermine if player is moving
// moving	Multiply factor while player is moving
// ducking	Factor while player is crouching
// default	Multiply factor when player is not doing any actions above
//
// Usage:	spread_wpn <weapon> <on_air> <speed> <moving> <ducking> <default>
//
//	COMMAND		WEAPON		AIR		SPEED		MOVING		DEFAULT
//
//	spread_wpn	galil		1.0		1.0		1.0		1.0
//	spread_wpn	ak47		1.0		1.0		1.0		1.0
//	spread_wpn	scout		1.0		1.0		1.0		1.0
//	spread_wpn	sg55		1.0		1.0		1.0		1.0
//	spread_wpn	awp		1.0		1.0		1.0		1.0
//	spread_wpn	g3sg1		1.0		1.0		1.0		1.0
//	spread_wpn	famas		1.0		1.0		1.0		1.0
//	spread_wpn	m4a1		1.0		1.0		1.0		1.0	
//	spread_wpn	aug		1.0		1.0		1.0		1.0
//	spread_wpn	sg550		1.0		1.0		1.0		1.0
//	spread_wpn	glock		1.0		1.0		1.0		1.0
//	spread_wpn	usp		1.0		1.0		1.0		1.0	
//	spread_wpn	p228		1.0		1.0		1.0		1.0
//	spread_wpn	deagle		1.0		1.0		1.0		1.0
//	spread_wpn	elites		1.0		1.0		1.0		1.0
//	spread_wpn	fn57		1.0		1.0		1.0		1.0
//	spread_wpn	mac10		1.0		1.0		1.0		1.0
//	spread_wpn	tmp		1.0		1.0		1.0		1.0
//	spread_wpn	mp5		1.0		1.0		1.0		1.0
//	spread_wpn	ump45		1.0		1.0		1.0		1.0
//	spread_wpn	p90		1.0		1.0		1.0		1.0
//	spread_wpn	m249		1.0		1.0		1.0		1.0

```
</details>
