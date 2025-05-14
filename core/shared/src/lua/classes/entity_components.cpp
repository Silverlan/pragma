/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "pragma/physics/raytraces.h"
#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/lua/classes/entity_components.hpp"
#include "pragma/entities/components/base_attachment_component.hpp"
#include "pragma/entities/components/base_game_component.hpp"
#include "pragma/entities/components/base_entity_component_member_register.hpp"
#include "pragma/entities/components/base_bvh_component.hpp"
#include "pragma/entities/components/base_child_component.hpp"
#include "pragma/entities/components/base_observer_component.hpp"
#include "pragma/entities/components/base_static_bvh_cache_component.hpp"
#include "pragma/entities/components/base_static_bvh_user_component.hpp"
#include "pragma/entities/components/intersection_handler_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/model/modelmesh.h"
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include "pragma/lua/policies/shared_from_this_policy.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/converters/pair_converter_t.hpp"
#include "pragma/lua/converters/property_converter_t.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/util_logging.hpp"
#include "pragma/lua/lua_component_event.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/lua/lua_util_component.hpp"
#include "pragma/lua/types/udm.hpp"
#include "pragma/lua/libraries/lprint.h"
#include "pragma/entities/components/parent_component.hpp"
#include "pragma/entities/components/base_entity_component_logging.hpp"
#include "pragma/physics/shape.hpp"
#include "pragma/util/render_tile.hpp"
#include "pragma/lua/ostream_operator_alias.hpp"
#include <udm.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/copy_policy.hpp>
#include <luabind/discard_result_policy.hpp>
#include <fmt/core.h>

namespace Lua {
	template<typename... Types>
	static luabind::class_<Types..., pragma::BaseEntityComponent> create_base_entity_component_class(const char *name)
	{
		auto def = pragma::lua::create_entity_component_class<Types..., pragma::BaseEntityComponent>(name);
		def.def(luabind::tostring(luabind::self));
		return def;
	}
};

namespace pragma::lua {
	namespace base_ai_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_animated_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_attachable_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_child_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_bot_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_character_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_color_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_surface_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_debug_box_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_debug_cone_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_debug_cylinder_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_debug_line_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_debug_plane_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_debug_point_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_debug_sphere_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_debug_text_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_decal_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_camera_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_explosion_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_filter_class_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_filter_name_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_fire_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_fog_controller_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_light_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_light_directional_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_light_point_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_light_spot_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_light_spot_vol_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_microphone_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_particle_system_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_quake_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_smoke_trail_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_soundscape_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_sound_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_sound_dsp_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_sprite_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_timescale_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_weather_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_env_wind_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_flammable_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_flashlight_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_flex_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_func_brush_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_func_button_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_func_kinematic_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_func_physics_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_func_portal_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_func_soft_physics_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_func_surface_material_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_func_water_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_gamemode_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_generic_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_health_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_info_landmark_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_io_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_logic_relay_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_model_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_name_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_networked_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_observable_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_ownable_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_physics_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_player_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_observer_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_point_at_target_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_point_constraint_ball_socket_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_point_constraint_cone_twist_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_point_constraint_dof_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_point_constraint_fixed_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_point_constraint_hinge_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_point_constraint_slider_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_point_path_node_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_point_render_target_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_point_target_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_prop_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_prop_dynamic_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_prop_physics_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_radius_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_field_angle_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_render_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_score_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_shooter_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_skybox_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_soft_body_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_sound_emitter_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_time_scale_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_toggle_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_touch_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_transform_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_trigger_gravity_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_trigger_hurt_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_trigger_push_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_trigger_remove_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_trigger_teleport_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_vehicle_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_weapon_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_wheel_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_world_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_liquid_control_component {
		static void register_class(luabind::module_ &mod);
	};
	namespace base_liquid_surface_simulation_component {
		static void register_class(luabind::module_ &mod);
	};
	// --template-namespace-declaration-location
};
std::optional<Lua::udm_type> pragma::lua::get_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo)
{
	return pragma::ents::visit_member(memberInfo.type, [&memberInfo, &component, l](auto tag) -> std::optional<Lua::udm_type> {
		using T = typename decltype(tag)::type;
		if constexpr(!pragma::is_valid_component_property_type_v<T>)
			return {};
		else {
			T value;
			memberInfo.getterFunction(memberInfo, component, &value);
			if constexpr(std::is_same_v<T, pragma::ents::Element>) {
				if(!value)
					return Lua::nil;
			}
			return Lua::udm_type {luabind::object {l, value}};
		}
	});
}
bool pragma::lua::set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, Lua::udm_type value)
{
	return pragma::ents::visit_member(memberInfo.type, [&memberInfo, &component, l, &value](auto tag) -> bool {
		using T = typename decltype(tag)::type;
		if constexpr(!pragma::is_valid_component_property_type_v<T> || std::is_same_v<T, pragma::ents::Element>)
			return false;
		else {
			if constexpr(Lua::is_native_type<T>) {
				if(memberInfo.IsEnum()) {
					if constexpr(udm::is_numeric_type(udm::type_to_enum<T>())) {
						if(luabind::type(value) == LUA_TSTRING) {
							auto e = memberInfo.EnumNameToValue(luabind::object_cast<std::string>(value));
							if(!e.has_value())
								return false;
							auto v = static_cast<T>(*e);
							memberInfo.setterFunction(memberInfo, component, &v);
							return true;
						}
						auto v = luabind::object_cast<T>(value);
						memberInfo.setterFunction(memberInfo, component, &v);
						return true;
					}
					// Unreachable
					return false;
				}
				auto v = luabind::object_cast<T>(value);
				memberInfo.setterFunction(memberInfo, component, &v);
			}
			else {
				auto *v = luabind::object_cast<T *>(value);
				if(!v)
					return false;
				memberInfo.setterFunction(memberInfo, component, v);
			}
			return true;
		}
	});
}
static_assert(umath::to_integral(pragma::ents::EntityMemberType::VersionIndex) == 0);
bool pragma::lua::set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, const pragma::EntityURef &eref)
{
	if(memberInfo.type != pragma::ents::EntityMemberType::Entity)
		return false;
	memberInfo.setterFunction(memberInfo, component, &eref);
	return true;
}
bool pragma::lua::set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, const pragma::MultiEntityURef &eref)
{
	if(memberInfo.type != pragma::ents::EntityMemberType::MultiEntity)
		return false;
	memberInfo.setterFunction(memberInfo, component, &eref);
	return true;
}
bool pragma::lua::set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, const pragma::EntityUComponentMemberRef &eref)
{
	if(memberInfo.type != pragma::ents::EntityMemberType::ComponentProperty)
		return false;
	memberInfo.setterFunction(memberInfo, component, &eref);
	return true;
}
static void get_dynamic_member_ids(pragma::BaseEntityComponent &c, std::vector<pragma::ComponentMemberIndex> &memberIndices)
{
	auto *reg = dynamic_cast<pragma::DynamicMemberRegister *>(&c);
	if(!reg)
		return;
	auto offset = c.GetStaticMemberCount();
	auto &members = reg->GetMembers();
	memberIndices.reserve(memberIndices.size() + members.size());
	for(size_t i = 0; i < members.size(); ++i)
		memberIndices.push_back(offset + i);
}
static std::vector<pragma::ComponentMemberIndex> get_dynamic_member_ids(pragma::BaseEntityComponent &c)
{
	std::vector<pragma::ComponentMemberIndex> memberIndices;
	get_dynamic_member_ids(c, memberIndices);
	return memberIndices;
}

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseEntityComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(util, Path);
#endif

template<typename TMemberIdentifier, typename TValue, auto TSetValue>
bool set_transform_member_value(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const TValue &value)
{
	if constexpr(std::is_same_v<TMemberIdentifier, std::string>) {
		auto idx = component.GetMemberIndex(memId);
		if(!idx)
			return false;
		return set_transform_member_value<pragma::ComponentMemberIndex, TValue, TSetValue>(component, *idx, space, value);
	}
	else
		return (component.*TSetValue)(memId, space, value);
	return false;
}

template<typename TMemberIdentifier>
static bool set_transform_member_pos(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const Vector3 &value)
{
	return set_transform_member_value<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::SetTransformMemberPos>(component, memId, space, value);
}

template<typename TMemberIdentifier>
static bool set_transform_member_rot(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const Quat &value)
{
	return set_transform_member_value<TMemberIdentifier, Quat, &pragma::BaseEntityComponent::SetTransformMemberRot>(component, memId, space, value);
}

template<typename TMemberIdentifier>
static bool set_transform_member_scale(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const Vector3 &value)
{
	return set_transform_member_value<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::SetTransformMemberScale>(component, memId, space, value);
}

template<typename TMemberIdentifier>
static bool set_transform_member_pose(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const umath::ScaledTransform &value)
{
	return set_transform_member_value<TMemberIdentifier, umath::ScaledTransform, &pragma::BaseEntityComponent::SetTransformMemberPose>(component, memId, space, value);
}

template<typename TMemberIdentifier, typename TValue, auto TGetValue, auto TGetParentValue, bool parentSpaceOnly = false>
std::optional<TValue> get_transform_member_value(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space)
{
	if constexpr(std::is_same_v<TMemberIdentifier, std::string>) {
		auto idx = component.GetMemberIndex(memId);
		if(!idx)
			return {};
		return get_transform_member_value<pragma::ComponentMemberIndex, TValue, TGetValue, TGetParentValue, parentSpaceOnly>(component, *idx, space);
	}
	else {
		TValue value;
		if constexpr(parentSpaceOnly) {
			if((component.*TGetParentValue)(memId, space, value))
				return value;
		}
		else {
			if((component.*TGetValue)(memId, space, value))
				return value;
		}
	}
	return {};
}

template<typename TMemberIdentifier, bool parentSpaceOnly = false>
static std::optional<Vector3> get_transform_member_pos(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space)
{
	return get_transform_member_value<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::GetTransformMemberPos, &pragma::BaseEntityComponent::GetTransformMemberParentPos, parentSpaceOnly>(component, memId, space);
}

template<typename TMemberIdentifier, bool parentSpaceOnly = false>
static std::optional<Quat> get_transform_member_rot(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space)
{
	return get_transform_member_value<TMemberIdentifier, Quat, &pragma::BaseEntityComponent::GetTransformMemberRot, &pragma::BaseEntityComponent::GetTransformMemberParentRot, parentSpaceOnly>(component, memId, space);
}

template<typename TMemberIdentifier, bool parentSpaceOnly = false>
static std::optional<Vector3> get_transform_member_scale(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space)
{
	return get_transform_member_value<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::GetTransformMemberScale, &pragma::BaseEntityComponent::GetTransformMemberParentScale, parentSpaceOnly>(component, memId, space);
}

template<typename TMemberIdentifier, bool parentSpaceOnly = false>
static std::optional<umath::ScaledTransform> get_transform_member_pose(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space)
{
	return get_transform_member_value<TMemberIdentifier, umath::ScaledTransform, &pragma::BaseEntityComponent::GetTransformMemberPose, &pragma::BaseEntityComponent::GetTransformMemberParentPose, parentSpaceOnly>(component, memId, space);
}

template<typename TMemberIdentifier, typename TValue, auto TConvert>
static std::optional<TValue> convert_transform_member_value_to_target_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const TValue &value)
{
	if constexpr(std::is_same_v<TMemberIdentifier, std::string>) {
		auto idx = component.GetMemberIndex(memId);
		if(!idx)
			return {};
		return convert_transform_member_value_to_target_space<pragma::ComponentMemberIndex, TValue, TConvert>(component, *idx, space, value);
	}
	else {
		auto newValue = value;
		if((component.*TConvert)(memId, space, newValue))
			return newValue;
	}
	return {};
}

template<typename TMemberIdentifier>
static std::optional<Vector3> convert_transform_member_pos_to_target_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const Vector3 &pos)
{
	return convert_transform_member_value_to_target_space<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::ConvertTransformMemberPosToTargetSpace>(component, memId, space, pos);
}
template<typename TMemberIdentifier>
static std::optional<Quat> convert_transform_member_rot_to_target_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const Quat &rot)
{
	return convert_transform_member_value_to_target_space<TMemberIdentifier, Quat, &pragma::BaseEntityComponent::ConvertTransformMemberRotToTargetSpace>(component, memId, space, rot);
}
template<typename TMemberIdentifier>
static std::optional<Vector3> convert_transform_member_scale_to_target_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const Vector3 &scale)
{
	return convert_transform_member_value_to_target_space<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::ConvertTransformMemberScaleToTargetSpace>(component, memId, space, scale);
}
template<typename TMemberIdentifier>
static std::optional<umath::ScaledTransform> convert_transform_member_pose_to_target_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const umath::ScaledTransform &pose)
{
	return convert_transform_member_value_to_target_space<TMemberIdentifier, umath::ScaledTransform, &pragma::BaseEntityComponent::ConvertTransformMemberPoseToTargetSpace>(component, memId, space, pose);
}

template<typename TMemberIdentifier, typename T, auto TConvert>
static std::optional<std::pair<T, umath::CoordinateSpace>> convert_value_to_member_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const T &value)
{
	if constexpr(std::is_same_v<TMemberIdentifier, std::string>) {
		auto idx = component.GetMemberIndex(memId);
		if(!idx)
			return {};
		return convert_value_to_member_space<pragma::ComponentMemberIndex, T, TConvert>(component, *idx, space, value);
	}
	else {
		umath::CoordinateSpace memberSpace;
		T newValue = value;
		if(!(component.*TConvert)(memId, space, newValue, &memberSpace))
			return {};
		return std::pair<T, umath::CoordinateSpace> {newValue, memberSpace};
	}
	return {};
}
template<typename TMemberIdentifier, typename T>
static std::optional<std::pair<T, umath::CoordinateSpace>> convert_pos_to_member_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const T &value)
{
	return convert_value_to_member_space<TMemberIdentifier, T, &pragma::BaseEntityComponent::ConvertPosToMemberSpace>(component, memId, space, value);
}
template<typename TMemberIdentifier, typename T>
static std::optional<std::pair<T, umath::CoordinateSpace>> convert_rot_to_member_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const T &value)
{
	return convert_value_to_member_space<TMemberIdentifier, T, &pragma::BaseEntityComponent::ConvertRotToMemberSpace>(component, memId, space, value);
}
template<typename TMemberIdentifier, typename T>
static std::optional<std::pair<T, umath::CoordinateSpace>> convert_scale_to_member_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const T &value)
{
	return convert_value_to_member_space<TMemberIdentifier, T, &pragma::BaseEntityComponent::ConvertScaleToMemberSpace>(component, memId, space, value);
}
template<typename TMemberIdentifier, typename T>
static std::optional<std::pair<T, umath::CoordinateSpace>> convert_pose_to_member_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, umath::CoordinateSpace space, const T &value)
{
	return convert_value_to_member_space<TMemberIdentifier, T, &pragma::BaseEntityComponent::ConvertPoseToMemberSpace>(component, memId, space, value);
}

template<size_t N>
void log_with_args(const pragma::BaseEntityComponent &component, const char *msg, spdlog::level::level_enum logLevel, lua_State *l, int32_t argOffset)
{
	std::array<std::string, N> args;
	for(size_t i = 0; i < args.size(); ++i)
		args[i] = Lua::logging::to_string(l, argOffset + (i + 1));

	auto log = [&](const auto &...elements) { component.Log(logLevel, fmt::vformat(msg, fmt::make_format_args(elements...))); };
	std::apply(log, args);
}

static int log(lua_State *l, spdlog::level::level_enum logLevel)
{
	auto &component = Lua::Check<pragma::BaseEntityComponent>(l, 1);
	const char *msg = Lua::CheckString(l, 2);
	int32_t argOffset = 2;
	auto n = lua_gettop(l) - argOffset; /* number of arguments */
	switch(n) {
	case 0:
		component.Log(logLevel, std::string {msg});
		break;
	case 1:
		{
			log_with_args<1>(component, msg, logLevel, l, argOffset);
			break;
		}
	case 2:
		{
			log_with_args<2>(component, msg, logLevel, l, argOffset);
			break;
		}
	case 3:
		{
			log_with_args<3>(component, msg, logLevel, l, argOffset);
			break;
		}
	case 4:
		{
			log_with_args<4>(component, msg, logLevel, l, argOffset);
			break;
		}
	case 5:
		{
			log_with_args<5>(component, msg, logLevel, l, argOffset);
			break;
		}
	case 6:
		{
			log_with_args<6>(component, msg, logLevel, l, argOffset);
			break;
		}
	case 7:
		{
			log_with_args<7>(component, msg, logLevel, l, argOffset);
			break;
		}
	case 8:
		{
			log_with_args<8>(component, msg, logLevel, l, argOffset);
			break;
		}
	case 9:
		{
			log_with_args<9>(component, msg, logLevel, l, argOffset);
			break;
		}
	case 10:
		{
			log_with_args<10>(component, msg, logLevel, l, argOffset);
			break;
		}
	default:
		component.Log(logLevel, std::string {msg});
		break;
	}
	return 0;
}

static CallbackHandle add_event_callback(lua_State *l, pragma::BaseEntityComponent &hComponent, uint32_t eventId, const Lua::func<void> &function)
{
	auto hCb = hComponent.AddEventCallback(eventId, [l, function](std::reference_wrapper<pragma::ComponentEvent> ev) -> util::EventReply {
		// We need to create a copy of the lua-state pointer, since the callback can remove itself, which
		// would also cause the std::function-object to be destroyed (and therefore the captured variables).
		auto lTmp = l;
		auto oCallbackTmp = function;

		auto nstack = Lua::GetStackTop(l);
		auto c = Lua::CallFunction(
		  l,
		  [&oCallbackTmp, &ev](lua_State *l) -> Lua::StatusCode {
			  oCallbackTmp.push(l);
			  ev.get().PushArguments(l);
			  return Lua::StatusCode::Ok;
		  },
		  LUA_MULTRET);
		auto numRet = Lua::GetStackTop(l) - nstack;
		if(c == Lua::StatusCode::Ok && numRet > 0 && Lua::IsNone(lTmp, -1) == false) {
			auto result = Lua::IsNumber(lTmp, -numRet) ? static_cast<util::EventReply>(Lua::CheckInt(lTmp, -numRet)) : util::EventReply::Unhandled;
			if(result == util::EventReply::Handled)
				ev.get().HandleReturnValues(l);
			Lua::Pop(lTmp, numRet); // Pop result(s)
			return result;
		}
		return util::EventReply::Unhandled;
	});
	return hCb;
}

template<typename TMemberIdentifier>
void register_entity_component_transform_methods(luabind::class_<pragma::BaseEntityComponent> &entityComponentDef)
{
	entityComponentDef.def("GetTransformMemberPos", &get_transform_member_pos<TMemberIdentifier, false>);
	entityComponentDef.def("GetTransformMemberRot", &get_transform_member_rot<TMemberIdentifier, false>);
	entityComponentDef.def("GetTransformMemberScale", &get_transform_member_scale<TMemberIdentifier, false>);
	entityComponentDef.def("GetTransformMemberPose", &get_transform_member_pose<TMemberIdentifier, false>);
	entityComponentDef.def("SetTransformMemberPos", &set_transform_member_pos<TMemberIdentifier>);
	entityComponentDef.def("SetTransformMemberRot", &set_transform_member_rot<TMemberIdentifier>);
	entityComponentDef.def("SetTransformMemberScale", &set_transform_member_scale<TMemberIdentifier>);
	entityComponentDef.def("SetTransformMemberPose", &set_transform_member_pose<TMemberIdentifier>);
	entityComponentDef.def("ConvertTransformMemberPosToTargetSpace", &convert_transform_member_pos_to_target_space<TMemberIdentifier>);
	entityComponentDef.def("ConvertTransformMemberRotToTargetSpace", &convert_transform_member_rot_to_target_space<TMemberIdentifier>);
	entityComponentDef.def("ConvertTransformMemberScaleToTargetSpace", &convert_transform_member_scale_to_target_space<TMemberIdentifier>);
	entityComponentDef.def("ConvertTransformMemberPoseToTargetSpace", &convert_transform_member_pose_to_target_space<TMemberIdentifier>);
	entityComponentDef.def("GetTransformMemberParentPos", &get_transform_member_pos<TMemberIdentifier, true>);
	entityComponentDef.def("GetTransformMemberParentRot", &get_transform_member_rot<TMemberIdentifier, true>);
	entityComponentDef.def("GetTransformMemberParentScale", &get_transform_member_scale<TMemberIdentifier, true>);
	entityComponentDef.def("GetTransformMemberParentPose", &get_transform_member_pose<TMemberIdentifier, true>);
	entityComponentDef.def("ConvertPosToMemberSpace", &convert_pos_to_member_space<TMemberIdentifier, Vector3>);
	entityComponentDef.def("ConvertRotToMemberSpace", &convert_rot_to_member_space<TMemberIdentifier, Quat>);
	entityComponentDef.def("ConvertScaleToMemberSpace", &convert_scale_to_member_space<TMemberIdentifier, Vector3>);
	entityComponentDef.def("ConvertPoseToMemberSpace", &convert_pose_to_member_space<TMemberIdentifier, umath::ScaledTransform>);
}

