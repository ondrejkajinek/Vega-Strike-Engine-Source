/*
 * configuration.cpp
 *
 * Copyright (C) 2021-2022 Daniel Horn, Roy Falk, ministerofinformation,
 * David Wales, Stephen G. Tuggy, and other Vega Strike contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "configuration.h"
#include "game_config.h"
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

using vega_config::GetGameConfig;

Configuration::Configuration() {
    //logging.verbose_debug = GetGameConfig().GetBool("data.verbose_debug", false);
}

/* Override the default value(provided by constructor) with the value from the user specified configuration file, if any.
* Behavior is undefined and erroneous if no default is provided in the constructor!
* Descriptions/invariants for each variable are/will be provided in the header file and are not repeated here, except where
* behavior/range bounding is complicated/necessary-for-stability abd additional validation code is required for checking/adjusting user inputs
*/
void Configuration::OverrideDefaultsWithUserConfiguration() {
    general_config_.pitch = GetGameConfig().GetFloat("general.pitch", general_config_.pitch);
    general_config_.yaw = GetGameConfig().GetFloat("general.yaw", general_config_.yaw);
    general_config_.roll = GetGameConfig().GetFloat("general.roll", general_config_.roll);
    general_config_.force_anonymous_mission_names = GetGameConfig().GetBool("general.force_anonymous_mission_names", general_config_.force_anonymous_mission_names);
    general_config_.write_savegame_on_exit = GetGameConfig().GetBool("general.write_savegame_on_exit", general_config_.write_savegame_on_exit);
    general_config_.times_to_show_help_screen = GetGameConfig().GetInt32("general.times_to_show_help_screen", general_config_.times_to_show_help_screen);
    general_config_.remember_savegame = GetGameConfig().GetBool("general.remember_savegame", general_config_.remember_savegame);
    general_config_.new_game_save_name = GetGameConfig().GetString("general.new_game_save_name", general_config_.new_game_save_name);
    // FIXME: This seems like a security risk, to have a buffer size be configurable externally -- stephengtuggy 2022-05-28
    general_config_.quick_savegame_summaries_buffer = GetGameConfig().GetSizeT("general.quick_savegame_summaries_buffer", general_config_.quick_savegame_summaries_buffer);
    general_config_.empty_mission = GetGameConfig().GetString("general.empty_mission", general_config_.empty_mission);
    general_config_.custom_python = GetGameConfig().GetString("general.custompython", general_config_.custom_python);
    general_config_.quick_savegame_summaries = GetGameConfig().GetBool("general.quick_savegame_summaries", general_config_.quick_savegame_summaries);
    general_config_.garbage_collect_frequency = GetGameConfig().GetInt32("general.garbagecollectfrequency", general_config_.garbage_collect_frequency);
    general_config_.num_old_systems = GetGameConfig().GetUInt32("general.numoldsystems", general_config_.num_old_systems);
    general_config_.delete_old_systems = GetGameConfig().GetBool("general.deleteoldsystems", general_config_.delete_old_systems);
    // vsdebug moved to logging section -- stephengtuggy 2022-05-28
    general_config_.while_loading_star_system = GetGameConfig().GetBool("general.while_loading_starsystem", general_config_.while_loading_star_system);

    data_config_.master_part_list = GetGameConfig().GetString("data.master_part_list", data_config_.master_part_list);
    data_config_.using_templates = GetGameConfig().GetBool("data.usingtemplates", data_config_.using_templates);

    ai.always_obedient                                  = GetGameConfig().GetBool("AI.always_obedient", ai.always_obedient);
    ai.assist_friend_in_need                            = GetGameConfig().GetBool("AI.assist_friend_in_need", ai.assist_friend_in_need);
    ai.ease_to_anger                                    = GetGameConfig().GetFloat("AI.EaseToAnger", ai.ease_to_anger);
    ai.ease_to_appease                                  = GetGameConfig().GetFloat("AI.EaseToAppease", ai.ease_to_appease);
    ai.hull_damage_anger                                = GetGameConfig().GetInt32("AI.HullDamageAnger", ai.hull_damage_anger);
    ai.hull_percent_for_comm                            = GetGameConfig().GetFloat("AI.HullPercentForComm", ai.hull_percent_for_comm);
    ai.lowest_negative_comm_choice                      = GetGameConfig().GetFloat("AI.LowestNegativeCommChoice", ai.lowest_negative_comm_choice);
    ai.lowest_positive_comm_choice                      = GetGameConfig().GetFloat("AI.LowestPositiveCommChoice", ai.lowest_positive_comm_choice);
    ai.mood_swing_level                                 = GetGameConfig().GetFloat("AI.MoodSwingLevel", ai.mood_swing_level);
    ai.random_response_range                            = GetGameConfig().GetFloat("AI.RandomResponseRange", ai.random_response_range);
    ai.shield_damage_anger                              = GetGameConfig().GetInt32("AI.ShieldDamageAnger", ai.shield_damage_anger);
    ai.jump_without_energy                              = GetGameConfig().GetBool("AI.jump_without_energy", ai.jump_without_energy);
    ai.friend_factor                                    = -GetGameConfig().GetFloat("AI.friend_factor", ai.friend_factor);
    ai.kill_factor                                      = -GetGameConfig().GetFloat("AI.kill_factor", ai.kill_factor);
    ai.escort_distance                                  = GetGameConfig().GetFloat("AI.Targetting.EscortDistance", ai.escort_distance);
    ai.turn_leader_distance                             = GetGameConfig().GetFloat("AI.Targetting.TurnLeaderDist", ai.turn_leader_distance);
    ai.time_to_recommand_wing                           = GetGameConfig().GetFloat("AI.Targetting.TargetCommandierTime", ai.time_to_recommand_wing);
    ai.min_relationship                                 = GetGameConfig().GetDouble("AI.min_relationship", ai.min_relationship);

    audio_config_.every_other_mount                     = GetGameConfig().GetBool("audio.every_other_mount", audio_config_.every_other_mount);
    audio_config_.shuffle_songs.clear_history_on_list_change = GetGameConfig().GetBool("audio.shuffle_songs.clear_history_on_list_change", audio_config_.shuffle_songs.clear_history_on_list_change);

    // collision_hacks substruct
    collision_hacks.collision_hack_distance             = GetGameConfig().GetFloat("physics.collision_avoidance_hack_distance", collision_hacks.collision_hack_distance);
    collision_hacks.collision_damage_to_ai              = GetGameConfig().GetBool("physics.collisionDamageToAI", collision_hacks.collision_damage_to_ai);
    collision_hacks.crash_dock_hangar                   = GetGameConfig().GetBool("physics.only_hangar_collision_docks", collision_hacks.crash_dock_hangar);
    collision_hacks.crash_dock_unit                     = GetGameConfig().GetBool("physics.unit_collision_docks", collision_hacks.crash_dock_unit);
    // (STYLE WARNING) front collision_hack_angle appears to expect to equal the cosine of a radian value, but default implies that it reads degrees! Value not specified directly, breaks pattern, should consider normalizing behavior with other config vars
    collision_hacks.front_collision_hack_angle = std::cos(
            M_PI * GetGameConfig().GetDouble("physics.front_collision_avoidance_hack_angle", 40.0)
                    / 180.0); // uses default value - must be changed in tandem with constructor!
    collision_hacks.front_collision_hack_distance = GetGameConfig().GetFloat("physics.front_collision_avoidance_hack_distance",
            collision_hacks.front_collision_hack_distance);
    collision_hacks.cargo_deals_collide_damage = GetGameConfig().GetBool("physics.cargo_deals_collide_damage",
            collision_hacks.cargo_deals_collide_damage);

    // computer substruct
    computer_config_.default_lock_cone = GetGameConfig().GetFloat("physics.lock_cone", computer_config_.default_lock_cone);
    computer_config_.default_max_range = GetGameConfig().GetFloat("graphics.hud.radarRange", computer_config_.default_max_range);
    computer_config_.default_tracking_cone = GetGameConfig().GetFloat("physics.autotracking", computer_config_.default_tracking_cone);

    // fuel substruct
    fuel.afterburner_fuel_usage =
            GetGameConfig().GetFloat("physics.AfterburnerFuelUsage", fuel.afterburner_fuel_usage);
    fuel.fmec_exit_velocity_inverse = GetGameConfig().GetFloat("physics.FMEC_exit_vel_inverse", fuel.fmec_exit_velocity_inverse);
//    fuel.fmec_exit_velocity_inverse =
//            1.0F / GetGameConfig().GetFloat("physics.FMEC_exit_vel", 1.0F / 0.0000002F);
    fuel.fuel_efficiency =
            GetGameConfig().GetFloat("physics.LithiumRelativeEfficiency_Lithium", fuel.fuel_efficiency);
    fuel.fuel_equals_warp = GetGameConfig().GetBool("physics.fuel_equals_warp", fuel.fuel_equals_warp);
    fuel.normal_fuel_usage = GetGameConfig().GetFloat("physics.FuelUsage", fuel.normal_fuel_usage);
    fuel.reactor_uses_fuel = GetGameConfig().GetBool("physics.reactor_uses_fuel", fuel.reactor_uses_fuel);
    fuel.vsd_mj_yield = GetGameConfig().GetFloat("physics.VSD_MJ_yield", fuel.vsd_mj_yield);
    fuel.no_fuel_thrust = GetGameConfig().GetFloat("physics.NoFuelThrust", fuel.no_fuel_thrust);
    fuel.no_fuel_afterburn = GetGameConfig().GetFloat("physics.NoFuelAfterburn", fuel.no_fuel_afterburn);
    fuel.variable_fuel_consumption = GetGameConfig().GetBool("physics.VariableFuelConsumption", fuel.variable_fuel_consumption);
    // Also known as Lithium6constant
    fuel.deuterium_relative_efficiency_lithium = GetGameConfig().GetFloat("physics.DeuteriumRelativeEfficiency_Lithium", fuel.deuterium_relative_efficiency_lithium);
    //Fuel Mass in metric tons expended per generation of 100MJ
    fuel.fmec_factor = GetGameConfig().GetFloat("physics.FMEC_factor", fuel.fmec_factor);
    fuel.reactor_idle_efficiency = GetGameConfig().GetFloat("physics.reactor_idle_efficiency", fuel.reactor_idle_efficiency);
    fuel.min_reactor_efficiency = GetGameConfig().GetFloat("physics.min_reactor_efficiency", fuel.min_reactor_efficiency);
    fuel.ecm_energy_cost = GetGameConfig().GetFloat("physics.ecm_energy_cost", fuel.ecm_energy_cost);
    fuel.fuel_conversion = GetGameConfig().GetFloat("physics.FuelConversion", fuel.fuel_conversion);

    // graphics substruct
    graphics_config_.automatic_landing_zone_warning = GetGameConfig().GetString("graphics.automatic_landing_zone_warning", graphics_config_.automatic_landing_zone_warning);
    graphics_config_.automatic_landing_zone_warning1 = GetGameConfig().GetString("graphics.automatic_landing_zone_warning1", graphics_config_.automatic_landing_zone_warning1);
    graphics_config_.automatic_landing_zone_warning2 = GetGameConfig().GetString("graphics.automatic_landing_zone_warning2", graphics_config_.automatic_landing_zone_warning2);
    graphics_config_.automatic_landing_zone_warning_text = GetGameConfig().GetEscapedString("graphics.automatic_landing_zone_warning_text", graphics_config_.automatic_landing_zone_warning_text);
    graphics_config_.draw_heading_marker = GetGameConfig().GetBool("graphics.draw_heading_marker", graphics_config_.draw_heading_marker);
    graphics_config_.missile_explosion_radius_mult = GetGameConfig().GetFloat("graphics.missile_explosion_radius_mult", graphics_config_.missile_explosion_radius_mult);
    graphics_config_.missile_sparkle = GetGameConfig().GetBool("graphics.missilesparkle", graphics_config_.missile_sparkle);
    // Sic - Should be emissive, I think
    graphics_config_.atmosphere_emmissive = GetGameConfig().GetFloat("graphics.atmosphere_emmissive", graphics_config_.atmosphere_emmissive);
    graphics_config_.atmosphere_diffuse = GetGameConfig().GetFloat("graphics.atmosphere_diffuse", graphics_config_.atmosphere_diffuse);
    graphics_config_.default_engine_activation = GetGameConfig().GetFloat("graphics.default_engine_activation", graphics_config_.default_engine_activation);
    graphics_config_.explosion_animation = GetGameConfig().GetString("graphics.explosion_animation", graphics_config_.explosion_animation);
    graphics_config_.shield_detail = GetGameConfig().GetInt32("graphics.shield_detail", graphics_config_.shield_detail);
    graphics_config_.shield_texture = GetGameConfig().GetString("graphics.shield_texture", graphics_config_.shield_texture);
    graphics_config_.shield_technique = GetGameConfig().GetString("graphics.shield_technique", graphics_config_.shield_technique);
    graphics_config_.fov = GetGameConfig().GetFloat("graphics.fov", graphics_config_.fov);
    graphics_config_.reduced_vdus_width = GetGameConfig().GetFloat("graphics.reduced_vdus_width", graphics_config_.reduced_vdus_width);
    graphics_config_.reduced_vdus_height = GetGameConfig().GetFloat("graphics.reduced_vdus_height", graphics_config_.reduced_vdus_height);
    graphics_config_.star_body_radius = GetGameConfig().GetFloat("graphics.star_body_radius", graphics_config_.star_body_radius);
    graphics_config_.draw_star_glow = GetGameConfig().GetBool("graphics.draw_star_glow", graphics_config_.draw_star_glow);
    graphics_config_.draw_star_body = GetGameConfig().GetBool("graphics.draw_star_body", graphics_config_.draw_star_body);
    graphics_config_.star_glow_radius = GetGameConfig().GetFloat("graphics.star_glow_radius", graphics_config_.star_glow_radius);
    graphics_config_.glow_ambient_star_light = GetGameConfig().GetBool("graphics.glow_ambient_star_light", graphics_config_.glow_ambient_star_light);
    graphics_config_.glow_diffuse_star_light = GetGameConfig().GetBool("graphics.glow_diffuse_star_light", graphics_config_.glow_diffuse_star_light);
    graphics_config_.planet_detail_stacks = GetGameConfig().GetInt32("graphics.planet_detail", graphics_config_.planet_detail_stacks);
    graphics_config_.wormhole_unit = GetGameConfig().GetString("graphics.wormhole", graphics_config_.wormhole_unit);
    graphics_config_.city_light_strength = GetGameConfig().GetFloat("graphics.city_light_strength", graphics_config_.city_light_strength);
    graphics_config_.day_city_light_strength = GetGameConfig().GetFloat("graphics.day_city_light_strength", graphics_config_.day_city_light_strength);
    graphics_config_.num_times_to_draw_shine = GetGameConfig().GetInt32("graphics.num_times_to_draw_shine", graphics_config_.num_times_to_draw_shine);

    graphics_config_.glow_flicker.flicker_time = GetGameConfig().GetFloat("graphics.glowflicker.time", graphics_config_.glow_flicker.flicker_time);
    graphics_config_.glow_flicker.flicker_off_time = GetGameConfig().GetFloat("graphics.glowflicker.off-time", graphics_config_.glow_flicker.flicker_off_time);
    graphics_config_.glow_flicker.hull_for_total_dark = GetGameConfig().GetFloat("graphics.glowflicker.hull-for-total-dark", graphics_config_.glow_flicker.hull_for_total_dark);
    graphics_config_.glow_flicker.num_times_per_second_on = GetGameConfig().GetFloat("graphics.glowflicker.num-times-per-second-on", graphics_config_.glow_flicker.num_times_per_second_on);
    graphics_config_.glow_flicker.min_flicker_cycle = GetGameConfig().GetFloat("graphics.glowflicker.min-cycle", graphics_config_.glow_flicker.min_flicker_cycle);

    // graphics ==> hud subsection
    graphics_config_.hud.draw_rendered_crosshairs = GetGameConfig().GetBool("graphics.hud.draw_rendered_crosshairs", GetGameConfig().GetBool("graphics.draw_rendered_crosshairs", graphics_config_.hud.draw_rendered_crosshairs));
    graphics_config_.hud.already_near_message = GetGameConfig().GetEscapedString("graphics.hud.AlreadyNearMessage", graphics_config_.hud.already_near_message);
    graphics_config_.hud.armor_hull_size = GetGameConfig().GetFloat("graphics.hud.armor_hull_size", graphics_config_.hud.armor_hull_size);
    graphics_config_.hud.asteroids_near_message = GetGameConfig().GetEscapedString("graphics.hud.AsteroidsNearMessage", graphics_config_.hud.asteroids_near_message);
    graphics_config_.hud.basename_colon_basename = GetGameConfig().GetBool("graphics.hud.basename:basename", graphics_config_.hud.basename_colon_basename);
    graphics_config_.hud.box_line_thickness = GetGameConfig().GetFloat("graphics.hud.BoxLineThickness", graphics_config_.hud.box_line_thickness);
    graphics_config_.hud.completed_objectives_last = GetGameConfig().GetBool("graphics.hud.completed_objectives_last", graphics_config_.hud.completed_objectives_last);
    graphics_config_.hud.crosshairs_on_chase_cam = GetGameConfig().GetBool("graphics.hud.crosshairs_on_chasecam", graphics_config_.hud.crosshairs_on_chase_cam);
    graphics_config_.hud.crosshairs_on_padlock = GetGameConfig().GetBool("graphics.hud.crosshairs_on_padlock", graphics_config_.hud.crosshairs_on_padlock);
    graphics_config_.hud.damage_report_heading = GetGameConfig().GetEscapedString("graphics.hud.damage_report_heading", graphics_config_.hud.damage_report_heading);
    graphics_config_.hud.debug_position = GetGameConfig().GetBool("graphics.hud.debug_position", graphics_config_.hud.debug_position);
    graphics_config_.hud.diamond_line_thickness = GetGameConfig().GetFloat("graphics.hud.DiamondLineThickness", graphics_config_.hud.diamond_line_thickness);
    graphics_config_.hud.diamond_rotation_speed = GetGameConfig().GetFloat("graphics.hud.DiamondRotationSpeed", graphics_config_.hud.diamond_rotation_speed);
    graphics_config_.hud.diamond_size = GetGameConfig().GetFloat("graphics.hud.DiamondSize", graphics_config_.hud.diamond_size);
    graphics_config_.hud.display_relative_velocity = GetGameConfig().GetBool("graphics.hud.display_relative_velocity", graphics_config_.hud.display_relative_velocity);
    graphics_config_.hud.display_warp_energy_if_no_jump_drive = GetGameConfig().GetBool("graphics.hud.display_warp_energy_if_no_jump_drive", graphics_config_.hud.display_warp_energy_if_no_jump_drive);
    graphics_config_.hud.draw_all_target_boxes = GetGameConfig().GetBool("graphics.hud.drawAllTargetBoxes", graphics_config_.hud.draw_all_target_boxes);
    graphics_config_.hud.draw_always_itts = GetGameConfig().GetBool("graphics.hud.drawAlwaysITTS", graphics_config_.hud.draw_always_itts);
    graphics_config_.hud.draw_line_to_itts = GetGameConfig().GetBool("graphics.hud.drawLineToITTS", graphics_config_.hud.draw_line_to_itts);
    graphics_config_.hud.draw_line_to_target = GetGameConfig().GetBool("graphics.hud.drawLineToTarget", graphics_config_.hud.draw_line_to_target);
    graphics_config_.hud.draw_line_to_targets_target = GetGameConfig().GetBool("graphics.hud.drawLineToTargetsTarget", graphics_config_.hud.draw_line_to_targets_target);
    graphics_config_.hud.draw_nav_symbol = GetGameConfig().GetBool("graphics.hud.drawNavSymbol", graphics_config_.hud.draw_nav_symbol);
    graphics_config_.hud.draw_tactical_target = GetGameConfig().GetBool("graphics.hud.DrawTacticalTarget", graphics_config_.hud.draw_tactical_target);
    graphics_config_.hud.draw_targeting_boxes = GetGameConfig().GetBool("graphics.hud.DrawTargettingBoxes", graphics_config_.hud.draw_targeting_boxes);
    graphics_config_.hud.draw_targeting_boxes_inside = GetGameConfig().GetBool("graphics.hud.DrawTargettingBoxesInside", graphics_config_.hud.draw_targeting_boxes_inside);
    graphics_config_.hud.draw_arrow_on_chase_cam = GetGameConfig().GetBool("graphics.hud.draw_arrow_on_chasecam", graphics_config_.hud.draw_arrow_on_chase_cam);
    graphics_config_.hud.draw_arrow_on_pan_cam = GetGameConfig().GetBool("graphics.hud.draw_arrow_on_pancam", graphics_config_.hud.draw_arrow_on_pan_cam);
    graphics_config_.hud.draw_arrow_on_pan_target = GetGameConfig().GetBool("graphics.hud.draw_arrow_on_pantgt", graphics_config_.hud.draw_arrow_on_pan_target);
    graphics_config_.hud.draw_arrow_to_target = GetGameConfig().GetBool("graphics.hud.draw_arrow_to_target", graphics_config_.hud.draw_arrow_to_target);
    graphics_config_.hud.draw_blips_on_both_radar = GetGameConfig().GetBool("graphics.hud.draw_blips_on_both_radar", graphics_config_.hud.draw_blips_on_both_radar);
    graphics_config_.hud.draw_damage_sprite = GetGameConfig().GetBool("graphics.hud.draw_damage_sprite", graphics_config_.hud.draw_damage_sprite);
    graphics_config_.hud.draw_gravitational_objects = GetGameConfig().GetBool("graphics.hud.draw_gravitational_objects", graphics_config_.hud.draw_gravitational_objects);
    graphics_config_.hud.draw_nav_grid = GetGameConfig().GetBool("graphics.hud.draw_nav_grid", graphics_config_.hud.draw_nav_grid);
    graphics_config_.hud.draw_significant_blips = GetGameConfig().GetBool("graphics.hud.draw_significant_blips", graphics_config_.hud.draw_significant_blips);
    graphics_config_.hud.draw_star_direction = GetGameConfig().GetBool("graphics.hud.draw_star_direction", graphics_config_.hud.draw_star_direction);
    graphics_config_.hud.draw_unit_on_chase_cam = GetGameConfig().GetBool("graphics.hud.draw_unit_on_chasecam", graphics_config_.hud.draw_unit_on_chase_cam);
    graphics_config_.hud.draw_vdu_view_shields = GetGameConfig().GetBool("graphics.hud.draw_vdu_view_shields", graphics_config_.hud.draw_vdu_view_shields);
    graphics_config_.hud.draw_weapon_sprite = GetGameConfig().GetBool("graphics.hud.draw_weapon_sprite", graphics_config_.hud.draw_weapon_sprite);
    graphics_config_.hud.enemy_near_message = GetGameConfig().GetEscapedString("graphics.hud.EnemyNearMessage", graphics_config_.hud.enemy_near_message);
    graphics_config_.hud.gun_list_columns = GetGameConfig().GetInt32("graphics.hud.gun_list_columns", graphics_config_.hud.gun_list_columns);
    graphics_config_.hud.invert_friendly_shields = GetGameConfig().GetBool("graphics.hud.invert_friendly_shields", graphics_config_.hud.invert_friendly_shields);
    graphics_config_.hud.invert_friendly_sprite = GetGameConfig().GetBool("graphics.hud.invert_friendly_sprite", graphics_config_.hud.invert_friendly_sprite);
    graphics_config_.hud.invert_target_shields = GetGameConfig().GetBool("graphics.hud.invert_target_shields", graphics_config_.hud.invert_target_shields);
    graphics_config_.hud.invert_target_sprite = GetGameConfig().GetBool("graphics.hud.invert_target_sprite", graphics_config_.hud.invert_target_sprite);
    graphics_config_.hud.invert_view_shields = GetGameConfig().GetBool("graphics.hud.invert_view_shields", graphics_config_.hud.invert_view_shields);
    graphics_config_.hud.itts_for_beams = GetGameConfig().GetBool("graphics.hud.ITTSForBeams", graphics_config_.hud.itts_for_beams);
    graphics_config_.hud.itts_for_lockable = GetGameConfig().GetBool("graphics.hud.ITTSForLockable", graphics_config_.hud.itts_for_lockable);
    graphics_config_.hud.itts_line_to_mark_alpha = GetGameConfig().GetFloat("graphics.hud.ITTSLineToMarkAlpha", graphics_config_.hud.itts_line_to_mark_alpha);
    graphics_config_.hud.itts_use_average_gun_speed = GetGameConfig().GetBool("graphics.hud.ITTSUseAverageGunSpeed", graphics_config_.hud.itts_use_average_gun_speed);
    graphics_config_.hud.lock_center_crosshair = GetGameConfig().GetBool("graphics.hud.LockCenterCrosshair", graphics_config_.hud.lock_center_crosshair);
    graphics_config_.hud.lock_confirm_line_length = GetGameConfig().GetFloat("graphics.hud.LockConfirmLineLength", graphics_config_.hud.lock_confirm_line_length);
    graphics_config_.hud.manifest_heading = GetGameConfig().GetEscapedString("graphics.hud.manifest_heading", graphics_config_.hud.manifest_heading);
    graphics_config_.hud.max_missile_bracket_size = GetGameConfig().GetFloat("graphics.hud.MaxMissileBracketSize", graphics_config_.hud.max_missile_bracket_size);
    graphics_config_.hud.message_prefix = GetGameConfig().GetEscapedString("graphics.hud.message_prefix", graphics_config_.hud.message_prefix);
    graphics_config_.hud.min_missile_bracket_size = GetGameConfig().GetFloat("graphics.hud.MinMissileBracketSize", graphics_config_.hud.min_missile_bracket_size);
    graphics_config_.hud.min_lock_box_size = GetGameConfig().GetFloat("graphics.hud.min_lock_box_size", graphics_config_.hud.min_lock_box_size);
    graphics_config_.hud.min_radar_blip_size = GetGameConfig().GetFloat("graphics.hud.min_radarblip_size", graphics_config_.hud.min_radar_blip_size);
    graphics_config_.hud.min_target_box_size = GetGameConfig().GetFloat("graphics.hud.min_target_box_size", graphics_config_.hud.min_target_box_size);
    graphics_config_.hud.mounts_list_empty = GetGameConfig().GetString("graphics.hud.mounts_list_empty", graphics_config_.hud.mounts_list_empty);
    graphics_config_.hud.nav_cross_line_thickness = GetGameConfig().GetFloat("graphics.hud.NavCrossLineThickness", graphics_config_.hud.nav_cross_line_thickness);
    graphics_config_.hud.not_included_in_damage_report = GetGameConfig().GetString("graphics.hud.not_included_in_damage_report", graphics_config_.hud.not_included_in_damage_report);
    graphics_config_.hud.out_of_cone_distance = GetGameConfig().GetBool("graphics.hud.out_of_cone_distance", graphics_config_.hud.out_of_cone_distance);
    graphics_config_.hud.padlock_view_lag = GetGameConfig().GetFloat("graphics.hud.PadlockViewLag", graphics_config_.hud.padlock_view_lag);
    graphics_config_.hud.padlock_view_lag_fix_zone = GetGameConfig().GetFloat("graphics.hud.PadlockViewLagFixZone", graphics_config_.hud.padlock_view_lag_fix_zone);
    graphics_config_.hud.planet_near_message = GetGameConfig().GetEscapedString("graphics.hud.PlanetNearMessage", graphics_config_.hud.planet_near_message);
    graphics_config_.hud.print_damage_percent = GetGameConfig().GetBool("graphics.hud.print_damage_percent", graphics_config_.hud.print_damage_percent);
    graphics_config_.hud.print_faction = GetGameConfig().GetBool("graphics.hud.print_faction", graphics_config_.hud.print_faction);
    graphics_config_.hud.print_fg_name = GetGameConfig().GetBool("graphics.hud.print_fg_name", graphics_config_.hud.print_fg_name);
    graphics_config_.hud.print_fg_sub_id = GetGameConfig().GetBool("graphics.hud.print_fg_sub_id", graphics_config_.hud.print_fg_sub_id);
    graphics_config_.hud.print_request_docking = GetGameConfig().GetBool("graphics.hud.print_request_docking", graphics_config_.hud.print_request_docking);
    graphics_config_.hud.print_ship_type = GetGameConfig().GetBool("graphics.hud.print_ship_type", graphics_config_.hud.print_ship_type);
    graphics_config_.hud.projectile_means_missile = GetGameConfig().GetBool("graphics.hud.projectile_means_missile", graphics_config_.hud.projectile_means_missile);
    graphics_config_.hud.radar_type = GetGameConfig().GetString("graphics.hud.radarType", graphics_config_.hud.radar_type);
    graphics_config_.hud.radar_search_extra_radius = GetGameConfig().GetFloat("graphics.hud.radar_search_extra_radius", graphics_config_.hud.radar_search_extra_radius);
    graphics_config_.hud.rotating_bracket_inner = GetGameConfig().GetBool("graphics.hud.RotatingBracketInner", graphics_config_.hud.rotating_bracket_inner);
    graphics_config_.hud.rotating_bracket_size = GetGameConfig().GetFloat("graphics.hud.RotatingBracketSize", graphics_config_.hud.rotating_bracket_size);
    graphics_config_.hud.rotating_bracket_width = GetGameConfig().GetFloat("graphics.hud.RotatingBracketWidth", graphics_config_.hud.rotating_bracket_width);
    graphics_config_.hud.scale_relationship_color = GetGameConfig().GetFloat("graphics.hud.scale_relationship_color", graphics_config_.hud.scale_relationship_color);
    graphics_config_.hud.shield_vdu_fade = GetGameConfig().GetBool("graphics.hud.shield_vdu_fade", graphics_config_.hud.shield_vdu_fade);
    bool do_fade = graphics_config_.hud.shield_vdu_fade;
    graphics_config_.hud.shield_vdu_thresh[0] = GetGameConfig().GetFloat("graphics.hud.shield_vdu_thresh0", (do_fade ? 0.0F : 0.25F));
    graphics_config_.hud.shield_vdu_thresh[1] = GetGameConfig().GetFloat("graphics.hud.shield_vdu_thresh1", (do_fade ? 0.33F : 0.5F));
    graphics_config_.hud.shield_vdu_thresh[2] = GetGameConfig().GetFloat("graphics.hud.shield_vdu_thresh2", (do_fade ? 0.66F : 0.75F));
    graphics_config_.hud.show_negative_blips_as_positive = GetGameConfig().GetBool("graphics.hud.show_negative_blips_as_positive", graphics_config_.hud.show_negative_blips_as_positive);
    graphics_config_.hud.simple_manifest = GetGameConfig().GetBool("graphics.hud.simple_manifest", graphics_config_.hud.simple_manifest);
    graphics_config_.hud.starship_near_message = GetGameConfig().GetEscapedString("graphics.StarshipNearMessage", graphics_config_.hud.starship_near_message);
    graphics_config_.hud.switch_to_target_mode_on_key = GetGameConfig().GetBool("graphics.hud.switchToTargetModeOnKey", graphics_config_.hud.switch_to_target_mode_on_key);
    graphics_config_.hud.switch_back_from_comms = GetGameConfig().GetBool("graphics.hud.switch_back_from_comms", graphics_config_.hud.switch_back_from_comms);
    graphics_config_.hud.tac_target_foci = GetGameConfig().GetFloat("graphics.hud.TacTargetFoci", graphics_config_.hud.tac_target_foci);
    graphics_config_.hud.tac_target_length = GetGameConfig().GetFloat("graphics.hud.TacTargetLength", graphics_config_.hud.tac_target_length);
    graphics_config_.hud.tac_target_thickness = GetGameConfig().GetFloat("graphics.hud.TacTargetThickness", graphics_config_.hud.tac_target_thickness);
    graphics_config_.hud.text_background_alpha = GetGameConfig().GetFloat("graphics.hud.text_background_alpha", graphics_config_.hud.text_background_alpha);
    graphics_config_.hud.top_view = GetGameConfig().GetBool("graphics.hud.top_view", graphics_config_.hud.top_view);
    graphics_config_.hud.untarget_beyond_cone = GetGameConfig().GetBool("graphics.hud.untarget_beyond_cone", graphics_config_.hud.untarget_beyond_cone);

    // logging substruct
    logging.vsdebug = GetGameConfig().GetInt8("general.verbose_output", logging.vsdebug);
    logging.verbose_debug = GetGameConfig().GetBool("data.verbose_debug", logging.verbose_debug);

    // physics substruct
    physics_config_.collision_scale_factor =
            GetGameConfig().GetFloat("physics.collision_damage_scale", physics_config_.collision_scale_factor);
    physics_config_.inelastic_scale = GetGameConfig().GetFloat("physics.inelastic_scale", physics_config_.inelastic_scale);
    physics_config_.kilojoules_per_damage =
            GetGameConfig().GetFloat("physics.kilojoules_per_unit_damage", physics_config_.kilojoules_per_damage);
    physics_config_.max_force_multiplier =
            GetGameConfig().GetFloat("physics.maxCollisionForceMultiplier", physics_config_.max_force_multiplier);
    physics_config_.max_shield_lowers_capacitance =
            GetGameConfig().GetBool("physics.max_shield_lowers_capacitance", physics_config_.max_shield_lowers_capacitance);
    physics_config_.max_torque_multiplier =
            GetGameConfig().GetFloat("physics.maxCollisionTorqueMultiplier", physics_config_.max_torque_multiplier);
    physics_config_.minimum_mass = GetGameConfig().GetFloat("physics.minimum_physics_object_mass", physics_config_.minimum_mass);
    physics_config_.minimum_time =
            GetGameConfig().GetFloat("physics.minimum_time_between_recorded_player_collisions", physics_config_.minimum_time);
    physics_config_.default_shield_tightness = GetGameConfig().GetFloat("physics.default_shield_tightness", physics_config_.default_shield_tightness);
    physics_config_.does_missile_bounce = GetGameConfig().GetBool("physics.missile_bounce", physics_config_.does_missile_bounce);
    physics_config_.system_damage_on_armor = GetGameConfig().GetBool("physics.system_damage_on_armor", physics_config_.system_damage_on_armor);
    physics_config_.indiscriminate_system_destruction = GetGameConfig().GetFloat("physics.indiscriminate_system_destruction", physics_config_.indiscriminate_system_destruction);
    physics_config_.separate_system_flakiness_component = GetGameConfig().GetBool("physics.separate_system_flakiness_component", physics_config_.separate_system_flakiness_component);
    physics_config_.shield_energy_capacitance = GetGameConfig().GetFloat("physics.shield_energy_capacitance", physics_config_.shield_energy_capacitance);
    physics_config_.use_max_shield_energy_usage = GetGameConfig().GetBool("physics.use_max_shield_energy_usage", physics_config_.use_max_shield_energy_usage);
    physics_config_.shields_in_spec = GetGameConfig().GetBool("physics.shields_in_spec", physics_config_.shields_in_spec);
    physics_config_.shield_maintenance_charge = GetGameConfig().GetFloat("physics.shield_maintenance_charge", physics_config_.shield_maintenance_charge);
    physics_config_.max_ecm = GetGameConfig().GetSizeT("physics.max_ecm", physics_config_.max_ecm);
    physics_config_.max_lost_target_live_time = GetGameConfig().GetFloat("physics.max_lost_target_live_time", physics_config_.max_lost_target_live_time);
    physics_config_.percent_missile_match_target_velocity = GetGameConfig().GetFloat("physics.percent_missile_match_target_velocity", physics_config_.percent_missile_match_target_velocity);
    physics_config_.game_speed = GetGameConfig().GetFloat("physics.game_speed", physics_config_.game_speed);
    physics_config_.game_accel = GetGameConfig().GetFloat("physics.game_accel", physics_config_.game_accel);
    physics_config_.velocity_max = GetGameConfig().GetFloat("physics.velocity_max", physics_config_.velocity_max);
    physics_config_.max_player_rotation_rate = GetGameConfig().GetFloat("physics.maxplayerrot", physics_config_.max_player_rotation_rate);
    physics_config_.max_non_player_rotation_rate = GetGameConfig().GetFloat("physics.maxNPCrot", physics_config_.max_non_player_rotation_rate);
    physics_config_.unit_table = GetGameConfig().GetBool("physics.UnitTable", physics_config_.unit_table);
    physics_config_.capship_size = GetGameConfig().GetFloat("physics.capship_size", physics_config_.capship_size);
    physics_config_.near_autotrack_cone = GetGameConfig().GetFloat("physics.near_autotrack_cone", physics_config_.near_autotrack_cone);
    physics_config_.close_enough_to_autotrack = GetGameConfig().GetFloat("physics.close_enough_to_autotrack", physics_config_.close_enough_to_autotrack);
    physics_config_.distance_to_warp = GetGameConfig().GetFloat("physics.distance_to_warp", physics_config_.distance_to_warp);
    physics_config_.target_distance_to_warp_bonus = GetGameConfig().GetFloat("physics.target_distance_to_warp_bonus", physics_config_.target_distance_to_warp_bonus);
    physics_config_.no_spec_jump = GetGameConfig().GetBool("physics.noSPECJUMP", physics_config_.no_spec_jump);
    physics_config_.difficulty_speed_exponent = GetGameConfig().GetFloat("physics.difficulty_speed_exponent", physics_config_.difficulty_speed_exponent);
    physics_config_.min_damage = GetGameConfig().GetFloat("physics.min_damage", physics_config_.min_damage);
    physics_config_.max_damage = GetGameConfig().GetFloat("physics.max_damage", physics_config_.max_damage);
    physics_config_.max_radar_cone_damage = GetGameConfig().GetFloat("physics.max_radar_cone_damage", physics_config_.max_radar_cone_damage);
    physics_config_.max_radar_lock_cone_damage = GetGameConfig().GetFloat("physics.max_radar_lockcone_damage", physics_config_.max_radar_lock_cone_damage);
    physics_config_.max_radar_track_cone_damage = GetGameConfig().GetFloat("physics.max_radar_trackcone_damage", physics_config_.max_radar_track_cone_damage);
    physics_config_.thruster_hit_chance = GetGameConfig().GetFloat("physics.thruster_hit_chance", physics_config_.thruster_hit_chance);
    physics_config_.friendly_auto_radius = GetGameConfig().GetFloat("physics.friendly_auto_radius", physics_config_.friendly_auto_radius) * physics_config_.game_speed;
    physics_config_.neutral_auto_radius = GetGameConfig().GetFloat("physics.neutral_auto_radius", physics_config_.neutral_auto_radius) * physics_config_.game_speed;
    physics_config_.hostile_auto_radius = GetGameConfig().GetFloat("physics.hostile_auto_radius", physics_config_.hostile_auto_radius) * physics_config_.game_speed;
    physics_config_.min_asteroid_distance = GetGameConfig().GetFloat("physics.min_asteroid_distance", physics_config_.min_asteroid_distance);
    physics_config_.steady_itts = GetGameConfig().GetBool("physics.steady_itts", physics_config_.steady_itts);
    physics_config_.no_unit_collisions = GetGameConfig().GetBool("physics.no_unit_collisions", physics_config_.no_unit_collisions);
    physics_config_.difficulty_based_shield_recharge = GetGameConfig().GetBool("physics.difficulty_based_shield_recharge", physics_config_.difficulty_based_shield_recharge);
    physics_config_.engine_energy_takes_priority = GetGameConfig().GetBool("physics.engine_energy_priority", physics_config_.engine_energy_takes_priority);
    physics_config_.density_of_rock = GetGameConfig().GetFloat("physics.density_of_rock", physics_config_.density_of_rock);
    physics_config_.density_of_jump_point = GetGameConfig().GetFloat("physics.density_of_jump_point", physics_config_.density_of_jump_point);
    physics_config_.planet_dock_port_size = GetGameConfig().GetFloat("physics.planet_port_size", physics_config_.planet_dock_port_size);
    physics_config_.planet_dock_port_min_size = GetGameConfig().GetFloat("physics.planet_port_min_size", physics_config_.planet_dock_port_min_size);
    physics_config_.jump_mesh_radius_scale = GetGameConfig().GetFloat("physics.jump_mesh_radius_scale", physics_config_.jump_mesh_radius_scale);
    physics_config_.planets_can_have_subunits = GetGameConfig().GetBool("physics.planets_can_have_subunits", physics_config_.planets_can_have_subunits);
    physics_config_.planets_always_neutral = GetGameConfig().GetBool("physics.planets_always_neutral", physics_config_.planets_always_neutral);
    physics_config_.no_damage_to_docked_ships = GetGameConfig().GetBool("physics.no_damage_to_docked_ships", physics_config_.no_damage_to_docked_ships);
    physics_config_.difficulty_based_enemy_damage = GetGameConfig().GetBool("physics.difficulty_based_enemy_damage", physics_config_.difficulty_based_enemy_damage);
    physics_config_.speeding_discharge = GetGameConfig().GetFloat("physics.speeding_discharge", physics_config_.speeding_discharge);
    physics_config_.min_shield_speeding_discharge = GetGameConfig().GetFloat("physics.min_shield_speeding_discharge", physics_config_.min_shield_speeding_discharge);
    physics_config_.nebula_shield_recharge = GetGameConfig().GetFloat("physics.nebula_shield_recharge", physics_config_.nebula_shield_recharge);

    // These calculations depend on the physics.game_speed and physics.game_accel values to be set already;
    // that's why they're down here instead of with the other graphics settings
    graphics_config_.in_system_jump_ani_second_ahead = GetGameConfig().GetFloat("graphics.insys_jump_ani_second_ahead", graphics_config_.in_system_jump_ani_second_ahead)
            / physics_config_.game_speed * physics_config_.game_accel;
    graphics_config_.in_system_jump_ani_second_ahead_end = GetGameConfig().GetFloat("graphics.insys_jump_ani_second_ahead_end", graphics_config_.in_system_jump_ani_second_ahead_end)
            / physics_config_.game_speed * physics_config_.game_accel;
    graphics_config_.in_system_jump_animation = GetGameConfig().GetString("graphics.insys_jump_animation", graphics_config_.in_system_jump_animation);

    eject_config_.eject_cargo_percent = GetGameConfig().GetFloat("physics.eject_cargo_percent", eject_config_.eject_cargo_percent);
    eject_config_.max_dumped_cargo = GetGameConfig().GetUInt32("physics.max_dumped_cargo", eject_config_.max_dumped_cargo);
    eject_config_.hull_damage_to_eject = GetGameConfig().GetFloat("physics.hull_damage_to_eject", eject_config_.hull_damage_to_eject);
    eject_config_.auto_eject_percent = GetGameConfig().GetFloat("physics.autoeject_percent", eject_config_.auto_eject_percent);
    eject_config_.player_auto_eject = GetGameConfig().GetBool("physics.player_autoeject", eject_config_.player_auto_eject);

    unit_config_.default_aggressivity = GetGameConfig().GetFloat("unit.aggressivity", unit_config_.default_aggressivity);

    // warp_config_ substruct
    warp_config_.insystem_jump_cost = GetGameConfig().GetFloat("physics.insystem_jump_cost", warp_config_.insystem_jump_cost);
    warp_config_.bleed_factor = GetGameConfig().GetFloat("physics.warpbleed", warp_config_.bleed_factor);
    warp_config_.warp_energy_multiplier = GetGameConfig().GetFloat("physics.warp_energy_multiplier", warp_config_.warp_energy_multiplier);
    warp_config_.player_warp_energy_multiplier = GetGameConfig().GetFloat("physics.player_warp_energy_multiplier", warp_config_.player_warp_energy_multiplier);
    warp_config_.warp_ramp_up_time = GetGameConfig().GetFloat("physics.warprampuptime", warp_config_.warp_ramp_up_time);
    warp_config_.computer_warp_ramp_up_time = GetGameConfig().GetFloat("physics.computerwarprampuptime", warp_config_.computer_warp_ramp_up_time);
    warp_config_.warp_ramp_down_time = GetGameConfig().GetFloat("physics.warprampdowntime", warp_config_.warp_ramp_down_time);
    warp_config_.warp_memory_effect = GetGameConfig().GetFloat("physics.WarpMemoryEffect", warp_config_.warp_memory_effect);
    warp_config_.warp_stretch_cutoff = GetGameConfig().GetFloat("physics.warp_stretch_cutoff", warp_config_.warp_stretch_cutoff) * physics_config_.game_speed;
    warp_config_.warp_stretch_decel_cutoff = GetGameConfig().GetFloat("physics.warp_stretch_decel_cutoff", warp_config_.warp_stretch_decel_cutoff) * physics_config_.game_speed;
    warp_config_.warp_multiplier_min = GetGameConfig().GetFloat("physics.warpMultiplierMin", warp_config_.warp_multiplier_min);
    warp_config_.warp_multiplier_max = GetGameConfig().GetFloat("physics.warpMultiplierMax", warp_config_.warp_multiplier_max);
    warp_config_.max_effective_velocity = GetGameConfig().GetFloat("physics.warpMaxEfVel", warp_config_.max_effective_velocity);
    warp_config_.use_warp_energy_for_cloak = GetGameConfig().GetBool("physics.warp_energy_for_cloak", warp_config_.use_warp_energy_for_cloak);

    // weapons substruct
    weapons.can_fire_in_cloak = GetGameConfig().GetBool("physics.can_fire_in_cloak", weapons.can_fire_in_cloak);
    weapons.can_fire_in_spec = GetGameConfig().GetBool("physics.can_fire_in_spec", weapons.can_fire_in_spec);
}

