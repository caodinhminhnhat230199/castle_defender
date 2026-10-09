# Input Key Map (KBM)

Owner: Foundation (`T-FND-06`). This is the only key map; features add actions here before binding them (spec-audit S6). All bindings live in Input Mapping Contexts under `Content/CastleDefender/Core/Input/`; C++ binds actions, never keys (D-12, R-FND-06).

## Mapping contexts and modes

`AHeroPlayerController` owns the mode stack (D-19). The top mode decides which contexts are active; its `ModeInput` map is set on `BP_HeroPlayerController`. Owners change these rows when their mode lands.

| Mode | Active contexts (priority) | Cursor | Owner fills |
|---|---|---|---|
| Combat | `IMC_Combat` (0) | hidden | CMB |
| Wheel | `IMC_Combat` (0), `IMC_CommandWheel` (10) | hidden | SQD. Movement stays active while the wheel is open (R-CMB-36). |
| Build | `IMC_Build` (0) | shown | DEF |
| Focus | `IMC_TacticalFocus` (10) | hidden | TFM |
| Spirit | `IMC_CommanderSpirit` (0) | hidden | CSM |
| Modal | none | shown | UXF, PRK. Modal choices use Input Actions, not raw keys. |
| (always, non-Shipping) | `IMC_Debug` (1000) | | FND |

A higher priority wins when two active contexts map the same key. Combat keys are reused inside the wheel only when the wheel context maps them at a higher priority (spec-audit proposal, decided by SQD at P1).

## Combat (`IMC_Combat`, P0 set)

Defaults from NEW-CMB-07.

| Action | Value | Key |
|---|---|---|
| `IA_Move` | Axis2D | W / A / S / D |
| `IA_Look` | Axis2D | Mouse XY (Y negated) |
| `IA_Sprint` | Bool (hold) | Left Shift |
| `IA_LightAttack` | Bool | Left Mouse Button |
| `IA_HeavyAttack` | Bool | Right Mouse Button |
| `IA_Dodge` | Bool | Space |
| `IA_Block` | Bool (hold) | Left Ctrl |
| `IA_Parry` | Bool (press) | E |
| `IA_LockOn` | Bool | Middle Mouse Button |
| `IA_LockOnSwitch` | Axis1D | Mouse Wheel Up (+1/right) / Down (-1/left) |
| `IA_Interact` | Bool | F |

## Reserved keys (not bound yet)

Combat bindings must never use these keys (GDD §29.2, R-CMB-36, AC-CMB-14). The owner creates the action and binds it.

| Key | Reserved for | Owner |
|---|---|---|
| Q (hold) | `IA_CommandWheel` | SQD |
| 1 / 2 / 3 | Squad selection | SQD |
| Tab (hold) | `IA_TacticalFocus` | TFM |
| B | Build mode | DEF |
| Escape | Cancel / pause / back | UXF |
| F1-F12 | Debug only (`IMC_Debug`) | FND |

## Debug (`IMC_Debug`, not used in Shipping)

| Action | Key | Effect |
|---|---|---|
| `IA_DebugToggleBuild` | F5 | Push or pop `Build` mode with reason `Debug` (AC-FND-05) |