void pragma::lua::register_entity_component_classes(lua_State *l, luabind::module_ &mod)
{
	auto entityComponentDef = pragma::lua::create_entity_component_class<pragma::BaseEntityComponent>("EntityComponent");
	entityComponentDef.def("BroadcastEvent", static_cast<util::EventReply (pragma::BaseEntityComponent::*)(pragma::ComponentEventId) const>(&pragma::BaseEntityComponent::BroadcastEvent));
	entityComponentDef.def("BroadcastEvent", static_cast<util::EventReply (*)(lua_State *, pragma::BaseEntityComponent &, uint32_t, const luabind::tableT<void> &)>([](lua_State *l, pragma::BaseEntityComponent &hComponent, uint32_t eventId, const luabind::tableT<void> &eventArgs) {
		int32_t t = 3;
		if(pragma::get_engine()->GetNetworkState(l)->GetGameState()->BroadcastEntityEvent(hComponent, eventId, t) == false) {
			LuaComponentEvent luaEvent {};
			auto numArgs = Lua::GetObjectLength(l, t);
			luaEvent.arguments.reserve(numArgs);
			for(auto i = decltype(numArgs) {0u}; i < numArgs; ++i) {
				Lua::PushInt(l, i + 1u);
				Lua::GetTableValue(l, t);
				luaEvent.arguments.push_back(luabind::object(luabind::from_stack(l, -1)));
				Lua::Pop(l, 1);
			}
			auto handled = hComponent.BroadcastEvent(eventId, luaEvent);
			//hComponent->InvokeEventCallbacks(eventId,luaEvent);
			return handled;
		}
		return util::EventReply::Handled;
	}));
	entityComponentDef.def("InvokeEventCallbacks", static_cast<util::EventReply (pragma::BaseEntityComponent::*)(pragma::ComponentEventId) const>(&pragma::BaseEntityComponent::InvokeEventCallbacks));
	entityComponentDef.def("InvokeEventCallbacks", static_cast<util::EventReply (*)(lua_State *, pragma::BaseEntityComponent &, uint32_t, const luabind::tableT<void> &)>([](lua_State *l, pragma::BaseEntityComponent &hComponent, uint32_t eventId, const luabind::tableT<void> &eventArgs) {
		int32_t t = 3;

		LuaComponentEvent luaEvent {};
		auto numArgs = Lua::GetObjectLength(l, t);
		luaEvent.arguments.reserve(numArgs);
		for(auto i = decltype(numArgs) {0u}; i < numArgs; ++i) {
			Lua::PushInt(l, i + 1u);
			Lua::GetTableValue(l, t);
			luaEvent.arguments.push_back(luabind::object(luabind::from_stack(l, -1)));
			Lua::Pop(l, 1);
		}
		return hComponent.InvokeEventCallbacks(eventId, luaEvent);
	}));
	entityComponentDef.def("GetUri", static_cast<std::string (pragma::BaseEntityComponent::*)() const>(&pragma::BaseEntityComponent::GetUri));
	entityComponentDef.def("GetMemberUri", static_cast<std::string (pragma::BaseEntityComponent::*)(const std::string &) const>(&pragma::BaseEntityComponent::GetMemberUri));
	entityComponentDef.def("GetMemberUri", static_cast<std::optional<std::string> (pragma::BaseEntityComponent::*)(ComponentMemberIndex) const>(&pragma::BaseEntityComponent::GetMemberUri));
	entityComponentDef.def("GetEntity", static_cast<BaseEntity &(pragma::BaseEntityComponent::*)()>(&pragma::BaseEntityComponent::GetEntity));
	entityComponentDef.def("GetComponentId", &pragma::BaseEntityComponent::GetComponentId);
	entityComponentDef.def("SetTickPolicy", &pragma::BaseEntityComponent::SetTickPolicy);
	entityComponentDef.def("GetTickPolicy", &pragma::BaseEntityComponent::GetTickPolicy);
	entityComponentDef.def("GetNextTick", &pragma::BaseEntityComponent::GetNextTick);
	entityComponentDef.def("SetNextTick", &pragma::BaseEntityComponent::SetNextTick);
	entityComponentDef.def("SetActive", &pragma::BaseEntityComponent::SetActive);
	entityComponentDef.def("IsActive", &pragma::BaseEntityComponent::IsActive);
	entityComponentDef.def("Activate", &pragma::BaseEntityComponent::Activate);
	entityComponentDef.def("Deactivate", &pragma::BaseEntityComponent::Deactivate);
	entityComponentDef.def("GetMemberIndex", &pragma::BaseEntityComponent::GetMemberIndex);
	entityComponentDef.def("GetMemberInfo", &pragma::BaseEntityComponent::GetMemberInfo);
	entityComponentDef.def("GetDynamicMemberIndices", static_cast<std::vector<pragma::ComponentMemberIndex> (*)(pragma::BaseEntityComponent &)>(&get_dynamic_member_ids));
	entityComponentDef.def("GetStaticMemberCount", &pragma::BaseEntityComponent::GetStaticMemberCount);
	register_entity_component_transform_methods<pragma::ComponentMemberIndex>(entityComponentDef);
	register_entity_component_transform_methods<std::string>(entityComponentDef);
	entityComponentDef.def(
	  "GetTransformMemberSpace", +[](pragma::BaseEntityComponent &c, ComponentMemberIndex memberIndex) -> std::optional<umath::CoordinateSpace> {
		  auto *memberInfo = c.GetMemberInfo(memberIndex);
		  if(!memberInfo)
			  return {};
		  auto *cMetaData = memberInfo->FindTypeMetaData<pragma::ents::CoordinateTypeMetaData>();
		  if(!cMetaData)
			  return {};
		  return cMetaData->space;
	  });
	entityComponentDef.def(
	  "GetMemberIndices", +[](lua_State *l, pragma::BaseEntityComponent &component) -> std::vector<pragma::ComponentMemberIndex> {
		  std::vector<pragma::ComponentMemberIndex> memberIndices;
		  auto numStaticMembers = component.GetStaticMemberCount();
		  memberIndices.reserve(numStaticMembers);
		  for(auto i = decltype(numStaticMembers) {0u}; i < numStaticMembers; ++i)
			  memberIndices.push_back(i);
		  get_dynamic_member_ids(component, memberIndices);
		  return memberIndices;
	  });
	entityComponentDef.def(
	  "InvokeElementMemberChangeCallback", +[](pragma::BaseEntityComponent &component, const std::string &memberName) {
		  auto *info = component.FindMemberInfo(memberName);
		  if(!info || info->type != ents::EntityMemberType::Element)
			  return;
		  ents::Element el;
		  info->getterFunction(*info, component, &el);
		  // The property will not actually be changed, but this will invoke the setter-function,
		  // which acts as a change callback.
		  info->setterFunction(*info, component, &el);
	  });
	entityComponentDef.def(
	  "InvokeElementMemberChangeCallback", +[](pragma::BaseEntityComponent &component, uint32_t memberIndex) {
		  auto *info = component.GetMemberInfo(memberIndex);
		  if(!info || info->type != ents::EntityMemberType::Element)
			  return;
		  ents::Element el;
		  info->getterFunction(*info, component, &el);
		  // The property will not actually be changed, but this will invoke the setter-function,
		  // which acts as a change callback.
		  info->setterFunction(*info, component, &el);
	  });
	entityComponentDef.def("GetMemberValue", &get_member_value);
	entityComponentDef.def(
	  "GetMemberValue", +[](lua_State *l, pragma::BaseEntityComponent &component, const std::string &memberName) -> std::optional<Lua::udm_type> {
		  auto *info = component.FindMemberInfo(memberName);
		  if(!info)
			  return {};
		  return get_member_value(l, component, *info);
	  });
	entityComponentDef.def(
	  "GetMemberValue", +[](lua_State *l, pragma::BaseEntityComponent &component, uint32_t memberIndex) -> std::optional<Lua::udm_type> {
		  auto *info = component.GetMemberInfo(memberIndex);
		  if(!info)
			  return {};
		  return get_member_value(l, component, *info);
	  });
	entityComponentDef.def("SetMemberValue", static_cast<bool (*)(lua_State *, pragma::BaseEntityComponent &, const pragma::ComponentMemberInfo &, Lua::udm_type)>(&set_member_value));
	entityComponentDef.def("SetMemberValue", static_cast<bool (*)(lua_State *, pragma::BaseEntityComponent &, const pragma::ComponentMemberInfo &, const pragma::EntityURef &)>(&set_member_value));
	entityComponentDef.def("SetMemberValue", static_cast<bool (*)(lua_State *, pragma::BaseEntityComponent &, const pragma::ComponentMemberInfo &, const pragma::MultiEntityURef &)>(&set_member_value));
	entityComponentDef.def("SetMemberValue", static_cast<bool (*)(lua_State *, pragma::BaseEntityComponent &, const pragma::ComponentMemberInfo &, const pragma::EntityUComponentMemberRef &)>(&set_member_value));
	entityComponentDef.def(
	  "SetMemberValue", +[](lua_State *l, pragma::BaseEntityComponent &component, uint32_t memberIndex, Lua::udm_type value) -> bool {
		  auto *info = component.GetMemberInfo(memberIndex);
		  if(!info)
			  return false;
		  return pragma::lua::set_member_value(l, component, *info, value);
	  });
	entityComponentDef.def(
	  "SetMemberValue", +[](lua_State *l, pragma::BaseEntityComponent &component, const std::string &memberName, Lua::udm_type value) -> bool {
		  auto *info = component.FindMemberInfo(memberName);
		  if(!info)
			  return false;
		  return pragma::lua::set_member_value(l, component, *info, value);
	  });
	entityComponentDef.def(
	  "SetMemberValue", +[](lua_State *l, pragma::BaseEntityComponent &component, const std::string &memberName, const pragma::EntityURef &eref) -> bool {
		  auto *info = component.FindMemberInfo(memberName);
		  if(!info)
			  return false;
		  return pragma::lua::set_member_value(l, component, *info, eref);
	  });
	entityComponentDef.def(
	  "SetMemberValue", +[](lua_State *l, pragma::BaseEntityComponent &component, const std::string &memberName, const pragma::MultiEntityURef &eref) -> bool {
		  auto *info = component.FindMemberInfo(memberName);
		  if(!info)
			  return false;
		  return pragma::lua::set_member_value(l, component, *info, eref);
	  });
	entityComponentDef.def(
	  "SetMemberValue", +[](lua_State *l, pragma::BaseEntityComponent &component, const std::string &memberName, const pragma::EntityUComponentMemberRef &eref) -> bool {
		  auto *info = component.FindMemberInfo(memberName);
		  if(!info)
			  return false;
		  return pragma::lua::set_member_value(l, component, *info, eref);
	  });
	entityComponentDef.def("IsValid", static_cast<bool (*)(lua_State *, pragma::BaseEntityComponent *)>([](lua_State *l, pragma::BaseEntityComponent *hComponent) { return hComponent != nullptr; }));
	entityComponentDef.def(
	  "RegisterNetEvent", +[](lua_State *l, pragma::BaseEntityComponent &hComponent, const std::string &eventName) {
		  auto id = hComponent.SetupNetEvent(eventName);
		  auto *nw = pragma::get_engine()->GetNetworkState(l);
		  auto *game = nw->GetGameState();
		  auto *componentInfo = game->GetEntityComponentManager().GetComponentInfo(hComponent.GetComponentId());
		  if(componentInfo && umath::is_flag_set(componentInfo->flags, pragma::ComponentFlags::Networked) == false)
			  ::operator<<(::operator<<(::operator<<(::operator<<(Con::cwar, "WARNING: Component '"), componentInfo->name),
			                 "' has uses net-events, but was not registered as networked, this means networking will be disabled for this component! Set the 'ents.EntityComponent.FREGISTER_BIT_NETWORKED' flag when registering the component to fix this!"),
			    Con::endl);
		  return id;
	  });
	entityComponentDef.def("GetComponentName", static_cast<std::string (*)(lua_State *, pragma::BaseEntityComponent &)>([](lua_State *l, pragma::BaseEntityComponent &component) {
		auto *nw = pragma::get_engine()->GetNetworkState(l);
		auto *game = nw->GetGameState();
		auto &componentManager = game->GetEntityComponentManager();
		auto &info = *componentManager.GetComponentInfo(component.GetComponentId());
		return std::string {*info.name};
	}));
	entityComponentDef.def("AddEventCallback", &add_event_callback);
	entityComponentDef.def("AddEventListener", &add_event_callback); // Alias
	entityComponentDef.def("InjectEvent", static_cast<util::EventReply (pragma::BaseEntityComponent::*)(pragma::ComponentEventId)>(&pragma::BaseEntityComponent::InjectEvent));
	entityComponentDef.def("InjectEvent", static_cast<void (*)(lua_State *, pragma::BaseEntityComponent &, uint32_t, const luabind::tableT<void> &)>([](lua_State *l, pragma::BaseEntityComponent &hComponent, uint32_t eventId, const luabind::tableT<void> &eventArgs) {
		int32_t t = 3;
		if(pragma::get_engine()->GetNetworkState(l)->GetGameState()->InjectEntityEvent(hComponent, eventId, t) == false) {
			LuaComponentEvent luaEvent {};
			auto numArgs = Lua::GetObjectLength(l, t);
			luaEvent.arguments.reserve(numArgs);
			for(auto i = decltype(numArgs) {0u}; i < numArgs; ++i) {
				Lua::PushInt(l, i + 1u);
				Lua::GetTableValue(l, t);
				luaEvent.arguments.push_back(luabind::object(luabind::from_stack(l, -1)));
				Lua::Pop(l, 1);
			}
			hComponent.InjectEvent(eventId, luaEvent);
		}
	}));
	entityComponentDef.def("Save", &pragma::BaseEntityComponent::Save);
	entityComponentDef.def("Load", static_cast<void (pragma::BaseEntityComponent::*)(udm::LinkedPropertyWrapperArg)>(&pragma::BaseEntityComponent::Load));
	entityComponentDef.def("Copy", static_cast<void (*)(lua_State *, pragma::BaseEntityComponent &, pragma::BaseEntityComponent &)>([](lua_State *l, pragma::BaseEntityComponent &hComponent, pragma::BaseEntityComponent &hComponentOther) {
		if(hComponent.GetComponentId() != hComponentOther.GetComponentId() || &hComponent == &hComponentOther)
			return;
		auto el = udm::Property::Create<udm::Element>();
		udm::LinkedPropertyWrapper prop {*el};
		hComponent.Save(prop);
		hComponentOther.Load(prop);
	}));
	entityComponentDef.def("GetMemberInfos", static_cast<luabind::tableT<pragma::ComponentMemberInfo> (*)(lua_State *, pragma::BaseEntityComponent &)>([](lua_State *l, pragma::BaseEntityComponent &hComponent) -> luabind::tableT<pragma::ComponentMemberInfo> {
		auto t = luabind::newtable(l);
		uint32_t idx = 0;
		auto *memberInfo = hComponent.GetMemberInfo(idx++);
		while(memberInfo) {
			t[idx] = memberInfo;
			memberInfo = hComponent.GetMemberInfo(idx++);
		}
		return t;
	}));
	entityComponentDef.def("Log", static_cast<void (pragma::BaseEntityComponent::*)(const std::string &, pragma::BaseEntityComponent::LogSeverity) const>(&pragma::BaseEntityComponent::Log));
	entityComponentDef.def("Log", static_cast<void (pragma::BaseEntityComponent::*)(const std::string &, pragma::BaseEntityComponent::LogSeverity) const>(&pragma::BaseEntityComponent::Log), luabind::default_parameter_policy<3, pragma::BaseEntityComponent::LogSeverity::Warning> {});
	entityComponentDef.def("SetPropertyAnimated", &pragma::BaseEntityComponent::SetPropertyAnimated);
	entityComponentDef.def("IsPropertyAnimated", &pragma::BaseEntityComponent::IsPropertyAnimated);
	entityComponentDef.add_static_constant("FREGISTER_NONE", umath::to_integral(pragma::ComponentFlags::None));
	entityComponentDef.add_static_constant("FREGISTER_BIT_NETWORKED", umath::to_integral(pragma::ComponentFlags::Networked));
	entityComponentDef.add_static_constant("FREGISTER_BIT_HIDE_IN_EDITOR", umath::to_integral(pragma::ComponentFlags::HideInEditor));

	entityComponentDef.add_static_constant("CALLBACK_TYPE_ENTITY", umath::to_integral(pragma::BaseEntityComponent::CallbackType::Entity));
	entityComponentDef.add_static_constant("CALLBACK_TYPE_COMPONENT", umath::to_integral(pragma::BaseEntityComponent::CallbackType::Component));
	entityComponentDef.add_static_constant("LOG_SEVERITY_NORMAL", umath::to_integral(pragma::BaseEntityComponent::LogSeverity::Normal));
	entityComponentDef.add_static_constant("LOG_SEVERITY_WARNING", umath::to_integral(pragma::BaseEntityComponent::LogSeverity::Warning));
	entityComponentDef.add_static_constant("LOG_SEVERITY_ERROR", umath::to_integral(pragma::BaseEntityComponent::LogSeverity::Error));
	entityComponentDef.add_static_constant("LOG_SEVERITY_CRITICAL", umath::to_integral(pragma::BaseEntityComponent::LogSeverity::Critical));
	entityComponentDef.add_static_constant("LOG_SEVERITY_DEBUG", umath::to_integral(pragma::BaseEntityComponent::LogSeverity::Debug));
	mod[entityComponentDef];

	luabind::object oLogger = luabind::globals(l)["ents"];
	oLogger = oLogger["EntityComponent"];
	Lua::logging::add_log_func<spdlog::level::trace, &::log>(l, oLogger, "LogTrace");
	Lua::logging::add_log_func<spdlog::level::debug, &::log>(l, oLogger, "LogDebug");
	Lua::logging::add_log_func<spdlog::level::info, &::log>(l, oLogger, "LogInfo");
	Lua::logging::add_log_func<spdlog::level::warn, &::log>(l, oLogger, "LogWarn");
	Lua::logging::add_log_func<spdlog::level::err, &::log>(l, oLogger, "LogErr");
	Lua::logging::add_log_func<spdlog::level::critical, &::log>(l, oLogger, "LogCritical");

	auto defBvh = Lua::create_base_entity_component_class<pragma::BaseBvhComponent>("BaseBvhComponent");

	auto defIntersectionMeshInfo = luabind::class_<pragma::MeshIntersectionInfo::MeshInfo>("IntersectionMeshInfo");
	defIntersectionMeshInfo.def_readonly("mesh", &pragma::MeshIntersectionInfo::MeshInfo::mesh);
	defIntersectionMeshInfo.def_readonly("entity", &pragma::MeshIntersectionInfo::MeshInfo::entity);
	defBvh.scope[defIntersectionMeshInfo];

	defBvh.def("RebuildBvh", static_cast<void (pragma::BaseBvhComponent::*)()>(&pragma::BaseBvhComponent::RebuildBvh));
	defBvh.def("GetVertex", &pragma::BaseBvhComponent::GetVertex);
	defBvh.def("GetTriangleCount", &pragma::BaseBvhComponent::GetTriangleCount);
	defBvh.def("DebugPrint", &pragma::BaseBvhComponent::DebugPrint);
	defBvh.def("DebugDraw", &pragma::BaseBvhComponent::DebugDraw);
	defBvh.def("DebugDrawBvhTree", &pragma::BaseBvhComponent::DebugDrawBvhTree);
	defBvh.def(
	  "FindPrimitiveMeshInfo", +[](lua_State *l, const pragma::BaseBvhComponent &bvhC, size_t primIdx) -> std::optional<std::pair<EntityHandle, std::shared_ptr<ModelSubMesh>>> {
		  auto *range = bvhC.FindPrimitiveMeshInfo(primIdx);
		  if(!range)
			  return std::optional<std::pair<EntityHandle, std::shared_ptr<ModelSubMesh>>> {};
		  auto *ent = range->entity ? range->entity : &bvhC.GetEntity();
		  return std::pair<EntityHandle, std::shared_ptr<ModelSubMesh>> {ent->GetHandle(), range->mesh};
	  });

	/*auto defBvhIntersectionInfo = luabind::class_<pragma::BvhIntersectionInfo>("IntersectionInfo");
	defBvhIntersectionInfo.def_readonly("primitives",&pragma::BvhIntersectionInfo::primitives);
	defBvh.scope[defBvhIntersectionInfo];

	auto defBvhMeshIntersectionInfo = luabind::class_<pragma::BvhMeshIntersectionInfo,pragma::BvhIntersectionInfo>("MeshIntersectionInfo");
	defBvh.scope[defBvhMeshIntersectionInfo];*/
	mod[defBvh];

	auto defStaticBvh = pragma::lua::create_entity_component_class<pragma::BaseStaticBvhCacheComponent, pragma::BaseBvhComponent>("BaseStaticBvhCacheComponent");
	defStaticBvh.def("SetEntityDirty", &pragma::BaseStaticBvhCacheComponent::SetEntityDirty);
	defStaticBvh.def("AddEntity", &pragma::BaseStaticBvhCacheComponent::AddEntity);
	defStaticBvh.def("RemoveEntity", +[](pragma::BaseStaticBvhCacheComponent &component, BaseEntity &ent) { component.RemoveEntity(ent); });
	mod[defStaticBvh];

	auto defStaticBvhUser = Lua::create_base_entity_component_class<pragma::BaseStaticBvhUserComponent>("BaseStaticBvhUserComponent");
	defStaticBvhUser.add_static_constant("EVENT_ON_ACTIVATION_STATE_CHANGED", pragma::BaseStaticBvhUserComponent::EVENT_ON_ACTIVATION_STATE_CHANGED);
	defStaticBvhUser.add_static_constant("EVENT_ON_STATIC_BVH_COMPONENT_CHANGED", pragma::BaseStaticBvhUserComponent::EVENT_ON_STATIC_BVH_COMPONENT_CHANGED);
	defStaticBvhUser.def("IsActive", &pragma::BaseStaticBvhUserComponent::IsActive);
	mod[defStaticBvhUser];

	base_ai_component::register_class(mod);
	base_animated_component::register_class(mod);
	base_attachable_component::register_class(mod);
	base_child_component::register_class(mod);
	base_bot_component::register_class(mod);
	base_character_component::register_class(mod);
	base_color_component::register_class(mod);
	base_surface_component::register_class(mod);
	base_debug_box_component::register_class(mod);
	base_debug_cone_component::register_class(mod);
	base_debug_cylinder_component::register_class(mod);
	base_debug_line_component::register_class(mod);
	base_debug_plane_component::register_class(mod);
	base_debug_point_component::register_class(mod);
	base_debug_sphere_component::register_class(mod);
	base_debug_text_component::register_class(mod);
	base_decal_component::register_class(mod);
	base_env_camera_component::register_class(mod);
	base_env_explosion_component::register_class(mod);
	base_env_filter_class_component::register_class(mod);
	base_env_filter_name_component::register_class(mod);
	base_env_fire_component::register_class(mod);
	base_env_fog_controller_component::register_class(mod);
	base_env_light_component::register_class(mod);
	base_env_light_directional_component::register_class(mod);
	base_env_light_point_component::register_class(mod);
	base_env_light_spot_component::register_class(mod);
	base_env_light_spot_vol_component::register_class(mod);
	base_env_microphone_component::register_class(mod);
	base_env_particle_system_component::register_class(mod);
	base_env_quake_component::register_class(mod);
	base_env_smoke_trail_component::register_class(mod);
	base_env_soundscape_component::register_class(mod);
	base_env_sound_component::register_class(mod);
	base_env_sound_dsp_component::register_class(mod);
	base_env_sprite_component::register_class(mod);
	base_env_timescale_component::register_class(mod);
	base_env_weather_component::register_class(mod);
	base_env_wind_component::register_class(mod);
	base_flammable_component::register_class(mod);
	base_flashlight_component::register_class(mod);
	base_flex_component::register_class(mod);
	base_func_brush_component::register_class(mod);
	base_func_button_component::register_class(mod);
	base_func_kinematic_component::register_class(mod);
	base_func_physics_component::register_class(mod);
	base_func_portal_component::register_class(mod);
	base_func_soft_physics_component::register_class(mod);
	base_func_surface_material_component::register_class(mod);
	base_func_water_component::register_class(mod);
	base_gamemode_component::register_class(mod);
	base_generic_component::register_class(mod);
	base_health_component::register_class(mod);
	base_info_landmark_component::register_class(mod);
	base_io_component::register_class(mod);
	base_logic_relay_component::register_class(mod);
	base_model_component::register_class(mod);
	base_name_component::register_class(mod);
	base_networked_component::register_class(mod);
	base_observable_component::register_class(mod);
	base_observer_component::register_class(mod);
	base_ownable_component::register_class(mod);
	base_physics_component::register_class(mod);
	base_player_component::register_class(mod);
	base_point_at_target_component::register_class(mod);
	base_point_constraint_ball_socket_component::register_class(mod);
	base_point_constraint_cone_twist_component::register_class(mod);
	base_point_constraint_dof_component::register_class(mod);
	base_point_constraint_fixed_component::register_class(mod);
	base_point_constraint_hinge_component::register_class(mod);
	base_point_constraint_slider_component::register_class(mod);
	base_point_path_node_component::register_class(mod);
	base_point_render_target_component::register_class(mod);
	base_point_target_component::register_class(mod);
	base_prop_component::register_class(mod);
	base_prop_dynamic_component::register_class(mod);
	base_prop_physics_component::register_class(mod);
	base_radius_component::register_class(mod);
	base_field_angle_component::register_class(mod);
	base_render_component::register_class(mod);
	base_score_component::register_class(mod);
	base_shooter_component::register_class(mod);
	base_skybox_component::register_class(mod);
	base_soft_body_component::register_class(mod);
	base_sound_emitter_component::register_class(mod);
	base_time_scale_component::register_class(mod);
	base_toggle_component::register_class(mod);
	base_touch_component::register_class(mod);
	base_transform_component::register_class(mod);
	base_trigger_gravity_component::register_class(mod);
	base_trigger_hurt_component::register_class(mod);
	base_trigger_push_component::register_class(mod);
	base_trigger_remove_component::register_class(mod);
	base_trigger_teleport_component::register_class(mod);
	base_vehicle_component::register_class(mod);
	base_weapon_component::register_class(mod);
	base_wheel_component::register_class(mod);
	base_world_component::register_class(mod);
	base_liquid_control_component::register_class(mod);
	base_liquid_surface_simulation_component::register_class(mod);
	// --template-register-call-location

	auto defGameComponent = Lua::create_base_entity_component_class<pragma::BaseGameComponent>("BaseGameComponent");
	mod[defGameComponent];
}
void pragma::lua::base_child_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseChildComponent>("BaseChildComponent");
	def.add_static_constant("EVENT_ON_PARENT_CHANGED", pragma::BaseChildComponent::EVENT_ON_PARENT_CHANGED);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("ClearParent", &pragma::BaseChildComponent::ClearParent);
	def.def("SetParent", &pragma::BaseChildComponent::SetParent);
	def.def("GetParent", &pragma::BaseChildComponent::GetParent, luabind::copy_policy<0> {});
	def.def("GetParentEntity", static_cast<BaseEntity *(pragma::BaseChildComponent::*)()>(&pragma::BaseChildComponent::GetParentEntity));
	def.def("HasParent", &pragma::BaseChildComponent::HasParent);
}
void pragma::lua::base_attachable_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseAttachmentComponent>("BaseAttachmentComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("AttachToEntity", &pragma::BaseAttachmentComponent::AttachToEntity, luabind::discard_result {});
	def.def("AttachToEntity", static_cast<AttachmentData *(*)(pragma::BaseAttachmentComponent &, BaseEntity *)>([](pragma::BaseAttachmentComponent &component, BaseEntity *ent) { return component.AttachToEntity(ent); }), luabind::discard_result {});
	//AttachmentData *AttachToEntity(BaseEntity *ent,const AttachmentInfo &attInfo={});
	//def.def("AttachToEntity",&pragma::BaseAttachmentComponent::AttachToEntity,luabind::default_parameter_policy<0,AttachmentInfo,att>{});
	//def.def("AttachToEntity",std::bind(&pragma::BaseAttachmentComponent::AttachToEntity,std::placeholders::_1,AttachmentInfo{}));
	def.def("ClearAttachment", &pragma::BaseAttachmentComponent::ClearAttachment);
	def.def("AttachToAttachment", static_cast<AttachmentData *(pragma::BaseAttachmentComponent::*)(BaseEntity *, uint32_t, const AttachmentInfo &)>(&pragma::BaseAttachmentComponent::AttachToAttachment), luabind::discard_result {});
	def.def("AttachToAttachment", static_cast<AttachmentData *(*)(pragma::BaseAttachmentComponent &, BaseEntity *, uint32_t)>([](pragma::BaseAttachmentComponent &component, BaseEntity *ent, uint32_t attIdx) { return component.AttachToAttachment(ent, attIdx); }),
	  luabind::discard_result {});
	//def.def("AttachToAttachment",std::bind(static_cast<AttachmentData*(pragma::BaseAttachmentComponent::*)(BaseEntity*,uint32_t,const AttachmentInfo&)>(&pragma::BaseAttachmentComponent::AttachToAttachment),std::placeholders::_1,std::placeholders::_2,AttachmentInfo{}));
	def.def("AttachToAttachment", static_cast<AttachmentData *(pragma::BaseAttachmentComponent::*)(BaseEntity *, std::string, const AttachmentInfo &)>(&pragma::BaseAttachmentComponent::AttachToAttachment), luabind::discard_result {});
	def.def("AttachToAttachment", static_cast<AttachmentData *(*)(pragma::BaseAttachmentComponent &, BaseEntity *, std::string)>([](pragma::BaseAttachmentComponent &component, BaseEntity *ent, std::string att) { return component.AttachToAttachment(ent, att); }),
	  luabind::discard_result {});
	//def.def("AttachToAttachment",std::bind(static_cast<AttachmentData*(pragma::BaseAttachmentComponent::*)(BaseEntity*,std::string,const AttachmentInfo&)>(&pragma::BaseAttachmentComponent::AttachToAttachment),std::placeholders::_1,std::placeholders::_2,AttachmentInfo{}));
	def.def("AttachToBone",
	  static_cast<void (*)(lua_State *, pragma::BaseAttachmentComponent &, BaseEntity &, std::string, AttachmentInfo &)>([](lua_State *l, pragma::BaseAttachmentComponent &hEnt, BaseEntity &parent, std::string bone, AttachmentInfo &attInfo) { hEnt.AttachToBone(&parent, bone, attInfo); }),
	  luabind::discard_result {});
	def.def("AttachToBone", static_cast<void (*)(lua_State *, pragma::BaseAttachmentComponent &, BaseEntity &, std::string)>([](lua_State *l, pragma::BaseAttachmentComponent &hEnt, BaseEntity &parent, std::string bone) { hEnt.AttachToBone(&parent, bone); }), luabind::discard_result {});
	def.def("AttachToBone",
	  static_cast<void (*)(lua_State *, pragma::BaseAttachmentComponent &, BaseEntity &, int, AttachmentInfo &)>([](lua_State *l, pragma::BaseAttachmentComponent &hEnt, BaseEntity &parent, int bone, AttachmentInfo &attInfo) { hEnt.AttachToBone(&parent, bone, attInfo); }),
	  luabind::discard_result {});
	def.def("AttachToBone", static_cast<void (*)(lua_State *, pragma::BaseAttachmentComponent &, BaseEntity &, int)>([](lua_State *l, pragma::BaseAttachmentComponent &hEnt, BaseEntity &parent, int bone) { hEnt.AttachToBone(&parent, bone); })), luabind::discard_result {};
	def.def("GetLocalPose", &pragma::BaseAttachmentComponent::GetLocalPose);
	def.def("SetLocalPose", &pragma::BaseAttachmentComponent::SetLocalPose);
	def.def("GetParent", &pragma::BaseAttachmentComponent::GetParent);

	def.def("GetBone", static_cast<luabind::object (*)(lua_State *, pragma::BaseAttachmentComponent &)>([](lua_State *l, pragma::BaseAttachmentComponent &hEnt) -> luabind::object {
		auto *data = hEnt.GetAttachmentData();
		if(data == nullptr)
			return luabind::object {};
		return luabind::object {l, data->bone};
	}));
	def.def("SetAttachmentFlags", &pragma::BaseAttachmentComponent::SetAttachmentFlags);
	def.def("GetAttachmentFlags", &pragma::BaseAttachmentComponent::GetAttachmentFlags);

	auto defAttInfo = luabind::class_<AttachmentInfo>("AttachmentInfo");
	defAttInfo.def(luabind::constructor<>());
	defAttInfo.def("SetOffset", static_cast<void (*)(lua_State *, AttachmentInfo &, const Vector3 &)>([](lua_State *l, AttachmentInfo &attInfo, const Vector3 &offset) { attInfo.offset = offset; }));
	defAttInfo.def("SetOffset", static_cast<void (*)(lua_State *, AttachmentInfo &)>([](lua_State *l, AttachmentInfo &attInfo) { attInfo.offset.reset(); }));
	defAttInfo.def("SetRotation", static_cast<void (*)(lua_State *, AttachmentInfo &, const Quat &)>([](lua_State *l, AttachmentInfo &attInfo, const Quat &rotation) { attInfo.rotation = rotation; }));
	defAttInfo.def("SetRotation", static_cast<void (*)(lua_State *, AttachmentInfo &)>([](lua_State *l, AttachmentInfo &attInfo) { attInfo.rotation.reset(); }));
	defAttInfo.def_readwrite("flags", reinterpret_cast<std::underlying_type_t<decltype(AttachmentInfo::flags)> AttachmentInfo::*>(&AttachmentInfo::flags));
	def.scope[defAttInfo];

	def.add_static_constant("EVENT_ON_ATTACHMENT_UPDATE", pragma::BaseAttachmentComponent::EVENT_ON_ATTACHMENT_UPDATE);

	def.add_static_constant("FATTACHMENT_MODE_POSITION_ONLY", umath::to_integral(FAttachmentMode::PositionOnly));
	def.add_static_constant("FATTACHMENT_MODE_BONEMERGE", umath::to_integral(FAttachmentMode::BoneMerge));
	def.add_static_constant("FATTACHMENT_MODE_UPDATE_EACH_FRAME", umath::to_integral(FAttachmentMode::UpdateEachFrame));
	def.add_static_constant("FATTACHMENT_MODE_PLAYER_VIEW", umath::to_integral(FAttachmentMode::PlayerView));
	def.add_static_constant("FATTACHMENT_MODE_PLAYER_VIEW_YAW", umath::to_integral(FAttachmentMode::PlayerViewYaw));
	def.add_static_constant("FATTACHMENT_MODE_SNAP_TO_ORIGIN", umath::to_integral(FAttachmentMode::SnapToOrigin));
	def.add_static_constant("FATTACHMENT_MODE_FORCE_TRANSLATION_IN_PLACE", umath::to_integral(FAttachmentMode::ForceTranslationInPlace));
	def.add_static_constant("FATTACHMENT_MODE_FORCE_ROTATION_IN_PLACE", umath::to_integral(FAttachmentMode::ForceRotationInPlace));
	def.add_static_constant("FATTACHMENT_MODE_FORCE_IN_PLACE", umath::to_integral(FAttachmentMode::ForceInPlace));
}

