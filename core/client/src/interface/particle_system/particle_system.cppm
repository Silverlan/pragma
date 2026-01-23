// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system;
export import :particle_system.enums;

export import :particle_system.initializer_color;
export import :particle_system.initializer_initial_angular_velocity;
export import :particle_system.initializer_initial_animation_frame;
export import :particle_system.initializer_initial_velocity;
export import :particle_system.initializer_lua;
export import :particle_system.initializer_position;
export import :particle_system.initializer_radius_random;
export import :particle_system.initializer_shoot;
export import :particle_system.initializer_speed;

export import :particle_system.lua_particle_modifier_manager;

export import :particle_system.modifier;
export import :particle_system.modifier_ease;
export import :particle_system.modifier_gradual_fade;
export import :particle_system.modifier_random_color;
export import :particle_system.modifier_random_variable;
export import :particle_system.modifier_time;

export import :particle_system.operator_angular_acceleration;
export import :particle_system.operator_animation_playback;
export import :particle_system.operator_color_fade;
export import :particle_system.operator_cylindrical_vortex;
export import :particle_system.operator_gravity;
export import :particle_system.operator_jitter;
export import :particle_system.operator_linear_drag;
export import :particle_system.operator_pause_emission;
export import :particle_system.operator_physics;
export import :particle_system.operator_quadratic_drag;
export import :particle_system.operator_radius_fade;
export import :particle_system.operator_random_emission_rate;
export import :particle_system.operator_texture_scrolling;
export import :particle_system.operator_toroidal_vortex;
export import :particle_system.operator_trail;
export import :particle_system.operator_velocity;
export import :particle_system.operator_wander;
export import :particle_system.operator_wind;
export import :particle_system.operator_world_base;

export import :particle_system.renderer_animated_sprites;
export import :particle_system.renderer_beam;
export import :particle_system.renderer_blob;
export import :particle_system.renderer_model;
export import :particle_system.renderer_rotational_buffer;
export import :particle_system.renderer_sprite;
export import :particle_system.renderer_sprite_trail;

export namespace pragma {
	namespace particle_system = pts;
}
