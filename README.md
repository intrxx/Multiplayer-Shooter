# Multiplayer Arena Shooter

The idea behind this project was to create a strong platform for learning and implementing networking in Unreal Engine 5 using c++. 

The result is a third person arena shooter with all main weapon archetypes like sniper, shotgun, full auto weapon etc. 
and features like: scoreboard, inventory(WIP), grenades, ammo/weapon pickups, powerups, complete UI and HUD and some crosshair customization.

This is wraped in 3 main gamemodes: 
- Free For All Deathmatch,
- Team's Deathmatch,
- CSGO like teams match with bomb (WIP).

When it comes to networking this project is utilizing:
- **Client Side Prediction** - for example: updating the hud, reloading, aiming.
- **Server Side Rewind** - for every type of weapon (excluding explosive weapons like Rocket Launcher although projectile predicting is also done with Assault Rifle).