#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/model/animation/animation.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/model/model.h"
namespace pragma {
	class BaseFuncLiquidComponent;
};

namespace Lua {
	DLLNETWORK bool get_bullet_master(BaseEntity &ent);
	DLLNETWORK AnimationEvent get_animation_event(lua_State *l, int32_t tArgs, uint32_t eventId);
	namespace Animated {
		static pragma::animation::BoneId get_bone_id(const pragma::BaseAnimatedComponent &hAnim, uint32_t boneId) { return boneId; }
		static pragma::animation::BoneId get_bone_id(const pragma::BaseAnimatedComponent &hAnim, const std::string &boneId)
		{
			auto &ent = hAnim.GetEntity();
			auto &mdl = ent.GetModel();
			if(!mdl)
				return std::numeric_limits<pragma::animation::BoneId>::max();
			return mdl->LookupBone(boneId);
		}
	};
};
#include "pragma/model/modelmesh.h"

template<typename TResult, typename TBoneIdentifier, bool (pragma::BaseAnimatedComponent::*GetValue)(pragma::animation::BoneId, TResult &, umath::CoordinateSpace) const>
std::optional<TResult> get_bone_value(const pragma::BaseAnimatedComponent &animC, TBoneIdentifier boneIdentifier, umath::CoordinateSpace space)
{
	pragma::animation::BoneId boneId;
	if constexpr(std::is_same_v<TBoneIdentifier, const std::string &>) {
		auto &mdl = animC.GetEntity().GetModel();
		if(!mdl)
			return {};
		boneId = mdl->LookupBone(boneIdentifier);
	}
	else
		boneId = boneIdentifier;
	TResult result;
	if(!(animC.*GetValue)(boneId, result, space))
		return {};
	return result;
}
template<typename TResult, typename TBoneIdentifier, bool (pragma::BaseAnimatedComponent::*GetValue)(pragma::animation::BoneId, TResult &, umath::CoordinateSpace) const>
std::optional<TResult> get_bone_value_os(const pragma::BaseAnimatedComponent &animC, TBoneIdentifier boneIdentifier)
{
	return get_bone_value<TResult, TBoneIdentifier, GetValue>(animC, boneIdentifier, umath::CoordinateSpace::Object);
}
template<typename TResult, typename TBoneIdentifier, bool (pragma::BaseAnimatedComponent::*GetValue)(pragma::animation::BoneId, TResult &, umath::CoordinateSpace) const>
std::optional<TResult> get_bone_value_ls(const pragma::BaseAnimatedComponent &animC, TBoneIdentifier boneIdentifier)
{
	return get_bone_value<TResult, TBoneIdentifier, GetValue>(animC, boneIdentifier, umath::CoordinateSpace::Local);
}

template<typename TValue, typename TBoneIdentifier, bool (pragma::BaseAnimatedComponent::*SetValue)(pragma::animation::BoneId, const TValue &, umath::CoordinateSpace)>
bool set_bone_value(pragma::BaseAnimatedComponent &animC, TBoneIdentifier boneIdentifier, const TValue &value, umath::CoordinateSpace space)
{
	pragma::animation::BoneId boneId;
	if constexpr(std::is_same_v<TBoneIdentifier, const std::string &>) {
		auto &mdl = animC.GetEntity().GetModel();
		if(!mdl)
			return {};
		boneId = mdl->LookupBone(boneIdentifier);
	}
	else
		boneId = boneIdentifier;
	return (animC.*SetValue)(boneId, value, space);
}

template<typename TValue, typename TBoneIdentifier, bool (pragma::BaseAnimatedComponent::*SetValue)(pragma::animation::BoneId, const TValue &, umath::CoordinateSpace)>
bool set_bone_value_ls(pragma::BaseAnimatedComponent &animC, TBoneIdentifier boneIdentifier, const TValue &value)
{
	return set_bone_value<TValue, TBoneIdentifier, SetValue>(animC, boneIdentifier, value, umath::CoordinateSpace::Local);
}

template<typename TBoneId>
static void register_base_animated_component_bone_methods(luabind::class_<pragma::BaseAnimatedComponent, pragma::BaseEntityComponent> &classDef)
{
	// Note: luabind::default_parameter_policy would be a better choice here, but doesn't work for the CoordinateSpace parameter for some unknown reason
	classDef.def("GetReferenceBonePose", &get_bone_value_os<umath::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBonePose>);
	classDef.def("GetReferenceBonePos", &get_bone_value_os<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBonePos>);
	classDef.def("GetReferenceBoneRot", &get_bone_value_os<Quat, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBoneRot>);
	classDef.def("GetReferenceBoneScale", &get_bone_value_os<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBoneScale>);

	classDef.def("GetReferenceBonePose", &get_bone_value<umath::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBonePose>);
	classDef.def("GetReferenceBonePos", &get_bone_value<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBonePos>);
	classDef.def("GetReferenceBoneRot", &get_bone_value<Quat, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBoneRot>);
	classDef.def("GetReferenceBoneScale", &get_bone_value<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBoneScale>);

	classDef.def("GetBonePose", &get_bone_value_ls<umath::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::GetBonePose>);
	classDef.def("GetBonePos", &get_bone_value_ls<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetBonePos>);
	classDef.def("GetBoneRot", &get_bone_value_ls<Quat, TBoneId, &pragma::BaseAnimatedComponent::GetBoneRot>);
	classDef.def("GetBoneScale", &get_bone_value_ls<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetBoneScale>);

	classDef.def("GetBonePose", &get_bone_value<umath::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::GetBonePose>);
	classDef.def("GetBonePos", &get_bone_value<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetBonePos>);
	classDef.def("GetBoneRot", &get_bone_value<Quat, TBoneId, &pragma::BaseAnimatedComponent::GetBoneRot>);
	classDef.def("GetBoneScale", &get_bone_value<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetBoneScale>);

	classDef.def("SetBonePose", &set_bone_value_ls<umath::Transform, TBoneId, &pragma::BaseAnimatedComponent::SetBonePose>);
	classDef.def("SetBonePose", &set_bone_value_ls<umath::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::SetBonePose>);
	classDef.def("SetBonePos", &set_bone_value_ls<Vector3, TBoneId, &pragma::BaseAnimatedComponent::SetBonePos>);
	classDef.def("SetBoneRot", &set_bone_value_ls<Quat, TBoneId, &pragma::BaseAnimatedComponent::SetBoneRot>);
	classDef.def("SetBoneScale", &set_bone_value_ls<Vector3, TBoneId, &pragma::BaseAnimatedComponent::SetBoneScale>);

	classDef.def("SetBonePose", &set_bone_value<umath::Transform, TBoneId, &pragma::BaseAnimatedComponent::SetBonePose>);
	classDef.def("SetBonePose", &set_bone_value<umath::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::SetBonePose>);
	classDef.def("SetBonePos", &set_bone_value<Vector3, TBoneId, &pragma::BaseAnimatedComponent::SetBonePos>);
	classDef.def("SetBoneRot", &set_bone_value<Quat, TBoneId, &pragma::BaseAnimatedComponent::SetBoneRot>);
	classDef.def("SetBoneScale", &set_bone_value<Vector3, TBoneId, &pragma::BaseAnimatedComponent::SetBoneScale>);

	classDef.def(
	  "GetBoneMatrix", +[](const pragma::BaseAnimatedComponent &animC, TBoneId boneIdentifier) -> std::optional<Mat4> {
		  pragma::animation::BoneId boneId;
		  if constexpr(std::is_same_v<TBoneId, const std::string &>) {
			  auto &mdl = animC.GetEntity().GetModel();
			  if(!mdl)
				  return {};
			  boneId = mdl->LookupBone(boneIdentifier);
		  }
		  else
			  boneId = boneIdentifier;
		  return animC.GetBoneMatrix(boneId);
	  });

	classDef.def(
	  "GetEffectiveBonePose", +[](const pragma::BaseAnimatedComponent &animC, TBoneId boneIdentifier) -> std::optional<umath::ScaledTransform> {
		  pragma::animation::BoneId boneId;
		  if constexpr(std::is_same_v<TBoneId, const std::string &>) {
			  auto &mdl = animC.GetEntity().GetModel();
			  if(!mdl)
				  return {};
			  boneId = mdl->LookupBone(boneIdentifier);
		  }
		  else
			  boneId = boneIdentifier;
		  auto &transforms = animC.GetProcessedBones();
		  if(boneId >= transforms.size())
			  return {};
		  return transforms[boneId];
	  });
	classDef.def(
	  "SetEffectiveBonePose", +[](pragma::BaseAnimatedComponent &animC, TBoneId boneIdentifier, const umath::ScaledTransform &pose) -> bool {
		  pragma::animation::BoneId boneId;
		  if constexpr(std::is_same_v<TBoneId, const std::string &>) {
			  auto &mdl = animC.GetEntity().GetModel();
			  if(!mdl)
				  return {};
			  boneId = mdl->LookupBone(boneIdentifier);
		  }
		  else
			  boneId = boneIdentifier;
		  auto &transforms = animC.GetProcessedBones();
		  if(boneId >= transforms.size())
			  return false;
		  transforms[boneId] = pose;
		  return true;
	  });
}
void pragma::lua::base_animated_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseAnimatedComponent>("BaseAnimatedComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("PlayAnimation", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, int, uint32_t)>([](lua_State *l, pragma::BaseAnimatedComponent &hAnim, int anim, uint32_t flags) { hAnim.PlayAnimation(anim, static_cast<pragma::FPlayAnim>(flags)); }));
	def.def("PlayAnimation",
	  static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, std::string, uint32_t)>([](lua_State *l, pragma::BaseAnimatedComponent &hAnim, std::string anim, uint32_t flags) { Lua::PushBool(l, hAnim.PlayAnimation(anim, static_cast<pragma::FPlayAnim>(flags))); }));
	def.def("PlayAnimation", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, int)>([](lua_State *l, pragma::BaseAnimatedComponent &hAnim, int anim) { hAnim.PlayAnimation(anim, pragma::FPlayAnim::Default); }));
	def.def("PlayAnimation", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, std::string)>([](lua_State *l, pragma::BaseAnimatedComponent &hAnim, std::string anim) { Lua::PushBool(l, hAnim.PlayAnimation(anim, pragma::FPlayAnim::Default)); }));
	def.def("GetAnimation", &pragma::BaseAnimatedComponent::GetAnimation);
	def.def("GetAnimationObject", &pragma::BaseAnimatedComponent::GetAnimationObject, luabind::shared_from_this_policy<0> {});
	def.def("PlayActivity",
	  static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, int, uint32_t)>([](lua_State *l, pragma::BaseAnimatedComponent &hAnim, int activity, uint32_t flags) { hAnim.PlayActivity(static_cast<Activity>(activity), static_cast<pragma::FPlayAnim>(flags)); }));
	def.def("PlayActivity", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, int)>([](lua_State *l, pragma::BaseAnimatedComponent &hAnim, int activity) { hAnim.PlayActivity(static_cast<Activity>(activity), pragma::FPlayAnim::Default); }));
	def.def("GetActivity", &pragma::BaseAnimatedComponent::GetActivity);
	def.def("ResetPose", &pragma::BaseAnimatedComponent::ResetPose);
	def.def("PlayLayeredAnimation", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, int, int)>([](lua_State *l, pragma::BaseAnimatedComponent &hAnim, int slot, int anim) { hAnim.PlayLayeredAnimation(slot, anim); }));
	def.def("PlayLayeredAnimation", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, int, std::string)>([](lua_State *l, pragma::BaseAnimatedComponent &hAnim, int slot, std::string anim) { hAnim.PlayLayeredAnimation(slot, anim); }));
	def.def("PlayLayeredActivity", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, int, int)>([](lua_State *l, pragma::BaseAnimatedComponent &hAnim, int slot, int activity) { hAnim.PlayLayeredActivity(slot, static_cast<Activity>(activity)); }));
	def.def("StopLayeredAnimation", &pragma::BaseAnimatedComponent::StopLayeredAnimation);
	def.def("StopLayeredAnimations", &pragma::BaseAnimatedComponent::StopLayeredAnimations);
	def.def("GetLayeredAnimation", &pragma::BaseAnimatedComponent::GetLayeredAnimation);
	def.def("GetLayeredActivity", &pragma::BaseAnimatedComponent::GetLayeredActivity);
	def.def("GetLayeredAnimations", static_cast<luabind::object (*)(lua_State *, pragma::BaseAnimatedComponent &)>([](lua_State *l, pragma::BaseAnimatedComponent &hAnim) -> luabind::object {
		auto t = luabind::newtable(l);
		for(auto &pair : hAnim.GetAnimationSlotInfos())
			t[pair.first] = pair.second.animation;
		return t;
	}));
	def.def("ApplyLayeredAnimations", &pragma::BaseAnimatedComponent::MaintainGestures);
	def.def("SetPlaybackRate", &pragma::BaseAnimatedComponent::SetPlaybackRate);
	def.def("GetPlaybackRate", &pragma::BaseAnimatedComponent::GetPlaybackRate);
	def.def("GetPlaybackRateProperty", &pragma::BaseAnimatedComponent::GetPlaybackRateProperty);
	def.def("GetEffectiveBonePoses", +[](pragma::BaseAnimatedComponent &animC) -> std::vector<umath::ScaledTransform> { return animC.GetProcessedBones(); });

	def.def("GetBoneMatrix", &pragma::BaseAnimatedComponent::GetBoneMatrix);
	def.def(
	  "GetBoneMatrix", +[](const pragma::BaseAnimatedComponent &animC, const std::string &boneName) -> std::optional<Mat4> {
		  auto boneId = Lua::Animated::get_bone_id(animC, boneName);
		  if(boneId == -1)
			  return {};
		  return animC.GetBoneMatrix(boneId);
	  });

	register_base_animated_component_bone_methods<pragma::animation::BoneId>(def);
	register_base_animated_component_bone_methods<const std::string &>(def);
	def.def("UpdateEffectiveBoneTransforms", &pragma::BaseAnimatedComponent::UpdateSkeleton);
	def.def("AdvanceAnimations", &pragma::BaseAnimatedComponent::MaintainAnimations);
	def.def("ClearPreviousAnimation", &pragma::BaseAnimatedComponent::ClearPreviousAnimation);
	def.def("GetBindPose", &pragma::BaseAnimatedComponent::GetBindPose, luabind::shared_from_this_policy<0> {});
	def.def("SetBindPose", &pragma::BaseAnimatedComponent::SetBindPose);
	def.def("SetCycle", &pragma::BaseAnimatedComponent::SetCycle);
	def.def("GetCycle", &pragma::BaseAnimatedComponent::GetCycle);
	def.def("AddAnimationEvent",
	  static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, uint32_t, uint32_t, uint32_t, const luabind::object &)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, uint32_t animId, uint32_t frameId, uint32_t eventId, const luabind::object &args) {
		  auto ev = Lua::get_animation_event(l, 5, eventId);
		  hEnt.AddAnimationEvent(animId, frameId, ev);
	  }));
	def.def("AddAnimationEvent", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, uint32_t, uint32_t, const luabind::object &)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, uint32_t animId, uint32_t frameId, const luabind::object &f) {
		if(Lua::IsFunction(l, 4)) {
			auto hCb = hEnt.AddAnimationEvent(animId, frameId, CallbackHandle {std::shared_ptr<TCallback>(new LuaCallback(f))});
			Lua::Push<CallbackHandle>(l, hCb);
		}
		else {
			AnimationEvent ev {};
			ev.eventID = static_cast<AnimationEvent::Type>(Lua::CheckInt(l, 4));
			hEnt.AddAnimationEvent(animId, frameId, ev);
		}
	}));
	def.def("AddAnimationEvent",
	  static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, const std::string &, uint32_t, uint32_t, const luabind::object &)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, const std::string &anim, uint32_t frameId, uint32_t eventId, const luabind::object &args) {
		  auto ev = Lua::get_animation_event(l, 5, eventId);
		  hEnt.AddAnimationEvent(anim, frameId, ev);
	  }));
	def.def("AddAnimationEvent", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, const std::string &, uint32_t, const luabind::object &)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, const std::string &anim, uint32_t frameId, const luabind::object &f) {
		if(Lua::IsFunction(l, 4)) {
			auto hCb = hEnt.AddAnimationEvent(anim, frameId, CallbackHandle {std::shared_ptr<TCallback>(new LuaCallback(f))});
			Lua::Push<CallbackHandle>(l, hCb);
		}
		else {
			AnimationEvent ev {};
			ev.eventID = static_cast<AnimationEvent::Type>(Lua::CheckInt(l, 4));
			hEnt.AddAnimationEvent(anim, frameId, ev);
		}
	}));
	def.def("ClearAnimationEvents", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt) { hEnt.ClearAnimationEvents(); }));
	def.def("ClearAnimationEvents", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, uint32_t)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, uint32_t animId) { hEnt.ClearAnimationEvents(animId); }));
	def.def("ClearAnimationEvents", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, uint32_t, uint32_t)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, uint32_t animId, uint32_t frameId) { hEnt.ClearAnimationEvents(animId, frameId); }));
	def.def("ClearAnimationEvents", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, const std::string &)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, const std::string &anim) { hEnt.ClearAnimationEvents(anim); }));
	def.def("ClearAnimationEvents", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, const std::string &, uint32_t)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, const std::string &anim, uint32_t frameId) { hEnt.ClearAnimationEvents(anim, frameId); }));

	def.def("InjectAnimationEvent", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, uint32_t)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, uint32_t eventId) {
		AnimationEvent ev {};
		ev.eventID = static_cast<AnimationEvent::Type>(eventId);
		hEnt.InjectAnimationEvent(ev);
	}));
	def.def("InjectAnimationEvent", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, uint32_t, const luabind::object &)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, uint32_t eventId, const luabind::object &args) {
		auto ev = Lua::get_animation_event(l, 3, eventId);
		hEnt.InjectAnimationEvent(ev);
	}));
	def.def("BindAnimationEvent", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, uint32_t, luabind::object)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, uint32_t eventId, luabind::object function) {
		Lua::CheckFunction(l, 3);
		hEnt.BindAnimationEvent(static_cast<AnimationEvent::Type>(eventId), [l, function](std::reference_wrapper<const AnimationEvent> ev) {
			Lua::CallFunction(l, [&function, &ev](lua_State *l) -> Lua::StatusCode {
				function.push(l);
				pragma::CEHandleAnimationEvent evData {ev};
				evData.PushArgumentVariadic(l);
				return Lua::StatusCode::Ok;
			});
		});
	}));
	def.def("BindAnimationEvent",
	  static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, uint32_t, pragma::BaseEntityComponent &, const std::string &)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, uint32_t eventId, pragma::BaseEntityComponent &component, const std::string &methodName) {
		  auto hComponent = component.GetHandle();
		  hEnt.BindAnimationEvent(static_cast<AnimationEvent::Type>(eventId), [hComponent, methodName](std::reference_wrapper<const AnimationEvent> ev) {
			  if(hComponent.expired())
				  return;
			  auto o = hComponent->GetLuaObject();
			  auto r = o[methodName];
			  if(r) {
				  auto *l = o.interpreter();
				  auto c = Lua::CallFunction(
				    l,
				    [o, &methodName, &ev](lua_State *l) -> Lua::StatusCode {
					    o.push(l);
					    Lua::PushString(l, methodName);
					    Lua::GetTableValue(l, -2);
					    Lua::RemoveValue(l, -2);

					    o.push(l);
					    pragma::CEHandleAnimationEvent evData {ev};
					    evData.PushArgumentVariadic(l);
					    return Lua::StatusCode::Ok;
				    },
				    0);
			  }
		  });
	  }));

	def.def("GetVertexTransformMatrix", static_cast<std::optional<Mat4> (pragma::BaseAnimatedComponent::*)(const ModelSubMesh &, uint32_t) const>(&pragma::BaseAnimatedComponent::GetVertexTransformMatrix));
	def.def(
	  "GetLocalVertexPosition", +[](lua_State *l, pragma::BaseAnimatedComponent &hEnt, ModelSubMesh &subMesh, uint32_t vertexId) -> std::optional<Vector3> {
		  Vector3 pos, n;
		  if(vertexId >= subMesh.GetVertexCount())
			  return {};
		  auto &v = subMesh.GetVertices()[vertexId];
		  pos = v.position;
		  n = v.normal;
		  if(hEnt.GetLocalVertexPosition(subMesh, vertexId, pos) == false)
			  return {};
		  return pos;
	  });
	def.def(
	  "GetVertexPosition", +[](lua_State *l, pragma::BaseAnimatedComponent &hEnt, uint32_t meshGroupId, uint32_t meshId, uint32_t subMeshId, uint32_t vertexId) -> std::optional<Vector3> {
		  auto &mdl = hEnt.GetEntity().GetModel();
		  if(!mdl)
			  return {};
		  auto *subMesh = mdl->GetSubMesh(meshGroupId, meshId, subMeshId);
		  if(!subMesh)
			  return {};
		  Vector3 pos;
		  if(vertexId >= subMesh->GetVertexCount())
			  return {};
		  auto &v = subMesh->GetVertices()[vertexId];
		  pos = v.position;
		  if(hEnt.GetVertexPosition(meshGroupId, meshId, subMeshId, vertexId, pos) == false)
			  return {};
		  return pos;
	  });
	def.def("GetVertexPosition",
	  static_cast<std::optional<Vector3> (*)(lua_State *, pragma::BaseAnimatedComponent &, const std::shared_ptr<ModelSubMesh> &, uint32_t)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, const std::shared_ptr<ModelSubMesh> &subMesh, uint32_t vertexId) -> std::optional<Vector3> {
		  Vector3 pos;
		  if(vertexId >= subMesh->GetVertexCount())
			  return {};
		  auto &v = subMesh->GetVertices()[vertexId];
		  pos = v.position;
		  if(hEnt.GetVertexPosition(*subMesh, vertexId, pos) == false)
			  return {};
		  return pos;
	  }));
	def.def("SetBlendController", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, unsigned int, float)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, unsigned int controller, float val) { hEnt.SetBlendController(controller, val); }));
	def.def("SetBlendController", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, std::string, float)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, std::string controller, float val) { hEnt.SetBlendController(controller, val); }));
	def.def("GetBlendController", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, std::string)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, std::string controller) { Lua::PushNumber(l, hEnt.GetBlendController(controller)); }));
	def.def("GetBlendController", static_cast<void (*)(lua_State *, pragma::BaseAnimatedComponent &, unsigned int)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, unsigned int controller) { Lua::PushNumber(l, hEnt.GetBlendController(controller)); }));
	def.def("GetBoneCount", &pragma::BaseAnimatedComponent::GetBoneCount);
	def.def("GetBaseAnimationFlags", &pragma::BaseAnimatedComponent::GetBaseAnimationFlags);
	def.def("SetBaseAnimationFlags", &pragma::BaseAnimatedComponent::SetBaseAnimationFlags);
	def.def("GetLayeredAnimationFlags", static_cast<luabind::object (*)(lua_State *, pragma::BaseAnimatedComponent &, uint32_t)>([](lua_State *l, pragma::BaseAnimatedComponent &hEnt, uint32_t layerIdx) -> luabind::object {
		auto flags = hEnt.GetLayeredAnimationFlags(layerIdx);
		if(flags.has_value() == false)
			return {};
		return luabind::object {l, *flags};
	}));
	def.def("SetLayeredAnimationFlags", &pragma::BaseAnimatedComponent::SetLayeredAnimationFlags);
	def.def("SetPostAnimationUpdateEnabled", &pragma::BaseAnimatedComponent::SetPostAnimationUpdateEnabled);
	def.def("IsPostAnimationUpdateEnabled", &pragma::BaseAnimatedComponent::IsPostAnimationUpdateEnabled);
	def.def("GetMetaBoneId", &pragma::BaseAnimatedComponent::GetMetaBoneId);
	def.def(
	  "GetMetaBonePose", +[](pragma::BaseAnimatedComponent &animC, animation::MetaRigBoneType boneType, umath::CoordinateSpace space) -> std::optional<umath::ScaledTransform> {
		  umath::ScaledTransform pose;
		  if(!animC.GetMetaBonePose(boneType, pose, space))
			  return {};
		  return pose;
	  });
	def.def("SetMetaBonePose", static_cast<bool (pragma::BaseAnimatedComponent::*)(pragma::animation::MetaRigBoneType, const umath::ScaledTransform &, umath::CoordinateSpace)>(&pragma::BaseAnimatedComponent::SetMetaBonePose));
	def.def(
	  "MetaBonePoseToSkeletal", +[](pragma::BaseAnimatedComponent &animC, animation::MetaRigBoneType boneType, const umath::ScaledTransform &pose) -> umath::ScaledTransform {
		  auto newPose = pose;
		  animC.MetaBonePoseToSkeletal(boneType, newPose);
		  return newPose;
	  });
	def.def(
	  "MetaBoneRotationToSkeletal", +[](pragma::BaseAnimatedComponent &animC, animation::MetaRigBoneType boneType, const Quat &rot) -> Quat {
		  auto newRot = rot;
		  animC.MetaBoneRotationToSkeletal(boneType, newRot);
		  return newRot;
	  });

	def.add_static_constant("EVENT_HANDLE_ANIMATION_EVENT", pragma::BaseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT);
	def.add_static_constant("EVENT_ON_PLAY_ANIMATION", pragma::BaseAnimatedComponent::EVENT_ON_PLAY_ANIMATION);
	def.add_static_constant("EVENT_ON_PLAY_LAYERED_ANIMATION", pragma::BaseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION);
	def.add_static_constant("EVENT_ON_PLAY_LAYERED_ACTIVITY", pragma::BaseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ACTIVITY);
	def.add_static_constant("EVENT_ON_ANIMATION_COMPLETE", pragma::BaseAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE);
	def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_START", pragma::BaseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_START);
	def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_COMPLETE", pragma::BaseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE);
	def.add_static_constant("EVENT_ON_ANIMATION_START", pragma::BaseAnimatedComponent::EVENT_ON_ANIMATION_START);
	def.add_static_constant("EVENT_TRANSLATE_LAYERED_ANIMATION", pragma::BaseAnimatedComponent::EVENT_TRANSLATE_LAYERED_ANIMATION);
	def.add_static_constant("EVENT_TRANSLATE_ANIMATION", pragma::BaseAnimatedComponent::EVENT_TRANSLATE_ANIMATION);
	def.add_static_constant("EVENT_TRANSLATE_ACTIVITY", pragma::BaseAnimatedComponent::EVENT_TRANSLATE_ACTIVITY);

	def.add_static_constant("EVENT_HANDLE_ANIMATION_EVENT", pragma::BaseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT);
	def.add_static_constant("EVENT_ON_PLAY_ANIMATION", pragma::BaseAnimatedComponent::EVENT_ON_PLAY_ANIMATION);
	def.add_static_constant("EVENT_ON_PLAY_LAYERED_ANIMATION", pragma::BaseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION);
	def.add_static_constant("EVENT_ON_PLAY_LAYERED_ACTIVITY", pragma::BaseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ACTIVITY);
	def.add_static_constant("EVENT_ON_ANIMATION_COMPLETE", pragma::BaseAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE);
	def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_START", pragma::BaseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_START);
	def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_COMPLETE", pragma::BaseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE);
	def.add_static_constant("EVENT_ON_ANIMATION_START", pragma::BaseAnimatedComponent::EVENT_ON_ANIMATION_START);
	def.add_static_constant("EVENT_TRANSLATE_LAYERED_ANIMATION", pragma::BaseAnimatedComponent::EVENT_TRANSLATE_LAYERED_ANIMATION);
	def.add_static_constant("EVENT_TRANSLATE_ANIMATION", pragma::BaseAnimatedComponent::EVENT_TRANSLATE_ANIMATION);
	def.add_static_constant("EVENT_TRANSLATE_ACTIVITY", pragma::BaseAnimatedComponent::EVENT_TRANSLATE_ACTIVITY);
	def.add_static_constant("EVENT_MAINTAIN_ANIMATIONS", pragma::BaseAnimatedComponent::EVENT_MAINTAIN_ANIMATIONS);
	def.add_static_constant("EVENT_MAINTAIN_ANIMATION_MOVEMENT", pragma::BaseAnimatedComponent::EVENT_MAINTAIN_ANIMATION_MOVEMENT);
	def.add_static_constant("EVENT_SHOULD_UPDATE_BONES", pragma::BaseAnimatedComponent::EVENT_SHOULD_UPDATE_BONES);

	def.add_static_constant("EVENT_ON_PLAY_ACTIVITY", pragma::BaseAnimatedComponent::EVENT_ON_PLAY_ACTIVITY);
	def.add_static_constant("EVENT_ON_STOP_LAYERED_ANIMATION", pragma::BaseAnimatedComponent::EVENT_ON_STOP_LAYERED_ANIMATION);
	def.add_static_constant("EVENT_ON_BONE_TRANSFORM_CHANGED", pragma::BaseAnimatedComponent::EVENT_ON_BONE_TRANSFORM_CHANGED);
	def.add_static_constant("EVENT_ON_ANIMATIONS_UPDATED", pragma::BaseAnimatedComponent::EVENT_ON_ANIMATIONS_UPDATED);
	def.add_static_constant("EVENT_PLAY_ANIMATION", pragma::BaseAnimatedComponent::EVENT_PLAY_ANIMATION);
	def.add_static_constant("EVENT_ON_ANIMATION_RESET", pragma::BaseAnimatedComponent::EVENT_ON_ANIMATION_RESET);
	def.add_static_constant("EVENT_ON_UPDATE_SKELETON", pragma::BaseAnimatedComponent::EVENT_ON_UPDATE_SKELETON);
	def.add_static_constant("EVENT_POST_ANIMATION_UPDATE", pragma::BaseAnimatedComponent::EVENT_POST_ANIMATION_UPDATE);
	def.add_static_constant("EVENT_ON_RESET_POSE", pragma::BaseAnimatedComponent::EVENT_ON_RESET_POSE);

	def.add_static_constant("FPLAYANIM_NONE", umath::to_integral(pragma::FPlayAnim::None));
	def.add_static_constant("FPLAYANIM_RESET", umath::to_integral(pragma::FPlayAnim::Reset));
	def.add_static_constant("FPLAYANIM_TRANSMIT", umath::to_integral(pragma::FPlayAnim::Transmit));
	def.add_static_constant("FPLAYANIM_SNAP_TO", umath::to_integral(pragma::FPlayAnim::SnapTo));
	def.add_static_constant("FPLAYANIM_DEFAULT", umath::to_integral(pragma::FPlayAnim::Default));
	def.add_static_constant("FPLAYANIM_LOOP", umath::to_integral(pragma::FPlayAnim::Loop));
}
#include "pragma/entities/components/liquid/base_liquid_component.hpp"
#include "pragma/entities/components/liquid/base_liquid_surface_component.hpp"
#include "pragma/entities/components/liquid/base_liquid_volume_component.hpp"
#include "pragma/entities/components/liquid/base_buoyancy_component.hpp"
namespace Lua::FuncWater {
	void CalcLineSurfaceIntersection(lua_State *l, pragma::BaseFuncLiquidComponent &hEnt, const Vector3 &lineOrigin, const Vector3 &lineDir, bool bCull)
	{
		double t, u, v;
		auto r = hEnt.CalcLineSurfaceIntersection(lineOrigin, lineDir, &t, &u, &v, bCull);
		Lua::PushBool(l, r);
		Lua::PushNumber(l, t);
		Lua::PushNumber(l, u);
		Lua::PushNumber(l, v);
	}
};
void pragma::lua::base_func_water_component::register_class(luabind::module_ &mod)
{
	{
		auto def = Lua::create_base_entity_component_class<pragma::BaseLiquidSurfaceComponent>("BaseLiquidSurfaceComponent");
		util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	}
	{
		auto def = Lua::create_base_entity_component_class<pragma::BaseLiquidVolumeComponent>("BaseLiquidVolumeComponent");
		util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	}
	{
		auto def = Lua::create_base_entity_component_class<pragma::BaseBuoyancyComponent>("BaseBuoyancyComponent");
		util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	}
	auto def = Lua::create_base_entity_component_class<pragma::BaseFuncLiquidComponent>("BaseFuncLiquidComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("CalcLineSurfaceIntersection", static_cast<void (*)(lua_State *, pragma::BaseFuncLiquidComponent &, const Vector3 &, const Vector3 &)>([](lua_State *l, pragma::BaseFuncLiquidComponent &hEnt, const Vector3 &lineOrigin, const Vector3 &lineDir) {
		Lua::FuncWater::CalcLineSurfaceIntersection(l, hEnt, lineOrigin, lineDir, false);
	}));
	def.def("CalcLineSurfaceIntersection", &pragma::BaseFuncLiquidComponent::CalcLineSurfaceIntersection);
}
#include "pragma/entities/components/basetoggle.h"
void pragma::lua::base_toggle_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseToggleComponent>("BaseToggleComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("TurnOn", &pragma::BaseToggleComponent::TurnOn);
	def.def("TurnOff", &pragma::BaseToggleComponent::TurnOff);
	def.def("Toggle", &pragma::BaseToggleComponent::Toggle);
	def.def("IsTurnedOn", &pragma::BaseToggleComponent::IsTurnedOn);
	def.def("IsTurnedOff", static_cast<bool (*)(lua_State *, pragma::BaseToggleComponent &)>([](lua_State *l, pragma::BaseToggleComponent &hEnt) { return !hEnt.IsTurnedOn(); }));
	def.def("SetTurnedOn", &pragma::BaseToggleComponent::SetTurnedOn);
	def.def("GetTurnedOnProperty", &pragma::BaseToggleComponent::GetTurnedOnProperty);
	def.add_static_constant("EVENT_ON_TURN_ON", pragma::BaseToggleComponent::EVENT_ON_TURN_ON);
	def.add_static_constant("EVENT_ON_TURN_OFF", pragma::BaseToggleComponent::EVENT_ON_TURN_OFF);

	def.add_static_constant("SPAWN_FLAG_START_ON_BIT", umath::to_integral(pragma::BaseToggleComponent::SpawnFlags::StartOn));
}
#include "pragma/entities/components/base_wheel_component.hpp"
void pragma::lua::base_wheel_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseWheelComponent>("BaseWheelComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	// TODO
}
#include "pragma/entities/environment/env_decal.h"
void pragma::lua::base_decal_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvDecalComponent>("BaseEnvDecalComponent");
	def.def("SetSize", &pragma::BaseEnvDecalComponent::SetSize);
	def.def("GetSize", &pragma::BaseEnvDecalComponent::GetSize);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}
