"""Rendered saved-content T-CMB-22 directional/actual-notify/perfect/counter acceptance.

Launch on L_CombatSandbox with -ExecutePythonScript. Captures screenshots in Saved.
The 0.25 actor dilation is only a visual inspection fixture, never a perfect-dodge reward.
"""
import json
import time
from pathlib import Path
import unreal

unreal.EditorPythonScripting.set_keep_python_script_alive(True)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
started = time.monotonic()
phase = 'setup'
finished = False
result = {'saved_playable_content': True, 'directions': [], 'frames': []}
cases = iter([('Forward', 0, 1), ('Backward', 0, -1), ('Left', -1, 0), ('Right', 1, 0)])
move = unreal.load_asset('/Game/CastleDefender/Core/Input/IA_Move')
dodge = unreal.load_asset('/Game/CastleDefender/Core/Input/IA_Dodge')
light = unreal.load_asset('/Game/CastleDefender/Core/Input/IA_LightAttack')
out = Path(unreal.Paths.project_saved_dir(), 'custom-dodge-pie.json')
cues = []

def on_feedback(tag, context):
    cues.append(str(unreal.GameplayTagLibrary.get_tag_name(tag)))

def hit():
    payload = unreal.CombatHit()
    payload.set_editor_property('damage', 20.0)
    payload.set_editor_property('poise_damage', 40.0)
    payload.set_editor_property('source_layer', unreal.CombatLayer.ENEMY)
    payload.set_editor_property('instigator', enemy)
    return unreal.CombatLibrary.deliver_hit(pawn, payload)

def ghosts():
    return pawn.get_components_by_class(unreal.PoseableMeshComponent)

def finish(error=None):
    global finished, finished_at
    finished = True
    finished_at = time.monotonic()
    result['success'] = error is None
    if error: result['error'] = str(error)
    out.write_text(json.dumps(result, indent=2), encoding='utf-8')
    unreal.log('CUSTOM_DODGE_PIE ' + json.dumps(result))
    if 'feedback' in globals():
        feedback.on_feedback_played.remove_callable(on_feedback)
    if 'pawn' in globals(): pawn.set_editor_property('custom_time_dilation', 1.0)
    levels.editor_request_end_play()

def inject(action, vector=unreal.Vector(1, 0, 0)):
    inputs.inject_input_vector_for_action(action, vector, [], [])

