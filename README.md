<p align="center">
  <img src="welllovely.png" alt="WellLovely" width="200">
</p>

<h1 align="center">WellLovely</h1>
<p align="center">A free, open-source Minecraft 1.8.9 ghost client DLL written in C++.</p>

---

## Features (52 modules)

### Combat
- **Aimbot** — smooth aim assist with configurable FOV and speed
- **Autoclicker** — left/right CPS randomization
- **Reach** — extended hit range
- **Triggerbot** — auto-attack on crosshair target
- **Criticals** — automatic critical hits
- **W-Tap** — sprint reset for extra knockback
- **Velocity** — knockback reduction
- **NoHitDelay** — remove hit cooldown
- **KeepSprint** — maintain sprint on hit
- **SprintReset** — optimized sprint reset timing
- **AutoWeapon** — auto-switch to best weapon
- **AutoTool** — auto-switch to best tool for block

### Movement
- **Sprint** — auto sprint
- **Strafe** — air control / strafe movement
- **NoFall** — cancel fall damage
- **SafeWalk** — prevent walking off edges
- **Step** — auto step up blocks
- **NoSlowdown** — remove item use slowdown
- **InvWalk** — walk while inventory is open
- **NoJumpDelay** — remove jump cooldown
- **InstantStop** — instant movement stop
- **FastAccel** — faster acceleration
- **NullMove** — cancel movement in air
- **GameSpeed (Timer)** — game speed modifier
- **Clutch** — auto-clutch placement
- **BridgeAssist** — bridging assistance
- **BlockIn** — auto block-in

### Render
- **ESP** — player bounding boxes
- **Tracers** — lines to players
- **Nametags** — enhanced nametag rendering
- **Hitboxes** — player hitbox visualization
- **Fullbright** — max brightness
- **NoHurtCam** — remove screen shake on damage
- **ChestESP** — highlight chests
- **ItemESP** — highlight dropped items
- **Trajectories** — projectile path prediction
- **Indicators** — combat indicators
- **Pointers** — directional pointers to players
- **ModOverlay** — mod status overlay

### Player
- **NickHider** — hide your name
- **Friends** — friend list system (no attack)
- **Teams** — team detection (no attack)
- **AntiBot** — filter out bot entities
- **AntiDebuff** — auto remove negative effects
- **AutoSoup** — auto eat soup to heal
- **AutoPot** — auto throw healing potions
- **Refill** — auto refill hotbar from inventory
- **FastPlace** — faster block placement
- **FastMine** — faster block breaking
- **NoUseDelay** — remove item use delay

### Misc
- **Notifications** — on-screen notifications
- **Configs** — save/load configuration profiles

## Supported Clients

| Client | Mapping |
|--------|---------|
| Forge / Vanilla | MCP |
| Custom Client | SRG |

## Building

- **Visual Studio 2022+** with C++17 support
- **Platform:** x64 Release

## Injection

Use the [WellLovely Loader](https://github.com/pixuszin1337/WellLovely-Loader) to inject the DLL.

Toggle the menu in-game with **UP**.