#include "pragma/entities/environment/lights/env_light.h"
void pragma::lua::base_env_light_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvLightComponent>("BaseEnvLightComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetFalloffExponent", &pragma::BaseEnvLightComponent::GetFalloffExponent);
	def.def("SetFalloffExponent", &pragma::BaseEnvLightComponent::SetFalloffExponent);
	def.def("IsBaked", &pragma::BaseEnvLightComponent::IsBaked);
	def.def("SetBaked", &pragma::BaseEnvLightComponent::SetBaked);
	def.def("SetLightIntensityType", &pragma::BaseEnvLightComponent::SetLightIntensityType);
	def.def("GetLightIntensityType", &pragma::BaseEnvLightComponent::GetLightIntensityType);
	def.def(
	  "SetLightIntensityType", +[](pragma::BaseEnvLightComponent &c, const std::string &type) {
		  auto e = magic_enum::enum_cast<pragma::BaseEnvLightComponent::LightIntensityType>(type);
		  if(e.has_value())
			  c.SetLightIntensityType(*e);
	  });
	def.def("SetLightIntensity", static_cast<void (pragma::BaseEnvLightComponent::*)(float)>(&pragma::BaseEnvLightComponent::SetLightIntensity));
	def.def("SetLightIntensity", static_cast<void (pragma::BaseEnvLightComponent::*)(float, pragma::BaseEnvLightComponent::LightIntensityType)>(&pragma::BaseEnvLightComponent::SetLightIntensity));
	def.def(
	  "SetLightIntensity", +[](pragma::BaseEnvLightComponent &c, float intensity, const std::string &type) {
		  auto e = magic_enum::enum_cast<pragma::BaseEnvLightComponent::LightIntensityType>(type);
		  if(e.has_value())
			  c.SetLightIntensity(intensity, *e);
	  });
	def.def("GetLightIntensity", &pragma::BaseEnvLightComponent::GetLightIntensity);
	def.def("GetLightIntensityCandela", static_cast<Candela (pragma::BaseEnvLightComponent::*)() const>(&pragma::BaseEnvLightComponent::GetLightIntensityCandela));
	def.def("CalcLightIntensityAtPoint", &pragma::BaseEnvLightComponent::CalcLightIntensityAtPoint);
	def.def("CalcLightDirectionToPoint", &pragma::BaseEnvLightComponent::CalcLightDirectionToPoint);
	def.add_static_constant("INTENSITY_TYPE_CANDELA", umath::to_integral(pragma::BaseEnvLightComponent::LightIntensityType::Candela));
	def.add_static_constant("INTENSITY_TYPE_LUMEN", umath::to_integral(pragma::BaseEnvLightComponent::LightIntensityType::Lumen));
	def.add_static_constant("INTENSITY_TYPE_LUX", umath::to_integral(pragma::BaseEnvLightComponent::LightIntensityType::Lux));

	def.add_static_constant("LIGHT_FLAG_NONE", umath::to_integral(pragma::BaseEnvLightComponent::LightFlags::None));
	def.add_static_constant("LIGHT_FLAG_BAKED_LIGHT_SOURCE_BIT", umath::to_integral(pragma::BaseEnvLightComponent::LightFlags::BakedLightSource));
}
#include "pragma/entities/environment/lights/env_light_spot.h"
void pragma::lua::base_env_light_spot_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvLightSpotComponent>("BaseEnvLightSpotComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetOuterConeAngle", &pragma::BaseEnvLightSpotComponent::SetOuterConeAngle);
	def.def("GetOuterConeAngle", &pragma::BaseEnvLightSpotComponent::GetOuterConeAngle);
	def.def("SetInnerConeAngle", &pragma::BaseEnvLightSpotComponent::SetInnerConeAngle);
	def.def("GetInnerConeAngle", &pragma::BaseEnvLightSpotComponent::GetInnerConeAngle);
	def.def("GetBlendFractionProperty", &pragma::BaseEnvLightSpotComponent::GetBlendFractionProperty);
	def.def("GetOuterConeAngleProperty", &pragma::BaseEnvLightSpotComponent::GetOuterConeAngleProperty);
	def.def("GetConeStartOffsetProperty", &pragma::BaseEnvLightSpotComponent::GetConeStartOffsetProperty);
	def.def("SetBlendFraction", &pragma::BaseEnvLightSpotComponent::SetBlendFraction);
	def.def("GetBlendFraction", &pragma::BaseEnvLightSpotComponent::GetBlendFraction);
	def.def("CalcConeFalloff", static_cast<float (pragma::BaseEnvLightSpotComponent::*)(const Vector3 &) const>(&pragma::BaseEnvLightSpotComponent::CalcConeFalloff));
	def.def("CalcDistanceFalloff", &pragma::BaseEnvLightSpotComponent::CalcDistanceFalloff);
}
#include "pragma/entities/environment/lights/env_light_point.h"
void pragma::lua::base_env_light_point_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvLightPointComponent>("BaseEnvLightPointComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("CalcDistanceFalloff", &pragma::BaseEnvLightPointComponent::CalcDistanceFalloff);
}
#include "pragma/entities/environment/lights/env_light_directional.h"
void pragma::lua::base_env_light_directional_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvLightDirectionalComponent>("BaseEnvLightDirectionalComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetAmbientColor", &pragma::BaseEnvLightDirectionalComponent::GetAmbientColor, luabind::copy_policy<0> {});
	def.def("GetAmbientColorProperty", &pragma::BaseEnvLightDirectionalComponent::GetAmbientColorProperty);
	def.def("SetAmbientColor", &pragma::BaseEnvLightDirectionalComponent::SetAmbientColor);
}
#include "pragma/entities/environment/effects/env_particle_system.h"
void pragma::lua::base_env_particle_system_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvParticleSystemComponent>("BaseEnvParticleSystemComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}
#include "pragma/entities/components/base_flammable_component.hpp"
void pragma::lua::base_flammable_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFlammableComponent>("BaseFlammableComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("Ignite",
	  static_cast<void (*)(lua_State *, pragma::BaseFlammableComponent &, float, BaseEntity &, BaseEntity &)>([](lua_State *l, pragma::BaseFlammableComponent &hEnt, float duration, BaseEntity &attacker, BaseEntity &inflictor) { hEnt.Ignite(duration, &attacker, &inflictor); }));
	def.def("Ignite", static_cast<void (*)(lua_State *, pragma::BaseFlammableComponent &, float, BaseEntity &)>([](lua_State *l, pragma::BaseFlammableComponent &hEnt, float duration, BaseEntity &attacker) { hEnt.Ignite(duration, &attacker); }));
	def.def("Ignite", static_cast<void (*)(lua_State *, pragma::BaseFlammableComponent &, float)>([](lua_State *l, pragma::BaseFlammableComponent &hEnt, float duration) { hEnt.Ignite(duration); }));
	def.def("Ignite", static_cast<void (*)(lua_State *, pragma::BaseFlammableComponent &)>([](lua_State *l, pragma::BaseFlammableComponent &hEnt) { hEnt.Ignite(0.f); }));
	def.def("IsOnFire", static_cast<void (*)(lua_State *, pragma::BaseFlammableComponent &)>([](lua_State *l, pragma::BaseFlammableComponent &hEnt) { Lua::PushBool(l, hEnt.IsOnFire()); }));
	def.def("IsIgnitable", static_cast<void (*)(lua_State *, pragma::BaseFlammableComponent &)>([](lua_State *l, pragma::BaseFlammableComponent &hEnt) { Lua::PushBool(l, hEnt.IsIgnitable()); }));
	def.def("Extinguish", static_cast<void (*)(lua_State *, pragma::BaseFlammableComponent &)>([](lua_State *l, pragma::BaseFlammableComponent &hEnt) { hEnt.Extinguish(); }));
	def.def("SetIgnitable", static_cast<void (*)(lua_State *, pragma::BaseFlammableComponent &, bool)>([](lua_State *l, pragma::BaseFlammableComponent &hEnt, bool b) { hEnt.SetIgnitable(b); }));
	def.def("GetOnFireProperty", &pragma::BaseFlammableComponent::GetOnFireProperty);
	def.def("GetIgnitableProperty", &pragma::BaseFlammableComponent::GetIgnitableProperty);
	def.add_static_constant("EVENT_ON_IGNITED", pragma::BaseFlammableComponent::EVENT_ON_IGNITED);
	def.add_static_constant("EVENT_ON_EXTINGUISHED", pragma::BaseFlammableComponent::EVENT_ON_EXTINGUISHED);
}

#include "pragma/entities/components/base_health_component.hpp"
void pragma::lua::base_health_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseHealthComponent>("BaseHealthComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetHealth", static_cast<void (*)(lua_State *, pragma::BaseHealthComponent &)>([](lua_State *l, pragma::BaseHealthComponent &hEnt) {
		unsigned short health = hEnt.GetHealth();
		Lua::PushInt(l, health);
	}));
	def.def("SetHealth", static_cast<void (*)(lua_State *, pragma::BaseHealthComponent &, unsigned short)>([](lua_State *l, pragma::BaseHealthComponent &hEnt, unsigned short health) { hEnt.SetHealth(health); }));
	def.def("SetMaxHealth", static_cast<void (*)(lua_State *, pragma::BaseHealthComponent &, uint16_t)>([](lua_State *l, pragma::BaseHealthComponent &hEnt, uint16_t maxHealth) { hEnt.SetMaxHealth(maxHealth); }));
	def.def("GetMaxHealth", static_cast<void (*)(lua_State *, pragma::BaseHealthComponent &)>([](lua_State *l, pragma::BaseHealthComponent &hEnt) { Lua::PushInt(l, hEnt.GetMaxHealth()); }));
	def.def("GetHealthProperty", &pragma::BaseHealthComponent::GetHealthProperty);
	def.def("GetMaxHealthProperty", &pragma::BaseHealthComponent::GetMaxHealthProperty);
	def.add_static_constant("EVENT_ON_TAKEN_DAMAGE", pragma::BaseHealthComponent::EVENT_ON_TAKEN_DAMAGE);
	def.add_static_constant("EVENT_ON_HEALTH_CHANGED", pragma::BaseHealthComponent::EVENT_ON_HEALTH_CHANGED);
}

#include "pragma/entities/components/base_name_component.hpp"
void pragma::lua::base_name_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseNameComponent>("BaseNameComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetName", static_cast<void (*)(lua_State *, pragma::BaseNameComponent &, std::string)>([](lua_State *l, pragma::BaseNameComponent &hEnt, std::string name) { hEnt.SetName(name); }));
	def.def("GetName", static_cast<void (*)(lua_State *, pragma::BaseNameComponent &)>([](lua_State *l, pragma::BaseNameComponent &hEnt) { Lua::PushString(l, hEnt.GetName()); }));
	def.def("GetNameProperty", &pragma::BaseNameComponent::GetNameProperty);
}

