/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LENTITY_COMPONENTS_BASE_TYPES_HPP__
#define __LENTITY_COMPONENTS_BASE_TYPES_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/util/bulletinfo.h"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/lua_entity_component.hpp"
#include "pragma/model/animation/animation_event.h"
#include "pragma/model/animation/animation.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include <sharedutils/util_weak_handle.hpp>
#include <mathutil/plane.hpp>

extern DLLNETWORK Engine *engine;

namespace Lua::TraceData {
	void FillTraceResultTable(lua_State *l, TraceResult &res);
};
namespace Lua {
	DLLNETWORK AnimationEvent get_animation_event(lua_State *l, int32_t tArgs, uint32_t eventId);
	namespace Physics {

	};

	namespace FuncWater {
	};

	template<class TLuaClass, class THandle>
	void register_base_func_water_component_methods(lua_State *l, TLuaClass &def)
	{
	}
	template<class TLuaClass, class THandle>
	void register_base_toggle_component_methods(lua_State *l, TLuaClass &def)
	{
	}
	template<class TLuaClass, class THandle>
	void register_base_wheel_component_methods(lua_State *l, TLuaClass &def)
	{
		// TODO
	}
	template<class TLuaClass, class THandle>
	void register_base_decal_component_methods(lua_State *l, TLuaClass &def)
	{
	}
	template<class TLuaClass, class THandle>
	void register_base_env_light_component_methods(lua_State *l, TLuaClass &def)
	{
	}
	template<class TLuaClass, class THandle>
	void register_base_env_light_spot_component_methods(lua_State *l, TLuaClass &def)
	{
	}
	template<class TLuaClass, class THandle>
	void register_base_env_light_point_component_methods(lua_State *l, TLuaClass &def)
	{
	}
	template<class TLuaClass, class THandle>
	void register_base_env_light_directional_component_methods(lua_State *l, TLuaClass &def)
	{
	}
	template<class TLuaClass, class THandle>
	void register_base_env_particle_system_component_methods(lua_State *l, TLuaClass &def)
	{
	}
	template<class TLuaClass, class THandle>
	void register_base_flammable_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_flex_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_health_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_name_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_networked_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_observable_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_observer_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_shooter_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_physics_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_render_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_soft_body_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_sound_emitter_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_transform_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_color_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_score_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_radius_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_sound_dsp_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_camera_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_explosion_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_fire_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_fog_controller_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_light_spot_vol_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_microphone_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_quake_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_smoke_trail_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_sound_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_soundscape_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_sprite_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_timescale_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_weather_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_wind_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_filter_name_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_env_filter_class_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_func_brush_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_func_button_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_func_kinematic_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_func_physics_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_func_portal_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_func_soft_physics_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_func_surface_material_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_logic_relay_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_bot_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_point_path_node_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_point_constraint_ball_socket_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_point_constraint_cone_twist_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_point_constraint_dof_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_point_constraint_fixed_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_point_constraint_hinge_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_point_constraint_slider_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_point_render_target_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_point_target_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_prop_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_prop_dynamic_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_prop_physics_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_trigger_hurt_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_trigger_push_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_trigger_remove_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_trigger_teleport_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_touch_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_trigger_gravity_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_flashlight_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_skybox_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_world_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_ai_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_character_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_vehicle_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_weapon_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_player_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_gamemode_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_io_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_model_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle, typename TBoneId>
	void register_base_animated_component_bone_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_time_scale_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_parent_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_ownable_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_debug_text_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_debug_point_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_debug_line_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_debug_box_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_debug_sphere_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_debug_cone_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_debug_cylinder_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_debug_plane_component_methods(lua_State *l, TLuaClass &def)
	{
	}

	template<class TLuaClass, class THandle>
	void register_base_point_at_target_component_methods(lua_State *l, TLuaClass &def)
	{
	}
};

#endif