def tick(delta):
    global phase, pawn, pc, combat, inputs, home, case, phase_time, origin, saw_iframe, frame_index, enemy, perfect_sent, ghost_origin, ghost_started, did_expire, hero_elapsed, counter_started, before_health, ghost_shot, expiry_observed_game_time, feedback
    world = editor.get_game_world()
    if finished:
        if world is None and time.monotonic() - finished_at > 2.0:
            unreal.unregister_slate_post_tick_callback(handle)
            unreal.SystemLibrary.execute_console_command(editor.get_editor_world(), 'QUIT_EDITOR')
        return
    if time.monotonic() - started > 110:
        finish('Timed out in ' + phase)
        return
    if world is None: return
    try:
        if phase == 'setup':
            pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
            if not pawn: return
            pc = unreal.GameplayStatics.get_player_controller(world, 0)
            combat = pawn.get_combat_component()
            home = pawn.get_actor_location()
            for actor in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.SandboxEnemyRespawner): actor.set_enabled(False)
            enemies = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.EnemyCharacter)
            enemy = enemies[0]
            for actor in enemies:
                actor.get_brain_component().pause_decisions()
                actor.set_actor_enable_collision(False)
                actor.set_actor_hidden_in_game(True)
            inputs = next(o for o in unreal.ObjectIterator(unreal.EnhancedInputLocalPlayerSubsystem) if o.get_outer().get_name().startswith('LocalPlayer'))
            feedback = next(o for o in unreal.ObjectIterator(unreal.FeedbackSubsystem) if o.get_outer() == world)
            feedback.on_feedback_played.add_callable(on_feedback)
            pawn.get_component_by_class(unreal.SpringArmComponent).set_editor_property('target_arm_length', 320.0)
            unreal.SystemLibrary.execute_console_command(world, 't.MaxFPS 60', pc)
            unreal.SystemLibrary.execute_console_command(world, 'r.MotionBlurQuality 0', pc)
            # Warm the saved material's first shader permutation before judging a short-lived effect.
            tag = unreal.GameplayTag()
            tag.import_text('(TagName="Feedback.Combat.PerfectDodge")')
            context = unreal.FeedbackEventContext()
            context.set_editor_property('target', pawn)
            feedback.play(tag, context)
            phase_time = time.monotonic()
            phase = 'shader_warmup'
        elif phase == 'shader_warmup':
            if time.monotonic() - phase_time > 4.0:
                cues.clear()
                result['presentation_shader_warmup_fixture'] = True
                phase = 'next'
        elif phase == 'next':
            case = next(cases, None)
            if case is None:
                pawn.set_editor_property('custom_time_dilation', 1.0)
                pawn.reset_hero_state()
                pawn.set_actor_location(home, False, True)
                pc.set_control_rotation(unreal.Rotator(0, 0, 0))
                phase = 'counter_start'
                return
            pawn.reset_hero_state()
            pawn.set_actor_location(home, False, True)
            pawn.set_actor_rotation(unreal.Rotator(0, 0, 0), True)
            pc.set_control_rotation(unreal.Rotator(-8, 0, 0))
            pawn.set_editor_property('custom_time_dilation', 0.25)
            phase_time = time.monotonic()
            phase = 'movement'
        elif phase == 'movement':
            inject(move, unreal.Vector(case[1], case[2], 0))
            if time.monotonic() - phase_time > 0.10:
                origin = pawn.get_actor_location()
                inject(dodge)
                saw_iframe = perfect_sent = did_expire = ghost_shot = False
                expiry_observed_game_time = None
                frame_index = 0
                hero_elapsed = 0.0
                phase_time = time.monotonic()
                phase = 'dodge'
        elif phase == 'dodge':
            hero_elapsed += delta * 0.25
            if time.monotonic() - phase_time < 0.15: inject(move, unreal.Vector(case[1], case[2], 0))
            saw_iframe |= combat.is_in_invulnerable_window()
            # Real authored BeginNotify/hero clock, not a manually opened defensive window.
            if case[0] == 'Forward' and combat.get_perfect_dodge_time_remaining() > 0 and not perfect_sent:
                hp = pawn.get_health_component().get_current_health()
                assert hit() == unreal.CombatHitResult.EVADED
                assert hit() == unreal.CombatHitResult.EVADED
                assert pawn.get_health_component().get_current_health() == hp
                assert combat.has_counter_window()
                assert cues.count('Feedback.Combat.PerfectDodge') == 1
                assert len(ghosts()) == 1
                ghost_origin = ghosts()[0].get_world_location()
                result['ghost_at_success'] = {'material': ghosts()[0].get_material(0).get_name(),
                                              'opacity': ghosts()[0].get_material(0).get_scalar_parameter_value('GhostOpacity'),
                                              'visible': ghosts()[0].is_visible()}
                ghost_started = time.monotonic()
                perfect_sent = True
                unreal.SystemLibrary.execute_console_command(world, 'Shot', pc)
                result['perfect_actual_notify'] = True
            if perfect_sent and ghosts():
                assert (ghosts()[0].get_world_location() - ghost_origin).length() < 0.01
                if not ghost_shot and time.monotonic() - ghost_started > 0.12:
                    unreal.SystemLibrary.execute_console_command(world, 'Shot', pc)
                    ghost_shot = True
            if perfect_sent and not did_expire and time.monotonic() - ghost_started > 0.45:
                now_game = unreal.GameplayStatics.get_time_seconds(world)
                if expiry_observed_game_time is None:
                    expiry_observed_game_time = now_game
                elif now_game > expiry_observed_game_time:
                    # Screenshots can block the game thread; allow its next core-ticker update.
                    assert not ghosts(), 'Afterimage did not expire on the next engine update'
                    did_expire = True
                    result['ghost_real_time_expiry_under_actor_dilation'] = True
            thresholds = [0.08, 0.18, 0.28, 0.38, 0.48]
            if frame_index < len(thresholds) and hero_elapsed >= thresholds[frame_index]:
                points = {}
                for bone in ['pelvis', 'head', 'foot_l', 'foot_r', 'hand_l', 'hand_r']:
                    v = pawn.mesh.get_socket_location(bone)
                    points[bone] = [v.x, v.y, v.z]
                result['frames'].append({'direction': case[0], 'approx_hero_seconds': hero_elapsed, 'bones': points})
                unreal.SystemLibrary.execute_console_command(world, 'Shot', pc)
                frame_index += 1
            if hero_elapsed > 0.66 and combat.get_action_state() == unreal.HeroActionState.IDLE:
                actual = str(combat.get_last_dodge_direction())
                assert case[0].upper() in actual.upper(), (case, actual)
                travel = pawn.get_actor_location() - origin
                projected = travel.x * case[2] + travel.y * case[1]
                assert projected > 100, f'Incorrect travel: {travel}'
                assert saw_iframe and pawn.mesh.get_anim_instance().get_editor_property('foot_ik_alpha') > 0.99
                result['directions'].append({'requested': case[0], 'actual': actual, 'travel_cm': projected, 'saw_iframe': saw_iframe, 'idle_and_foot_ik_restored': True})
                phase = 'next'
        elif phase == 'counter_start':
            inject(move, unreal.Vector(0, 1, 0))
            inject(dodge)
            counter_started = False
            phase = 'counter_dodge'
        elif phase == 'counter_dodge':
            if combat.get_perfect_dodge_time_remaining() > 0:
                hit()
                assert combat.has_counter_window()
                enemy.set_actor_hidden_in_game(False)
                enemy.set_actor_enable_collision(True)
                enemy.set_actor_location(pawn.get_actor_location() + unreal.Vector(145, 0, 0), False, True)
                before_health = enemy.get_health_component().get_current_health()
                phase = 'counter_recovery'
        elif phase == 'counter_recovery':
            # Place the real target at melee range after root travel settles.
            enemy.set_actor_location(pawn.get_actor_location() + unreal.Vector(145, 0, 0), False, True)
            if combat.is_in_cancel_window() or combat.get_action_state() == unreal.HeroActionState.IDLE:
                inject(light)
                phase_time = time.monotonic()
                phase = 'counter_hit'
        elif phase == 'counter_hit':
            if combat.get_action_state() == unreal.HeroActionState.LIGHT_ATTACK: counter_started = True
            if enemy.get_health_component().get_current_health() < before_health:
                damage = before_health - enemy.get_health_component().get_current_health()
                assert abs(damage - 15.0) < 0.01, f'Counter damage {damage}'
                assert counter_started and combat.get_counter_time_remaining() == 0
                result['counter_actual_input_first_payload_damage'] = damage
                result['perfect_cue_count'] = cues.count('Feedback.Combat.PerfectDodge')
                finish()
            elif time.monotonic() - phase_time > 2.0:
                raise AssertionError('Real counter swing did not reach target')
    except Exception as exc:
        finish(exc)

handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_request_begin_play()