#include "pragma/entities/components/base_networked_component.hpp"
void pragma::lua::base_networked_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseNetworkedComponent>("BaseNetworkedComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
#if NETWORKED_VARS_ENABLED != 0
	def.def("GetNetVarProperty", static_cast<void (*)(lua_State *, THandle &, uint32_t)>([](lua_State *l, THandle &hEnt, uint32_t id) {
		auto &prop = hEnt.GetNetworkedVariableProperty(id);
		if(prop == nullptr)
			return;
		Lua::Property::push(l, *prop);
	}));
	def.def("CreateNetVar", static_cast<void (*)(lua_State *, THandle &, const std::string &, uint32_t)>([](lua_State *l, THandle &hEnt, const std::string &name, uint32_t type) {
		auto id = hEnt.CreateNetworkedVariable(name, static_cast<pragma::BaseNetworkedComponent::NetworkedVariable::Type>(type));
		Lua::PushInt(l, id);
	}));
	def.def("SetNetVarValue", static_cast<void (*)(lua_State *, THandle &, uint32_t, luabind::object)>([](lua_State *l, THandle &hEnt, uint32_t id, luabind::object value) {
		auto type = hEnt.GetNetworkedVariableType(id);
		if(type == pragma::BaseNetworkedComponent::NetworkedVariable::Type::Invalid)
			return;
		switch(type) {
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Bool:
			{
				auto v = Lua::CheckBool(l, 3);
				hEnt.SetNetworkedVariable<bool>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Double:
			{
				auto v = Lua::CheckNumber(l, 3);
				hEnt.SetNetworkedVariable<double>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Float:
			{
				auto v = Lua::CheckNumber(l, 3);
				hEnt.SetNetworkedVariable<float>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int8:
			{
				auto v = Lua::CheckInt(l, 3);
				hEnt.SetNetworkedVariable<int8_t>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int16:
			{
				auto v = Lua::CheckInt(l, 3);
				hEnt.SetNetworkedVariable<int16_t>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int32:
			{
				auto v = Lua::CheckInt(l, 3);
				hEnt.SetNetworkedVariable<int32_t>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int64:
			{
				auto v = Lua::CheckInt(l, 3);
				hEnt.SetNetworkedVariable<int64_t>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::LongDouble:
			{
				auto v = Lua::CheckNumber(l, 3);
				hEnt.SetNetworkedVariable<long double>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::String:
			{
				std::string v = Lua::CheckString(l, 3);
				hEnt.SetNetworkedVariable<std::string>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt8:
			{
				auto v = Lua::CheckInt(l, 3);
				hEnt.SetNetworkedVariable<uint8_t>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt16:
			{
				auto v = Lua::CheckInt(l, 3);
				hEnt.SetNetworkedVariable<uint16_t>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt32:
			{
				auto v = Lua::CheckInt(l, 3);
				hEnt.SetNetworkedVariable<uint32_t>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt64:
			{
				auto v = Lua::CheckInt(l, 3);
				hEnt.SetNetworkedVariable<uint64_t>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::EulerAngles:
			{
				auto &v = *Lua::CheckEulerAngles(l, 3);
				hEnt.SetNetworkedVariable<EulerAngles>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Color:
			{
				auto &v = *Lua::CheckColor(l, 3);
				hEnt.SetNetworkedVariable<Color>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector:
			{
				auto &v = *Lua::CheckVector(l, 3);
				hEnt.SetNetworkedVariable<Vector3>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector2:
			{
				auto &v = *Lua::CheckVector2(l, 3);
				hEnt.SetNetworkedVariable<Vector2>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector4:
			{
				auto &v = *Lua::CheckVector4(l, 3);
				hEnt.SetNetworkedVariable<Vector4>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Entity:
			{
				auto *v = Lua::CheckEntity(l, 3);
				auto h = (v != nullptr) ? v->GetHandle() : EntityHandle {};
				hEnt.SetNetworkedVariable<EntityHandle>(id, h);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Quaternion:
			{
				auto &v = *Lua::CheckQuaternion(l, 3);
				hEnt.SetNetworkedVariable<Quat>(id, v);
				return;
			}
		}
	}));
	def.def("GetNetVarValue", static_cast<void (*)(lua_State *, THandle &, uint32_t)>([](lua_State *l, THandle &hEnt, uint32_t id) {
		auto type = hEnt.GetNetworkedVariableType(id);
		auto &prop = hEnt.GetNetworkedVariableProperty(id);
		if(type == pragma::BaseNetworkedComponent::NetworkedVariable::Type::Invalid || prop == nullptr)
			return;
		switch(type) {
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Bool:
			Lua::PushBool(l, static_cast<::util::BoolProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Double:
			Lua::PushNumber(l, static_cast<::util::DoubleProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Float:
			Lua::PushNumber(l, static_cast<::util::FloatProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int8:
			Lua::PushInt(l, static_cast<::util::Int8Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int16:
			Lua::PushInt(l, static_cast<::util::Int16Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int32:
			Lua::PushInt(l, static_cast<::util::Int32Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int64:
			Lua::PushInt(l, static_cast<::util::Int64Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::LongDouble:
			Lua::PushNumber(l, static_cast<::util::LongDoubleProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::String:
			Lua::PushString(l, static_cast<::util::StringProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt8:
			Lua::PushInt(l, static_cast<::util::UInt8Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt16:
			Lua::PushInt(l, static_cast<::util::UInt16Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt32:
			Lua::PushInt(l, static_cast<::util::UInt32Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt64:
			Lua::PushInt(l, static_cast<::util::UInt64Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::EulerAngles:
			Lua::Push<EulerAngles>(l, static_cast<::util::EulerAnglesProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Color:
			Lua::Push<Color>(l, static_cast<::util::ColorProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector:
			Lua::Push<Vector3>(l, static_cast<::util::Vector3Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector2:
			Lua::Push<Vector2>(l, static_cast<::util::Vector2Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector4:
			Lua::Push<Vector4>(l, static_cast<::util::Vector4Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Entity:
			{
				auto &hEnt = static_cast<::pragma::EntityProperty &>(*prop).GetValue();
				if(hEnt.IsValid())
					return;
				hEnt.GetLuaObject()->push(l);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Quaternion:
			Lua::Push<Quat>(l, static_cast<::util::QuatProperty &>(*prop).GetValue());
			return;
		}
	}));
	def.add_static_constant("NET_VAR_TYPE_BOOL", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Bool));
	def.add_static_constant("NET_VAR_TYPE_DOUBLE", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Double));
	def.add_static_constant("NET_VAR_TYPE_FLOAT", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Float));
	def.add_static_constant("NET_VAR_TYPE_INT8", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int8));
	def.add_static_constant("NET_VAR_TYPE_INT16", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int16));
	def.add_static_constant("NET_VAR_TYPE_INT32", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int32));
	def.add_static_constant("NET_VAR_TYPE_INT64", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int64));
	def.add_static_constant("NET_VAR_TYPE_LONG_DOUBLE", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::LongDouble));
	def.add_static_constant("NET_VAR_TYPE_STRING", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::String));
	def.add_static_constant("NET_VAR_TYPE_UINT8", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt8));
	def.add_static_constant("NET_VAR_TYPE_UINT16", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt16));
	def.add_static_constant("NET_VAR_TYPE_UINT32", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt32));
	def.add_static_constant("NET_VAR_TYPE_UINT64", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt64));
	def.add_static_constant("NET_VAR_TYPE_EULER_ANGLES", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::EulerAngles));
	def.add_static_constant("NET_VAR_TYPE_COLOR", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Color));
	def.add_static_constant("NET_VAR_TYPE_VECTOR", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector));
	def.add_static_constant("NET_VAR_TYPE_VECTOR2", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector2));
	def.add_static_constant("NET_VAR_TYPE_VECTOR4", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector4));
	def.add_static_constant("NET_VAR_TYPE_ENTITY", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Entity));
	def.add_static_constant("NET_VAR_TYPE_QUATERNION", umath::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Quaternion));
#endif
}

#include "pragma/entities/components/base_observable_component.hpp"
void pragma::lua::base_observable_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseObservableComponent>("BaseObservableComponent");
	def.add_static_constant("EVENT_ON_OBSERVER_CHANGED", pragma::BaseObservableComponent::EVENT_ON_OBSERVER_CHANGED);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetLocalCameraOrigin", static_cast<void (*)(lua_State *, pragma::BaseObservableComponent &, uint32_t, const Vector3 &)>([](lua_State *l, pragma::BaseObservableComponent &hEnt, uint32_t camType, const Vector3 &origin) {
		hEnt.SetLocalCameraOrigin(static_cast<pragma::BaseObservableComponent::CameraType>(camType), origin);
	}));
	def.def("GetLocalCameraOrigin",
	  static_cast<void (*)(lua_State *, pragma::BaseObservableComponent &, uint32_t)>([](lua_State *l, pragma::BaseObservableComponent &hEnt, uint32_t camType) { Lua::Push<Vector3>(l, hEnt.GetLocalCameraOrigin(static_cast<pragma::BaseObservableComponent::CameraType>(camType))); }));
	def.def("SetLocalCameraOffset", static_cast<void (*)(lua_State *, pragma::BaseObservableComponent &, uint32_t, const Vector3 &)>([](lua_State *l, pragma::BaseObservableComponent &hEnt, uint32_t camType, const Vector3 &offset) {
		hEnt.SetLocalCameraOffset(static_cast<pragma::BaseObservableComponent::CameraType>(camType), offset);
	}));
	def.def("GetLocalCameraOffset",
	  static_cast<void (*)(lua_State *, pragma::BaseObservableComponent &, uint32_t)>([](lua_State *l, pragma::BaseObservableComponent &hEnt, uint32_t camType) { Lua::Push<Vector3>(l, hEnt.GetLocalCameraOffset(static_cast<pragma::BaseObservableComponent::CameraType>(camType))); }));
	def.def("GetCameraData", static_cast<void (*)(lua_State *, pragma::BaseObservableComponent &, uint32_t)>([](lua_State *l, pragma::BaseObservableComponent &hEnt, uint32_t camType) {
		Lua::Push<pragma::ObserverCameraData *>(l, &hEnt.GetCameraData(static_cast<pragma::BaseObservableComponent::CameraType>(camType)));
	}));
	def.def("SetCameraEnabled",
	  static_cast<void (*)(lua_State *, pragma::BaseObservableComponent &, uint32_t, bool)>([](lua_State *l, pragma::BaseObservableComponent &hEnt, uint32_t camType, bool enabled) { hEnt.SetCameraEnabled(static_cast<pragma::BaseObservableComponent::CameraType>(camType), enabled); }));
	def.def("GetCameraEnabledProperty", &pragma::BaseObservableComponent::GetCameraEnabledProperty);
	def.def("GetCameraOffsetProperty", &pragma::BaseObservableComponent::GetCameraOffsetProperty);
	def.def(
	  "GetObserver", +[](pragma::BaseObservableComponent &observableC) -> luabind::object {
		  auto *observerC = observableC.GetObserver();
		  if(!observerC)
			  return Lua::nil;
		  return observerC->GetLuaObject();
	  });
	def.def("GetViewOffset", &pragma::BaseObservableComponent::GetViewOffset);
	def.def("SetViewOffset", &pragma::BaseObservableComponent::SetViewOffset);
	def.add_static_constant("CAMERA_TYPE_FIRST_PERSON", umath::to_integral(pragma::BaseObservableComponent::CameraType::FirstPerson));
	def.add_static_constant("CAMERA_TYPE_THIRD_PERSON", umath::to_integral(pragma::BaseObservableComponent::CameraType::ThirdPerson));

	auto defObsCamData = luabind::class_<pragma::ObserverCameraData>("CameraData");
	defObsCamData.def_readwrite("rotateWithObservee", &pragma::ObserverCameraData::rotateWithObservee);
	defObsCamData.def("SetAngleLimits",
	  static_cast<void (*)(lua_State *, pragma::ObserverCameraData &, const EulerAngles &, const EulerAngles &)>([](lua_State *l, pragma::ObserverCameraData &obsCamData, const EulerAngles &min, const EulerAngles &max) { obsCamData.angleLimits = {min, max}; }));
	defObsCamData.def("GetAngleLimits", static_cast<void (*)(lua_State *, pragma::ObserverCameraData &)>([](lua_State *l, pragma::ObserverCameraData &obsCamData) {
		if(obsCamData.angleLimits.has_value() == false)
			return;
		Lua::Push<EulerAngles>(l, obsCamData.angleLimits->first);
		Lua::Push<EulerAngles>(l, obsCamData.angleLimits->second);
	}));
	defObsCamData.def("ClearAngleLimits", static_cast<void (*)(lua_State *, pragma::ObserverCameraData &)>([](lua_State *l, pragma::ObserverCameraData &obsCamData) { obsCamData.angleLimits = {}; }));
	defObsCamData.property("enabled", static_cast<void (*)(lua_State *, pragma::ObserverCameraData &)>([](lua_State *l, pragma::ObserverCameraData &obsCamData) { Lua::PushBool(l, *obsCamData.enabled); }),
	  static_cast<void (*)(lua_State *, pragma::ObserverCameraData &, bool)>([](lua_State *l, pragma::ObserverCameraData &obsCamData, bool enabled) { *obsCamData.enabled = enabled; }));
	defObsCamData.property("localOrigin", static_cast<void (*)(lua_State *, pragma::ObserverCameraData &)>([](lua_State *l, pragma::ObserverCameraData &obsCamData) {
		if(obsCamData.localOrigin.has_value() == false)
			return;
		Lua::Push<Vector3>(l, *obsCamData.localOrigin);
	}),
	  static_cast<void (*)(lua_State *, pragma::ObserverCameraData &, const Vector3 &)>([](lua_State *l, pragma::ObserverCameraData &obsCamData, const Vector3 &origin) { obsCamData.localOrigin = origin; }));
	defObsCamData.property("offset", static_cast<void (*)(lua_State *, pragma::ObserverCameraData &)>([](lua_State *l, pragma::ObserverCameraData &obsCamData) { Lua::Push<Vector3>(l, *obsCamData.offset); }),
	  static_cast<void (*)(lua_State *, pragma::ObserverCameraData &, const Vector3 &)>([](lua_State *l, pragma::ObserverCameraData &obsCamData, const Vector3 &offset) { *obsCamData.offset = offset; }));
	def.scope[defObsCamData];
}

#include "pragma/entities/components/base_shooter_component.hpp"
#include "pragma/util/bulletinfo.h"
namespace Lua::Shooter {
	void FireBullets(lua_State *l, pragma::BaseShooterComponent &hEnt, const luabind::object &, bool bHitReport, bool bMaster)
	{
		auto *bulletInfo = Lua::CheckBulletInfo(l, 2);

		std::vector<TraceResult> results;
		hEnt.FireBullets(*bulletInfo, results, bMaster);
		if(bHitReport == false)
			return;
		auto t = luabind::newtable(l);
		for(auto i = decltype(results.size()) {0}; i < results.size(); ++i) {
			auto &r = results[i];
			t[i + 1] = r;
		}
	}
};
void pragma::lua::base_shooter_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseShooterComponent>("BaseShooterComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("FireBullets",
	  static_cast<void (*)(lua_State *, pragma::BaseShooterComponent &, const luabind::object &, bool, bool)>([](lua_State *l, pragma::BaseShooterComponent &hEnt, const luabind::object &o, bool bHitReport, bool bMaster) { Lua::Shooter::FireBullets(l, hEnt, o, bHitReport, bMaster); }));
	def.def("FireBullets", static_cast<void (*)(lua_State *, pragma::BaseShooterComponent &, const luabind::object &, bool)>([](lua_State *l, pragma::BaseShooterComponent &hEnt, const luabind::object &o, bool bHitReport) {
		Lua::Shooter::FireBullets(l, hEnt, o, bHitReport, Lua::get_bullet_master(hEnt.GetEntity()));
	}));
	def.def("FireBullets",
	  static_cast<void (*)(lua_State *, pragma::BaseShooterComponent &, const luabind::object &)>([](lua_State *l, pragma::BaseShooterComponent &hEnt, const luabind::object &o) { Lua::Shooter::FireBullets(l, hEnt, o, false, Lua::get_bullet_master(hEnt.GetEntity())); }));
	def.add_static_constant("EVENT_ON_FIRE_BULLETS", pragma::BaseShooterComponent::EVENT_ON_FIRE_BULLETS);
}

#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/physics/constraint.hpp"
#include "pragma/physics/movetypes.h"
namespace Lua::Physics {
	void InitializePhysics(lua_State *l, pragma::BasePhysicsComponent &hEnt, ::util::TSharedHandle<pragma::physics::IConvexShape> &shape, uint32_t flags)
	{
		auto *phys = hEnt.InitializePhysics(*shape, static_cast<pragma::BasePhysicsComponent::PhysFlags>(flags));
		if(phys != NULL)
			luabind::object(l, phys->GetHandle()).push(l);
	}
	void InitializePhysics(lua_State *l, pragma::BasePhysicsComponent &hEnt, ::util::TSharedHandle<pragma::physics::IConvexShape> &shape) { InitializePhysics(l, hEnt, shape, umath::to_integral(pragma::BasePhysicsComponent::PhysFlags::None)); }
};
void pragma::lua::base_physics_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePhysicsComponent>("BasePhysicsComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetCollisionBounds", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, Vector3, Vector3)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, Vector3 min, Vector3 max) { hEnt.SetCollisionBounds(min, max); }));
	def.def("GetCollisionBounds", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt) {
		Vector3 min, max;
		hEnt.GetCollisionBounds(&min, &max);
		luabind::object(l, min).push(l);
		luabind::object(l, max).push(l);
	}));
	def.def("GetCollisionExtents", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt) { luabind::object(l, hEnt.GetCollisionExtents()).push(l); }));
	def.def("GetCollisionCenter", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt) { luabind::object(l, hEnt.GetCollisionCenter()).push(l); }));
	def.def("GetMoveType", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt) {
		MOVETYPE mt = hEnt.GetMoveType();
		Lua::PushInt(l, int(mt));
	}));
	def.def("SetMoveType", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, int)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, int moveType) { hEnt.SetMoveType(MOVETYPE(moveType)); }));
	def.def("GetPhysicsObject", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt) {
		PhysObj *phys = hEnt.GetPhysicsObject();
		if(phys == NULL)
			return;
		luabind::object(l, phys->GetHandle()).push(l);
	}));
	def.def("InitializePhysics", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, uint32_t, uint32_t)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, uint32_t type, uint32_t physFlags) {
		PhysObj *phys = hEnt.InitializePhysics(PHYSICSTYPE(type), static_cast<pragma::BasePhysicsComponent::PhysFlags>(physFlags));
		if(phys != NULL)
			luabind::object(l, phys->GetHandle()).push(l);
	}));
	def.def("InitializePhysics", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, uint32_t)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, uint32_t type) {
		PhysObj *phys = hEnt.InitializePhysics(PHYSICSTYPE(type));
		if(phys != NULL)
			luabind::object(l, phys->GetHandle()).push(l);
	}));
	def.def("InitializePhysics",
	  static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, ::util::TSharedHandle<pragma::physics::IConvexShape> &, uint32_t)>(
	    [](lua_State *l, pragma::BasePhysicsComponent &hEnt, ::util::TSharedHandle<pragma::physics::IConvexShape> &shape, uint32_t physFlags) { Lua::Physics::InitializePhysics(l, hEnt, shape, physFlags); }));
	def.def("InitializePhysics",
	  static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, ::util::TSharedHandle<pragma::physics::IConvexShape> &, uint32_t)>(
	    [](lua_State *l, pragma::BasePhysicsComponent &hEnt, ::util::TSharedHandle<pragma::physics::IConvexShape> &shape, uint32_t physFlags) { Lua::Physics::InitializePhysics(l, hEnt, shape, physFlags); }));
	def.def("InitializePhysics", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, ::util::TSharedHandle<pragma::physics::IConvexShape> &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, ::util::TSharedHandle<pragma::physics::IConvexShape> &shape) {
		Lua::Physics::InitializePhysics(l, hEnt, shape, 0);
	}));
	def.def("DestroyPhysicsObject", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt) { hEnt.DestroyPhysicsObject(); }));
	def.def("DropToFloor", &pragma::BasePhysicsComponent::DropToFloor);
	def.def("IsTrigger", &pragma::BasePhysicsComponent::IsTrigger);
	def.def("SetKinematic", &pragma::BasePhysicsComponent::SetKinematic);
	def.def("IsKinematic", &pragma::BasePhysicsComponent::IsKinematic);

	def.def("GetCollisionCallbacksEnabled", &pragma::BasePhysicsComponent::GetCollisionCallbacksEnabled);
	def.def("SetCollisionCallbacksEnabled", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, bool)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, bool) { hEnt.SetCollisionCallbacksEnabled(true); }));
	def.def("GetCollisionContactReportEnabled", &pragma::BasePhysicsComponent::GetCollisionContactReportEnabled);
	def.def("SetCollisionContactReportEnabled", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, bool)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, bool) { hEnt.SetCollisionContactReportEnabled(true); }));
	def.def("SetCollisionFilterMask", &pragma::BasePhysicsComponent::SetCollisionFilterMask);
	def.def("GetCollisionFilterMask", &pragma::BasePhysicsComponent::GetCollisionFilterMask);
	def.def("SetCollisionFilterGroup", &pragma::BasePhysicsComponent::SetCollisionFilterGroup);
	def.def("GetCollisionFilterGroup", static_cast<CollisionMask (pragma::BasePhysicsComponent::*)() const>(&pragma::BasePhysicsComponent::GetCollisionFilter));
	def.def("SetCollisionFilter", static_cast<void (pragma::BasePhysicsComponent::*)(CollisionMask, CollisionMask)>(&pragma::BasePhysicsComponent::SetCollisionFilter));
	def.def("EnableCollisions", &pragma::BasePhysicsComponent::EnableCollisions);
	def.def("DisableCollisions", &pragma::BasePhysicsComponent::DisableCollisions);
	def.def("SetCollisionsEnabled", static_cast<void (pragma::BasePhysicsComponent::*)(bool)>(&pragma::BasePhysicsComponent::SetCollisionsEnabled));
	def.def("SetCollisionsEnabled", static_cast<void (pragma::BasePhysicsComponent::*)(BaseEntity *, bool)>(&pragma::BasePhysicsComponent::SetCollisionsEnabled));
	def.def("EnableCollisions", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt) { hEnt.SetCollisionsEnabled(true); }));
	def.def("DisableCollisions", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt) { hEnt.SetCollisionsEnabled(false); }));
	def.def("SetSimulationEnabled", &pragma::BasePhysicsComponent::SetSimulationEnabled);
	def.def("IsSimulationEnabled", &pragma::BasePhysicsComponent::GetSimulationEnabled);
	def.def("ResetCollisions", &pragma::BasePhysicsComponent::ResetCollisions);

	def.def("GetPhysJointConstraints", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt) {
		auto &joints = hEnt.GetPhysConstraints();
		auto table = Lua::CreateTable(l); /* 1 */
		auto n = 1;
		for(auto it = joints.begin(); it != joints.end(); ++it) {
			auto &joint = *it;
			if(joint.constraint.IsValid()) {
				auto tConstraint = Lua::CreateTable(l); /* 2 */

				Lua::PushString(l, "source");       /* 3 */
				Lua::PushInt(l, joint.source);      /* 4 */
				Lua::SetTableValue(l, tConstraint); /* 2 */

				Lua::PushString(l, "target");       /* 3 */
				Lua::PushInt(l, joint.target);      /* 4 */
				Lua::SetTableValue(l, tConstraint); /* 2 */

				Lua::PushString(l, "constraint");   /* 3 */
				joint.constraint->Push(l);          /* 4 */
				Lua::SetTableValue(l, tConstraint); /* 2 */

				Lua::PushInt(l, n);             /* 3 */
				Lua::PushValue(l, tConstraint); /* 4 */
				Lua::SetTableValue(l, table);   /* 2 */
				Lua::Pop(l, 1);                 /* 1 */
				n++;
			}
		}
	}));
	def.def("GetRotatedCollisionBounds", &pragma::BasePhysicsComponent::GetRotatedCollisionBounds, luabind::meta::join<luabind::out_value<2>, luabind::out_value<3>>::type {});

	def.def("GetPhysicsType", &pragma::BasePhysicsComponent::GetPhysicsType);
	def.def("GetCollisionRadius", &pragma::BasePhysicsComponent::GetCollisionRadius);
	def.def("IsPhysicsProp", static_cast<bool (*)(lua_State *, pragma::BasePhysicsComponent &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt) {
		auto physType = hEnt.GetPhysicsType();
		return (physType != PHYSICSTYPE::NONE && physType != PHYSICSTYPE::STATIC && physType != PHYSICSTYPE::BOXCONTROLLER && physType != PHYSICSTYPE::CAPSULECONTROLLER) ? true : false;
	}));

	def.def("GetAABBDistance", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, const Vector3 &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, const Vector3 &p) { Lua::PushNumber(l, hEnt.GetAABBDistance(p)); }));
	def.def("GetAABBDistance", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, pragma::BasePhysicsComponent &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, pragma::BasePhysicsComponent &hOther) { Lua::PushNumber(l, hEnt.GetAABBDistance(hOther.GetEntity())); }));
	def.def("GetAABBDistance", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, BaseEntity &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, BaseEntity &other) { Lua::PushNumber(l, hEnt.GetAABBDistance(other)); }));

	def.def("IsRagdoll", &pragma::BasePhysicsComponent::IsRagdoll);

	def.def("ApplyForce", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, const Vector3 &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, const Vector3 &force) { hEnt.ApplyForce(force); }));
	def.def("ApplyForce", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, const Vector3 &, const Vector3 &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, const Vector3 &force, const Vector3 &relPos) { hEnt.ApplyForce(force, relPos); }));
	def.def("ApplyImpulse", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, const Vector3 &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, const Vector3 &impulse) { hEnt.ApplyImpulse(impulse); }));
	def.def("ApplyImpulse", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, const Vector3 &, const Vector3 &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, const Vector3 &impulse, const Vector3 &relPos) { hEnt.ApplyImpulse(impulse, relPos); }));
	def.def("ApplyTorque", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, const Vector3 &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, const Vector3 &torque) { hEnt.ApplyTorque(torque); }));
	def.def("ApplyTorqueImpulse", static_cast<void (*)(lua_State *, pragma::BasePhysicsComponent &, const Vector3 &)>([](lua_State *l, pragma::BasePhysicsComponent &hEnt, const Vector3 &torque) { hEnt.ApplyTorqueImpulse(torque); }));
	def.def("GetMass", &pragma::BasePhysicsComponent::GetMass);
	def.add_static_constant("EVENT_ON_PHYSICS_INITIALIZED", pragma::BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED);
	def.add_static_constant("EVENT_ON_PHYSICS_DESTROYED", pragma::BasePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED);
	def.add_static_constant("EVENT_ON_PHYSICS_UPDATED", pragma::BasePhysicsComponent::EVENT_ON_PHYSICS_UPDATED);
	def.add_static_constant("EVENT_ON_DYNAMIC_PHYSICS_UPDATED", pragma::BasePhysicsComponent::EVENT_ON_DYNAMIC_PHYSICS_UPDATED);
	def.add_static_constant("EVENT_ON_PRE_PHYSICS_SIMULATE", pragma::BasePhysicsComponent::EVENT_ON_PRE_PHYSICS_SIMULATE);
	def.add_static_constant("EVENT_ON_POST_PHYSICS_SIMULATE", pragma::BasePhysicsComponent::EVENT_ON_POST_PHYSICS_SIMULATE);
	def.add_static_constant("EVENT_ON_WAKE", pragma::BasePhysicsComponent::EVENT_ON_WAKE);
	def.add_static_constant("EVENT_ON_SLEEP", pragma::BasePhysicsComponent::EVENT_ON_SLEEP);
	def.add_static_constant("EVENT_HANDLE_RAYCAST", pragma::BasePhysicsComponent::EVENT_HANDLE_RAYCAST);
	def.add_static_constant("EVENT_INITIALIZE_PHYSICS", pragma::BasePhysicsComponent::EVENT_INITIALIZE_PHYSICS);

	def.add_static_constant("MOVETYPE_NONE", umath::to_integral(MOVETYPE::NONE));
	def.add_static_constant("MOVETYPE_WALK", umath::to_integral(MOVETYPE::WALK));
	def.add_static_constant("MOVETYPE_NOCLIP", umath::to_integral(MOVETYPE::NOCLIP));
	def.add_static_constant("MOVETYPE_FLY", umath::to_integral(MOVETYPE::FLY));
	def.add_static_constant("MOVETYPE_FREE", umath::to_integral(MOVETYPE::FREE));
	def.add_static_constant("MOVETYPE_PHYSICS", umath::to_integral(MOVETYPE::PHYSICS));
}

#include "pragma/entities/components/base_render_component.hpp"
void pragma::lua::base_render_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseRenderComponent>("BaseRenderComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetCastShadows", &pragma::BaseRenderComponent::SetCastShadows);
	def.def("GetCastShadows", &pragma::BaseRenderComponent::GetCastShadows);
}

#include "pragma/entities/components/base_softbody_component.hpp"
void pragma::lua::base_soft_body_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseSoftBodyComponent>("BaseSoftBodyComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/components/base_sound_emitter_component.hpp"
namespace Lua::SoundEmitter {
	DLLNETWORK luabind::class_<pragma::BaseSoundEmitterComponent::SoundInfo> RegisterSoundInfo()
	{
		auto defSoundInfo = luabind::class_<pragma::BaseSoundEmitterComponent::SoundInfo>("SoundInfo");
		defSoundInfo.def(luabind::constructor<float, float>());
		defSoundInfo.def(luabind::constructor<float>());
		defSoundInfo.def(luabind::constructor<>());
		defSoundInfo.def_readwrite("gain", &pragma::BaseSoundEmitterComponent::SoundInfo::gain);
		defSoundInfo.def_readwrite("pitch", &pragma::BaseSoundEmitterComponent::SoundInfo::pitch);
		defSoundInfo.def_readwrite("transmit", &pragma::BaseSoundEmitterComponent::SoundInfo::transmit);
		return defSoundInfo;
	}
};
void pragma::lua::base_sound_emitter_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseSoundEmitterComponent>("BaseSoundEmitterComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("CreateSound", &pragma::BaseSoundEmitterComponent::CreateSound);
	def.def("CreateSound", +[](pragma::BaseSoundEmitterComponent &c, std::string snd, ALSoundType type) { return c.CreateSound(std::move(snd), type); });
	def.def("EmitSound", &pragma::BaseSoundEmitterComponent::EmitSound);
	def.def("EmitSound", +[](pragma::BaseSoundEmitterComponent &c, std::string snd, ALSoundType type) { return c.EmitSound(std::move(snd), type); });
	def.def("StopSounds", &pragma::BaseSoundEmitterComponent::StopSounds);
	def.def("GetSounds", static_cast<void (*)(lua_State *, pragma::BaseSoundEmitterComponent &)>([](lua_State *l, pragma::BaseSoundEmitterComponent &hEnt) {
		std::vector<std::shared_ptr<ALSound>> *sounds;
		hEnt.GetSounds(&sounds);
		lua_newtable(l);
		int top = lua_gettop(l);
		for(int i = 0; i < sounds->size(); i++) {
			luabind::object(l, (*sounds)[i]).push(l);
			lua_rawseti(l, top, i + 1);
		}
	}));
	def.add_static_constant("EVENT_ON_SOUND_CREATED", pragma::BaseSoundEmitterComponent::EVENT_ON_SOUND_CREATED);
}

#include "pragma/entities/components/base_transform_component.hpp"
namespace Lua::Transform {
	void GetDirection(lua_State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther, bool bIgnoreYAxis) { Lua::Push<Vector3>(l, hEnt.GetDirection(hOther.GetEntity(), bIgnoreYAxis)); }

	void GetAngles(lua_State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther, bool bIgnoreYAxis) { Lua::Push<EulerAngles>(l, hEnt.GetAngles(hOther.GetEntity(), bIgnoreYAxis)); }

	void GetDotProduct(lua_State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther, bool bIgnoreYAxis) { Lua::PushNumber(l, hEnt.GetDotProduct(hOther.GetEntity(), bIgnoreYAxis)); }
};
void pragma::lua::base_transform_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseTransformComponent>("BaseTransformComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetOrigin", &pragma::BaseTransformComponent::GetOrigin);
	def.def("GetPos", static_cast<const Vector3 &(pragma::BaseTransformComponent::*)() const>(&pragma::BaseTransformComponent::GetPosition), luabind::copy_policy<0> {});
	def.def("GetPos", static_cast<Vector3 (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetPosition));
	def.def("SetPos", static_cast<void (pragma::BaseTransformComponent::*)(const Vector3 &)>(&pragma::BaseTransformComponent::SetPosition));
	def.def("SetPos", static_cast<void (pragma::BaseTransformComponent::*)(const Vector3 &, bool, pragma::CoordinateSpace)>(&pragma::BaseTransformComponent::SetPosition));
	def.def("SetPos", static_cast<void (pragma::BaseTransformComponent::*)(const Vector3 &, bool, pragma::CoordinateSpace)>(&pragma::BaseTransformComponent::SetPosition), luabind::default_parameter_policy<4, pragma::CoordinateSpace::World> {});
	def.def("GetRotation", static_cast<const Quat &(pragma::BaseTransformComponent::*)() const>(&pragma::BaseTransformComponent::GetRotation), luabind::copy_policy<0> {});
	def.def("GetRotation", static_cast<Quat (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetRotation));
	def.def("SetRotation", static_cast<void (pragma::BaseTransformComponent::*)(const Quat &)>(&pragma::BaseTransformComponent::SetRotation));
	def.def("SetRotation", static_cast<void (pragma::BaseTransformComponent::*)(const Quat &, pragma::CoordinateSpace)>(&pragma::BaseTransformComponent::SetRotation));
	def.def("GetAngles", static_cast<EulerAngles (pragma::BaseTransformComponent::*)(const Vector3 &, bool) const>(&pragma::BaseTransformComponent::GetAngles));
	def.def("GetAngles", static_cast<EulerAngles (pragma::BaseTransformComponent::*)(const Vector3 &, bool) const>(&pragma::BaseTransformComponent::GetAngles), luabind::default_parameter_policy<3, false> {});
	def.def("GetAngles", static_cast<EulerAngles (pragma::BaseTransformComponent::*)(const BaseEntity &, bool) const>(&pragma::BaseTransformComponent::GetAngles));
	def.def("GetAngles", static_cast<EulerAngles (pragma::BaseTransformComponent::*)(const BaseEntity &, bool) const>(&pragma::BaseTransformComponent::GetAngles), luabind::default_parameter_policy<3, false> {});
	def.def("GetAngles", static_cast<EulerAngles (pragma::BaseTransformComponent::*)() const>(&pragma::BaseTransformComponent::GetAngles));
	def.def("SetAngles", static_cast<void (pragma::BaseTransformComponent::*)(const EulerAngles &, pragma::CoordinateSpace)>(&pragma::BaseTransformComponent::SetAngles));
	def.def("SetAngles", static_cast<void (pragma::BaseTransformComponent::*)(const EulerAngles &)>(&pragma::BaseTransformComponent::SetAngles));
	def.def("GetForward", static_cast<Vector3 (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetForward));
	def.def("GetForward", static_cast<Vector3 (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetForward), luabind::default_parameter_policy<4, pragma::CoordinateSpace::World> {});
	def.def("GetRight", static_cast<Vector3 (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetRight));
	def.def("GetRight", static_cast<Vector3 (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetRight), luabind::default_parameter_policy<4, pragma::CoordinateSpace::World> {});
	def.def("GetUp", static_cast<Vector3 (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetUp));
	def.def("GetUp", static_cast<Vector3 (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetUp), luabind::default_parameter_policy<4, pragma::CoordinateSpace::World> {});

	def.def("GetEyePos", &pragma::BaseTransformComponent::GetEyePosition);
	def.def("GetEyeOffset", &pragma::BaseTransformComponent::GetEyeOffset);
	def.def("SetEyeOffset", &pragma::BaseTransformComponent::SetEyeOffset);

	def.def("LocalToWorld", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, Vector3)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, Vector3 origin) {
		hEnt.LocalToWorld(&origin);
		Lua::Push<Vector3>(l, origin);
	}));
	def.def("LocalToWorld", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, Quat)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, Quat rot) {
		hEnt.LocalToWorld(&rot);
		Lua::Push<Quat>(l, rot);
	}));
	def.def("LocalToWorld", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, Vector3, Quat)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, Vector3 origin, Quat rot) {
		hEnt.LocalToWorld(&origin, &rot);
		Lua::Push<Vector3>(l, origin);
		Lua::Push<Quat>(l, rot);
	}));
	def.def("WorldToLocal", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, Vector3)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, Vector3 origin) {
		hEnt.WorldToLocal(&origin);
		Lua::Push<Vector3>(l, origin);
	}));
	def.def("WorldToLocal", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, Quat)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, Quat rot) {
		hEnt.WorldToLocal(&rot);
		Lua::Push<Quat>(l, rot);
	}));
	def.def("WorldToLocal", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, Vector3, Quat)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, Vector3 origin, Quat rot) {
		hEnt.WorldToLocal(&origin, &rot);
		Lua::Push<Vector3>(l, origin);
		Lua::Push<Quat>(l, rot);
	}));

	def.def("GetPitch", static_cast<float (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetPitch));
	def.def("GetPitch", static_cast<float (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetPitch), luabind::default_parameter_policy<2, pragma::CoordinateSpace::World> {});
	def.def("GetYaw", static_cast<float (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetYaw));
	def.def("GetYaw", static_cast<float (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetYaw), luabind::default_parameter_policy<2, pragma::CoordinateSpace::World> {});
	def.def("GetRoll", static_cast<float (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetRoll));
	def.def("GetRoll", static_cast<float (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetRoll), luabind::default_parameter_policy<2, pragma::CoordinateSpace::World> {});
	def.def("SetPitch", static_cast<void (pragma::BaseTransformComponent::*)(float, pragma::CoordinateSpace)>(&pragma::BaseTransformComponent::SetPitch));
	def.def("SetPitch", static_cast<void (pragma::BaseTransformComponent::*)(float, pragma::CoordinateSpace)>(&pragma::BaseTransformComponent::SetPitch), luabind::default_parameter_policy<3, pragma::CoordinateSpace::World> {});
	def.def("SetYaw", static_cast<void (pragma::BaseTransformComponent::*)(float, pragma::CoordinateSpace)>(&pragma::BaseTransformComponent::SetYaw));
	def.def("SetYaw", static_cast<void (pragma::BaseTransformComponent::*)(float, pragma::CoordinateSpace)>(&pragma::BaseTransformComponent::SetYaw), luabind::default_parameter_policy<3, pragma::CoordinateSpace::World> {});
	def.def("SetRoll", static_cast<void (pragma::BaseTransformComponent::*)(float, pragma::CoordinateSpace)>(&pragma::BaseTransformComponent::SetRoll));
	def.def("SetRoll", static_cast<void (pragma::BaseTransformComponent::*)(float, pragma::CoordinateSpace)>(&pragma::BaseTransformComponent::SetRoll), luabind::default_parameter_policy<3, pragma::CoordinateSpace::World> {});

	def.def("GetLastMoveTime", &pragma::BaseTransformComponent::GetLastMoveTime);
	def.def("GetScale", static_cast<const Vector3 &(pragma::BaseTransformComponent::*)() const>(&pragma::BaseTransformComponent::GetScale), luabind::copy_policy<0> {});
	def.def("GetScale", static_cast<Vector3 (pragma::BaseTransformComponent::*)(pragma::CoordinateSpace) const>(&pragma::BaseTransformComponent::GetScale));
	def.def("GetMaxAxisScale", &pragma::BaseTransformComponent::GetMaxAxisScale);
	def.def("GetAbsMaxAxisScale", &pragma::BaseTransformComponent::GetAbsMaxAxisScale);
	def.def("SetScale", static_cast<void (pragma::BaseTransformComponent::*)(float)>(&pragma::BaseTransformComponent::SetScale));
	def.def("SetScale", static_cast<void (pragma::BaseTransformComponent::*)(const Vector3 &)>(&pragma::BaseTransformComponent::SetScale));
	def.def("SetScale", static_cast<void (pragma::BaseTransformComponent::*)(const Vector3 &, pragma::CoordinateSpace)>(&pragma::BaseTransformComponent::SetScale));

	def.def("GetDistance", static_cast<float (pragma::BaseTransformComponent::*)(const Vector3 &) const>(&pragma::BaseTransformComponent::GetDistance));
	def.def("GetDistance", static_cast<float (pragma::BaseTransformComponent::*)(const BaseEntity &) const>(&pragma::BaseTransformComponent::GetDistance));
	def.def("GetDistance", static_cast<float (*)(lua_State *, pragma::BaseTransformComponent &, pragma::BaseTransformComponent &)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther) { return hEnt.GetDistance(hOther.GetEntity()); }));

	def.def("GetDirection", static_cast<Vector3 (pragma::BaseTransformComponent::*)(const BaseEntity &, bool) const>(&pragma::BaseTransformComponent::GetDirection));
	def.def("GetDirection", static_cast<Vector3 (pragma::BaseTransformComponent::*)(const BaseEntity &, bool) const>(&pragma::BaseTransformComponent::GetDirection), luabind::default_parameter_policy<3, false> {});
	def.def("GetDirection", static_cast<Vector3 (pragma::BaseTransformComponent::*)(const Vector3 &, bool) const>(&pragma::BaseTransformComponent::GetDirection));
	def.def("GetDirection", static_cast<Vector3 (pragma::BaseTransformComponent::*)(const Vector3 &, bool) const>(&pragma::BaseTransformComponent::GetDirection), luabind::default_parameter_policy<3, false> {});
	def.def("GetDirection", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, pragma::BaseTransformComponent &, bool)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther, bool bIgnoreYAxis) {
		Lua::Push<Vector3>(l, hEnt.GetDirection(hOther.GetEntity(), bIgnoreYAxis));
	}));

	def.def("GetAngles", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, pragma::BaseTransformComponent &, bool)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther, bool bIgnoreYAxis) {
		Lua::Push<EulerAngles>(l, hEnt.GetAngles(hOther.GetEntity(), bIgnoreYAxis));
	}));
	def.def("GetAngles", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, BaseEntity &, bool)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, BaseEntity &other, bool bIgnoreYAxis) { Lua::Push<EulerAngles>(l, hEnt.GetAngles(other, bIgnoreYAxis)); }));
	def.def("GetDotProduct", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, const Vector3 &, bool)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, const Vector3 &p, bool bIgnoreYAxis) { Lua::PushNumber(l, hEnt.GetDotProduct(p, bIgnoreYAxis)); }));
	def.def("GetDotProduct", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, pragma::BaseTransformComponent &, bool)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther, bool bIgnoreYAxis) {
		Lua::PushNumber(l, hEnt.GetDotProduct(hOther.GetEntity(), bIgnoreYAxis));
	}));
	def.def("GetDotProduct", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, BaseEntity &, bool)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, BaseEntity &other, bool bIgnoreYAxis) { Lua::PushNumber(l, hEnt.GetDotProduct(other, bIgnoreYAxis)); }));

	def.def("GetDirection", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, pragma::BaseTransformComponent &)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther) { Lua::Transform::GetDirection(l, hEnt, hOther, false); }));
	def.def("GetAngles", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, pragma::BaseTransformComponent &)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther) { Lua::Transform::GetAngles(l, hEnt, hOther, false); }));
	def.def("GetDotProduct", static_cast<void (*)(lua_State *, pragma::BaseTransformComponent &, pragma::BaseTransformComponent &)>([](lua_State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther) { Lua::Transform::GetDotProduct(l, hEnt, hOther, false); }));
	def.def("GetDotProduct", static_cast<float (pragma::BaseTransformComponent::*)(const BaseEntity &, bool) const>(&pragma::BaseTransformComponent::GetDotProduct));
	def.def("GetDotProduct", static_cast<float (pragma::BaseTransformComponent::*)(const BaseEntity &, bool) const>(&pragma::BaseTransformComponent::GetDotProduct), luabind::default_parameter_policy<3, false> {});
	def.def("GetDotProduct", static_cast<float (pragma::BaseTransformComponent::*)(const Vector3 &, bool) const>(&pragma::BaseTransformComponent::GetDotProduct));
	def.def("GetDotProduct", static_cast<float (pragma::BaseTransformComponent::*)(const Vector3 &, bool) const>(&pragma::BaseTransformComponent::GetDotProduct), luabind::default_parameter_policy<3, false> {});
}

