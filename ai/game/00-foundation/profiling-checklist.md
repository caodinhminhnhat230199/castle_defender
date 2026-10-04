# Profiling Checklist

Measure on the reference PC (technical-plan §15) in a packaged Development build, never only in the editor (R-FND-07, D-15).

UE 5.8.3 note (NEW-FND-2): enabling the `cpu` channel on the process command line reproducibly makes the rendered package exit 777003 after its log closes on the reference PC. Start the file with the other required channels, then enable `cpu` after startup. A 30-second capture made in this order exits 0 and completes CPU, GPU and memory analysis in Insights. This changes only capture timing; it does not change the renderer, driver, crash reporting or project configuration. The owner confirmed the current machine as the reference PC on 2026-10-04. See [readiness review](readiness-review-2026-10-04.md) for evidence.

## 1. Build

```powershell
Tools/package.bat                            # Development, output in Saved/Packaged/Windows/
```

## 2. Launch with tracing

```powershell
& "Saved\Packaged\Windows\CastleDefender\Binaries\Win64\CastleDefender.exe" <MapName> '-trace=gpu,frame,memory,bookmark,log' -statnamedevents -windowed -ResX=1920 -ResY=1080
```

- Leave `<MapName>` out to load the default map (`L_Boot`).
- Use the inner game executable shown above so PowerShell reports the game process exit code rather than the package bootstrapper's status.
- Close other heavy apps. Plug in the laptop if there is one. Note the driver version.
- With Unreal Insights open first, the trace streams to it live. Otherwise it is written to `%LOCALAPPDATA%\UnrealEngine\Common\UnrealTrace\Store\`.
- Do not add `cpu` to this launch command on the pinned 5.8.3 build. Enable it from the console after the map has settled. Runtime channel control is supported by the [UE 5.8 Insights reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-reference-in-unreal-engine-5).

## 3. In-game console (`~`)

| Command | Read |
|---|---|
| `stat unit` | Frame, Game, Draw, GPU, RHIT times. The highest of Game, Draw and GPU is the bottleneck. |
| `stat fps` | Frame rate |
| `stat game` | Gameplay tick, CharacterMovement, AI |
| `stat ai`, `stat navigation` | AI and navmesh costs (P1+) |
| `stat gpu` | GPU passes |
| `game.debug.Combat 1` | Confirms debug CVars work in the Development build |
| `trace.enable cpu` | Starts CPU timing after startup; required before the measured window on UE 5.8.3 |
| `trace.status` | Confirms `cpu`, `gpu`, `frame`, `memory`, `bookmark` and `log` are enabled |
| `trace.bookmark <name>` | Marks a moment in the trace (e.g. `wave3_start`) |

## 4. Capture

1. Let the scene settle for 10 s.
2. Run `trace.enable cpu`, then `trace.status`; confirm all required channels are enabled.
3. Capture 30 s of the scenario under test. Foundation uses `L_Boot` with the debug dummy (the Foundation harness only). Later gameplay gates use maximum allowed enemies on `L_SiegeSite_Proto` (spec-audit S8).
4. Add start/end bookmarks, then quit with `quit` so the game closes the trace cleanly.
5. Check the actual game process exit status and log. Do not infer a successful run from the bootstrap executable returning, a trace file existing, or Insights being able to read it.

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
