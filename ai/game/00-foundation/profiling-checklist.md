# Profiling Checklist

Measure on the reference PC (technical-plan §15) in a packaged Development build, never only in the editor (R-FND-07, D-15).

## 1. Build

```powershell
powershell -File Tools/package.ps1            # Development, output in Saved/Packaged/Windows/
```

## 2. Launch with tracing

```powershell
& "Saved\Packaged\Windows\CastleDefender.exe" <MapName> -trace=cpu,gpu,frame,memory,bookmark,log -statnamedevents -windowed -ResX=1920 -ResY=1080
```

- Leave `<MapName>` out to load the default map (`L_Boot`).
- Close other heavy apps. Plug in the laptop if there is one. Note the driver version.
- With Unreal Insights open first, the trace streams to it live. Otherwise it is written to `%LOCALAPPDATA%\UnrealEngine\Common\UnrealTrace\Store\`.

## 3. In-game console (`~`)

| Command | Read |
|---|---|
| `stat unit` | Frame, Game, Draw, GPU, RHIT times. The highest of Game, Draw and GPU is the bottleneck. |
| `stat fps` | Frame rate |
| `stat game` | Gameplay tick, CharacterMovement, AI |
| `stat ai`, `stat navigation` | AI and navmesh costs (P1+) |
| `stat gpu` | GPU passes |
| `game.debug.Combat 1` | Confirms debug CVars work in the Development build |
| `trace.bookmark <name>` | Marks a moment in the trace (e.g. `wave3_start`) |

## 4. Capture

1. Let the scene settle for 10 s.
2. Capture 30 s of the scenario under test. For gate checks, use the worst case: maximum allowed enemies on `L_SiegeSite_Proto` (spec-audit S8).
3. Quit with `quit` so the trace file closes cleanly.

## 5. Read in Unreal Insights

```powershell
& "<UE_ROOT>\Engine\Binaries\Win64\UnrealInsights.exe"
```

- Open the newest session in the Trace Store.
- Timing view: the frame graph, then the slowest frames. Check the Game thread vs GPU tracks.
- Memory Insights: allocations per tag (needs `memory` on the trace channel list).

## 6. Store results

- Traces stay **outside git** (they are large). Keep them in `D:\Unreal Project\Traces\<YYYY-MM-DD>_<build>_<scenario>.utrace` or another local folder.
- Record the result in the task or gate note: build/commit hash, map, scenario, average and worst frame time from `stat unit`, the bottleneck thread, and the trace file name.