#include "pragma/entities/components/base_color_component.hpp"
void pragma::lua::base_color_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseColorComponent>("BaseColorComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetColorProperty", &pragma::BaseColorComponent::GetColorProperty);

	def.def("GetColor", &pragma::BaseColorComponent::GetColor, luabind::copy_policy<0> {});
	def.def("SetColor", static_cast<void (pragma::BaseColorComponent::*)(const Color &)>(&pragma::BaseColorComponent::SetColor));
	def.def("SetColor", static_cast<void (pragma::BaseColorComponent::*)(const Vector3 &)>(&pragma::BaseColorComponent::SetColor));
	def.def("SetColor", static_cast<void (pragma::BaseColorComponent::*)(const Vector4 &)>(&pragma::BaseColorComponent::SetColor));
	def.add_static_constant("EVENT_ON_COLOR_CHANGED", pragma::BaseColorComponent::EVENT_ON_COLOR_CHANGED);
}

#include "pragma/entities/components/base_surface_component.hpp"

static std::optional<std::tuple<std::shared_ptr<ModelMesh>, std::shared_ptr<ModelSubMesh>, Material *>> FindAndAssignSurfaceMesh(pragma::BaseSurfaceComponent &c, luabind::object oFilter)
{
	std::function<int32_t(ModelMesh &, ModelSubMesh &, Material &, const std::string &)> filter = nullptr;
	if(oFilter) {
		filter = [&oFilter](ModelMesh &mesh, ModelSubMesh &subMesh, Material &mat, const std::string &shader) -> int32_t {
			auto res = oFilter(&mat, shader);
			return luabind::object_cast<int32_t>(res);
		};
	}
	auto res = c.FindAndAssignMesh(filter);
	if(!res.has_value())
		return {};
	return std::tuple<std::shared_ptr<ModelMesh>, std::shared_ptr<ModelSubMesh>, Material *> {res->mesh->shared_from_this(), res->subMesh->shared_from_this(), res->material};
}

void pragma::lua::base_surface_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseSurfaceComponent>("BaseSurfaceComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetPlane", static_cast<void (pragma::BaseSurfaceComponent::*)(const umath::Plane &)>(&pragma::BaseSurfaceComponent::SetPlane));
	def.def("SetPlane", static_cast<void (pragma::BaseSurfaceComponent::*)(const Vector3 &, float)>(&pragma::BaseSurfaceComponent::SetPlane));
	def.def("GetPlane", static_cast<const umath::Plane &(pragma::BaseSurfaceComponent::*)() const>(&pragma::BaseSurfaceComponent::GetPlane), luabind::copy_policy<0> {});
	def.def("GetPlaneWs", static_cast<umath::Plane (pragma::BaseSurfaceComponent::*)() const>(&pragma::BaseSurfaceComponent::GetPlaneWs));
	def.def("GetPlaneNormal", &pragma::BaseSurfaceComponent::GetPlaneNormal);
	def.def("SetPlaneNormal", &pragma::BaseSurfaceComponent::SetPlaneNormal);
	def.def("GetPlaneDistance", &pragma::BaseSurfaceComponent::GetPlaneDistance);
	def.def("SetPlaneDistance", &pragma::BaseSurfaceComponent::SetPlaneDistance);
	def.def("ProjectToSurface", &pragma::BaseSurfaceComponent::ProjectToSurface);
	def.def("Clear", &pragma::BaseSurfaceComponent::Clear);
	def.def("IsPointBelowSurface", &pragma::BaseSurfaceComponent::IsPointBelowSurface);
	def.def("GetPlaneRotation", &pragma::BaseSurfaceComponent::GetPlaneRotation);
	def.def(
	  "CalcLineSurfaceIntersection", +[](pragma::BaseSurfaceComponent &c, const Vector3 &lineOrigin, const Vector3 &lineDir) {
		  double t;
		  auto r = c.CalcLineSurfaceIntersection(lineOrigin, lineDir, &t);
		  return std::pair<bool, double> {r, t};
	  });
	def.def("GetMesh", static_cast<ModelSubMesh *(pragma::BaseSurfaceComponent::*)()>(&pragma::BaseSurfaceComponent::GetMesh), luabind::shared_from_this_policy<0> {});
	def.def("FindAndAssignSurfaceMesh", +[](pragma::BaseSurfaceComponent &c, luabind::object oFilter) -> std::optional<std::tuple<std::shared_ptr<ModelMesh>, std::shared_ptr<ModelSubMesh>, Material *>> { return FindAndAssignSurfaceMesh(c, oFilter); });
	def.def("FindAndAssignSurfaceMesh", +[](pragma::BaseSurfaceComponent &c) -> std::optional<std::tuple<std::shared_ptr<ModelMesh>, std::shared_ptr<ModelSubMesh>, Material *>> { return FindAndAssignSurfaceMesh(c, Lua::nil); });
	def.add_static_constant("EVENT_ON_SURFACE_PLANE_CHANGED", pragma::BaseSurfaceComponent::EVENT_ON_SURFACE_PLANE_CHANGED);
}

#include "pragma/entities/components/base_score_component.hpp"
void pragma::lua::base_score_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseScoreComponent>("BaseScoreComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetScoreProperty", &pragma::BaseScoreComponent::GetScoreProperty);
	def.def("GetScore", &pragma::BaseScoreComponent::GetScore);
	def.def("SetScore", &pragma::BaseScoreComponent::SetScore);
	def.def("AddScore", &pragma::BaseScoreComponent::AddScore);
	def.def("SubtractScore", &pragma::BaseScoreComponent::SubtractScore);
	def.add_static_constant("EVENT_ON_SCORE_CHANGED", pragma::BaseScoreComponent::EVENT_ON_SCORE_CHANGED);
}

#include "pragma/entities/components/base_radius_component.hpp"
void pragma::lua::base_radius_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseRadiusComponent>("BaseRadiusComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetRadiusProperty", &pragma::BaseRadiusComponent::GetRadiusProperty);
	def.def("GetRadius", &pragma::BaseRadiusComponent::GetRadius);
	def.def("SetRadius", &pragma::BaseRadiusComponent::SetRadius);
	def.add_static_constant("EVENT_ON_RADIUS_CHANGED", pragma::BaseRadiusComponent::EVENT_ON_RADIUS_CHANGED);
}

#include "pragma/entities/components/base_field_angle_component.hpp"
void pragma::lua::base_field_angle_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFieldAngleComponent>("BaseFieldAngleComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetFieldAngleProperty", &pragma::BaseFieldAngleComponent::GetFieldAngleProperty);
	def.def("GetFieldAngle", &pragma::BaseFieldAngleComponent::GetFieldAngle);
	def.def("SetFieldAngle", &pragma::BaseFieldAngleComponent::SetFieldAngle);
	def.add_static_constant("EVENT_ON_FIELD_ANGLE_CHANGED", pragma::BaseFieldAngleComponent::EVENT_ON_FIELD_ANGLE_CHANGED);
}

#include "pragma/entities/environment/audio/env_sound_dsp.h"
void pragma::lua::base_env_sound_dsp_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvSoundDspComponent>("BaseEnvSoundDspComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/env_camera.h"
void pragma::lua::base_env_camera_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvCameraComponent>("BaseEnvCameraComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.add_static_constant("DEFAULT_NEAR_Z", pragma::BaseEnvCameraComponent::DEFAULT_NEAR_Z);
	def.add_static_constant("DEFAULT_FAR_Z", pragma::BaseEnvCameraComponent::DEFAULT_FAR_Z);
	def.add_static_constant("DEFAULT_FOV", pragma::BaseEnvCameraComponent::DEFAULT_FOV);
	def.add_static_constant("DEFAULT_VIEWMODEL_FOV", pragma::BaseEnvCameraComponent::DEFAULT_VIEWMODEL_FOV);
	def.scope[luabind::def("calc_projection_matrix", +[](umath::Radian fov, float aspectRatio, float nearZ, float farZ, const rendering::Tile *optTile) -> Mat4 { return pragma::BaseEnvCameraComponent::CalcProjectionMatrix(fov, aspectRatio, nearZ, farZ, optTile); })];
	def.def("GetProjectionMatrix", &pragma::BaseEnvCameraComponent::GetProjectionMatrix, luabind::copy_policy<0> {});
	def.def("GetViewMatrix", &pragma::BaseEnvCameraComponent::GetViewMatrix, luabind::copy_policy<0> {});
	def.def("LookAt", static_cast<void (*)(lua_State *, pragma::BaseEnvCameraComponent &, const Vector3 &)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent, const Vector3 &lookAtPos) {
		auto *trComponent = hComponent.GetEntity().GetTransformComponent();
		if(!trComponent)
			return;
		auto &camPos = trComponent->GetPosition();
		auto dir = lookAtPos - camPos;
		uvec::normalize(&dir);
		trComponent->SetRotation(uquat::create_look_rotation(dir, trComponent->GetUp()));
	}));
	def.def("UpdateMatrices", &pragma::BaseEnvCameraComponent::UpdateMatrices);
	def.def("UpdateViewMatrix", &pragma::BaseEnvCameraComponent::UpdateViewMatrix);
	def.def("UpdateProjectionMatrix", &pragma::BaseEnvCameraComponent::UpdateProjectionMatrix);
	def.def("SetFOV", &pragma::BaseEnvCameraComponent::SetFOV);
	def.def("GetProjectionMatrixProperty", &pragma::BaseEnvCameraComponent::GetProjectionMatrixProperty);
	def.def("GetViewMatrixProperty", &pragma::BaseEnvCameraComponent::GetViewMatrixProperty);
	def.def("GetNearZProperty", &pragma::BaseEnvCameraComponent::GetNearZProperty);
	def.def("GetFarZProperty", &pragma::BaseEnvCameraComponent::GetFarZProperty);
	def.def("GetFOVProperty", &pragma::BaseEnvCameraComponent::GetFOVProperty);
	def.def("GetAspectRatioProperty", &pragma::BaseEnvCameraComponent::GetAspectRatioProperty);
	def.def("SetAspectRatio", &pragma::BaseEnvCameraComponent::SetAspectRatio);
	def.def("SetNearZ", &pragma::BaseEnvCameraComponent::SetNearZ);
	def.def("SetFarZ", &pragma::BaseEnvCameraComponent::SetFarZ);
	def.def("GetFOV", &pragma::BaseEnvCameraComponent::GetFOV);
	def.def("GetFOVRad", &pragma::BaseEnvCameraComponent::GetFOVRad);
	def.def("GetAspectRatio", &pragma::BaseEnvCameraComponent::GetAspectRatio);
	def.def("GetNearZ", &pragma::BaseEnvCameraComponent::GetNearZ);
	def.def("GetFarZ", &pragma::BaseEnvCameraComponent::GetFarZ);
	def.def("GetFocalDistanceProperty", &pragma::BaseEnvCameraComponent::GetFocalDistanceProperty);
	def.def("GetFocalDistance", &pragma::BaseEnvCameraComponent::GetFocalDistance);
	def.def("SetFocalDistance", &pragma::BaseEnvCameraComponent::SetFocalDistance);
	def.def("GetFrustumPlanes", static_cast<std::vector<umath::Plane> (*)(lua_State *, pragma::BaseEnvCameraComponent &)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent) {
		std::vector<umath::Plane> planes;
		hComponent.GetFrustumPlanes(planes);
		return planes;
	}));
	def.def("GetFarPlaneCenter", &pragma::BaseEnvCameraComponent::GetFarPlaneCenter);
	def.def("GetNearPlaneCenter", &pragma::BaseEnvCameraComponent::GetNearPlaneCenter);
	def.def("GetPlaneCenter", &pragma::BaseEnvCameraComponent::GetPlaneCenter);
	def.def("GetFarPlaneBoundaries", static_cast<std::array<Vector3, 4> (*)(lua_State *, pragma::BaseEnvCameraComponent &)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent) -> std::array<Vector3, 4> {
		std::array<Vector3, 4> farBounds;
		hComponent.GetFarPlaneBoundaries(farBounds);
		return farBounds;
	}));
	def.def("GetNearPlaneBoundaries", static_cast<std::array<Vector3, 4> (*)(lua_State *, pragma::BaseEnvCameraComponent &)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent) {
		std::array<Vector3, 4> nearBounds;
		hComponent.GetNearPlaneBoundaries(nearBounds);
		return nearBounds;
	}));
	def.def("GetPlaneBoundaries", static_cast<std::array<Vector3, 4> (*)(lua_State *, pragma::BaseEnvCameraComponent &, float)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent, float z) {
		std::array<Vector3, 4> bounds;
		hComponent.GetPlaneBoundaries(z, bounds);
		return bounds;
	}));
	def.def("GetPlaneBoundaries", static_cast<Lua::mult<luabind::tableT<Vector3>, luabind::tableT<Vector3>> (*)(lua_State *, pragma::BaseEnvCameraComponent &)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent) -> Lua::mult<luabind::tableT<Vector3>, luabind::tableT<Vector3>> {
		std::array<Vector3, 8> bounds;
		hComponent.GetPlaneBoundaries(bounds);

		auto t0 = luabind::newtable(l);
		for(unsigned int i = 0; i < 4; i++)
			t0[i + 1] = bounds[i];

		auto t1 = luabind::newtable(l);
		for(unsigned int i = 4; i < 8; i++)
			t1[i + 1] = bounds[i];
		return {l, t0, t1};
	}));
	def.def("SetProjectionMatrix", &pragma::BaseEnvCameraComponent::SetProjectionMatrix);
	def.def("SetViewMatrix", &pragma::BaseEnvCameraComponent::SetViewMatrix);
	def.def("GetNearPlaneBounds", &pragma::BaseEnvCameraComponent::GetNearPlaneBounds, luabind::meta::join<luabind::out_value<2>, luabind::out_value<3>>::type {});
	def.def("GetFarPlaneBounds", &pragma::BaseEnvCameraComponent::GetFarPlaneBounds, luabind::meta::join<luabind::out_value<2>, luabind::out_value<3>>::type {});
	def.def("GetFarPlaneBounds", &pragma::BaseEnvCameraComponent::GetPlaneBounds, luabind::meta::join<luabind::out_value<3>, luabind::out_value<4>>::type {});
	def.def("GetFrustumPoints", static_cast<void (pragma::BaseEnvCameraComponent::*)(std::vector<Vector3> &) const>(&pragma::BaseEnvCameraComponent::GetFrustumPoints), luabind::out_value<2> {});
	def.def("GetNearPlanePoint", &pragma::BaseEnvCameraComponent::GetNearPlanePoint);
	def.def("GetFarPlanePoint", &pragma::BaseEnvCameraComponent::GetFarPlanePoint);
	def.def("GetPlanePoint", &pragma::BaseEnvCameraComponent::GetPlanePoint);
	def.def("GetFrustumNeighbors", static_cast<void (*)(lua_State *, pragma::BaseEnvCameraComponent &, int)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent, int planeID) {
		if(planeID < 0 || planeID > 5)
			return;
		FrustumPlane neighborIDs[4];
		hComponent.GetFrustumNeighbors(FrustumPlane(planeID), &neighborIDs[0]);
		int table = Lua::CreateTable(l);
		for(unsigned int i = 0; i < 4; i++) {
			Lua::PushInt(l, i + 1);
			Lua::PushInt(l, static_cast<int>(neighborIDs[i]));
			Lua::SetTableValue(l, table);
		}
	}));
	def.def("GetFrustumPlaneCornerPoints", static_cast<void (*)(lua_State *, pragma::BaseEnvCameraComponent &, int, int)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent, int planeA, int planeB) {
		if(planeA < 0 || planeB < 0 || planeA > 5 || planeB > 5)
			return;

		FrustumPoint cornerPoints[2];
		hComponent.GetFrustumPlaneCornerPoints(FrustumPlane(planeA), FrustumPlane(planeB), &cornerPoints[0]);

		Lua::PushInt(l, static_cast<int>(cornerPoints[0]));
		Lua::PushInt(l, static_cast<int>(cornerPoints[1]));
	}));
	def.def("CreateFrustumKDop", static_cast<void (pragma::BaseEnvCameraComponent::*)(const Vector2 &, const Vector2 &, std::vector<umath::Plane> &) const>(&pragma::BaseEnvCameraComponent::CreateFrustumKDop), luabind::out_value<4> {});
	def.def("CreateFrustumMesh", static_cast<void (*)(lua_State *, pragma::BaseEnvCameraComponent &, const Vector2 &, const Vector2 &)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent, const Vector2 &uvStart, const Vector2 &uvEnd) {
		std::vector<Vector3> verts;
		std::vector<uint16_t> indices;
		hComponent.CreateFrustumMesh(uvStart, uvEnd, verts, indices);
		auto t = Lua::CreateTable(l);
		auto vertIdx = 1u;
		for(auto &v : verts) {
			Lua::PushInt(l, vertIdx++);
			Lua::Push<Vector3>(l, v);
			Lua::SetTableValue(l, t);
		}

		t = Lua::CreateTable(l);
		auto idx = 1u;
		for(auto i : indices) {
			Lua::PushInt(l, idx++);
			Lua::PushInt(l, i);
			Lua::SetTableValue(l, t);
		}
	}));
	def.def("ScreenSpaceToWorldSpace", static_cast<Vector3 (*)(lua_State *, pragma::BaseEnvCameraComponent &, const Vector2 &, float, float)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent, const Vector2 &uv, float width, float height) -> Vector3 {
		auto &ent = hComponent.GetEntity();
		return uvec::calc_world_direction_from_2d_coordinates(ent.GetForward(), ent.GetRight(), ent.GetUp(), hComponent.GetFOVRad(), hComponent.GetNearZ(), hComponent.GetFarZ(), hComponent.GetAspectRatio(), width, height, uv);
	}));
	def.def("WorldSpaceToScreenSpace", static_cast<std::pair<Vector2, float> (*)(lua_State *, pragma::BaseEnvCameraComponent &, const Vector3 &)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent, const Vector3 &point) -> std::pair<Vector2, float> {
		float dist;
		auto uv = uvec::calc_screenspace_uv_from_worldspace_position(point, hComponent.GetProjectionMatrix() * hComponent.GetViewMatrix(), hComponent.GetNearZ(), hComponent.GetFarZ(), dist);
		return {uv, dist};
	}));
	def.def("WorldSpaceToScreenSpaceDirection", static_cast<Vector2 (*)(lua_State *, pragma::BaseEnvCameraComponent &, const Vector3 &)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent, const Vector3 &dir) -> Vector2 {
		return uvec::calc_screenspace_direction_from_worldspace_direction(dir, hComponent.GetProjectionMatrix() * hComponent.GetViewMatrix());
	}));
	def.def("CalcScreenSpaceDistance", static_cast<float (*)(lua_State *, pragma::BaseEnvCameraComponent &, const Vector3 &)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent, const Vector3 &point) -> float {
		return uvec::calc_screenspace_distance_to_worldspace_position(point, hComponent.GetProjectionMatrix() * hComponent.GetViewMatrix(), hComponent.GetNearZ(), hComponent.GetFarZ());
	}));
	def.def("DepthToDistance", static_cast<float (*)(lua_State *, pragma::BaseEnvCameraComponent &, double, float, float)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent, double depth, float nearZ, float farZ) -> float { return uvec::depth_to_distance(depth, nearZ, farZ); }));
	def.def("CalcRayDirection", static_cast<Vector3 (*)(lua_State *, pragma::BaseEnvCameraComponent &, const Vector2 &)>([](lua_State *l, pragma::BaseEnvCameraComponent &hComponent, const Vector2 &uv) -> Vector3 {
		auto &ent = hComponent.GetEntity();
		return uvec::calc_world_direction_from_2d_coordinates(ent.GetForward(), ent.GetRight(), ent.GetUp(), hComponent.GetFOVRad(), hComponent.GetNearZ(), hComponent.GetFarZ(), hComponent.GetAspectRatio(), 0.f, 0.f, uv);
	}));
}

