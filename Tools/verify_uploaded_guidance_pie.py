"""Rendered L_CombatSandbox check for T-SYN-02 and existing combat/dodge/animation integration.

Run with UnrealEditor -ExecutePythonScript=... on L_CombatSandbox (without NullRHI).
Uses saved playable content and Enhanced Input; all scene changes stay in the PIE world.
"""
import json
import time
from pathlib import Path
import unreal

unreal.EditorPythonScripting.set_keep_python_script_alive(True)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
started = time.monotonic()
phase = "setup"
finished = False
result = {"saved_playable_content": True, "transient_animation_fixture": False, "dodge_caps": []}
output = Path(unreal.Paths.project_saved_dir(), "uploaded-guidance-pie.json")
tag = unreal.GameplayTag()
tag.import_text('(TagName="State.Combat.ArmorBroken")')
caps = iter([30, 60, 120])
feedback_events = []
pc = pawn = enemy = inputs = combat = None


def finish(error=None):
    global finished
    finished = True
    result["success"] = error is None
    if error:
        result["error"] = str(error)
    output.write_text(json.dumps(result, indent=2), encoding="utf-8")
    unreal.log("UPLOADED_GUIDANCE_PIE " + json.dumps(result))
    levels.editor_request_end_play()


def on_feedback(played_tag, context):
    feedback_events.append(str(unreal.GameplayTagLibrary.get_tag_name(played_tag)))


def hit(target, layer, damage):
    payload = unreal.CombatHit()
    payload.set_editor_property("damage", damage)
    payload.set_editor_property("source_layer", layer)
    return unreal.CombatLibrary.deliver_hit(target, payload)


def inject(name):
    action = unreal.load_asset("/Game/CastleDefender/Core/Input/" + name)
    inputs.inject_input_vector_for_action(action, unreal.Vector(1, 0, 0), [], [])