// Each of the following constructors MUST specify default values for ALL struct elements - will be subsequently overridden by user specified values, if any

vega_config::CollisionHacks::CollisionHacks() :
        collision_hack_distance(10000.0F),
        collision_damage_to_ai(false),
        crash_dock_hangar(false),
        crash_dock_unit(false),
        front_collision_hack_angle(std::cos(M_PI * 40.0
                / 180.0)), /* Note: Does not follow pattern of directly setting itself to user specified config value, must change default value in override as well if changing it here*/
        front_collision_hack_distance(200000.0F),
        cargo_deals_collide_damage(false) {
}

vega_config::ComputerConfig::ComputerConfig() :
        default_lock_cone(0.8f),
        default_max_range(20000.0f),
        default_tracking_cone(0.93f) {
}

vega_config::Fuel::Fuel() :
        afterburner_fuel_usage(4.0f),
        fuel_efficiency(1.0f),
        fuel_equals_warp(false),
        normal_fuel_usage(1.0f),
        reactor_uses_fuel(false) {
}

vega_config::PhysicsConfig::PhysicsConfig() :
        collision_scale_factor(1.0f),
        inelastic_scale(0.8f),
        kilojoules_per_damage(5400.0f),
        max_force_multiplier(5.0f),
        max_shield_lowers_capacitance(false),
        max_torque_multiplier(0.67f),
        minimum_mass(1e-6f), /* this is actually a bit high (1 gram, in the current, non-SI, units that VS uses ), fwiw - we may want to change this some day to, say, a milligram .*/
        minimum_time(0.1f) {
}

vega_config::WeaponsConfig::WeaponsConfig() :
        can_fire_in_cloak(false),
        can_fire_in_spec(false) {
}

std::shared_ptr<Configuration> configuration() {
    static const std::shared_ptr<Configuration> kConfiguration = std::make_shared<Configuration>();
    return kConfiguration;
}