#include "pragma/entities/environment/effects/env_explosion.h"
void pragma::lua::base_env_explosion_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvExplosionComponent>("BaseEnvExplosionComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/effects/env_fire.h"
void pragma::lua::base_env_fire_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvFireComponent>("BaseEnvFireComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/env_fog_controller.h"
void pragma::lua::base_env_fog_controller_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvFogControllerComponent>("BaseEnvFogControllerComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/lights/env_light_spot_vol.h"
void pragma::lua::base_env_light_spot_vol_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvLightSpotVolComponent>("BaseEnvLightSpotVolComponent");
	def.def("SetIntensity", &pragma::BaseEnvLightSpotVolComponent::SetIntensityFactor);
	def.def("GetIntensity", &pragma::BaseEnvLightSpotVolComponent::GetIntensityFactor);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/env_microphone_base.h"
void pragma::lua::base_env_microphone_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvMicrophoneComponent>("BaseEnvMicrophoneComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/env_quake.h"
void pragma::lua::base_env_quake_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvQuakeComponent>("BaseEnvQuakeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/effects/env_smoke_trail.h"
void pragma::lua::base_env_smoke_trail_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvSmokeTrailComponent>("BaseEnvSmokeTrailComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/audio/env_sound.h"
void pragma::lua::base_env_sound_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvSoundComponent>("BaseEnvSoundComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetSoundSource", &pragma::BaseEnvSoundComponent::SetSoundSource);
	def.def("SetPitch", &pragma::BaseEnvSoundComponent::SetPitch);
	def.def("SetGain", &pragma::BaseEnvSoundComponent::SetGain);
	def.def("SetRolloffFactor", &pragma::BaseEnvSoundComponent::SetRolloffFactor);
	def.def("SetMinGain", &pragma::BaseEnvSoundComponent::SetMinGain);
	def.def("SetMaxGain", &pragma::BaseEnvSoundComponent::SetMaxGain);
	def.def("SetInnerConeAngle", &pragma::BaseEnvSoundComponent::SetInnerConeAngle);
	def.def("SetOuterConeAngle", &pragma::BaseEnvSoundComponent::SetOuterConeAngle);
	def.def("SetOffset", &pragma::BaseEnvSoundComponent::SetOffset);
	def.def("GetOffset", &pragma::BaseEnvSoundComponent::GetOffset);
	def.def("SetTimeOffset", &pragma::BaseEnvSoundComponent::SetTimeOffset);
	def.def("GetTimeOffset", &pragma::BaseEnvSoundComponent::GetTimeOffset);
	def.def("SetReferenceDistance", &pragma::BaseEnvSoundComponent::SetReferenceDistance);
	def.def("SetMaxDistance", &pragma::BaseEnvSoundComponent::SetMaxDistance);
	def.def("SetRelativeToListener", &pragma::BaseEnvSoundComponent::SetRelativeToListener);
	def.def("SetPlayOnSpawn", &pragma::BaseEnvSoundComponent::SetPlayOnSpawn);
	def.def("SetLooping", &pragma::BaseEnvSoundComponent::SetLooping);
	def.def("SetSoundTypes", &pragma::BaseEnvSoundComponent::SetSoundType);
	def.def("Play", &pragma::BaseEnvSoundComponent::Play);
	def.def("Stop", &pragma::BaseEnvSoundComponent::Stop);
	def.def("Pause", &pragma::BaseEnvSoundComponent::Pause);
	def.def("GetSound", &pragma::BaseEnvSoundComponent::GetSound);
	def.def("IsPlaying", &pragma::BaseEnvSoundComponent::IsPlaying);
	def.def("IsPaused", &pragma::BaseEnvSoundComponent::IsPaused);
}

#include "pragma/entities/environment/audio/env_soundscape.h"
void pragma::lua::base_env_soundscape_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvSoundScapeComponent>("BaseEnvSoundScapeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/effects/env_sprite.h"
void pragma::lua::base_env_sprite_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvSpriteComponent>("BaseEnvSpriteComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/env_timescale.h"
void pragma::lua::base_env_timescale_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvTimescaleComponent>("BaseEnvTimescaleComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/env_weather.h"
void pragma::lua::base_env_weather_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvWeatherComponent>("BaseEnvWeatherComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/environment/env_wind.hpp"
void pragma::lua::base_env_wind_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEnvWindComponent>("BaseEnvWindComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/basefilterentity.h"
void pragma::lua::base_env_filter_name_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFilterNameComponent>("BaseFilterNameComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("ShouldPass", &pragma::BaseFilterNameComponent::ShouldPass);
	def.add_static_constant("EVENT_ON_NAME_CHANGED", pragma::BaseNameComponent::EVENT_ON_NAME_CHANGED);
}

void pragma::lua::base_env_filter_class_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFilterClassComponent>("BaseFilterClassComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("ShouldPass", &pragma::BaseFilterClassComponent::ShouldPass);
}

#include "pragma/entities/func/basefuncbrush.h"
void pragma::lua::base_func_brush_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFuncBrushComponent>("BaseFuncBrushComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/func/basefuncbutton.h"
void pragma::lua::base_func_button_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFuncButtonComponent>("BaseFuncButtonComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/func/basefunckinematic.hpp"
void pragma::lua::base_func_kinematic_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFuncKinematicComponent>("BaseFuncKinematicComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/func/basefuncphysics.h"
void pragma::lua::base_func_physics_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFuncPhysicsComponent>("BaseFuncPhysicsComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/func/basefuncportal.h"
void pragma::lua::base_func_portal_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFuncPortalComponent>("BaseFuncPortalComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/func/basefuncsoftphysics.hpp"
void pragma::lua::base_func_soft_physics_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFuncSoftPhysicsComponent>("BaseFuncSoftPhysicsComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/func/basefuncsurfacematerial.hpp"
void pragma::lua::base_func_surface_material_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFuncSurfaceMaterialComponent>("BaseFuncSurfaceMaterialComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/logic/logic_relay.h"
void pragma::lua::base_logic_relay_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseLogicRelayComponent>("BaseLogicRelayComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/basebot.h"
void pragma::lua::base_bot_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseBotComponent>("BaseBotComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/components/basepointpathnode.h"
void pragma::lua::base_point_path_node_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePointPathNodeComponent>("BasePointPathNodeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/point/constraints/point_constraint_ballsocket.h"
void pragma::lua::base_point_constraint_ball_socket_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePointConstraintBallSocketComponent>("BasePointConstraintBallSocketComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/point/constraints/point_constraint_conetwist.h"
void pragma::lua::base_point_constraint_cone_twist_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePointConstraintConeTwistComponent>("BasePointConstraintConeTwistComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/point/constraints/point_constraint_dof.h"
void pragma::lua::base_point_constraint_dof_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePointConstraintDoFComponent>("BasePointConstraintDoFComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/point/constraints/point_constraint_fixed.h"
void pragma::lua::base_point_constraint_fixed_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePointConstraintFixedComponent>("BasePointConstraintFixedComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/point/constraints/point_constraint_hinge.h"
void pragma::lua::base_point_constraint_hinge_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePointConstraintHingeComponent>("BasePointConstraintHingeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/point/constraints/point_constraint_slider.h"
void pragma::lua::base_point_constraint_slider_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePointConstraintSliderComponent>("BasePointConstraintSliderComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/point/point_rendertarget.h"
void pragma::lua::base_point_render_target_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePointRenderTargetComponent>("BasePointRenderTargetComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/point/point_target.h"
void pragma::lua::base_point_target_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePointTargetComponent>("BasePointTargetComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/prop/prop_base.h"
void pragma::lua::base_prop_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePropComponent>("BasePropComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/prop/prop_dynamic.hpp"
void pragma::lua::base_prop_dynamic_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePropDynamicComponent>("BasePropDynamicComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/prop/prop_physics.hpp"
void pragma::lua::base_prop_physics_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePropPhysicsComponent>("BasePropPhysicsComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/trigger/base_trigger_hurt.hpp"
void pragma::lua::base_trigger_hurt_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseTriggerHurtComponent>("BaseTriggerHurtComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/trigger/base_trigger_push.hpp"
void pragma::lua::base_trigger_push_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseTriggerPushComponent>("BaseTriggerPushComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/trigger/base_trigger_remove.h"
void pragma::lua::base_trigger_remove_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseTriggerRemoveComponent>("BaseTriggerRemoveComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/trigger/base_trigger_teleport.hpp"
void pragma::lua::base_trigger_teleport_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseTriggerTeleportComponent>("BaseTriggerTeleportComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/trigger/base_trigger_touch.hpp"
void pragma::lua::base_touch_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseTouchComponent>("BaseTouchComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetTriggerFlags", &pragma::BaseTouchComponent::SetTriggerFlags);
	def.def("GetTriggerFlags", &pragma::BaseTouchComponent::GetTriggerFlags);
	def.def("GetTouchingEntities", static_cast<luabind::object (*)(lua_State *, pragma::BaseTouchComponent &)>([](lua_State *l, pragma::BaseTouchComponent &hComponent) -> luabind::object {
		auto t = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &touchInfo : hComponent.GetTouchingInfo()) {
			if(touchInfo.touch.entity.valid() == false || touchInfo.triggered == false)
				continue;
			t[idx++] = touchInfo.touch.entity.get()->GetLuaObject();
		}
		return t;
	}));
	def.def("GetTouchingEntityCount", static_cast<uint32_t (*)(lua_State *, pragma::BaseTouchComponent &)>([](lua_State *l, pragma::BaseTouchComponent &hComponent) -> uint32_t {
		auto &touchingInfo = hComponent.GetTouchingInfo();
		return std::count_if(touchingInfo.begin(), touchingInfo.end(), [](const pragma::BaseTouchComponent::TouchInfo &touchInfo) -> bool { return touchInfo.triggered && touchInfo.touch.entity.valid(); });
	}));

	def.add_static_constant("EVENT_CAN_TRIGGER", pragma::BaseTouchComponent::EVENT_CAN_TRIGGER);
	def.add_static_constant("EVENT_ON_START_TOUCH", pragma::BaseTouchComponent::EVENT_ON_START_TOUCH);
	def.add_static_constant("EVENT_ON_END_TOUCH", pragma::BaseTouchComponent::EVENT_ON_END_TOUCH);
	def.add_static_constant("EVENT_ON_TRIGGER", pragma::BaseTouchComponent::EVENT_ON_TRIGGER);
	def.add_static_constant("EVENT_ON_TRIGGER_INITIALIZED", pragma::BaseTouchComponent::EVENT_ON_TRIGGER_INITIALIZED);

	def.add_static_constant("TRIGGER_FLAG_NONE", umath::to_integral(pragma::BaseTouchComponent::TriggerFlags::None));
	def.add_static_constant("TRIGGER_FLAG_BIT_PLAYERS", umath::to_integral(pragma::BaseTouchComponent::TriggerFlags::Players));
	def.add_static_constant("TRIGGER_FLAG_BIT_NPCS", umath::to_integral(pragma::BaseTouchComponent::TriggerFlags::NPCs));
	def.add_static_constant("TRIGGER_FLAG_BIT_PHYSICS", umath::to_integral(pragma::BaseTouchComponent::TriggerFlags::Physics));
	def.add_static_constant("TRIGGER_FLAG_EVERYTHING", umath::to_integral(pragma::BaseTouchComponent::TriggerFlags::Everything));
}

#include "pragma/entities/components/basetriggergravity.hpp"
void pragma::lua::base_trigger_gravity_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseEntityTriggerGravityComponent>("BaseEntityTriggerGravityComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/baseflashlight.h"
void pragma::lua::base_flashlight_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFlashlightComponent>("BaseFlashlightComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/components/base_flex_component.hpp"
void pragma::lua::base_flex_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseFlexComponent>("BaseFlexComponent");
	def.add_static_constant("EVENT_ON_FLEX_CONTROLLER_CHANGED", pragma::BaseFlexComponent::EVENT_ON_FLEX_CONTROLLER_CHANGED);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/baseskybox.h"
void pragma::lua::base_skybox_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseSkyboxComponent>("BaseSkyboxComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetSkyAngles", &pragma::BaseSkyboxComponent::SetSkyAngles);
	def.def("GetSkyAngles", &pragma::BaseSkyboxComponent::GetSkyAngles, luabind::copy_policy<0> {});
}

#include "pragma/entities/baseworld.h"
void pragma::lua::base_world_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseWorldComponent>("BaseWorldComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/components/base_ai_component.hpp"
namespace Lua::AI {
	void GetMoveSpeed(lua_State *l, pragma::BaseAIComponent &hNPC, uint32_t animId) { Lua::PushNumber(l, hNPC.GetMoveSpeed(animId)); }

	void SetLookTarget(lua_State *l, pragma::BaseAIComponent &hNPC, const Vector3 &tgt, float t) { hNPC.SetLookTarget(tgt, t); }

	void SetLookTarget(lua_State *l, pragma::BaseAIComponent &hNPC, BaseEntity &ent, float t) { hNPC.SetLookTarget(ent, t); }
};
void pragma::lua::base_ai_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseAIComponent>("BaseAIComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetMoveSpeed", static_cast<void (pragma::BaseAIComponent::*)(int32_t, float)>(&pragma::BaseAIComponent::SetMoveSpeed));
	def.def("SetMoveSpeed", static_cast<void (pragma::BaseAIComponent::*)(const std::string &, float)>(&pragma::BaseAIComponent::SetMoveSpeed));
	def.def("GetMoveSpeed", static_cast<void (*)(lua_State *, pragma::BaseAIComponent &, const std::string &)>([](lua_State *l, pragma::BaseAIComponent &hNPC, const std::string &anim) {
		auto &mdl = hNPC.GetEntity().GetModel();
		if(mdl == nullptr)
			return;
		auto animId = mdl->LookupAnimation(anim);
		if(animId < 0)
			return;
		Lua::AI::GetMoveSpeed(l, hNPC, animId);
	}));
	def.def("GetMoveSpeed", static_cast<void (*)(lua_State *, pragma::BaseAIComponent &, uint32_t)>([](lua_State *l, pragma::BaseAIComponent &hNPC, uint32_t animId) { Lua::AI::GetMoveSpeed(l, hNPC, animId); }));
	def.def("ClearMoveSpeed", static_cast<void (pragma::BaseAIComponent::*)(int32_t)>(&pragma::BaseAIComponent::ClearMoveSpeed));
	def.def("ClearMoveSpeed", static_cast<void (pragma::BaseAIComponent::*)(const std::string &)>(&pragma::BaseAIComponent::ClearMoveSpeed));
	def.def("ClearLookTarget", &pragma::BaseAIComponent::ClearLookTarget);
	def.def("SetLookTarget", static_cast<void (*)(lua_State *, pragma::BaseAIComponent &, const Vector3 &, float)>([](lua_State *l, pragma::BaseAIComponent &hNPC, const Vector3 &tgt, float t) { Lua::AI::SetLookTarget(l, hNPC, tgt, t); }));
	def.def("SetLookTarget", static_cast<void (*)(lua_State *, pragma::BaseAIComponent &, const Vector3 &)>([](lua_State *l, pragma::BaseAIComponent &hNPC, const Vector3 &tgt) { Lua::AI::SetLookTarget(l, hNPC, tgt, std::numeric_limits<float>::max()); }));
	def.def("SetLookTarget", static_cast<void (*)(lua_State *, pragma::BaseAIComponent &, BaseEntity &, float)>([](lua_State *l, pragma::BaseAIComponent &hNPC, BaseEntity &ent, float t) { Lua::AI::SetLookTarget(l, hNPC, ent, t); }));
	def.def("SetLookTarget", static_cast<void (*)(lua_State *, pragma::BaseAIComponent &, BaseEntity &)>([](lua_State *l, pragma::BaseAIComponent &hNPC, BaseEntity &ent) { Lua::AI::SetLookTarget(l, hNPC, ent, std::numeric_limits<float>::max()); }));
	def.def("GetLookTarget", &pragma::BaseAIComponent::GetLookTarget);
}

#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/physics/raytraces.h"
void pragma::lua::base_character_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseCharacterComponent>("BaseCharacterComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	// Actor
	def.def("GetFrozenProperty", &pragma::BaseCharacterComponent::GetFrozenProperty);
	def.add_static_constant("EVENT_ON_KILLED", pragma::BaseActorComponent::EVENT_ON_KILLED);
	def.add_static_constant("EVENT_ON_RESPAWN", pragma::BaseActorComponent::EVENT_ON_RESPAWN);

	// Character
	def.def("IsAlive", &pragma::BaseCharacterComponent::IsAlive);
	def.def("IsDead", &pragma::BaseCharacterComponent::IsDead);
	def.def("IsFrozen", &pragma::BaseCharacterComponent::IsFrozen);
	def.def("SetFrozen", &pragma::BaseCharacterComponent::SetFrozen);
	def.def("Kill", &pragma::BaseCharacterComponent::Kill);
	def.def("GetOrientationAxesRotation", &pragma::BaseCharacterComponent::GetOrientationAxesRotation, luabind::copy_policy<0> {});
	def.def("GetShootPos", &pragma::BaseCharacterComponent::GetShootPosition);
	def.def("GetSlopeLimit", &pragma::BaseCharacterComponent::GetSlopeLimit);
	def.def("GetStepOffset", &pragma::BaseCharacterComponent::GetStepOffset);
	def.def("GetTurnSpeed", &pragma::BaseCharacterComponent::GetTurnSpeed);
	def.def("GetViewAngles", &pragma::BaseCharacterComponent::GetViewAngles);
	def.def("GetViewForward", &pragma::BaseCharacterComponent::GetViewForward);
	def.def("GetViewRotation", static_cast<const Quat &(pragma::BaseCharacterComponent::*)() const>(&pragma::BaseCharacterComponent::GetViewOrientation), luabind::copy_policy<0> {});
	def.def("GetViewRight", &pragma::BaseCharacterComponent::GetViewRight);
	def.def("GetViewUp", &pragma::BaseCharacterComponent::GetViewUp);
	def.def("Ragdolize", &pragma::BaseCharacterComponent::Ragdolize);
	def.def("SetSlopeLimit", static_cast<void (*)(lua_State *, pragma::BaseCharacterComponent &, float)>([](lua_State *l, pragma::BaseCharacterComponent &hEntity, float slopeLimit) { hEntity.SetSlopeLimit(umath::deg_to_rad(slopeLimit)); }));
	def.def("SetStepOffset", &pragma::BaseCharacterComponent::SetStepOffset);
	def.def("SetTurnSpeed", &pragma::BaseCharacterComponent::SetTurnSpeed);
	def.def("SetViewAngles", &pragma::BaseCharacterComponent::SetViewAngles);
	def.def("SetViewRotation", &pragma::BaseCharacterComponent::SetViewOrientation);
	def.def("NormalizeViewRotation", static_cast<void (pragma::BaseCharacterComponent::*)(Quat &)>(&pragma::BaseCharacterComponent::NormalizeViewOrientation));
	def.def("NormalizeViewRotation", static_cast<const Quat &(pragma::BaseCharacterComponent::*)()>(&pragma::BaseCharacterComponent::NormalizeViewOrientation), luabind::copy_policy<0> {});
	def.def("GetHitboxPhysicsObject", &pragma::BaseCharacterComponent::GetHitboxPhysicsObject);
	def.def("GetWeapons", static_cast<void (*)(lua_State *, pragma::BaseCharacterComponent &)>([](lua_State *l, pragma::BaseCharacterComponent &hEnt) {
		auto &weapons = hEnt.GetWeapons();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto it = weapons.begin(); it != weapons.end(); ++it) {
			if(it->valid()) {
				Lua::PushInt(l, idx++);
				(*it)->GetLuaObject().push(l);
				Lua::SetTableValue(l, t);
			}
		}
	}));
	def.def("GetWeapons", static_cast<void (*)(lua_State *, pragma::BaseCharacterComponent &, const std::string &)>([](lua_State *l, pragma::BaseCharacterComponent &hEnt, const std::string &className) {
		auto weapons = hEnt.GetWeapons(className);
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto it = weapons.begin(); it != weapons.end(); ++it) {
			Lua::PushInt(l, idx++);
			(*it)->GetLuaObject().push(l);
			Lua::SetTableValue(l, t);
		}
	}));
	def.def("GetWeaponCount", static_cast<size_t (*)(lua_State *, pragma::BaseCharacterComponent &)>([](lua_State *l, pragma::BaseCharacterComponent &hEnt) -> size_t { return hEnt.GetWeapons().size(); }));
	def.def("GetActiveWeapon", &pragma::BaseCharacterComponent::GetActiveWeapon);
	def.def("HasWeapon", &pragma::BaseCharacterComponent::HasWeapon);
	def.def("GetAimRayData", static_cast<void (*)(lua_State *, pragma::BaseCharacterComponent &)>([](lua_State *l, pragma::BaseCharacterComponent &hEnt) { Lua::Push<::TraceData>(l, hEnt.GetAimTraceData()); }));
	def.def("GetAimRayData", static_cast<void (*)(lua_State *, pragma::BaseCharacterComponent &, float)>([](lua_State *l, pragma::BaseCharacterComponent &hEnt, float maxDist) { Lua::Push<::TraceData>(l, hEnt.GetAimTraceData(maxDist)); }));
	def.def("FootStep", &pragma::BaseCharacterComponent::FootStep);
	def.def("IsMoving", &pragma::BaseCharacterComponent::IsMoving);
	def.def("SetNeckControllers", &pragma::BaseCharacterComponent::SetNeckControllers);
	def.def("GetNeckYawController", &pragma::BaseCharacterComponent::GetNeckYawBlendController);
	def.def("GetNeckPitchController", &pragma::BaseCharacterComponent::GetNeckPitchBlendController);
	def.def("SetMoveController", static_cast<void (pragma::BaseCharacterComponent::*)(const std::string &)>(&pragma::BaseCharacterComponent::SetMoveController));
	def.def("SetMoveController", static_cast<void (pragma::BaseCharacterComponent::*)(const std::string &, const std::string &)>(&pragma::BaseCharacterComponent::SetMoveController));
	def.def("GetMoveController", static_cast<luabind::mult<int32_t, int32_t> (*)(lua_State *, pragma::BaseCharacterComponent &)>([](lua_State *l, pragma::BaseCharacterComponent &hEnt) -> luabind::mult<int32_t, int32_t> { return {l, hEnt.GetMoveController(), hEnt.GetMoveControllerY()}; }));

	def.def("SetOrientation", &pragma::BaseCharacterComponent::SetCharacterOrientation);
	def.def("GetLocalOrientationAngles", &pragma::BaseCharacterComponent::GetLocalOrientationAngles);
	def.def("GetLocalOrientationRotation", &pragma::BaseCharacterComponent::GetLocalOrientationRotation);
	def.def("GetLocalOrientationViewAngles", &pragma::BaseCharacterComponent::GetLocalOrientationViewAngles);
	def.def("GetLocalOrientationViewRotation", &pragma::BaseCharacterComponent::GetLocalOrientationViewRotation);

	def.def("WorldToLocalOrientation", static_cast<Quat (pragma::BaseCharacterComponent::*)(const Quat &)>(&pragma::BaseCharacterComponent::WorldToLocalOrientation));
	def.def("WorldToLocalOrientation", static_cast<EulerAngles (pragma::BaseCharacterComponent::*)(const EulerAngles &)>(&pragma::BaseCharacterComponent::WorldToLocalOrientation));
	def.def("LocalOrientationToWorld", static_cast<Quat (pragma::BaseCharacterComponent::*)(const Quat &)>(&pragma::BaseCharacterComponent::LocalOrientationToWorld));
	def.def("LocalOrientationToWorld", static_cast<EulerAngles (pragma::BaseCharacterComponent::*)(const EulerAngles &)>(&pragma::BaseCharacterComponent::LocalOrientationToWorld));

	def.def("GetAmmoCount", static_cast<uint16_t (pragma::BaseCharacterComponent::*)(const std::string &) const>(&pragma::BaseCharacterComponent::GetAmmoCount));
	def.def("GetAmmoCount", static_cast<uint16_t (pragma::BaseCharacterComponent::*)(uint32_t) const>(&pragma::BaseCharacterComponent::GetAmmoCount));
	def.def("SetAmmoCount", static_cast<void (pragma::BaseCharacterComponent::*)(const std::string &, uint16_t)>(&pragma::BaseCharacterComponent::SetAmmoCount));
	def.def("SetAmmoCount", static_cast<void (pragma::BaseCharacterComponent::*)(uint32_t, uint16_t)>(&pragma::BaseCharacterComponent::SetAmmoCount));

	def.def("AddAmmo", static_cast<void (pragma::BaseCharacterComponent::*)(const std::string &, int16_t)>(&pragma::BaseCharacterComponent::AddAmmo));
	def.def("AddAmmo", static_cast<void (pragma::BaseCharacterComponent::*)(uint32_t, int16_t)>(&pragma::BaseCharacterComponent::AddAmmo));
	def.def("RemoveAmmo", static_cast<void (pragma::BaseCharacterComponent::*)(const std::string &, int16_t)>(&pragma::BaseCharacterComponent::RemoveAmmo));
	def.def("RemoveAmmo", static_cast<void (pragma::BaseCharacterComponent::*)(uint32_t, int16_t)>(&pragma::BaseCharacterComponent::RemoveAmmo));
	def.def("GetSlopeLimitProperty", &pragma::BaseCharacterComponent::GetSlopeLimitProperty);
	def.def("GetStepOffsetProperty", &pragma::BaseCharacterComponent::GetStepOffsetProperty);
	def.def("GetJumpPower", &pragma::BaseCharacterComponent::GetJumpPower);
	def.def("SetJumpPower", &pragma::BaseCharacterComponent::SetJumpPower);
	def.def("Jump", static_cast<bool (pragma::BaseCharacterComponent::*)()>(&pragma::BaseCharacterComponent::Jump));
	def.def("Jump", static_cast<bool (pragma::BaseCharacterComponent::*)(const Vector3 &)>(&pragma::BaseCharacterComponent::Jump));
	def.def("GetJumpPowerProperty", &pragma::BaseCharacterComponent::GetJumpPowerProperty);

	def.add_static_constant("EVENT_ON_FOOT_STEP", pragma::BaseCharacterComponent::EVENT_ON_FOOT_STEP);
	def.add_static_constant("EVENT_ON_CHARACTER_ORIENTATION_CHANGED", pragma::BaseCharacterComponent::EVENT_ON_CHARACTER_ORIENTATION_CHANGED);
	def.add_static_constant("EVENT_ON_DEPLOY_WEAPON", pragma::BaseCharacterComponent::EVENT_ON_DEPLOY_WEAPON);
	def.add_static_constant("EVENT_ON_SET_ACTIVE_WEAPON", pragma::BaseCharacterComponent::EVENT_ON_SET_ACTIVE_WEAPON);
	def.add_static_constant("EVENT_PLAY_FOOTSTEP_SOUND", pragma::BaseCharacterComponent::EVENT_PLAY_FOOTSTEP_SOUND);
	def.add_static_constant("EVENT_ON_DEATH", pragma::BaseCharacterComponent::EVENT_ON_DEATH);
	def.add_static_constant("EVENT_IS_MOVING", pragma::BaseCharacterComponent::EVENT_IS_MOVING);
	def.add_static_constant("EVENT_HANDLE_VIEW_ROTATION", pragma::BaseCharacterComponent::EVENT_HANDLE_VIEW_ROTATION);
	def.add_static_constant("EVENT_ON_JUMP", pragma::BaseCharacterComponent::EVENT_ON_JUMP);

	def.add_static_constant("FOOT_LEFT", umath::to_integral(pragma::BaseCharacterComponent::FootType::Left));
	def.add_static_constant("FOOT_RIGHT", umath::to_integral(pragma::BaseCharacterComponent::FootType::Right));
}

#include "pragma/entities/components/base_vehicle_component.hpp"
void pragma::lua::base_vehicle_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseVehicleComponent>("BaseVehicleComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetSpeedKmh", &pragma::BaseVehicleComponent::GetSpeedKmh);
	def.def("GetSteeringFactor", &pragma::BaseVehicleComponent::GetSteeringFactor);
	def.def("GetSteeringWheel", &pragma::BaseVehicleComponent::GetSteeringWheel);
	def.def("HasDriver", &pragma::BaseVehicleComponent::HasDriver);
	def.def("GetDriver", &pragma::BaseVehicleComponent::GetDriver);
	def.def("SetDriver", &pragma::BaseVehicleComponent::SetDriver);
	def.def("ClearDriver", &pragma::BaseVehicleComponent::ClearDriver);
	def.def("SetupSteeringWheel", &pragma::BaseVehicleComponent::SetupSteeringWheel);
	def.def("GetPhysicsVehicle", static_cast<physics::IVehicle *(pragma::BaseVehicleComponent::*)()>(&pragma::BaseVehicleComponent::GetPhysicsVehicle));
	def.def("SetupPhysics", &pragma::BaseVehicleComponent::SetupVehicle);
	def.add_static_constant("EVENT_ON_DRIVER_ENTERED", pragma::BaseVehicleComponent::EVENT_ON_DRIVER_ENTERED);
	def.add_static_constant("EVENT_ON_DRIVER_EXITED", pragma::BaseVehicleComponent::EVENT_ON_DRIVER_EXITED);
}

#include "pragma/entities/components/base_weapon_component.hpp"
namespace Lua::Weapon {
	void PrimaryAttack(lua_State *l, pragma::BaseWeaponComponent &hEnt, bool bOnce)
	{
		auto &wep = hEnt;
		wep.PrimaryAttack();
		if(bOnce == true)
			wep.EndPrimaryAttack();
	}

	void SecondaryAttack(lua_State *l, pragma::BaseWeaponComponent &hEnt, bool bOnce)
	{
		auto &wep = hEnt;
		wep.SecondaryAttack();
		if(bOnce == true)
			wep.EndSecondaryAttack();
	}
};
void pragma::lua::base_weapon_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseWeaponComponent>("BaseWeaponComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetNextPrimaryAttack", &pragma::BaseWeaponComponent::SetNextPrimaryAttack);
	def.def("SetNextSecondaryAttack", &pragma::BaseWeaponComponent::SetNextSecondaryAttack);
	def.def("SetNextAttack", &pragma::BaseWeaponComponent::SetNextAttack);
	def.def("SetAutomaticPrimary", &pragma::BaseWeaponComponent::SetAutomaticPrimary);
	def.def("SetAutomaticSecondary", &pragma::BaseWeaponComponent::SetAutomaticSecondary);
	def.def("IsAutomaticPrimary", &pragma::BaseWeaponComponent::IsAutomaticPrimary);
	def.def("IsAutomaticSecondary", &pragma::BaseWeaponComponent::IsAutomaticSecondary);
	def.def("IsDeployed", &pragma::BaseWeaponComponent::IsDeployed);
	def.def("GetPrimaryAmmoType", &pragma::BaseWeaponComponent::GetPrimaryAmmoType);
	def.def("GetPrimaryAmmoTypeProperty", &pragma::BaseWeaponComponent::GetPrimaryAmmoTypeProperty);
	def.def("GetSecondaryAmmoType", &pragma::BaseWeaponComponent::GetSecondaryAmmoType);
	def.def("GetSecondaryAmmoTypeProperty", &pragma::BaseWeaponComponent::GetSecondaryAmmoTypeProperty);
	def.def("HasPrimaryAmmo", &pragma::BaseWeaponComponent::HasPrimaryAmmo);
	def.def("HasSecondaryAmmo", &pragma::BaseWeaponComponent::HasSecondaryAmmo);
	def.def("IsPrimaryClipEmpty", &pragma::BaseWeaponComponent::IsPrimaryClipEmpty);
	def.def("IsSecondaryClipEmpty", &pragma::BaseWeaponComponent::IsSecondaryClipEmpty);
	def.def("HasAmmo", &pragma::BaseWeaponComponent::HasAmmo);
	def.def("GetPrimaryClipSize", &pragma::BaseWeaponComponent::GetPrimaryClipSize);
	def.def("GetPrimaryClipSizeProperty", &pragma::BaseWeaponComponent::GetPrimaryClipSizeProperty);
	def.def("GetSecondaryClipSize", &pragma::BaseWeaponComponent::GetSecondaryClipSize);
	def.def("GetSecondaryClipSizeProperty", &pragma::BaseWeaponComponent::GetSecondaryClipSizeProperty);
	def.def("GetMaxPrimaryClipSize", &pragma::BaseWeaponComponent::GetMaxPrimaryClipSize);
	def.def("GetMaxPrimaryClipSizeProperty", &pragma::BaseWeaponComponent::GetMaxPrimaryClipSizeProperty);
	def.def("GetMaxSecondaryClipSize", &pragma::BaseWeaponComponent::GetMaxSecondaryClipSize);
	def.def("GetMaxSecondaryClipSizeProperty", &pragma::BaseWeaponComponent::GetMaxSecondaryClipSizeProperty);
	def.def("PrimaryAttack", static_cast<void (*)(lua_State *, pragma::BaseWeaponComponent &, bool)>([](lua_State *l, pragma::BaseWeaponComponent &hEnt, bool bOnce) { Lua::Weapon::PrimaryAttack(l, hEnt, bOnce); }));
	def.def("PrimaryAttack", static_cast<void (*)(lua_State *, pragma::BaseWeaponComponent &)>([](lua_State *l, pragma::BaseWeaponComponent &hEnt) { Lua::Weapon::PrimaryAttack(l, hEnt, false); }));
	def.def("SecondaryAttack", static_cast<void (*)(lua_State *, pragma::BaseWeaponComponent &, bool)>([](lua_State *l, pragma::BaseWeaponComponent &hEnt, bool bOnce) { Lua::Weapon::SecondaryAttack(l, hEnt, bOnce); }));
	def.def("SecondaryAttack", static_cast<void (*)(lua_State *, pragma::BaseWeaponComponent &)>([](lua_State *l, pragma::BaseWeaponComponent &hEnt) { Lua::Weapon::SecondaryAttack(l, hEnt, false); }));
	def.def("TertiaryAttack", &pragma::BaseWeaponComponent::TertiaryAttack);
	def.def("Attack4", &pragma::BaseWeaponComponent::Attack4);
	def.def("Reload", &pragma::BaseWeaponComponent::Reload);
	def.def("Deploy", &pragma::BaseWeaponComponent::Deploy);
	def.def("Holster", &pragma::BaseWeaponComponent::Holster);
	def.def("EndAttack", &pragma::BaseWeaponComponent::EndAttack);
	def.def("EndPrimaryAttack", &pragma::BaseWeaponComponent::EndPrimaryAttack);
	def.def("EndSecondaryAttack", &pragma::BaseWeaponComponent::EndSecondaryAttack);

	def.add_static_constant("EVENT_ON_DEPLOY", pragma::BaseWeaponComponent::EVENT_ON_DEPLOY);
	def.add_static_constant("EVENT_ON_HOLSTER", pragma::BaseWeaponComponent::EVENT_ON_HOLSTER);
	def.add_static_constant("EVENT_ON_PRIMARY_ATTACK", pragma::BaseWeaponComponent::EVENT_ON_PRIMARY_ATTACK);
	def.add_static_constant("EVENT_ON_SECONDARY_ATTACK", pragma::BaseWeaponComponent::EVENT_ON_SECONDARY_ATTACK);
	def.add_static_constant("EVENT_ON_TERTIARY_ATTACK", pragma::BaseWeaponComponent::EVENT_ON_TERTIARY_ATTACK);
	def.add_static_constant("EVENT_ON_ATTACK4", pragma::BaseWeaponComponent::EVENT_ON_ATTACK4);
	def.add_static_constant("EVENT_ON_END_PRIMARY_ATTACK", pragma::BaseWeaponComponent::EVENT_ON_END_PRIMARY_ATTACK);
	def.add_static_constant("EVENT_ON_END_SECONDARY_ATTACK", pragma::BaseWeaponComponent::EVENT_ON_END_SECONDARY_ATTACK);
	def.add_static_constant("EVENT_ON_RELOAD", pragma::BaseWeaponComponent::EVENT_ON_RELOAD);
	def.add_static_constant("EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED", pragma::BaseWeaponComponent::EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED);
	def.add_static_constant("EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED", pragma::BaseWeaponComponent::EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED);
}

#include "pragma/entities/components/base_player_component.hpp"
void pragma::lua::base_player_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePlayerComponent>("BasePlayerComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetTimeConnected", &pragma::BasePlayerComponent::TimeConnected);
	def.def("IsKeyDown", &pragma::BasePlayerComponent::IsKeyDown);
	def.def("GetWalkSpeed", &pragma::BasePlayerComponent::GetWalkSpeed);
	def.def("GetRunSpeed", &pragma::BasePlayerComponent::GetRunSpeed);
	def.def("GetSprintSpeed", &pragma::BasePlayerComponent::GetSprintSpeed);
	def.def("SetWalkSpeed", &pragma::BasePlayerComponent::SetWalkSpeed);
	def.def("SetRunSpeed", &pragma::BasePlayerComponent::SetRunSpeed);
	def.def("SetSprintSpeed", &pragma::BasePlayerComponent::SetSprintSpeed);
	def.def("GetCrouchedWalkSpeed", &pragma::BasePlayerComponent::GetCrouchedWalkSpeed);
	def.def("SetCrouchedWalkSpeed", &pragma::BasePlayerComponent::SetCrouchedWalkSpeed);
	def.def("IsLocalPlayer", &pragma::BasePlayerComponent::IsLocalPlayer);
	def.def("GetStandHeight", &pragma::BasePlayerComponent::GetStandHeight);
	def.def("SetStandHeight", &pragma::BasePlayerComponent::SetStandHeight);
	def.def("GetCrouchHeight", &pragma::BasePlayerComponent::GetCrouchHeight);
	def.def("SetCrouchHeight", &pragma::BasePlayerComponent::SetCrouchHeight);
	def.def("GetStandEyeLevel", &pragma::BasePlayerComponent::GetStandEyeLevel);
	def.def("SetStandEyeLevel", &pragma::BasePlayerComponent::SetStandEyeLevel);
	def.def("GetCrouchEyeLevel", &pragma::BasePlayerComponent::GetCrouchEyeLevel);
	def.def("SetCrouchEyeLevel", &pragma::BasePlayerComponent::SetCrouchEyeLevel);
	def.def("SetFlashlightEnabled", &pragma::BasePlayerComponent::SetFlashlight);
	def.def("ToggleFlashlight", &pragma::BasePlayerComponent::ToggleFlashlight);
	def.def("IsFlashlightEnabled", &pragma::BasePlayerComponent::IsFlashlightOn);
	def.def("GetViewPos", &pragma::BasePlayerComponent::GetViewPos, luabind::copy_policy<0> {});

	def.def("ApplyViewRotationOffset", &pragma::BasePlayerComponent::ApplyViewRotationOffset);
	def.def("ApplyViewRotationOffset", static_cast<void (*)(lua_State *, pragma::BasePlayerComponent &, EulerAngles &)>([](lua_State *l, pragma::BasePlayerComponent &hPl, EulerAngles &ang) { hPl.ApplyViewRotationOffset(ang); }));
	def.def("PrintMessage", &pragma::BasePlayerComponent::PrintMessage);

	def.add_static_constant("MESSAGE_TYPE_CONSOLE", umath::to_integral(MESSAGE::PRINTCONSOLE));
	def.add_static_constant("MESSAGE_TYPE_CHAT", umath::to_integral(MESSAGE::PRINTCHAT));
}

#include "pragma/entities/components/base_observer_component.hpp"
void pragma::lua::base_observer_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseObserverComponent>("BaseObserverComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetObserverMode", &pragma::BaseObserverComponent::SetObserverMode);
	def.def("GetObserverMode", &pragma::BaseObserverComponent::GetObserverMode);
	def.def("GetObserverModeProperty", &pragma::BaseObserverComponent::GetObserverModeProperty);
	def.def("SetObserverTarget", &pragma::BaseObserverComponent::SetObserverTarget);
	def.def("GetObserverTarget", &pragma::BaseObserverComponent::GetObserverTarget);

	def.add_static_constant("EVENT_ON_OBSERVATION_MODE_CHANGED", pragma::BaseObserverComponent::EVENT_ON_OBSERVATION_MODE_CHANGED);

	// Enums
	def.add_static_constant("OBSERVERMODE_NONE", umath::to_integral(ObserverMode::None));
	def.add_static_constant("OBSERVERMODE_FIRSTPERSON", umath::to_integral(ObserverMode::FirstPerson));
	def.add_static_constant("OBSERVERMODE_THIRDPERSON", umath::to_integral(ObserverMode::ThirdPerson));
	def.add_static_constant("OBSERVERMODE_SHOULDER", umath::to_integral(ObserverMode::Shoulder));
	def.add_static_constant("OBSERVERMODE_ROAMING", umath::to_integral(ObserverMode::Roaming));
	static_assert(umath::to_integral(ObserverMode::Count) == 5, "Update this list when new modes are added!");
}

#include "pragma/entities/components/base_gamemode_component.hpp"
void pragma::lua::base_gamemode_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseGamemodeComponent>("BaseGamemodeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetName", &pragma::BaseGamemodeComponent::GetName);
	def.def("GetIdentifier", &pragma::BaseGamemodeComponent::GetIdentifier);
	def.def("GetComponentName", &pragma::BaseGamemodeComponent::GetComponentName);
	def.def("GetAuthor", &pragma::BaseGamemodeComponent::GetAuthor);
	def.def("GetGamemodeVersion", &pragma::BaseGamemodeComponent::GetGamemodeVersion);

	// Enums
	def.add_static_constant("EVENT_ON_PLAYER_DEATH", pragma::BaseGamemodeComponent::EVENT_ON_PLAYER_DEATH);
	def.add_static_constant("EVENT_ON_PLAYER_SPAWNED", pragma::BaseGamemodeComponent::EVENT_ON_PLAYER_SPAWNED);
	def.add_static_constant("EVENT_ON_PLAYER_DROPPED", pragma::BaseGamemodeComponent::EVENT_ON_PLAYER_DROPPED);
	def.add_static_constant("EVENT_ON_PLAYER_READY", pragma::BaseGamemodeComponent::EVENT_ON_PLAYER_READY);
	def.add_static_constant("EVENT_ON_PLAYER_JOINED", pragma::BaseGamemodeComponent::EVENT_ON_PLAYER_JOINED);
	def.add_static_constant("EVENT_ON_GAME_INITIALIZED", pragma::BaseGamemodeComponent::EVENT_ON_GAME_INITIALIZED);
	def.add_static_constant("EVENT_ON_MAP_INITIALIZED", pragma::BaseGamemodeComponent::EVENT_ON_MAP_INITIALIZED);
	def.add_static_constant("EVENT_ON_GAME_READY", pragma::BaseGamemodeComponent::EVENT_ON_GAME_READY);
}

#include "pragma/entities/components/base_generic_component.hpp"
void pragma::lua::base_generic_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseGenericComponent>("BaseGenericComponent");
	def.add_static_constant("EVENT_ON_COMPONENT_ADDED", pragma::BaseGenericComponent::EVENT_ON_ENTITY_COMPONENT_ADDED);
	def.add_static_constant("EVENT_ON_COMPONENT_REMOVED", pragma::BaseGenericComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED);
	def.add_static_constant("EVENT_ON_MEMBERS_CHANGED", pragma::BaseGenericComponent::EVENT_ON_MEMBERS_CHANGED);
	def.add_static_constant("EVENT_ON_ACTIVE_STATE_CHANGED", pragma::BaseGenericComponent::EVENT_ON_ACTIVE_STATE_CHANGED);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/info/info_landmark.hpp"
void pragma::lua::base_info_landmark_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseInfoLandmarkComponent>("BaseInfoLandmarkComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/components/base_io_component.hpp"
void pragma::lua::base_io_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseIOComponent>("BaseIOComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("Input", static_cast<void (*)(lua_State *, pragma::BaseIOComponent &, std::string, BaseEntity &, BaseEntity &, std::string)>([](lua_State *l, pragma::BaseIOComponent &hIo, std::string input, BaseEntity &activator, BaseEntity &caller, std::string data) {
		hIo.Input(input, &activator, &caller, data);
	}));
	def.def("Input", static_cast<void (*)(lua_State *, pragma::BaseIOComponent &, std::string, BaseEntity &, std::string)>([](lua_State *l, pragma::BaseIOComponent &hIo, std::string input, BaseEntity &activator, std::string data) { hIo.Input(input, &activator, NULL, data); }));
	def.def("Input", static_cast<void (*)(lua_State *, pragma::BaseIOComponent &, std::string, std::string)>([](lua_State *l, pragma::BaseIOComponent &hIo, std::string input, std::string data) { hIo.Input(input, NULL, NULL, data); }));
	def.def("Input", static_cast<void (*)(lua_State *, pragma::BaseIOComponent &, std::string)>([](lua_State *l, pragma::BaseIOComponent &hIo, std::string input) { hIo.Input(input, NULL, NULL, ""); }));
	def.def("StoreOutput",
	  static_cast<void (*)(lua_State *, pragma::BaseIOComponent &, const std::string &, const std::string &, const std::string &, const std::string &, float, int32_t)>(
	    [](lua_State *l, pragma::BaseIOComponent &hIo, const std::string &name, const std::string &entities, const std::string &input, const std::string &param, float delay, int32_t times) { hIo.StoreOutput(name, entities, input, param, delay, times); }));
	def.def("StoreOutput",
	  static_cast<void (*)(lua_State *, pragma::BaseIOComponent &, const std::string &, const std::string &, const std::string &, const std::string &, float)>(
	    [](lua_State *l, pragma::BaseIOComponent &hIo, const std::string &name, const std::string &entities, const std::string &input, const std::string &param, float delay) { hIo.StoreOutput(name, entities, input, param, delay); }));
	def.def("StoreOutput",
	  static_cast<void (*)(lua_State *, pragma::BaseIOComponent &, const std::string &, const std::string &, const std::string &, const std::string &)>(
	    [](lua_State *l, pragma::BaseIOComponent &hIo, const std::string &name, const std::string &entities, const std::string &input, const std::string &param) { hIo.StoreOutput(name, entities, input, param); }));
	def.def("StoreOutput", static_cast<void (*)(lua_State *, pragma::BaseIOComponent &, const std::string &, const std::string &)>([](lua_State *l, pragma::BaseIOComponent &hIo, const std::string &name, const std::string &info) { hIo.StoreOutput(name, info); }));
	def.def("FireOutput", static_cast<void (*)(lua_State *, pragma::BaseIOComponent &, const std::string &, BaseEntity &)>([](lua_State *l, pragma::BaseIOComponent &hIo, const std::string &name, BaseEntity &ent) { hIo.TriggerOutput(name, &ent); }));
	def.def("FireOutput", &pragma::BaseIOComponent::TriggerOutput);
	def.add_static_constant("EVENT_HANDLE_INPUT", pragma::BaseIOComponent::EVENT_HANDLE_INPUT);

	def.add_static_constant("IO_FLAG_NONE", umath::to_integral(pragma::BaseIOComponent::IoFlags::None));
	def.add_static_constant("IO_FLAG_BIT_FORCE_DELAYED_FIRE", umath::to_integral(pragma::BaseIOComponent::IoFlags::ForceDelayedFire));
}

#include "pragma/entities/components/base_model_component.hpp"
void pragma::lua::base_model_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseModelComponent>("BaseModelComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetModel", static_cast<void (*)(lua_State *, pragma::BaseModelComponent &)>([](lua_State *l, pragma::BaseModelComponent &hModel) { hModel.SetModel(std::shared_ptr<Model> {nullptr}); }));
	def.def("SetModel", static_cast<void (pragma::BaseModelComponent::*)(const std::string &)>(&pragma::BaseModelComponent::SetModel));
	def.def("SetModel", static_cast<void (pragma::BaseModelComponent::*)(const std::shared_ptr<Model> &)>(&pragma::BaseModelComponent::SetModel));
	def.def("SetSkin", &pragma::BaseModelComponent::SetSkin);
	def.def("GetSkin", &pragma::BaseModelComponent::GetSkin);
	def.def("GetSkinProperty", &pragma::BaseModelComponent::GetSkinProperty);
	def.def("SetRandomSkin", static_cast<void (*)(lua_State *, pragma::BaseModelComponent &)>([](lua_State *l, pragma::BaseModelComponent &hModel) {
		auto &mdl = hModel.GetModel();
		if(mdl == nullptr)
			return;
		hModel.SetSkin(umath::random(0, umath::max(mdl->GetTextureGroups().size(), static_cast<size_t>(1)) - 1));
	}));
	def.def("GetModel", static_cast<luabind::optional<::Model> (*)(lua_State *, pragma::BaseModelComponent &)>([](lua_State *l, pragma::BaseModelComponent &hModel) -> luabind::optional<::Model> {
		auto mdl = hModel.GetModel();
		if(mdl == nullptr)
			return Lua::nil;
		return luabind::object {l, mdl};
	}));
	def.def("GetBodyGroup", &pragma::BaseModelComponent::GetBodyGroup);
	def.def("GetBodyGroups", &pragma::BaseModelComponent::GetBodyGroups);
	def.def("SetBodyGroup", static_cast<bool (pragma::BaseModelComponent::*)(uint32_t, uint32_t)>(&pragma::BaseModelComponent::SetBodyGroup));
	def.def("SetBodyGroup", static_cast<void (pragma::BaseModelComponent::*)(const std::string &, uint32_t)>(&pragma::BaseModelComponent::SetBodyGroup));
	def.def("SetBodyGroups", static_cast<void (*)(lua_State *, pragma::BaseModelComponent &, luabind::table<>)>([](lua_State *l, pragma::BaseModelComponent &hModel, luabind::table<> t) {
		auto bodyGroups = Lua::table_to_vector<uint32_t>(l, t, 2);
		for(auto i = decltype(bodyGroups.size()) {0u}; i < bodyGroups.size(); ++i)
			hModel.SetBodyGroup(i, bodyGroups[i]);
	}));
	def.def("LookupAnimation", &pragma::BaseModelComponent::LookupAnimation);
	def.def("SetModelName", &pragma::BaseModelComponent::SetModelName);
	def.def("GetModelName", &pragma::BaseModelComponent::GetModelName);
	def.def("LookupBlendController", &pragma::BaseModelComponent::LookupBlendController);
	def.def("LookupAttachment", &pragma::BaseModelComponent::LookupAttachment);
	def.def("GetHitboxCount", &pragma::BaseModelComponent::GetHitboxCount);
	def.def(
	  "GetHitboxBounds", +[](lua_State *l, pragma::BaseModelComponent &hEnt, uint32_t boneId) -> std::optional<std::tuple<Vector3, Vector3, Vector3, Quat>> {
		  Vector3 min, max, origin;
		  auto rot = uquat::identity();
		  if(!hEnt.GetHitboxBounds(boneId, min, max, origin, rot))
			  return {};
		  return std::tuple<Vector3, Vector3, Vector3, Quat> {min, max, origin, rot};
	  });
	def.def(
	  "GetHitboxBounds", +[](lua_State *l, pragma::BaseModelComponent &hEnt, uint32_t boneId, umath::CoordinateSpace space) -> std::optional<std::tuple<Vector3, Vector3, Vector3, Quat>> {
		  Vector3 min, max, origin;
		  auto rot = uquat::identity();
		  if(!hEnt.GetHitboxBounds(boneId, min, max, origin, rot, space))
			  return {};
		  return std::tuple<Vector3, Vector3, Vector3, Quat> {min, max, origin, rot};
	  });
	def.def("LookupBone", &pragma::BaseModelComponent::LookupBone);
	def.def("GetAttachmentTransform", static_cast<luabind::optional<luabind::mult<Vector3, Quat>> (*)(lua_State *, pragma::BaseModelComponent &, std::string)>([](lua_State *l, pragma::BaseModelComponent &hEnt, std::string attachment) -> luabind::optional<luabind::mult<Vector3, Quat>> {
		Vector3 offset(0, 0, 0);
		auto rot = uquat::identity();
		if(hEnt.GetAttachment(attachment, &offset, &rot) == false)
			return Lua::nil;
		return luabind::mult<Vector3, Quat> {l, offset, rot};
	}));
	def.def("GetAttachmentTransform", static_cast<luabind::optional<luabind::mult<Vector3, Quat>> (*)(lua_State *, pragma::BaseModelComponent &, int)>([](lua_State *l, pragma::BaseModelComponent &hEnt, int attachment) -> luabind::optional<luabind::mult<Vector3, Quat>> {
		Vector3 offset(0, 0, 0);
		auto rot = uquat::identity();
		if(hEnt.GetAttachment(attachment, &offset, &rot) == false)
			return Lua::nil;
		return luabind::mult<Vector3, Quat> {l, offset, rot};
	}));

	def.add_static_constant("EVENT_ON_MODEL_CHANGED", pragma::BaseModelComponent::EVENT_ON_MODEL_CHANGED);
	def.add_static_constant("EVENT_ON_MODEL_MATERIALS_LOADED", pragma::BaseModelComponent::EVENT_ON_MODEL_MATERIALS_LOADED);
}

#include "pragma/entities/components/base_time_scale_component.hpp"
void pragma::lua::base_time_scale_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseTimeScaleComponent>("BaseTimeScaleComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetTimeScale", &pragma::BaseTimeScaleComponent::GetTimeScale);
	def.def("SetTimeScale", &pragma::BaseTimeScaleComponent::GetTimeScale);
	def.def("GetEffectiveTimeScale", &pragma::BaseTimeScaleComponent::GetEffectiveTimeScale);
}

#include "pragma/entities/components/base_ownable_component.hpp"
void pragma::lua::base_ownable_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseOwnableComponent>("BaseOwnableComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetOwner", static_cast<void (*)(lua_State *, pragma::BaseOwnableComponent &, BaseEntity *)>([](lua_State *l, pragma::BaseOwnableComponent &hEnt, BaseEntity *owner) {
		auto &ownerComponent = hEnt;
		if(owner)
			ownerComponent.SetOwner(*owner);
		else
			ownerComponent.ClearOwner();
	}));
	def.def("SetOwner", &pragma::BaseOwnableComponent::ClearOwner);
	def.def("GetOwner", static_cast<BaseEntity *(pragma::BaseOwnableComponent::*)()>(&pragma::BaseOwnableComponent::GetOwner));
	def.add_static_constant("EVENT_ON_OWNER_CHANGED", pragma::BaseOwnableComponent::EVENT_ON_OWNER_CHANGED);
}

#include "pragma/entities/components/base_debug_component.hpp"
void pragma::lua::base_debug_text_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseDebugTextComponent>("BaseDebugTextComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetText", &pragma::BaseDebugTextComponent::SetText);
}

void pragma::lua::base_debug_point_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseDebugPointComponent>("BaseDebugPointComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::lua::base_debug_line_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseDebugLineComponent>("BaseDebugLineComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::lua::base_debug_box_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseDebugBoxComponent>("BaseDebugBoxComponent");
	def.def("SetBounds", &pragma::BaseDebugBoxComponent::SetBounds);
	def.def("GetBounds", &pragma::BaseDebugBoxComponent::GetBounds);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::lua::base_debug_sphere_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseDebugSphereComponent>("BaseDebugSphereComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::lua::base_debug_cone_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseDebugConeComponent>("BaseDebugConeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::lua::base_debug_cylinder_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseDebugCylinderComponent>("BaseDebugCylinderComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::lua::base_debug_plane_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseDebugPlaneComponent>("BaseDebugPlaneComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

#include "pragma/entities/components/base_point_at_target_component.hpp"
void pragma::lua::base_point_at_target_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BasePointAtTargetComponent>("BasePointAtTargetComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetPointAtTarget", static_cast<void (*)(lua_State *, pragma::BasePointAtTargetComponent &, BaseEntity *)>([](lua_State *l, pragma::BasePointAtTargetComponent &hEnt, BaseEntity *target) {
		if(target)
			hEnt.SetPointAtTarget(*target);
		else
			hEnt.ClearPointAtTarget();
	}));
	def.def("SetPointAtTarget", &pragma::BasePointAtTargetComponent::ClearPointAtTarget);
	def.def("GetPointAtTarget", &pragma::BasePointAtTargetComponent::GetPointAtTarget);
}

#include "pragma/entities/components/liquid/base_liquid_control_component.hpp"
void pragma::lua::base_liquid_control_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseLiquidControlComponent>("BaseLiquidControlComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("CreateSplash", &pragma::BaseLiquidControlComponent::CreateSplash);
	def.def("GetStiffness", &pragma::BaseLiquidControlComponent::GetStiffness);
	def.def("SetStiffness", &pragma::BaseLiquidControlComponent::SetStiffness);
	def.def("GetPropagation", &pragma::BaseLiquidControlComponent::GetPropagation);
	def.def("SetPropagation", &pragma::BaseLiquidControlComponent::SetPropagation);
	def.def("GetWaterVelocity", &pragma::BaseLiquidControlComponent::GetLiquidVelocity, luabind::copy_policy<0> {});
	def.def("SetWaterVelocity", &pragma::BaseLiquidControlComponent::SetLiquidVelocity);
	def.def("GetDensity", &pragma::BaseLiquidControlComponent::GetDensity);
	def.def("SetDensity", &pragma::BaseLiquidControlComponent::SetDensity);
	def.def("GetLinearDragCoefficient", &pragma::BaseLiquidControlComponent::GetLinearDragCoefficient);
	def.def("SetLinearDragCoefficient", &pragma::BaseLiquidControlComponent::SetLinearDragCoefficient);
	def.def("GetTorqueDragCoefficient", &pragma::BaseLiquidControlComponent::GetTorqueDragCoefficient);
	def.def("SetTorqueDragCoefficient", &pragma::BaseLiquidControlComponent::SetTorqueDragCoefficient);
}

#include "pragma/entities/components/liquid/base_liquid_surface_simulation_component.hpp"
void pragma::lua::base_liquid_surface_simulation_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<pragma::BaseLiquidSurfaceSimulationComponent>("BaseLiquidSurfaceSimulationComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.add_static_constant("EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED", pragma::BaseLiquidSurfaceSimulationComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED);
}

// --template-register-definition