def tick(delta):
    global phase, pc, pawn, enemy, inputs, combat, cap, heavy_before, hero_home, heavy_started_at, aim_started
    world = editor.get_game_world()
    if finished:
        if world is None:
            unreal.unregister_slate_post_tick_callback(handle)
            unreal.SystemLibrary.execute_console_command(editor.get_editor_world(), "QUIT_EDITOR")
        return
    if time.monotonic() - started > 100:
        finish("Timed out in " + phase)
        return
    if world is None:
        return
    try:
        if phase == "setup":
            pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
            if not pawn:
                return
            pc = unreal.GameplayStatics.get_player_controller(world, 0)
            combat = pawn.get_combat_component()
            hero_home = pawn.get_actor_location()
            pc.set_control_rotation(unreal.Rotator(0, 0, 0))
            pawn.set_actor_rotation(unreal.Rotator(0, 0, 0), True)
            for respawner in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.SandboxEnemyRespawner):
                respawner.set_enabled(False)
            for other in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.EnemyCharacter):
                other.get_brain_component().pause_decisions()
                other.set_actor_enable_collision(False)
                other.set_actor_hidden_in_game(True)
            inputs = next(o for o in unreal.ObjectIterator(unreal.EnhancedInputLocalPlayerSubsystem)
                          if o.get_outer().get_name().startswith("LocalPlayer"))
            feedback = next(o for o in unreal.ObjectIterator(unreal.FeedbackSubsystem) if o.get_outer() == world)
            feedback.on_feedback_played.add_callable(on_feedback)
            unreal.SystemLibrary.execute_console_command(world, "SpawnEnemy DA_Enemy_ArmorTest 1", pc)
            enemy = next(o for o in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.EnemyCharacter)
                         if o.get_archetype().get_name() == "DA_Enemy_ArmorTest")
            enemy.get_brain_component().pause_decisions()
            enemy.set_actor_location(hero_home + unreal.Vector(145, 0, 0), False, True)
            enemy.set_actor_rotation(unreal.Rotator(0, 180, 0), True)
            health = enemy.get_health_component()
            before = health.get_current_health()
            hit(enemy, unreal.CombatLayer.ARMY, 20)
            result["army_before_heavy"] = before - health.get_current_health()
            assert abs(result["army_before_heavy"] - 10) < 0.01
            unreal.SystemLibrary.execute_console_command(world, "game.debug.CombatStates 1", pc)
            unreal.SystemLibrary.execute_console_command(world, "game.debug.CombatTrace 1", pc)
            heavy_before = health.get_current_health()
            inject("IA_HeavyAttack")
            phase = "heavy_start"
        elif phase == "heavy_start":
            if combat.get_action_state() == unreal.HeroActionState.HEAVY_ATTACK:
                heavy_started_at = unreal.GameplayStatics.get_time_seconds(world)
                result["heavy_from_enhanced_input"] = True
                phase = "heavy_hit"
        elif phase == "heavy_hit":
            states = enemy.get_combat_state_component()
            if states.has_state(tag):
                result["heavy_damage"] = heavy_before - enemy.get_health_component().get_current_health()
                expected = pawn.get_hero_class_definition().get_editor_property("heavy").get_editor_property("damage") * 0.5
                assert abs(result["heavy_damage"] - expected) < 0.01, "Applying Heavy used its own Armor Broken bonus"
                result["duration_at_apply"] = states.get_state_remaining(tag)
                assert 5.8 < result["duration_at_apply"] <= 6.01
                before = enemy.get_health_component().get_current_health()
                hit(enemy, unreal.CombatLayer.ARMY, 20)
                result["army_during_break"] = before - enemy.get_health_component().get_current_health()
                assert abs(result["army_during_break"] - 17.5) < 0.01
                unreal.SystemLibrary.execute_console_command(world, "Shot showui", pc)
                phase = "expiry"
        elif phase == "expiry":
            if not enemy.get_combat_state_component().has_state(tag):
                before = enemy.get_health_component().get_current_health()
                hit(enemy, unreal.CombatLayer.ARMY, 20)
                result["army_after_expiry"] = before - enemy.get_health_component().get_current_health()
                assert abs(result["army_after_expiry"] - 10) < 0.01
                result["armor_applied_events"] = feedback_events.count("Feedback.State.ArmorBroken.Applied")
                assert result["armor_applied_events"] == 1
                enemy.set_actor_location(pawn.get_actor_location() + unreal.Vector(400, 0, 0), False, True)
                aim_started = time.monotonic()
                phase = "cheat_aim"
        elif phase == "cheat_aim":
            # Allow the spring-arm camera to converge on the target before checking the real crosshair cheat.
            pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(
                pc.player_camera_manager.get_camera_location(), enemy.get_actor_location()))
            if time.monotonic() - aim_started > 0.5:
                before = enemy.get_health_component().get_current_health()
                unreal.SystemLibrary.execute_console_command(world, "DebugHitTarget 20 0", pc)
                result["crosshair_cheat_damage"] = before - enemy.get_health_component().get_current_health()
                assert abs(result["crosshair_cheat_damage"] - 10) < 0.01, "DebugHitTarget did not hit the aimed armored enemy"
                enemy.set_actor_enable_collision(False)
                pc.set_control_rotation(unreal.Rotator(0, 0, 0))
                phase = "dodge_next"
        elif phase == "dodge_next":
            cap = next(caps, None)
            if cap is None:
                assert isinstance(pawn.mesh.get_anim_instance(), unreal.HeroAnimInstance)
                result["anim_instance"] = pawn.mesh.get_anim_instance().get_class().get_name()
                result["foot_ik_restored"] = pawn.mesh.get_anim_instance().get_editor_property("foot_ik_alpha")
                assert result["foot_ik_restored"] > 0.99
                finish()
                return
            pawn.reset_hero_state()
            pawn.set_actor_location(hero_home, False, True)
            unreal.SystemLibrary.execute_console_command(world, f"t.MaxFPS {cap}", pc)
            inject("IA_Dodge")
            phase = "iframe"
        elif phase == "iframe":
            if combat.is_in_invulnerable_window():
                before = pawn.get_health_component().get_current_health()
                outcome = hit(pawn, unreal.CombatLayer.ENEMY, 20)
                assert outcome == unreal.CombatHitResult.EVADED
                assert pawn.get_health_component().get_current_health() == before
                result["dodge_caps"].append({"cap": cap, "evaded": True, "observed_slate_delta": delta})
                phase = "outside_iframe"
        elif phase == "outside_iframe":
            if not combat.is_in_invulnerable_window():
                before = pawn.get_health_component().get_current_health()
                hit(pawn, unreal.CombatLayer.ENEMY, 20)
                assert abs(before - pawn.get_health_component().get_current_health() - 20) < 0.01
                result["dodge_caps"][-1]["damage_after_iframes"] = 20
                phase = "dodge_exit"
        elif phase == "dodge_exit":
            if combat.get_action_state() == unreal.HeroActionState.IDLE and pawn.mesh.get_anim_instance().get_editor_property("foot_ik_alpha") > 0.99:
                phase = "dodge_next"
    except Exception as exc:
        finish(exc)


handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_request_begin_play()
