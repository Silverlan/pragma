// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.shared;

import :scripting.lua.classes.entity_components;
import :scripting.lua.entity_component_event;

namespace Lua {
	template<typename... Types>
	static luabind::class_<Types..., pragma::BaseEntityComponent> create_base_entity_component_class(const char *name)
	{
		auto def = pragma::LuaCore::create_entity_component_class<Types..., pragma::BaseEntityComponent>(name);
		def.def(luabind::tostring(luabind::self));
		return def;
	}
};

namespace pragma::LuaCore {
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
std::optional<Lua::udm_type> pragma::LuaCore::get_member_value(lua::State *l, BaseEntityComponent &component, const ComponentMemberInfo &memberInfo)
{
	return pragma::ents::visit_member(memberInfo.type, [&memberInfo, &component, l](auto tag) -> std::optional<Lua::udm_type> {
		using T = typename decltype(tag)::type;
		if constexpr(!pragma::is_valid_component_property_type_v<T>)
			return {};
		else {
			T value;
			memberInfo.getterFunction(memberInfo, component, &value);
			if constexpr(std::is_same_v<T, ents::Element>) {
				if(!value)
					return Lua::nil;
			}
			return Lua::udm_type {luabind::object {l, value}};
		}
	});
}
bool pragma::LuaCore::set_member_value(lua::State *l, BaseEntityComponent &component, const ComponentMemberInfo &memberInfo, Lua::udm_type value)
{
	return pragma::ents::visit_member(memberInfo.type, [&memberInfo, &component, l, &value](auto tag) -> bool {
		using T = typename decltype(tag)::type;
		if constexpr(!pragma::is_valid_component_property_type_v<T> || std::is_same_v<T, ents::Element>)
			return false;
		else {
			if constexpr(Lua::is_native_type<T>) {
				if(memberInfo.IsEnum()) {
					if constexpr(udm::is_numeric_type(udm::type_to_enum<T>())) {
						if(static_cast<Lua::Type>(luabind::type(value)) == Lua::Type::String) {
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
static_assert(pragma::math::to_integral(pragma::ents::EntityMemberType::VersionIndex) == 0);
bool pragma::LuaCore::set_member_value(lua::State *l, BaseEntityComponent &component, const ComponentMemberInfo &memberInfo, const EntityURef &eref)
{
	if(memberInfo.type != ents::EntityMemberType::Entity)
		return false;
	memberInfo.setterFunction(memberInfo, component, &eref);
	return true;
}
bool pragma::LuaCore::set_member_value(lua::State *l, BaseEntityComponent &component, const ComponentMemberInfo &memberInfo, const MultiEntityURef &eref)
{
	if(memberInfo.type != ents::EntityMemberType::MultiEntity)
		return false;
	memberInfo.setterFunction(memberInfo, component, &eref);
	return true;
}
bool pragma::LuaCore::set_member_value(lua::State *l, BaseEntityComponent &component, const ComponentMemberInfo &memberInfo, const EntityUComponentMemberRef &eref)
{
	if(memberInfo.type != ents::EntityMemberType::ComponentProperty)
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

template<typename TMemberIdentifier, typename TValue, auto TSetValue>
bool set_transform_member_value(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const TValue &value)
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
static bool set_transform_member_pos(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const Vector3 &value)
{
	return set_transform_member_value<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::SetTransformMemberPos>(component, memId, space, value);
}

template<typename TMemberIdentifier>
static bool set_transform_member_rot(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const Quat &value)
{
	return set_transform_member_value<TMemberIdentifier, Quat, &pragma::BaseEntityComponent::SetTransformMemberRot>(component, memId, space, value);
}

template<typename TMemberIdentifier>
static bool set_transform_member_scale(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const Vector3 &value)
{
	return set_transform_member_value<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::SetTransformMemberScale>(component, memId, space, value);
}

template<typename TMemberIdentifier>
static bool set_transform_member_pose(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const pragma::math::ScaledTransform &value)
{
	return set_transform_member_value<TMemberIdentifier, pragma::math::ScaledTransform, &pragma::BaseEntityComponent::SetTransformMemberPose>(component, memId, space, value);
}

template<typename TMemberIdentifier, typename TValue, auto TGetValue, auto TGetParentValue, bool parentSpaceOnly = false>
std::optional<TValue> get_transform_member_value(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space)
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
static std::optional<Vector3> get_transform_member_pos(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space)
{
	return get_transform_member_value<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::GetTransformMemberPos, &pragma::BaseEntityComponent::GetTransformMemberParentPos, parentSpaceOnly>(component, memId, space);
}

template<typename TMemberIdentifier, bool parentSpaceOnly = false>
static std::optional<Quat> get_transform_member_rot(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space)
{
	return get_transform_member_value<TMemberIdentifier, Quat, &pragma::BaseEntityComponent::GetTransformMemberRot, &pragma::BaseEntityComponent::GetTransformMemberParentRot, parentSpaceOnly>(component, memId, space);
}

template<typename TMemberIdentifier, bool parentSpaceOnly = false>
static std::optional<Vector3> get_transform_member_scale(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space)
{
	return get_transform_member_value<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::GetTransformMemberScale, &pragma::BaseEntityComponent::GetTransformMemberParentScale, parentSpaceOnly>(component, memId, space);
}

template<typename TMemberIdentifier, bool parentSpaceOnly = false>
static std::optional<pragma::math::ScaledTransform> get_transform_member_pose(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space)
{
	return get_transform_member_value<TMemberIdentifier, pragma::math::ScaledTransform, &pragma::BaseEntityComponent::GetTransformMemberPose, &pragma::BaseEntityComponent::GetTransformMemberParentPose, parentSpaceOnly>(component, memId, space);
}

template<typename TMemberIdentifier, typename TValue, auto TConvert>
static std::optional<TValue> convert_transform_member_value_to_target_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const TValue &value)
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
static std::optional<Vector3> convert_transform_member_pos_to_target_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const Vector3 &pos)
{
	return convert_transform_member_value_to_target_space<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::ConvertTransformMemberPosToTargetSpace>(component, memId, space, pos);
}
template<typename TMemberIdentifier>
static std::optional<Quat> convert_transform_member_rot_to_target_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const Quat &rot)
{
	return convert_transform_member_value_to_target_space<TMemberIdentifier, Quat, &pragma::BaseEntityComponent::ConvertTransformMemberRotToTargetSpace>(component, memId, space, rot);
}
template<typename TMemberIdentifier>
static std::optional<Vector3> convert_transform_member_scale_to_target_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const Vector3 &scale)
{
	return convert_transform_member_value_to_target_space<TMemberIdentifier, Vector3, &pragma::BaseEntityComponent::ConvertTransformMemberScaleToTargetSpace>(component, memId, space, scale);
}
template<typename TMemberIdentifier>
static std::optional<pragma::math::ScaledTransform> convert_transform_member_pose_to_target_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const pragma::math::ScaledTransform &pose)
{
	return convert_transform_member_value_to_target_space<TMemberIdentifier, pragma::math::ScaledTransform, &pragma::BaseEntityComponent::ConvertTransformMemberPoseToTargetSpace>(component, memId, space, pose);
}

template<typename TMemberIdentifier, typename T, auto TConvert>
static std::optional<std::pair<T, pragma::math::CoordinateSpace>> convert_value_to_member_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const T &value)
{
	if constexpr(std::is_same_v<TMemberIdentifier, std::string>) {
		auto idx = component.GetMemberIndex(memId);
		if(!idx)
			return {};
		return convert_value_to_member_space<pragma::ComponentMemberIndex, T, TConvert>(component, *idx, space, value);
	}
	else {
		pragma::math::CoordinateSpace memberSpace;
		T newValue = value;
		if(!(component.*TConvert)(memId, space, newValue, &memberSpace))
			return {};
		return std::pair<T, pragma::math::CoordinateSpace> {newValue, memberSpace};
	}
	return {};
}
template<typename TMemberIdentifier, typename T>
static std::optional<std::pair<T, pragma::math::CoordinateSpace>> convert_pos_to_member_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const T &value)
{
	return convert_value_to_member_space<TMemberIdentifier, T, &pragma::BaseEntityComponent::ConvertPosToMemberSpace>(component, memId, space, value);
}
template<typename TMemberIdentifier, typename T>
static std::optional<std::pair<T, pragma::math::CoordinateSpace>> convert_rot_to_member_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const T &value)
{
	return convert_value_to_member_space<TMemberIdentifier, T, &pragma::BaseEntityComponent::ConvertRotToMemberSpace>(component, memId, space, value);
}
template<typename TMemberIdentifier, typename T>
static std::optional<std::pair<T, pragma::math::CoordinateSpace>> convert_scale_to_member_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const T &value)
{
	return convert_value_to_member_space<TMemberIdentifier, T, &pragma::BaseEntityComponent::ConvertScaleToMemberSpace>(component, memId, space, value);
}
template<typename TMemberIdentifier, typename T>
static std::optional<std::pair<T, pragma::math::CoordinateSpace>> convert_pose_to_member_space(pragma::BaseEntityComponent &component, const TMemberIdentifier &memId, pragma::math::CoordinateSpace space, const T &value)
{
	return convert_value_to_member_space<TMemberIdentifier, T, &pragma::BaseEntityComponent::ConvertPoseToMemberSpace>(component, memId, space, value);
}

template<size_t N>
void log_with_args(const pragma::BaseEntityComponent &component, const char *msg, spdlog::level::level_enum logLevel, lua::State *l, int32_t argOffset)
{
	std::array<std::string, N> args;
	for(size_t i = 0; i < args.size(); ++i)
		args[i] = Lua::logging::to_string(l, argOffset + (i + 1));

	auto log = [&](const auto &...elements) { component.Log(logLevel, std::vformat(msg, std::make_format_args(elements...))); };
	std::apply(log, args);
}

static int log(lua::State *l, spdlog::level::level_enum logLevel)
{
	auto &component = Lua::Check<pragma::BaseEntityComponent>(l, 1);
	const char *msg = Lua::CheckString(l, 2);
	int32_t argOffset = 2;
	auto n = Lua::GetStackTop(l) - argOffset; /* number of arguments */
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

static CallbackHandle add_event_callback(lua::State *l, pragma::BaseEntityComponent &hComponent, uint32_t eventId, const Lua::func<void> &function)
{
	auto hCb = hComponent.AddEventCallback(eventId, [l, function](std::reference_wrapper<pragma::ComponentEvent> ev) -> pragma::util::EventReply {
		// We need to create a copy of the lua-state pointer, since the callback can remove itself, which
		// would also cause the std::function-object to be destroyed (and therefore the captured variables).
		auto lTmp = l;
		auto oCallbackTmp = function;

		auto nstack = Lua::GetStackTop(l);
		auto c = Lua::CallFunction(
		  l,
		  [&oCallbackTmp, &ev](lua::State *l) -> Lua::StatusCode {
			  oCallbackTmp.push(l);
			  ev.get().PushArguments(l);
			  return Lua::StatusCode::Ok;
		  },
		  lua::MultiReturn);
		auto numRet = Lua::GetStackTop(l) - nstack;
		if(c == Lua::StatusCode::Ok && numRet > 0 && Lua::IsNone(lTmp, -1) == false) {
			auto result = Lua::IsNumber(lTmp, -numRet) ? static_cast<pragma::util::EventReply>(Lua::CheckInt(lTmp, -numRet)) : pragma::util::EventReply::Unhandled;
			if(result == pragma::util::EventReply::Handled)
				ev.get().HandleReturnValues(l);
			Lua::Pop(lTmp, numRet); // Pop result(s)
			return result;
		}
		return pragma::util::EventReply::Unhandled;
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
	entityComponentDef.def("ConvertPoseToMemberSpace", &convert_pose_to_member_space<TMemberIdentifier, pragma::math::ScaledTransform>);
}

void pragma::LuaCore::register_entity_component_classes(lua::State *l, luabind::module_ &mod)
{
	auto entityComponentDef = pragma::LuaCore::create_entity_component_class<BaseEntityComponent>("EntityComponent");
	entityComponentDef.def("BroadcastEvent", static_cast<util::EventReply (BaseEntityComponent::*)(ComponentEventId) const>(&BaseEntityComponent::BroadcastEvent));
	entityComponentDef.def("BroadcastEvent", static_cast<util::EventReply (*)(lua::State *, BaseEntityComponent &, uint32_t, const luabind::tableT<void> &)>([](lua::State *l, BaseEntityComponent &hComponent, uint32_t eventId, const luabind::tableT<void> &eventArgs) {
		int32_t t = 3;
		if(get_engine()->GetNetworkState(l)->GetGameState()->BroadcastEntityEvent(hComponent, eventId, t) == false) {
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
	entityComponentDef.def("InvokeEventCallbacks", static_cast<util::EventReply (BaseEntityComponent::*)(ComponentEventId) const>(&BaseEntityComponent::InvokeEventCallbacks));
	entityComponentDef.def("InvokeEventCallbacks", static_cast<util::EventReply (*)(lua::State *, BaseEntityComponent &, uint32_t, const luabind::tableT<void> &)>([](lua::State *l, BaseEntityComponent &hComponent, uint32_t eventId, const luabind::tableT<void> &eventArgs) {
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
	entityComponentDef.def("GetUri", static_cast<std::string (BaseEntityComponent::*)() const>(&BaseEntityComponent::GetUri));
	entityComponentDef.def("GetMemberUri", static_cast<std::string (BaseEntityComponent::*)(const std::string &) const>(&BaseEntityComponent::GetMemberUri));
	entityComponentDef.def("GetMemberUri", static_cast<std::optional<std::string> (BaseEntityComponent::*)(ComponentMemberIndex) const>(&BaseEntityComponent::GetMemberUri));
	entityComponentDef.def("GetEntity", static_cast<ecs::BaseEntity &(BaseEntityComponent::*)()>(&BaseEntityComponent::GetEntity));
	entityComponentDef.def("GetComponentId", &BaseEntityComponent::GetComponentId);
	entityComponentDef.def("SetTickPolicy", &BaseEntityComponent::SetTickPolicy);
	entityComponentDef.def("GetTickPolicy", &BaseEntityComponent::GetTickPolicy);
	entityComponentDef.def("GetNextTick", &BaseEntityComponent::GetNextTick);
	entityComponentDef.def("SetNextTick", &BaseEntityComponent::SetNextTick);
	entityComponentDef.def("SetActive", &BaseEntityComponent::SetActive);
	entityComponentDef.def("IsActive", &BaseEntityComponent::IsActive);
	entityComponentDef.def("Activate", &BaseEntityComponent::Activate);
	entityComponentDef.def("Deactivate", &BaseEntityComponent::Deactivate);
	entityComponentDef.def("GetMemberIndex", &BaseEntityComponent::GetMemberIndex);
	entityComponentDef.def("GetMemberInfo", &BaseEntityComponent::GetMemberInfo);
	entityComponentDef.def("GetDynamicMemberIndices", static_cast<std::vector<ComponentMemberIndex> (*)(BaseEntityComponent &)>(&get_dynamic_member_ids));
	entityComponentDef.def("GetStaticMemberCount", &BaseEntityComponent::GetStaticMemberCount);
	register_entity_component_transform_methods<ComponentMemberIndex>(entityComponentDef);
	register_entity_component_transform_methods<std::string>(entityComponentDef);
	entityComponentDef.def(
	  "GetTransformMemberSpace", +[](BaseEntityComponent &c, ComponentMemberIndex memberIndex) -> std::optional<math::CoordinateSpace> {
		  auto *memberInfo = c.GetMemberInfo(memberIndex);
		  if(!memberInfo)
			  return {};
		  auto *cMetaData = memberInfo->FindTypeMetaData<ents::CoordinateTypeMetaData>();
		  if(!cMetaData)
			  return {};
		  return cMetaData->space;
	  });
	entityComponentDef.def(
	  "GetMemberIndices", +[](lua::State *l, BaseEntityComponent &component) -> std::vector<ComponentMemberIndex> {
		  std::vector<ComponentMemberIndex> memberIndices;
		  auto numStaticMembers = component.GetStaticMemberCount();
		  memberIndices.reserve(numStaticMembers);
		  for(auto i = decltype(numStaticMembers) {0u}; i < numStaticMembers; ++i)
			  memberIndices.push_back(i);
		  get_dynamic_member_ids(component, memberIndices);
		  return memberIndices;
	  });
	entityComponentDef.def(
	  "InvokeElementMemberChangeCallback", +[](BaseEntityComponent &component, const std::string &memberName) {
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
	  "InvokeElementMemberChangeCallback", +[](BaseEntityComponent &component, uint32_t memberIndex) {
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
	  "GetMemberValue", +[](lua::State *l, BaseEntityComponent &component, const std::string &memberName) -> std::optional<Lua::udm_type> {
		  auto *info = component.FindMemberInfo(memberName);
		  if(!info)
			  return {};
		  return get_member_value(l, component, *info);
	  });
	entityComponentDef.def(
	  "GetMemberValue", +[](lua::State *l, BaseEntityComponent &component, uint32_t memberIndex) -> std::optional<Lua::udm_type> {
		  auto *info = component.GetMemberInfo(memberIndex);
		  if(!info)
			  return {};
		  return get_member_value(l, component, *info);
	  });
	entityComponentDef.def("SetMemberValue", static_cast<bool (*)(lua::State *, BaseEntityComponent &, const ComponentMemberInfo &, Lua::udm_type)>(&set_member_value));
	entityComponentDef.def("SetMemberValue", static_cast<bool (*)(lua::State *, BaseEntityComponent &, const ComponentMemberInfo &, const EntityURef &)>(&set_member_value));
	entityComponentDef.def("SetMemberValue", static_cast<bool (*)(lua::State *, BaseEntityComponent &, const ComponentMemberInfo &, const MultiEntityURef &)>(&set_member_value));
	entityComponentDef.def("SetMemberValue", static_cast<bool (*)(lua::State *, BaseEntityComponent &, const ComponentMemberInfo &, const EntityUComponentMemberRef &)>(&set_member_value));
	entityComponentDef.def(
	  "SetMemberValue", +[](lua::State *l, BaseEntityComponent &component, uint32_t memberIndex, Lua::udm_type value) -> bool {
		  auto *info = component.GetMemberInfo(memberIndex);
		  if(!info)
			  return false;
		  return set_member_value(l, component, *info, value);
	  });
	entityComponentDef.def(
	  "SetMemberValue", +[](lua::State *l, BaseEntityComponent &component, const std::string &memberName, Lua::udm_type value) -> bool {
		  auto *info = component.FindMemberInfo(memberName);
		  if(!info)
			  return false;
		  return set_member_value(l, component, *info, value);
	  });
	entityComponentDef.def(
	  "SetMemberValue", +[](lua::State *l, BaseEntityComponent &component, const std::string &memberName, const EntityURef &eref) -> bool {
		  auto *info = component.FindMemberInfo(memberName);
		  if(!info)
			  return false;
		  return set_member_value(l, component, *info, eref);
	  });
	entityComponentDef.def(
	  "SetMemberValue", +[](lua::State *l, BaseEntityComponent &component, const std::string &memberName, const MultiEntityURef &eref) -> bool {
		  auto *info = component.FindMemberInfo(memberName);
		  if(!info)
			  return false;
		  return set_member_value(l, component, *info, eref);
	  });
	entityComponentDef.def(
	  "SetMemberValue", +[](lua::State *l, BaseEntityComponent &component, const std::string &memberName, const EntityUComponentMemberRef &eref) -> bool {
		  auto *info = component.FindMemberInfo(memberName);
		  if(!info)
			  return false;
		  return set_member_value(l, component, *info, eref);
	  });
	entityComponentDef.def("IsValid", static_cast<bool (*)(lua::State *, BaseEntityComponent *)>([](lua::State *l, BaseEntityComponent *hComponent) { return hComponent != nullptr; }));
	entityComponentDef.def(
	  "RegisterNetEvent", +[](lua::State *l, BaseEntityComponent &hComponent, const std::string &eventName) {
		  auto id = hComponent.SetupNetEvent(eventName);
		  auto *nw = get_engine()->GetNetworkState(l);
		  auto *game = nw->GetGameState();
		  auto *componentInfo = game->GetEntityComponentManager().GetComponentInfo(hComponent.GetComponentId());
		  if(componentInfo && math::is_flag_set(componentInfo->flags, ComponentFlags::Networked) == false)
			  ::operator<<(::operator<<(::operator<<(::operator<<(Con::CWAR, "WARNING: Component '"), componentInfo->name),
			                 "' has uses net-events, but was not registered as networked, this means networking will be disabled for this component! Set the 'ents.EntityComponent.FREGISTER_BIT_NETWORKED' flag when registering the component to fix this!"),
			    Con::endl);
		  return id;
	  });
	entityComponentDef.def("GetComponentName", static_cast<std::string (*)(lua::State *, BaseEntityComponent &)>([](lua::State *l, BaseEntityComponent &component) {
		auto *nw = get_engine()->GetNetworkState(l);
		auto *game = nw->GetGameState();
		auto &componentManager = game->GetEntityComponentManager();
		auto &info = *componentManager.GetComponentInfo(component.GetComponentId());
		return std::string {*info.name};
	}));
	entityComponentDef.def("AddEventCallback", &add_event_callback);
	entityComponentDef.def("AddEventListener", &add_event_callback); // Alias
	entityComponentDef.def("InjectEvent", static_cast<util::EventReply (BaseEntityComponent::*)(ComponentEventId)>(&BaseEntityComponent::InjectEvent));
	entityComponentDef.def("InjectEvent", static_cast<void (*)(lua::State *, BaseEntityComponent &, uint32_t, const luabind::tableT<void> &)>([](lua::State *l, BaseEntityComponent &hComponent, uint32_t eventId, const luabind::tableT<void> &eventArgs) {
		int32_t t = 3;
		if(get_engine()->GetNetworkState(l)->GetGameState()->InjectEntityEvent(hComponent, eventId, t) == false) {
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
	entityComponentDef.def("Save", &BaseEntityComponent::Save);
	entityComponentDef.def("Load", static_cast<void (BaseEntityComponent::*)(udm::LinkedPropertyWrapperArg)>(&BaseEntityComponent::Load));
	entityComponentDef.def("Copy", static_cast<void (*)(lua::State *, BaseEntityComponent &, BaseEntityComponent &)>([](lua::State *l, BaseEntityComponent &hComponent, BaseEntityComponent &hComponentOther) {
		if(hComponent.GetComponentId() != hComponentOther.GetComponentId() || &hComponent == &hComponentOther)
			return;
		auto el = udm::Property::Create<udm::Element>();
		udm::LinkedPropertyWrapper prop {*el};
		hComponent.Save(prop);
		hComponentOther.Load(prop);
	}));
	entityComponentDef.def("GetMemberInfos", static_cast<luabind::tableT<ComponentMemberInfo> (*)(lua::State *, BaseEntityComponent &)>([](lua::State *l, BaseEntityComponent &hComponent) -> luabind::tableT<ComponentMemberInfo> {
		auto t = luabind::newtable(l);
		uint32_t idx = 0;
		auto *memberInfo = hComponent.GetMemberInfo(idx++);
		while(memberInfo) {
			t[idx] = memberInfo;
			memberInfo = hComponent.GetMemberInfo(idx++);
		}
		return t;
	}));
	entityComponentDef.def("Log", static_cast<void (BaseEntityComponent::*)(const std::string &, BaseEntityComponent::LogSeverity) const>(&BaseEntityComponent::Log));
	entityComponentDef.def("Log", static_cast<void (BaseEntityComponent::*)(const std::string &, BaseEntityComponent::LogSeverity) const>(&BaseEntityComponent::Log), luabind::default_parameter_policy<3, BaseEntityComponent::LogSeverity::Warning> {});
	entityComponentDef.def("SetPropertyAnimated", &BaseEntityComponent::SetPropertyAnimated);
	entityComponentDef.def("IsPropertyAnimated", &BaseEntityComponent::IsPropertyAnimated);
	entityComponentDef.add_static_constant("FREGISTER_NONE", math::to_integral(ComponentFlags::None));
	entityComponentDef.add_static_constant("FREGISTER_BIT_NETWORKED", math::to_integral(ComponentFlags::Networked));
	entityComponentDef.add_static_constant("FREGISTER_BIT_HIDE_IN_EDITOR", math::to_integral(ComponentFlags::HideInEditor));

	entityComponentDef.add_static_constant("CALLBACK_TYPE_ENTITY", math::to_integral(BaseEntityComponent::CallbackType::Entity));
	entityComponentDef.add_static_constant("CALLBACK_TYPE_COMPONENT", math::to_integral(BaseEntityComponent::CallbackType::Component));
	entityComponentDef.add_static_constant("LOG_SEVERITY_NORMAL", math::to_integral(BaseEntityComponent::LogSeverity::Normal));
	entityComponentDef.add_static_constant("LOG_SEVERITY_WARNING", math::to_integral(BaseEntityComponent::LogSeverity::Warning));
	entityComponentDef.add_static_constant("LOG_SEVERITY_ERROR", math::to_integral(BaseEntityComponent::LogSeverity::Error));
	entityComponentDef.add_static_constant("LOG_SEVERITY_CRITICAL", math::to_integral(BaseEntityComponent::LogSeverity::Critical));
	entityComponentDef.add_static_constant("LOG_SEVERITY_DEBUG", math::to_integral(BaseEntityComponent::LogSeverity::Debug));
	mod[entityComponentDef];

	luabind::object oLogger = luabind::globals(l)["ents"];
	oLogger = oLogger["EntityComponent"];
	Lua::logging::add_log_func<spdlog::level::trace, &::log>(l, oLogger, "LogTrace");
	Lua::logging::add_log_func<spdlog::level::debug, &::log>(l, oLogger, "LogDebug");
	Lua::logging::add_log_func<spdlog::level::info, &::log>(l, oLogger, "LogInfo");
	Lua::logging::add_log_func<spdlog::level::warn, &::log>(l, oLogger, "LogWarn");
	Lua::logging::add_log_func<spdlog::level::err, &::log>(l, oLogger, "LogErr");
	Lua::logging::add_log_func<spdlog::level::critical, &::log>(l, oLogger, "LogCritical");

	auto defBvh = Lua::create_base_entity_component_class<BaseBvhComponent>("BaseBvhComponent");

	auto defIntersectionMeshInfo = luabind::class_<MeshIntersectionInfo::MeshInfo>("IntersectionMeshInfo");
	defIntersectionMeshInfo.def_readonly("mesh", &MeshIntersectionInfo::MeshInfo::mesh);
	defIntersectionMeshInfo.def_readonly("entity", &MeshIntersectionInfo::MeshInfo::entity);
	defBvh.scope[defIntersectionMeshInfo];

	defBvh.def("RebuildBvh", static_cast<void (BaseBvhComponent::*)()>(&BaseBvhComponent::RebuildBvh));
	defBvh.def("GetVertex", &BaseBvhComponent::GetVertex);
	defBvh.def("GetTriangleCount", &BaseBvhComponent::GetTriangleCount);
	defBvh.def("DebugPrint", &BaseBvhComponent::DebugPrint);
	defBvh.def("DebugDraw", &BaseBvhComponent::DebugDraw);
	defBvh.def("DebugDrawBvhTree", &BaseBvhComponent::DebugDrawBvhTree);
	defBvh.def(
	  "FindPrimitiveMeshInfo", +[](lua::State *l, const BaseBvhComponent &bvhC, size_t primIdx) -> std::optional<std::pair<EntityHandle, std::shared_ptr<geometry::ModelSubMesh>>> {
		  auto *range = bvhC.FindPrimitiveMeshInfo(primIdx);
		  if(!range)
			  return std::optional<std::pair<EntityHandle, std::shared_ptr<geometry::ModelSubMesh>>> {};
		  auto *ent = range->entity ? range->entity : &bvhC.GetEntity();
		  return std::pair<EntityHandle, std::shared_ptr<geometry::ModelSubMesh>> {ent->GetHandle(), range->mesh};
	  });

	/*auto defBvhIntersectionInfo = luabind::class_<pragma::BvhIntersectionInfo>("IntersectionInfo");
	defBvhIntersectionInfo.def_readonly("primitives",&pragma::BvhIntersectionInfo::primitives);
	defBvh.scope[defBvhIntersectionInfo];

	auto defBvhMeshIntersectionInfo = luabind::class_<pragma::BvhMeshIntersectionInfo,pragma::BvhIntersectionInfo>("MeshIntersectionInfo");
	defBvh.scope[defBvhMeshIntersectionInfo];*/
	mod[defBvh];

	auto defStaticBvh = pragma::LuaCore::create_entity_component_class<BaseStaticBvhCacheComponent, BaseBvhComponent>("BaseStaticBvhCacheComponent");
	defStaticBvh.def("SetEntityDirty", &BaseStaticBvhCacheComponent::SetEntityDirty);
	defStaticBvh.def("AddEntity", &BaseStaticBvhCacheComponent::AddEntity);
	defStaticBvh.def("RemoveEntity", +[](BaseStaticBvhCacheComponent &component, ecs::BaseEntity &ent) { component.RemoveEntity(ent); });
	mod[defStaticBvh];

	auto defStaticBvhUser = Lua::create_base_entity_component_class<BaseStaticBvhUserComponent>("BaseStaticBvhUserComponent");
	defStaticBvhUser.add_static_constant("EVENT_ON_ACTIVATION_STATE_CHANGED", baseStaticBvhUserComponent::EVENT_ON_ACTIVATION_STATE_CHANGED);
	defStaticBvhUser.add_static_constant("EVENT_ON_STATIC_BVH_COMPONENT_CHANGED", baseStaticBvhUserComponent::EVENT_ON_STATIC_BVH_COMPONENT_CHANGED);
	defStaticBvhUser.def("IsActive", &BaseStaticBvhUserComponent::IsActive);
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

	auto defGameComponent = Lua::create_base_entity_component_class<BaseGameComponent>("BaseGameComponent");
	mod[defGameComponent];
}
void pragma::LuaCore::base_child_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseChildComponent>("BaseChildComponent");
	def.add_static_constant("EVENT_ON_PARENT_CHANGED", baseChildComponent::EVENT_ON_PARENT_CHANGED);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("ClearParent", &BaseChildComponent::ClearParent);
	def.def("SetParent", &BaseChildComponent::SetParent);
	def.def("GetParent", &BaseChildComponent::GetParent, luabind::copy_policy<0> {});
	def.def("GetParentEntity", static_cast<ecs::BaseEntity *(BaseChildComponent::*)()>(&BaseChildComponent::GetParentEntity));
	def.def("HasParent", &BaseChildComponent::HasParent);
}
void pragma::LuaCore::base_attachable_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseAttachmentComponent>("BaseAttachmentComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("AttachToEntity", &BaseAttachmentComponent::AttachToEntity, luabind::discard_result {});
	def.def("AttachToEntity", static_cast<AttachmentData *(*)(BaseAttachmentComponent &, ecs::BaseEntity *)>([](BaseAttachmentComponent &component, ecs::BaseEntity *ent) { return component.AttachToEntity(ent); }), luabind::discard_result {});
	//AttachmentData *AttachToEntity(BaseEntity *ent,const AttachmentInfo &attInfo={});
	//def.def("AttachToEntity",&pragma::BaseAttachmentComponent::AttachToEntity,luabind::default_parameter_policy<0,AttachmentInfo,att>{});
	//def.def("AttachToEntity",std::bind(&pragma::BaseAttachmentComponent::AttachToEntity,std::placeholders::_1,AttachmentInfo{}));
	def.def("ClearAttachment", &BaseAttachmentComponent::ClearAttachment);
	def.def("AttachToAttachment", static_cast<AttachmentData *(BaseAttachmentComponent::*)(ecs::BaseEntity *, uint32_t, const AttachmentInfo &)>(&BaseAttachmentComponent::AttachToAttachment), luabind::discard_result {});
	def.def("AttachToAttachment", static_cast<AttachmentData *(*)(BaseAttachmentComponent &, ecs::BaseEntity *, uint32_t)>([](BaseAttachmentComponent &component, ecs::BaseEntity *ent, uint32_t attIdx) { return component.AttachToAttachment(ent, attIdx); }),
	  luabind::discard_result {});
	//def.def("AttachToAttachment",std::bind(static_cast<AttachmentData*(pragma::BaseAttachmentComponent::*)(BaseEntity*,uint32_t,const AttachmentInfo&)>(&pragma::BaseAttachmentComponent::AttachToAttachment),std::placeholders::_1,std::placeholders::_2,AttachmentInfo{}));
	def.def("AttachToAttachment", static_cast<AttachmentData *(BaseAttachmentComponent::*)(ecs::BaseEntity *, std::string, const AttachmentInfo &)>(&BaseAttachmentComponent::AttachToAttachment), luabind::discard_result {});
	def.def("AttachToAttachment", static_cast<AttachmentData *(*)(BaseAttachmentComponent &, ecs::BaseEntity *, std::string)>([](BaseAttachmentComponent &component, ecs::BaseEntity *ent, std::string att) { return component.AttachToAttachment(ent, att); }),
	  luabind::discard_result {});
	//def.def("AttachToAttachment",std::bind(static_cast<AttachmentData*(pragma::BaseAttachmentComponent::*)(BaseEntity*,std::string,const AttachmentInfo&)>(&pragma::BaseAttachmentComponent::AttachToAttachment),std::placeholders::_1,std::placeholders::_2,AttachmentInfo{}));
	def.def("AttachToBone",
	  static_cast<void (*)(lua::State *, BaseAttachmentComponent &, ecs::BaseEntity &, std::string, AttachmentInfo &)>(
	    [](lua::State *l, BaseAttachmentComponent &hEnt, ecs::BaseEntity &parent, std::string bone, AttachmentInfo &attInfo) { hEnt.AttachToBone(&parent, bone, attInfo); }),
	  luabind::discard_result {});
	def.def("AttachToBone", static_cast<void (*)(lua::State *, BaseAttachmentComponent &, ecs::BaseEntity &, std::string)>([](lua::State *l, BaseAttachmentComponent &hEnt, ecs::BaseEntity &parent, std::string bone) { hEnt.AttachToBone(&parent, bone); }),
	  luabind::discard_result {});
	def.def("AttachToBone", static_cast<void (*)(lua::State *, BaseAttachmentComponent &, ecs::BaseEntity &, int, AttachmentInfo &)>([](lua::State *l, BaseAttachmentComponent &hEnt, ecs::BaseEntity &parent, int bone, AttachmentInfo &attInfo) {
		hEnt.AttachToBone(&parent, bone, attInfo);
	}),
	  luabind::discard_result {});
	def.def("AttachToBone", static_cast<void (*)(lua::State *, BaseAttachmentComponent &, ecs::BaseEntity &, int)>([](lua::State *l, BaseAttachmentComponent &hEnt, ecs::BaseEntity &parent, int bone) { hEnt.AttachToBone(&parent, bone); })),
	  luabind::discard_result {};
	def.def("GetLocalPose", &BaseAttachmentComponent::GetLocalPose);
	def.def("SetLocalPose", &BaseAttachmentComponent::SetLocalPose);
	def.def("GetParent", &BaseAttachmentComponent::GetParent);

	def.def("GetBone", static_cast<luabind::object (*)(lua::State *, BaseAttachmentComponent &)>([](lua::State *l, BaseAttachmentComponent &hEnt) -> luabind::object {
		auto *data = hEnt.GetAttachmentData();
		if(data == nullptr)
			return luabind::object {};
		return luabind::object {l, data->bone};
	}));
	def.def("SetAttachmentFlags", &BaseAttachmentComponent::SetAttachmentFlags);
	def.def("GetAttachmentFlags", &BaseAttachmentComponent::GetAttachmentFlags);

	auto defAttInfo = luabind::class_<AttachmentInfo>("AttachmentInfo");
	defAttInfo.def(luabind::constructor<>());
	defAttInfo.def("SetOffset", static_cast<void (*)(lua::State *, AttachmentInfo &, const Vector3 &)>([](lua::State *l, AttachmentInfo &attInfo, const Vector3 &offset) { attInfo.offset = offset; }));
	defAttInfo.def("SetOffset", static_cast<void (*)(lua::State *, AttachmentInfo &)>([](lua::State *l, AttachmentInfo &attInfo) { attInfo.offset.reset(); }));
	defAttInfo.def("SetRotation", static_cast<void (*)(lua::State *, AttachmentInfo &, const Quat &)>([](lua::State *l, AttachmentInfo &attInfo, const Quat &rotation) { attInfo.rotation = rotation; }));
	defAttInfo.def("SetRotation", static_cast<void (*)(lua::State *, AttachmentInfo &)>([](lua::State *l, AttachmentInfo &attInfo) { attInfo.rotation.reset(); }));
	defAttInfo.def_readwrite("flags", reinterpret_cast<std::underlying_type_t<decltype(AttachmentInfo::flags)> AttachmentInfo::*>(&AttachmentInfo::flags));
	def.scope[defAttInfo];

	def.add_static_constant("EVENT_ON_ATTACHMENT_UPDATE", baseAttachmentComponent::EVENT_ON_ATTACHMENT_UPDATE);

	def.add_static_constant("FATTACHMENT_MODE_POSITION_ONLY", math::to_integral(FAttachmentMode::PositionOnly));
	def.add_static_constant("FATTACHMENT_MODE_BONEMERGE", math::to_integral(FAttachmentMode::BoneMerge));
	def.add_static_constant("FATTACHMENT_MODE_UPDATE_EACH_FRAME", math::to_integral(FAttachmentMode::UpdateEachFrame));
	def.add_static_constant("FATTACHMENT_MODE_PLAYER_VIEW", math::to_integral(FAttachmentMode::PlayerView));
	def.add_static_constant("FATTACHMENT_MODE_PLAYER_VIEW_YAW", math::to_integral(FAttachmentMode::PlayerViewYaw));
	def.add_static_constant("FATTACHMENT_MODE_SNAP_TO_ORIGIN", math::to_integral(FAttachmentMode::SnapToOrigin));
	def.add_static_constant("FATTACHMENT_MODE_FORCE_TRANSLATION_IN_PLACE", math::to_integral(FAttachmentMode::ForceTranslationInPlace));
	def.add_static_constant("FATTACHMENT_MODE_FORCE_ROTATION_IN_PLACE", math::to_integral(FAttachmentMode::ForceRotationInPlace));
	def.add_static_constant("FATTACHMENT_MODE_FORCE_IN_PLACE", math::to_integral(FAttachmentMode::ForceInPlace));
}

namespace pragma {
	class BaseFuncLiquidComponent;
};

namespace Lua {
	DLLNETWORK bool get_bullet_master(pragma::ecs::BaseEntity &ent);
	DLLNETWORK pragma::AnimationEvent get_animation_event(lua::State *l, int32_t tArgs, uint32_t eventId);
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

template<typename TResult, typename TBoneIdentifier, bool (pragma::BaseAnimatedComponent::*GetValue)(pragma::animation::BoneId, TResult &, pragma::math::CoordinateSpace) const>
std::optional<TResult> get_bone_value(const pragma::BaseAnimatedComponent &animC, TBoneIdentifier boneIdentifier, pragma::math::CoordinateSpace space)
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
template<typename TResult, typename TBoneIdentifier, bool (pragma::BaseAnimatedComponent::*GetValue)(pragma::animation::BoneId, TResult &, pragma::math::CoordinateSpace) const>
std::optional<TResult> get_bone_value_os(const pragma::BaseAnimatedComponent &animC, TBoneIdentifier boneIdentifier)
{
	return get_bone_value<TResult, TBoneIdentifier, GetValue>(animC, boneIdentifier, pragma::math::CoordinateSpace::Object);
}
template<typename TResult, typename TBoneIdentifier, bool (pragma::BaseAnimatedComponent::*GetValue)(pragma::animation::BoneId, TResult &, pragma::math::CoordinateSpace) const>
std::optional<TResult> get_bone_value_ls(const pragma::BaseAnimatedComponent &animC, TBoneIdentifier boneIdentifier)
{
	return get_bone_value<TResult, TBoneIdentifier, GetValue>(animC, boneIdentifier, pragma::math::CoordinateSpace::Local);
}

template<typename TValue, typename TBoneIdentifier, bool (pragma::BaseAnimatedComponent::*SetValue)(pragma::animation::BoneId, const TValue &, pragma::math::CoordinateSpace)>
bool set_bone_value(pragma::BaseAnimatedComponent &animC, TBoneIdentifier boneIdentifier, const TValue &value, pragma::math::CoordinateSpace space)
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

template<typename TValue, typename TBoneIdentifier, bool (pragma::BaseAnimatedComponent::*SetValue)(pragma::animation::BoneId, const TValue &, pragma::math::CoordinateSpace)>
bool set_bone_value_ls(pragma::BaseAnimatedComponent &animC, TBoneIdentifier boneIdentifier, const TValue &value)
{
	return set_bone_value<TValue, TBoneIdentifier, SetValue>(animC, boneIdentifier, value, pragma::math::CoordinateSpace::Local);
}

template<typename TBoneId>
static void register_base_animated_component_bone_methods(luabind::class_<pragma::BaseAnimatedComponent, pragma::BaseEntityComponent> &classDef)
{
	// Note: luabind::default_parameter_policy would be a better choice here, but doesn't work for the CoordinateSpace parameter for some unknown reason
	classDef.def("GetReferenceBonePose", &get_bone_value_os<pragma::math::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBonePose>);
	classDef.def("GetReferenceBonePos", &get_bone_value_os<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBonePos>);
	classDef.def("GetReferenceBoneRot", &get_bone_value_os<Quat, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBoneRot>);
	classDef.def("GetReferenceBoneScale", &get_bone_value_os<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBoneScale>);

	classDef.def("GetReferenceBonePose", &get_bone_value<pragma::math::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBonePose>);
	classDef.def("GetReferenceBonePos", &get_bone_value<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBonePos>);
	classDef.def("GetReferenceBoneRot", &get_bone_value<Quat, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBoneRot>);
	classDef.def("GetReferenceBoneScale", &get_bone_value<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetReferenceBoneScale>);

	classDef.def("GetBonePose", &get_bone_value_ls<pragma::math::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::GetBonePose>);
	classDef.def("GetBonePos", &get_bone_value_ls<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetBonePos>);
	classDef.def("GetBoneRot", &get_bone_value_ls<Quat, TBoneId, &pragma::BaseAnimatedComponent::GetBoneRot>);
	classDef.def("GetBoneScale", &get_bone_value_ls<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetBoneScale>);

	classDef.def("GetBonePose", &get_bone_value<pragma::math::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::GetBonePose>);
	classDef.def("GetBonePos", &get_bone_value<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetBonePos>);
	classDef.def("GetBoneRot", &get_bone_value<Quat, TBoneId, &pragma::BaseAnimatedComponent::GetBoneRot>);
	classDef.def("GetBoneScale", &get_bone_value<Vector3, TBoneId, &pragma::BaseAnimatedComponent::GetBoneScale>);

	classDef.def("SetBonePose", &set_bone_value_ls<pragma::math::Transform, TBoneId, &pragma::BaseAnimatedComponent::SetBonePose>);
	classDef.def("SetBonePose", &set_bone_value_ls<pragma::math::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::SetBonePose>);
	classDef.def("SetBonePos", &set_bone_value_ls<Vector3, TBoneId, &pragma::BaseAnimatedComponent::SetBonePos>);
	classDef.def("SetBoneRot", &set_bone_value_ls<Quat, TBoneId, &pragma::BaseAnimatedComponent::SetBoneRot>);
	classDef.def("SetBoneScale", &set_bone_value_ls<Vector3, TBoneId, &pragma::BaseAnimatedComponent::SetBoneScale>);

	classDef.def("SetBonePose", &set_bone_value<pragma::math::Transform, TBoneId, &pragma::BaseAnimatedComponent::SetBonePose>);
	classDef.def("SetBonePose", &set_bone_value<pragma::math::ScaledTransform, TBoneId, &pragma::BaseAnimatedComponent::SetBonePose>);
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
	  "GetEffectiveBonePose", +[](const pragma::BaseAnimatedComponent &animC, TBoneId boneIdentifier) -> std::optional<pragma::math::ScaledTransform> {
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
	  "SetEffectiveBonePose", +[](pragma::BaseAnimatedComponent &animC, TBoneId boneIdentifier, const pragma::math::ScaledTransform &pose) -> bool {
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
void pragma::LuaCore::base_animated_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseAnimatedComponent>("BaseAnimatedComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("PlayAnimation", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, int, uint32_t)>([](lua::State *l, BaseAnimatedComponent &hAnim, int anim, uint32_t flags) { hAnim.PlayAnimation(anim, static_cast<FPlayAnim>(flags)); }));
	def.def("PlayAnimation",
	  static_cast<void (*)(lua::State *, BaseAnimatedComponent &, std::string, uint32_t)>([](lua::State *l, BaseAnimatedComponent &hAnim, std::string anim, uint32_t flags) { Lua::PushBool(l, hAnim.PlayAnimation(anim, static_cast<FPlayAnim>(flags))); }));
	def.def("PlayAnimation", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, int)>([](lua::State *l, BaseAnimatedComponent &hAnim, int anim) { hAnim.PlayAnimation(anim, FPlayAnim::Default); }));
	def.def("PlayAnimation", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, std::string)>([](lua::State *l, BaseAnimatedComponent &hAnim, std::string anim) { Lua::PushBool(l, hAnim.PlayAnimation(anim, FPlayAnim::Default)); }));
	def.def("GetAnimation", &BaseAnimatedComponent::GetAnimation);
	def.def("GetAnimationObject", &BaseAnimatedComponent::GetAnimationObject, luabind::shared_from_this_policy<0> {});
	def.def("PlayActivity",
	  static_cast<void (*)(lua::State *, BaseAnimatedComponent &, int, uint32_t)>([](lua::State *l, BaseAnimatedComponent &hAnim, int activity, uint32_t flags) { hAnim.PlayActivity(static_cast<Activity>(activity), static_cast<FPlayAnim>(flags)); }));
	def.def("PlayActivity", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, int)>([](lua::State *l, BaseAnimatedComponent &hAnim, int activity) { hAnim.PlayActivity(static_cast<Activity>(activity), FPlayAnim::Default); }));
	def.def("GetActivity", &BaseAnimatedComponent::GetActivity);
	def.def("ResetPose", &BaseAnimatedComponent::ResetPose);
	def.def("PlayLayeredAnimation", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, int, int)>([](lua::State *l, BaseAnimatedComponent &hAnim, int slot, int anim) { hAnim.PlayLayeredAnimation(slot, anim); }));
	def.def("PlayLayeredAnimation", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, int, std::string)>([](lua::State *l, BaseAnimatedComponent &hAnim, int slot, std::string anim) { hAnim.PlayLayeredAnimation(slot, anim); }));
	def.def("PlayLayeredActivity", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, int, int)>([](lua::State *l, BaseAnimatedComponent &hAnim, int slot, int activity) { hAnim.PlayLayeredActivity(slot, static_cast<Activity>(activity)); }));
	def.def("StopLayeredAnimation", &BaseAnimatedComponent::StopLayeredAnimation);
	def.def("StopLayeredAnimations", &BaseAnimatedComponent::StopLayeredAnimations);
	def.def("GetLayeredAnimation", &BaseAnimatedComponent::GetLayeredAnimation);
	def.def("GetLayeredActivity", &BaseAnimatedComponent::GetLayeredActivity);
	def.def("GetLayeredAnimations", static_cast<luabind::object (*)(lua::State *, BaseAnimatedComponent &)>([](lua::State *l, BaseAnimatedComponent &hAnim) -> luabind::object {
		auto t = luabind::newtable(l);
		for(auto &pair : hAnim.GetAnimationSlotInfos())
			t[pair.first] = pair.second.animation;
		return t;
	}));
	def.def("ApplyLayeredAnimations", &BaseAnimatedComponent::MaintainGestures);
	def.def("SetPlaybackRate", &BaseAnimatedComponent::SetPlaybackRate);
	def.def("GetPlaybackRate", &BaseAnimatedComponent::GetPlaybackRate);
	def.def("GetPlaybackRateProperty", &BaseAnimatedComponent::GetPlaybackRateProperty);
	def.def("GetEffectiveBonePoses", +[](BaseAnimatedComponent &animC) -> std::vector<math::ScaledTransform> { return animC.GetProcessedBones(); });

	def.def("GetBoneMatrix", &BaseAnimatedComponent::GetBoneMatrix);
	def.def(
	  "GetBoneMatrix", +[](const BaseAnimatedComponent &animC, const std::string &boneName) -> std::optional<Mat4> {
		  auto boneId = Lua::Animated::get_bone_id(animC, boneName);
		  if(boneId == -1)
			  return {};
		  return animC.GetBoneMatrix(boneId);
	  });

	register_base_animated_component_bone_methods<animation::BoneId>(def);
	register_base_animated_component_bone_methods<const std::string &>(def);
	def.def("UpdateEffectiveBoneTransforms", &BaseAnimatedComponent::UpdateSkeleton);
	def.def("AdvanceAnimations", &BaseAnimatedComponent::MaintainAnimations);
	def.def("ClearPreviousAnimation", &BaseAnimatedComponent::ClearPreviousAnimation);
	def.def("GetBindPose", &BaseAnimatedComponent::GetBindPose, luabind::shared_from_this_policy<0> {});
	def.def("SetBindPose", &BaseAnimatedComponent::SetBindPose);
	def.def("SetCycle", &BaseAnimatedComponent::SetCycle);
	def.def("GetCycle", &BaseAnimatedComponent::GetCycle);
	def.def("AddAnimationEvent",
	  static_cast<void (*)(lua::State *, BaseAnimatedComponent &, uint32_t, uint32_t, uint32_t, const luabind::object &)>([](lua::State *l, BaseAnimatedComponent &hEnt, uint32_t animId, uint32_t frameId, uint32_t eventId, const luabind::object &args) {
		  auto ev = Lua::get_animation_event(l, 5, eventId);
		  hEnt.AddAnimationEvent(animId, frameId, ev);
	  }));
	def.def("AddAnimationEvent", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, uint32_t, uint32_t, const luabind::object &)>([](lua::State *l, BaseAnimatedComponent &hEnt, uint32_t animId, uint32_t frameId, const luabind::object &f) {
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
	  static_cast<void (*)(lua::State *, BaseAnimatedComponent &, const std::string &, uint32_t, uint32_t, const luabind::object &)>([](lua::State *l, BaseAnimatedComponent &hEnt, const std::string &anim, uint32_t frameId, uint32_t eventId, const luabind::object &args) {
		  auto ev = Lua::get_animation_event(l, 5, eventId);
		  hEnt.AddAnimationEvent(anim, frameId, ev);
	  }));
	def.def("AddAnimationEvent", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, const std::string &, uint32_t, const luabind::object &)>([](lua::State *l, BaseAnimatedComponent &hEnt, const std::string &anim, uint32_t frameId, const luabind::object &f) {
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
	def.def("ClearAnimationEvents", static_cast<void (*)(lua::State *, BaseAnimatedComponent &)>([](lua::State *l, BaseAnimatedComponent &hEnt) { hEnt.ClearAnimationEvents(); }));
	def.def("ClearAnimationEvents", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, uint32_t)>([](lua::State *l, BaseAnimatedComponent &hEnt, uint32_t animId) { hEnt.ClearAnimationEvents(animId); }));
	def.def("ClearAnimationEvents", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, uint32_t, uint32_t)>([](lua::State *l, BaseAnimatedComponent &hEnt, uint32_t animId, uint32_t frameId) { hEnt.ClearAnimationEvents(animId, frameId); }));
	def.def("ClearAnimationEvents", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, const std::string &)>([](lua::State *l, BaseAnimatedComponent &hEnt, const std::string &anim) { hEnt.ClearAnimationEvents(anim); }));
	def.def("ClearAnimationEvents", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, const std::string &, uint32_t)>([](lua::State *l, BaseAnimatedComponent &hEnt, const std::string &anim, uint32_t frameId) { hEnt.ClearAnimationEvents(anim, frameId); }));

	def.def("InjectAnimationEvent", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, uint32_t)>([](lua::State *l, BaseAnimatedComponent &hEnt, uint32_t eventId) {
		AnimationEvent ev {};
		ev.eventID = static_cast<AnimationEvent::Type>(eventId);
		hEnt.InjectAnimationEvent(ev);
	}));
	def.def("InjectAnimationEvent", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, uint32_t, const luabind::object &)>([](lua::State *l, BaseAnimatedComponent &hEnt, uint32_t eventId, const luabind::object &args) {
		auto ev = Lua::get_animation_event(l, 3, eventId);
		hEnt.InjectAnimationEvent(ev);
	}));
	def.def("BindAnimationEvent", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, uint32_t, luabind::object)>([](lua::State *l, BaseAnimatedComponent &hEnt, uint32_t eventId, luabind::object function) {
		Lua::CheckFunction(l, 3);
		hEnt.BindAnimationEvent(static_cast<AnimationEvent::Type>(eventId), [l, function](std::reference_wrapper<const AnimationEvent> ev) {
			Lua::CallFunction(l, [&function, &ev](lua::State *l) -> Lua::StatusCode {
				function.push(l);
				CEHandleAnimationEvent evData {ev};
				evData.PushArgumentVariadic(l);
				return Lua::StatusCode::Ok;
			});
		});
	}));
	def.def("BindAnimationEvent",
	  static_cast<void (*)(lua::State *, BaseAnimatedComponent &, uint32_t, BaseEntityComponent &, const std::string &)>([](lua::State *l, BaseAnimatedComponent &hEnt, uint32_t eventId, BaseEntityComponent &component, const std::string &methodName) {
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
				    [o, &methodName, &ev](lua::State *l) -> Lua::StatusCode {
					    o.push(l);
					    Lua::PushString(l, methodName);
					    Lua::GetTableValue(l, -2);
					    Lua::RemoveValue(l, -2);

					    o.push(l);
					    CEHandleAnimationEvent evData {ev};
					    evData.PushArgumentVariadic(l);
					    return Lua::StatusCode::Ok;
				    },
				    0);
			  }
		  });
	  }));

	def.def("GetVertexTransformMatrix", static_cast<std::optional<Mat4> (BaseAnimatedComponent::*)(const geometry::ModelSubMesh &, uint32_t) const>(&BaseAnimatedComponent::GetVertexTransformMatrix));
	def.def(
	  "GetLocalVertexPosition", +[](lua::State *l, BaseAnimatedComponent &hEnt, geometry::ModelSubMesh &subMesh, uint32_t vertexId) -> std::optional<Vector3> {
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
	  "GetVertexPosition", +[](lua::State *l, BaseAnimatedComponent &hEnt, uint32_t meshGroupId, uint32_t meshId, uint32_t subMeshId, uint32_t vertexId) -> std::optional<Vector3> {
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
	  static_cast<std::optional<Vector3> (*)(lua::State *, BaseAnimatedComponent &, const std::shared_ptr<geometry::ModelSubMesh> &, uint32_t)>(
	    [](lua::State *l, BaseAnimatedComponent &hEnt, const std::shared_ptr<geometry::ModelSubMesh> &subMesh, uint32_t vertexId) -> std::optional<Vector3> {
		    Vector3 pos;
		    if(vertexId >= subMesh->GetVertexCount())
			    return {};
		    auto &v = subMesh->GetVertices()[vertexId];
		    pos = v.position;
		    if(hEnt.GetVertexPosition(*subMesh, vertexId, pos) == false)
			    return {};
		    return pos;
	    }));
	def.def("SetBlendController", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, unsigned int, float)>([](lua::State *l, BaseAnimatedComponent &hEnt, unsigned int controller, float val) { hEnt.SetBlendController(controller, val); }));
	def.def("SetBlendController", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, std::string, float)>([](lua::State *l, BaseAnimatedComponent &hEnt, std::string controller, float val) { hEnt.SetBlendController(controller, val); }));
	def.def("GetBlendController", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, std::string)>([](lua::State *l, BaseAnimatedComponent &hEnt, std::string controller) { Lua::PushNumber(l, hEnt.GetBlendController(controller)); }));
	def.def("GetBlendController", static_cast<void (*)(lua::State *, BaseAnimatedComponent &, unsigned int)>([](lua::State *l, BaseAnimatedComponent &hEnt, unsigned int controller) { Lua::PushNumber(l, hEnt.GetBlendController(controller)); }));
	def.def("GetBoneCount", &BaseAnimatedComponent::GetBoneCount);
	def.def("GetBaseAnimationFlags", &BaseAnimatedComponent::GetBaseAnimationFlags);
	def.def("SetBaseAnimationFlags", &BaseAnimatedComponent::SetBaseAnimationFlags);
	def.def("GetLayeredAnimationFlags", static_cast<luabind::object (*)(lua::State *, BaseAnimatedComponent &, uint32_t)>([](lua::State *l, BaseAnimatedComponent &hEnt, uint32_t layerIdx) -> luabind::object {
		auto flags = hEnt.GetLayeredAnimationFlags(layerIdx);
		if(flags.has_value() == false)
			return {};
		return luabind::object {l, *flags};
	}));
	def.def("SetLayeredAnimationFlags", &BaseAnimatedComponent::SetLayeredAnimationFlags);
	def.def("SetPostAnimationUpdateEnabled", &BaseAnimatedComponent::SetPostAnimationUpdateEnabled);
	def.def("IsPostAnimationUpdateEnabled", &BaseAnimatedComponent::IsPostAnimationUpdateEnabled);
	def.def("GetMetaBoneId", &BaseAnimatedComponent::GetMetaBoneId);
	def.def(
	  "GetMetaBonePose", +[](BaseAnimatedComponent &animC, animation::MetaRigBoneType boneType, math::CoordinateSpace space) -> std::optional<math::ScaledTransform> {
		  math::ScaledTransform pose;
		  if(!animC.GetMetaBonePose(boneType, pose, space))
			  return {};
		  return pose;
	  });
	def.def("SetMetaBonePose", static_cast<bool (BaseAnimatedComponent::*)(animation::MetaRigBoneType, const math::ScaledTransform &, math::CoordinateSpace)>(&BaseAnimatedComponent::SetMetaBonePose));
	def.def(
	  "MetaBonePoseToSkeletal", +[](BaseAnimatedComponent &animC, animation::MetaRigBoneType boneType, const math::ScaledTransform &pose) -> math::ScaledTransform {
		  auto newPose = pose;
		  animC.MetaBonePoseToSkeletal(boneType, newPose);
		  return newPose;
	  });
	def.def(
	  "MetaBoneRotationToSkeletal", +[](BaseAnimatedComponent &animC, animation::MetaRigBoneType boneType, const Quat &rot) -> Quat {
		  auto newRot = rot;
		  animC.MetaBoneRotationToSkeletal(boneType, newRot);
		  return newRot;
	  });

	def.add_static_constant("EVENT_HANDLE_ANIMATION_EVENT", baseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT);
	def.add_static_constant("EVENT_ON_PLAY_ANIMATION", baseAnimatedComponent::EVENT_ON_PLAY_ANIMATION);
	def.add_static_constant("EVENT_ON_PLAY_LAYERED_ANIMATION", baseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION);
	def.add_static_constant("EVENT_ON_PLAY_LAYERED_ACTIVITY", baseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ACTIVITY);
	def.add_static_constant("EVENT_ON_ANIMATION_COMPLETE", baseAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE);
	def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_START", baseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_START);
	def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_COMPLETE", baseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE);
	def.add_static_constant("EVENT_ON_ANIMATION_START", baseAnimatedComponent::EVENT_ON_ANIMATION_START);
	def.add_static_constant("EVENT_TRANSLATE_LAYERED_ANIMATION", baseAnimatedComponent::EVENT_TRANSLATE_LAYERED_ANIMATION);
	def.add_static_constant("EVENT_TRANSLATE_ANIMATION", baseAnimatedComponent::EVENT_TRANSLATE_ANIMATION);
	def.add_static_constant("EVENT_TRANSLATE_ACTIVITY", baseAnimatedComponent::EVENT_TRANSLATE_ACTIVITY);

	def.add_static_constant("EVENT_HANDLE_ANIMATION_EVENT", baseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT);
	def.add_static_constant("EVENT_ON_PLAY_ANIMATION", baseAnimatedComponent::EVENT_ON_PLAY_ANIMATION);
	def.add_static_constant("EVENT_ON_PLAY_LAYERED_ANIMATION", baseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION);
	def.add_static_constant("EVENT_ON_PLAY_LAYERED_ACTIVITY", baseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ACTIVITY);
	def.add_static_constant("EVENT_ON_ANIMATION_COMPLETE", baseAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE);
	def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_START", baseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_START);
	def.add_static_constant("EVENT_ON_LAYERED_ANIMATION_COMPLETE", baseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE);
	def.add_static_constant("EVENT_ON_ANIMATION_START", baseAnimatedComponent::EVENT_ON_ANIMATION_START);
	def.add_static_constant("EVENT_TRANSLATE_LAYERED_ANIMATION", baseAnimatedComponent::EVENT_TRANSLATE_LAYERED_ANIMATION);
	def.add_static_constant("EVENT_TRANSLATE_ANIMATION", baseAnimatedComponent::EVENT_TRANSLATE_ANIMATION);
	def.add_static_constant("EVENT_TRANSLATE_ACTIVITY", baseAnimatedComponent::EVENT_TRANSLATE_ACTIVITY);
	def.add_static_constant("EVENT_MAINTAIN_ANIMATIONS", baseAnimatedComponent::EVENT_MAINTAIN_ANIMATIONS);
	def.add_static_constant("EVENT_MAINTAIN_ANIMATION_MOVEMENT", baseAnimatedComponent::EVENT_MAINTAIN_ANIMATION_MOVEMENT);
	def.add_static_constant("EVENT_SHOULD_UPDATE_BONES", baseAnimatedComponent::EVENT_SHOULD_UPDATE_BONES);

	def.add_static_constant("EVENT_ON_PLAY_ACTIVITY", baseAnimatedComponent::EVENT_ON_PLAY_ACTIVITY);
	def.add_static_constant("EVENT_ON_STOP_LAYERED_ANIMATION", baseAnimatedComponent::EVENT_ON_STOP_LAYERED_ANIMATION);
	def.add_static_constant("EVENT_ON_BONE_TRANSFORM_CHANGED", baseAnimatedComponent::EVENT_ON_BONE_TRANSFORM_CHANGED);
	def.add_static_constant("EVENT_ON_ANIMATIONS_UPDATED", baseAnimatedComponent::EVENT_ON_ANIMATIONS_UPDATED);
	def.add_static_constant("EVENT_PLAY_ANIMATION", baseAnimatedComponent::EVENT_PLAY_ANIMATION);
	def.add_static_constant("EVENT_ON_ANIMATION_RESET", baseAnimatedComponent::EVENT_ON_ANIMATION_RESET);
	def.add_static_constant("EVENT_ON_UPDATE_SKELETON", baseAnimatedComponent::EVENT_ON_UPDATE_SKELETON);
	def.add_static_constant("EVENT_POST_ANIMATION_UPDATE", baseAnimatedComponent::EVENT_POST_ANIMATION_UPDATE);
	def.add_static_constant("EVENT_ON_RESET_POSE", baseAnimatedComponent::EVENT_ON_RESET_POSE);

	def.add_static_constant("FPLAYANIM_NONE", math::to_integral(FPlayAnim::None));
	def.add_static_constant("FPLAYANIM_RESET", math::to_integral(FPlayAnim::Reset));
	def.add_static_constant("FPLAYANIM_TRANSMIT", math::to_integral(FPlayAnim::Transmit));
	def.add_static_constant("FPLAYANIM_SNAP_TO", math::to_integral(FPlayAnim::SnapTo));
	def.add_static_constant("FPLAYANIM_DEFAULT", math::to_integral(FPlayAnim::Default));
	def.add_static_constant("FPLAYANIM_LOOP", math::to_integral(FPlayAnim::Loop));
}
namespace Lua::FuncWater {
	void CalcLineSurfaceIntersection(lua::State *l, pragma::BaseFuncLiquidComponent &hEnt, const Vector3 &lineOrigin, const Vector3 &lineDir, bool bCull)
	{
		double t, u, v;
		auto r = hEnt.CalcLineSurfaceIntersection(lineOrigin, lineDir, &t, &u, &v, bCull);
		PushBool(l, r);
		PushNumber(l, t);
		PushNumber(l, u);
		PushNumber(l, v);
	}
};
void pragma::LuaCore::base_func_water_component::register_class(luabind::module_ &mod)
{
	{
		auto def = Lua::create_base_entity_component_class<BaseLiquidSurfaceComponent>("BaseLiquidSurfaceComponent");
		util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	}
	{
		auto def = Lua::create_base_entity_component_class<BaseLiquidVolumeComponent>("BaseLiquidVolumeComponent");
		util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	}
	{
		auto def = Lua::create_base_entity_component_class<BaseBuoyancyComponent>("BaseBuoyancyComponent");
		util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	}
	auto def = Lua::create_base_entity_component_class<BaseFuncLiquidComponent>("BaseFuncLiquidComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("CalcLineSurfaceIntersection", static_cast<void (*)(lua::State *, BaseFuncLiquidComponent &, const Vector3 &, const Vector3 &)>([](lua::State *l, BaseFuncLiquidComponent &hEnt, const Vector3 &lineOrigin, const Vector3 &lineDir) {
		Lua::FuncWater::CalcLineSurfaceIntersection(l, hEnt, lineOrigin, lineDir, false);
	}));
	def.def("CalcLineSurfaceIntersection", &BaseFuncLiquidComponent::CalcLineSurfaceIntersection);
}
void pragma::LuaCore::base_toggle_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseToggleComponent>("BaseToggleComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("TurnOn", &BaseToggleComponent::TurnOn);
	def.def("TurnOff", &BaseToggleComponent::TurnOff);
	def.def("Toggle", &BaseToggleComponent::Toggle);
	def.def("IsTurnedOn", &BaseToggleComponent::IsTurnedOn);
	def.def("IsTurnedOff", static_cast<bool (*)(lua::State *, BaseToggleComponent &)>([](lua::State *l, BaseToggleComponent &hEnt) { return !hEnt.IsTurnedOn(); }));
	def.def("SetTurnedOn", &BaseToggleComponent::SetTurnedOn);
	def.def("GetTurnedOnProperty", &BaseToggleComponent::GetTurnedOnProperty);
	def.add_static_constant("EVENT_ON_TURN_ON", baseToggleComponent::EVENT_ON_TURN_ON);
	def.add_static_constant("EVENT_ON_TURN_OFF", baseToggleComponent::EVENT_ON_TURN_OFF);

	def.add_static_constant("SPAWN_FLAG_START_ON_BIT", math::to_integral(BaseToggleComponent::SpawnFlags::StartOn));
}
void pragma::LuaCore::base_wheel_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseWheelComponent>("BaseWheelComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	// TODO
}
void pragma::LuaCore::base_decal_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvDecalComponent>("BaseEnvDecalComponent");
	def.def("SetSize", &BaseEnvDecalComponent::SetSize);
	def.def("GetSize", &BaseEnvDecalComponent::GetSize);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}
void pragma::LuaCore::base_env_light_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvLightComponent>("BaseEnvLightComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetFalloffExponent", &BaseEnvLightComponent::GetFalloffExponent);
	def.def("SetFalloffExponent", &BaseEnvLightComponent::SetFalloffExponent);
	def.def("IsBaked", &BaseEnvLightComponent::IsBaked);
	def.def("SetBaked", &BaseEnvLightComponent::SetBaked);
	def.def("SetLightIntensityType", &BaseEnvLightComponent::SetLightIntensityType);
	def.def("GetLightIntensityType", &BaseEnvLightComponent::GetLightIntensityType);
	def.def(
	  "SetLightIntensityType", +[](BaseEnvLightComponent &c, const std::string &type) {
		  auto e = magic_enum::enum_cast<BaseEnvLightComponent::LightIntensityType>(type);
		  if(e.has_value())
			  c.SetLightIntensityType(*e);
	  });
	def.def("SetLightIntensity", static_cast<void (BaseEnvLightComponent::*)(float)>(&BaseEnvLightComponent::SetLightIntensity));
	def.def("SetLightIntensity", static_cast<void (BaseEnvLightComponent::*)(float, BaseEnvLightComponent::LightIntensityType)>(&BaseEnvLightComponent::SetLightIntensity));
	def.def(
	  "SetLightIntensity", +[](BaseEnvLightComponent &c, float intensity, const std::string &type) {
		  auto e = magic_enum::enum_cast<BaseEnvLightComponent::LightIntensityType>(type);
		  if(e.has_value())
			  c.SetLightIntensity(intensity, *e);
	  });
	def.def("GetLightIntensity", &BaseEnvLightComponent::GetLightIntensity);
	def.def("GetLightIntensityCandela", static_cast<Candela (BaseEnvLightComponent::*)() const>(&BaseEnvLightComponent::GetLightIntensityCandela));
	def.def("CalcLightIntensityAtPoint", &BaseEnvLightComponent::CalcLightIntensityAtPoint);
	def.def("CalcLightDirectionToPoint", &BaseEnvLightComponent::CalcLightDirectionToPoint);
	def.add_static_constant("INTENSITY_TYPE_CANDELA", math::to_integral(BaseEnvLightComponent::LightIntensityType::Candela));
	def.add_static_constant("INTENSITY_TYPE_LUMEN", math::to_integral(BaseEnvLightComponent::LightIntensityType::Lumen));
	def.add_static_constant("INTENSITY_TYPE_LUX", math::to_integral(BaseEnvLightComponent::LightIntensityType::Lux));

	def.add_static_constant("LIGHT_FLAG_NONE", math::to_integral(BaseEnvLightComponent::LightFlags::None));
	def.add_static_constant("LIGHT_FLAG_BAKED_LIGHT_SOURCE_BIT", math::to_integral(BaseEnvLightComponent::LightFlags::BakedLightSource));
}
void pragma::LuaCore::base_env_light_spot_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvLightSpotComponent>("BaseEnvLightSpotComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetOuterConeAngle", &BaseEnvLightSpotComponent::SetOuterConeAngle);
	def.def("GetOuterConeAngle", &BaseEnvLightSpotComponent::GetOuterConeAngle);
	def.def("SetInnerConeAngle", &BaseEnvLightSpotComponent::SetInnerConeAngle);
	def.def("GetInnerConeAngle", &BaseEnvLightSpotComponent::GetInnerConeAngle);
	def.def("GetBlendFractionProperty", &BaseEnvLightSpotComponent::GetBlendFractionProperty);
	def.def("GetOuterConeAngleProperty", &BaseEnvLightSpotComponent::GetOuterConeAngleProperty);
	def.def("GetConeStartOffsetProperty", &BaseEnvLightSpotComponent::GetConeStartOffsetProperty);
	def.def("SetBlendFraction", &BaseEnvLightSpotComponent::SetBlendFraction);
	def.def("GetBlendFraction", &BaseEnvLightSpotComponent::GetBlendFraction);
	def.def("CalcConeFalloff", static_cast<float (BaseEnvLightSpotComponent::*)(const Vector3 &) const>(&BaseEnvLightSpotComponent::CalcConeFalloff));
	def.def("CalcDistanceFalloff", &BaseEnvLightSpotComponent::CalcDistanceFalloff);
}
void pragma::LuaCore::base_env_light_point_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvLightPointComponent>("BaseEnvLightPointComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("CalcDistanceFalloff", &BaseEnvLightPointComponent::CalcDistanceFalloff);
}
void pragma::LuaCore::base_env_light_directional_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvLightDirectionalComponent>("BaseEnvLightDirectionalComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetAmbientColor", &BaseEnvLightDirectionalComponent::GetAmbientColor, luabind::copy_policy<0> {});
	def.def("GetAmbientColorProperty", &BaseEnvLightDirectionalComponent::GetAmbientColorProperty);
	def.def("SetAmbientColor", &BaseEnvLightDirectionalComponent::SetAmbientColor);
}
void pragma::LuaCore::base_env_particle_system_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvParticleSystemComponent>("BaseEnvParticleSystemComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}
void pragma::LuaCore::base_flammable_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFlammableComponent>("BaseFlammableComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("Ignite",
	  static_cast<void (*)(lua::State *, BaseFlammableComponent &, float, ecs::BaseEntity &, ecs::BaseEntity &)>(
	    [](lua::State *l, BaseFlammableComponent &hEnt, float duration, ecs::BaseEntity &attacker, ecs::BaseEntity &inflictor) { hEnt.Ignite(duration, &attacker, &inflictor); }));
	def.def("Ignite", static_cast<void (*)(lua::State *, BaseFlammableComponent &, float, ecs::BaseEntity &)>([](lua::State *l, BaseFlammableComponent &hEnt, float duration, ecs::BaseEntity &attacker) { hEnt.Ignite(duration, &attacker); }));
	def.def("Ignite", static_cast<void (*)(lua::State *, BaseFlammableComponent &, float)>([](lua::State *l, BaseFlammableComponent &hEnt, float duration) { hEnt.Ignite(duration); }));
	def.def("Ignite", static_cast<void (*)(lua::State *, BaseFlammableComponent &)>([](lua::State *l, BaseFlammableComponent &hEnt) { hEnt.Ignite(0.f); }));
	def.def("IsOnFire", static_cast<void (*)(lua::State *, BaseFlammableComponent &)>([](lua::State *l, BaseFlammableComponent &hEnt) { Lua::PushBool(l, hEnt.IsOnFire()); }));
	def.def("IsIgnitable", static_cast<void (*)(lua::State *, BaseFlammableComponent &)>([](lua::State *l, BaseFlammableComponent &hEnt) { Lua::PushBool(l, hEnt.IsIgnitable()); }));
	def.def("Extinguish", static_cast<void (*)(lua::State *, BaseFlammableComponent &)>([](lua::State *l, BaseFlammableComponent &hEnt) { hEnt.Extinguish(); }));
	def.def("SetIgnitable", static_cast<void (*)(lua::State *, BaseFlammableComponent &, bool)>([](lua::State *l, BaseFlammableComponent &hEnt, bool b) { hEnt.SetIgnitable(b); }));
	def.def("GetOnFireProperty", &BaseFlammableComponent::GetOnFireProperty);
	def.def("GetIgnitableProperty", &BaseFlammableComponent::GetIgnitableProperty);
	def.add_static_constant("EVENT_ON_IGNITED", baseFlammableComponent::EVENT_ON_IGNITED);
	def.add_static_constant("EVENT_ON_EXTINGUISHED", baseFlammableComponent::EVENT_ON_EXTINGUISHED);
}

void pragma::LuaCore::base_health_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseHealthComponent>("BaseHealthComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetHealth", static_cast<void (*)(lua::State *, BaseHealthComponent &)>([](lua::State *l, BaseHealthComponent &hEnt) {
		unsigned short health = hEnt.GetHealth();
		Lua::PushInt(l, health);
	}));
	def.def("SetHealth", static_cast<void (*)(lua::State *, BaseHealthComponent &, unsigned short)>([](lua::State *l, BaseHealthComponent &hEnt, unsigned short health) { hEnt.SetHealth(health); }));
	def.def("SetMaxHealth", static_cast<void (*)(lua::State *, BaseHealthComponent &, uint16_t)>([](lua::State *l, BaseHealthComponent &hEnt, uint16_t maxHealth) { hEnt.SetMaxHealth(maxHealth); }));
	def.def("GetMaxHealth", static_cast<void (*)(lua::State *, BaseHealthComponent &)>([](lua::State *l, BaseHealthComponent &hEnt) { Lua::PushInt(l, hEnt.GetMaxHealth()); }));
	def.def("GetHealthProperty", &BaseHealthComponent::GetHealthProperty);
	def.def("GetMaxHealthProperty", &BaseHealthComponent::GetMaxHealthProperty);
	def.add_static_constant("EVENT_ON_TAKEN_DAMAGE", baseHealthComponent::EVENT_ON_TAKEN_DAMAGE);
	def.add_static_constant("EVENT_ON_HEALTH_CHANGED", baseHealthComponent::EVENT_ON_HEALTH_CHANGED);
}

void pragma::LuaCore::base_name_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseNameComponent>("BaseNameComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetName", static_cast<void (*)(lua::State *, BaseNameComponent &, std::string)>([](lua::State *l, BaseNameComponent &hEnt, std::string name) { hEnt.SetName(name); }));
	def.def("GetName", static_cast<void (*)(lua::State *, BaseNameComponent &)>([](lua::State *l, BaseNameComponent &hEnt) { Lua::PushString(l, hEnt.GetName()); }));
	def.def("GetNameProperty", &BaseNameComponent::GetNameProperty);
}

void pragma::LuaCore::base_networked_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseNetworkedComponent>("BaseNetworkedComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
#if NETWORKED_VARS_ENABLED != 0
	def.def("GetNetVarProperty", static_cast<void (*)(lua::State *, THandle &, uint32_t)>([](lua::State *l, THandle &hEnt, uint32_t id) {
		auto &prop = hEnt.GetNetworkedVariableProperty(id);
		if(prop == nullptr)
			return;
		Lua::Property::push(l, *prop);
	}));
	def.def("CreateNetVar", static_cast<void (*)(lua::State *, THandle &, const std::string &, uint32_t)>([](lua::State *l, THandle &hEnt, const std::string &name, uint32_t type) {
		auto id = hEnt.CreateNetworkedVariable(name, static_cast<pragma::BaseNetworkedComponent::NetworkedVariable::Type>(type));
		Lua::PushInt(l, id);
	}));
	def.def("SetNetVarValue", static_cast<void (*)(lua::State *, THandle &, uint32_t, luabind::object)>([](lua::State *l, THandle &hEnt, uint32_t id, luabind::object value) {
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
				auto &v = Lua::Check<::Color>(l, 3);
				hEnt.SetNetworkedVariable<Color>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector:
			{
				auto &v = Lua::Check<Vector3>(l, 3);
				hEnt.SetNetworkedVariable<Vector3>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector2:
			{
				auto &v = Lua::Check<Vector2>(l, 3);
				hEnt.SetNetworkedVariable<Vector2>(id, v);
				return;
			}
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector4:
			{
				auto &v = Lua::Check<Vector3>(l, 3);
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
				auto &v = *Lua::Check<Quat>(l, 3);
				hEnt.SetNetworkedVariable<Quat>(id, v);
				return;
			}
		}
	}));
	def.def("GetNetVarValue", static_cast<void (*)(lua::State *, THandle &, uint32_t)>([](lua::State *l, THandle &hEnt, uint32_t id) {
		auto type = hEnt.GetNetworkedVariableType(id);
		auto &prop = hEnt.GetNetworkedVariableProperty(id);
		if(type == pragma::BaseNetworkedComponent::NetworkedVariable::Type::Invalid || prop == nullptr)
			return;
		switch(type) {
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Bool:
			Lua::PushBool(l, static_cast<pragma::util::BoolProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Double:
			Lua::PushNumber(l, static_cast<pragma::util::DoubleProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Float:
			Lua::PushNumber(l, static_cast<pragma::util::FloatProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int8:
			Lua::PushInt(l, static_cast<pragma::util::Int8Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int16:
			Lua::PushInt(l, static_cast<pragma::util::Int16Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int32:
			Lua::PushInt(l, static_cast<pragma::util::Int32Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int64:
			Lua::PushInt(l, static_cast<pragma::util::Int64Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::LongDouble:
			Lua::PushNumber(l, static_cast<pragma::util::LongDoubleProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::String:
			Lua::PushString(l, static_cast<pragma::util::StringProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt8:
			Lua::PushInt(l, static_cast<pragma::util::UInt8Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt16:
			Lua::PushInt(l, static_cast<pragma::util::UInt16Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt32:
			Lua::PushInt(l, static_cast<pragma::util::UInt32Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt64:
			Lua::PushInt(l, static_cast<pragma::util::UInt64Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::EulerAngles:
			Lua::Push<EulerAngles>(l, static_cast<pragma::util::EulerAnglesProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Color:
			Lua::Push<Color>(l, static_cast<pragma::util::ColorProperty &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector:
			Lua::Push<Vector3>(l, static_cast<pragma::util::Vector3Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector2:
			Lua::Push<Vector2>(l, static_cast<pragma::util::Vector2Property &>(*prop).GetValue());
			return;
		case pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector4:
			Lua::Push<Vector4>(l, static_cast<pragma::util::Vector4Property &>(*prop).GetValue());
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
			Lua::Push<Quat>(l, static_cast<pragma::util::QuatProperty &>(*prop).GetValue());
			return;
		}
	}));
	def.add_static_constant("NET_VAR_TYPE_BOOL", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Bool));
	def.add_static_constant("NET_VAR_TYPE_DOUBLE", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Double));
	def.add_static_constant("NET_VAR_TYPE_FLOAT", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Float));
	def.add_static_constant("NET_VAR_TYPE_INT8", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int8));
	def.add_static_constant("NET_VAR_TYPE_INT16", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int16));
	def.add_static_constant("NET_VAR_TYPE_INT32", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int32));
	def.add_static_constant("NET_VAR_TYPE_INT64", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Int64));
	def.add_static_constant("NET_VAR_TYPE_LONG_DOUBLE", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::LongDouble));
	def.add_static_constant("NET_VAR_TYPE_STRING", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::String));
	def.add_static_constant("NET_VAR_TYPE_UINT8", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt8));
	def.add_static_constant("NET_VAR_TYPE_UINT16", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt16));
	def.add_static_constant("NET_VAR_TYPE_UINT32", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt32));
	def.add_static_constant("NET_VAR_TYPE_UINT64", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::UInt64));
	def.add_static_constant("NET_VAR_TYPE_EULER_ANGLES", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::EulerAngles));
	def.add_static_constant("NET_VAR_TYPE_COLOR", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Color));
	def.add_static_constant("NET_VAR_TYPE_VECTOR", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector));
	def.add_static_constant("NET_VAR_TYPE_VECTOR2", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector2));
	def.add_static_constant("NET_VAR_TYPE_VECTOR4", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Vector4));
	def.add_static_constant("NET_VAR_TYPE_ENTITY", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Entity));
	def.add_static_constant("NET_VAR_TYPE_QUATERNION", pragma::math::to_integral(pragma::BaseNetworkedComponent::NetworkedVariable::Type::Quaternion));
#endif
}

void pragma::LuaCore::base_observable_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseObservableComponent>("BaseObservableComponent");
	def.add_static_constant("EVENT_ON_OBSERVER_CHANGED", baseObservableComponent::EVENT_ON_OBSERVER_CHANGED);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetLocalCameraOrigin", static_cast<void (*)(lua::State *, BaseObservableComponent &, uint32_t, const Vector3 &)>([](lua::State *l, BaseObservableComponent &hEnt, uint32_t camType, const Vector3 &origin) {
		hEnt.SetLocalCameraOrigin(static_cast<BaseObservableComponent::CameraType>(camType), origin);
	}));
	def.def("GetLocalCameraOrigin",
	  static_cast<void (*)(lua::State *, BaseObservableComponent &, uint32_t)>([](lua::State *l, BaseObservableComponent &hEnt, uint32_t camType) { Lua::Push<Vector3>(l, hEnt.GetLocalCameraOrigin(static_cast<BaseObservableComponent::CameraType>(camType))); }));
	def.def("SetLocalCameraOffset", static_cast<void (*)(lua::State *, BaseObservableComponent &, uint32_t, const Vector3 &)>([](lua::State *l, BaseObservableComponent &hEnt, uint32_t camType, const Vector3 &offset) {
		hEnt.SetLocalCameraOffset(static_cast<BaseObservableComponent::CameraType>(camType), offset);
	}));
	def.def("GetLocalCameraOffset",
	  static_cast<void (*)(lua::State *, BaseObservableComponent &, uint32_t)>([](lua::State *l, BaseObservableComponent &hEnt, uint32_t camType) { Lua::Push<Vector3>(l, hEnt.GetLocalCameraOffset(static_cast<BaseObservableComponent::CameraType>(camType))); }));
	def.def("GetCameraData", static_cast<void (*)(lua::State *, BaseObservableComponent &, uint32_t)>([](lua::State *l, BaseObservableComponent &hEnt, uint32_t camType) {
		Lua::Push<ObserverCameraData *>(l, &hEnt.GetCameraData(static_cast<BaseObservableComponent::CameraType>(camType)));
	}));
	def.def("SetCameraEnabled",
	  static_cast<void (*)(lua::State *, BaseObservableComponent &, uint32_t, bool)>([](lua::State *l, BaseObservableComponent &hEnt, uint32_t camType, bool enabled) { hEnt.SetCameraEnabled(static_cast<BaseObservableComponent::CameraType>(camType), enabled); }));
	def.def("GetCameraEnabledProperty", &BaseObservableComponent::GetCameraEnabledProperty);
	def.def("GetCameraOffsetProperty", &BaseObservableComponent::GetCameraOffsetProperty);
	def.def(
	  "GetObserver", +[](BaseObservableComponent &observableC) -> luabind::object {
		  auto *observerC = observableC.GetObserver();
		  if(!observerC)
			  return Lua::nil;
		  return observerC->GetLuaObject();
	  });
	def.def("GetViewOffset", &BaseObservableComponent::GetViewOffset);
	def.def("SetViewOffset", &BaseObservableComponent::SetViewOffset);
	def.add_static_constant("CAMERA_TYPE_FIRST_PERSON", math::to_integral(BaseObservableComponent::CameraType::FirstPerson));
	def.add_static_constant("CAMERA_TYPE_THIRD_PERSON", math::to_integral(BaseObservableComponent::CameraType::ThirdPerson));

	auto defObsCamData = luabind::class_<ObserverCameraData>("CameraData");
	defObsCamData.def_readwrite("rotateWithObservee", &ObserverCameraData::rotateWithObservee);
	defObsCamData.def("SetAngleLimits",
	  static_cast<void (*)(lua::State *, ObserverCameraData &, const EulerAngles &, const EulerAngles &)>([](lua::State *l, ObserverCameraData &obsCamData, const EulerAngles &min, const EulerAngles &max) { obsCamData.angleLimits = {min, max}; }));
	defObsCamData.def("GetAngleLimits", static_cast<void (*)(lua::State *, ObserverCameraData &)>([](lua::State *l, ObserverCameraData &obsCamData) {
		if(obsCamData.angleLimits.has_value() == false)
			return;
		Lua::Push<EulerAngles>(l, obsCamData.angleLimits->first);
		Lua::Push<EulerAngles>(l, obsCamData.angleLimits->second);
	}));
	defObsCamData.def("ClearAngleLimits", static_cast<void (*)(lua::State *, ObserverCameraData &)>([](lua::State *l, ObserverCameraData &obsCamData) { obsCamData.angleLimits = {}; }));
	defObsCamData.property("enabled", static_cast<void (*)(lua::State *, ObserverCameraData &)>([](lua::State *l, ObserverCameraData &obsCamData) { Lua::PushBool(l, *obsCamData.enabled); }),
	  static_cast<void (*)(lua::State *, ObserverCameraData &, bool)>([](lua::State *l, ObserverCameraData &obsCamData, bool enabled) { *obsCamData.enabled = enabled; }));
	defObsCamData.property("localOrigin", static_cast<void (*)(lua::State *, ObserverCameraData &)>([](lua::State *l, ObserverCameraData &obsCamData) {
		if(obsCamData.localOrigin.has_value() == false)
			return;
		Lua::Push<Vector3>(l, *obsCamData.localOrigin);
	}),
	  static_cast<void (*)(lua::State *, ObserverCameraData &, const Vector3 &)>([](lua::State *l, ObserverCameraData &obsCamData, const Vector3 &origin) { obsCamData.localOrigin = origin; }));
	defObsCamData.property("offset", static_cast<void (*)(lua::State *, ObserverCameraData &)>([](lua::State *l, ObserverCameraData &obsCamData) { Lua::Push<Vector3>(l, *obsCamData.offset); }),
	  static_cast<void (*)(lua::State *, ObserverCameraData &, const Vector3 &)>([](lua::State *l, ObserverCameraData &obsCamData, const Vector3 &offset) { *obsCamData.offset = offset; }));
	def.scope[defObsCamData];
}

namespace Lua::Shooter {
	void FireBullets(lua::State *l, pragma::ecs::BaseShooterComponent &hEnt, const luabind::object &, bool bHitReport, bool bMaster)
	{
		auto &bulletInfo = Lua::Check<pragma::game::BulletInfo>(l, 2);

		std::vector<pragma::physics::TraceResult> results;
		hEnt.FireBullets(bulletInfo, results, bMaster);
		if(bHitReport == false)
			return;
		auto t = luabind::newtable(l);
		for(auto i = decltype(results.size()) {0}; i < results.size(); ++i) {
			auto &r = results[i];
			t[i + 1] = r;
		}
	}
};
void pragma::LuaCore::base_shooter_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<ecs::BaseShooterComponent>("BaseShooterComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("FireBullets", static_cast<void (*)(lua::State *, ecs::BaseShooterComponent &, const luabind::object &, bool, bool)>([](lua::State *l, ecs::BaseShooterComponent &hEnt, const luabind::object &o, bool bHitReport, bool bMaster) {
		Lua::Shooter::FireBullets(l, hEnt, o, bHitReport, bMaster);
	}));
	def.def("FireBullets", static_cast<void (*)(lua::State *, ecs::BaseShooterComponent &, const luabind::object &, bool)>([](lua::State *l, ecs::BaseShooterComponent &hEnt, const luabind::object &o, bool bHitReport) {
		Lua::Shooter::FireBullets(l, hEnt, o, bHitReport, Lua::get_bullet_master(hEnt.GetEntity()));
	}));
	def.def("FireBullets",
	  static_cast<void (*)(lua::State *, ecs::BaseShooterComponent &, const luabind::object &)>([](lua::State *l, ecs::BaseShooterComponent &hEnt, const luabind::object &o) { Lua::Shooter::FireBullets(l, hEnt, o, false, Lua::get_bullet_master(hEnt.GetEntity())); }));
	def.add_static_constant("EVENT_ON_FIRE_BULLETS", ecs::baseShooterComponent::EVENT_ON_FIRE_BULLETS);
}

namespace Lua::Physics {
	void InitializePhysics(lua::State *l, pragma::BasePhysicsComponent &hEnt, pragma::util::TSharedHandle<pragma::physics::IConvexShape> &shape, uint32_t flags)
	{
		auto *phys = hEnt.InitializePhysics(*shape, static_cast<pragma::BasePhysicsComponent::PhysFlags>(flags));
		if(phys != nullptr)
			luabind::object(l, phys->GetHandle()).push(l);
	}
	void InitializePhysics(lua::State *l, pragma::BasePhysicsComponent &hEnt, pragma::util::TSharedHandle<pragma::physics::IConvexShape> &shape) { InitializePhysics(l, hEnt, shape, pragma::math::to_integral(pragma::BasePhysicsComponent::PhysFlags::None)); }
};
void pragma::LuaCore::base_physics_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePhysicsComponent>("BasePhysicsComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetCollisionBounds", static_cast<void (*)(lua::State *, BasePhysicsComponent &, Vector3, Vector3)>([](lua::State *l, BasePhysicsComponent &hEnt, Vector3 min, Vector3 max) { hEnt.SetCollisionBounds(min, max); }));
	def.def("GetCollisionBounds", static_cast<void (*)(lua::State *, BasePhysicsComponent &)>([](lua::State *l, BasePhysicsComponent &hEnt) {
		Vector3 min, max;
		hEnt.GetCollisionBounds(&min, &max);
		luabind::object(l, min).push(l);
		luabind::object(l, max).push(l);
	}));
	def.def("GetCollisionExtents", static_cast<void (*)(lua::State *, BasePhysicsComponent &)>([](lua::State *l, BasePhysicsComponent &hEnt) { luabind::object(l, hEnt.GetCollisionExtents()).push(l); }));
	def.def("GetCollisionCenter", static_cast<void (*)(lua::State *, BasePhysicsComponent &)>([](lua::State *l, BasePhysicsComponent &hEnt) { luabind::object(l, hEnt.GetCollisionCenter()).push(l); }));
	def.def("GetMoveType", static_cast<void (*)(lua::State *, BasePhysicsComponent &)>([](lua::State *l, BasePhysicsComponent &hEnt) {
		physics::MoveType mt = hEnt.GetMoveType();
		Lua::PushInt(l, int(mt));
	}));
	def.def("SetMoveType", static_cast<void (*)(lua::State *, BasePhysicsComponent &, int)>([](lua::State *l, BasePhysicsComponent &hEnt, int moveType) { hEnt.SetMoveType(physics::MoveType(moveType)); }));
	def.def("GetPhysicsObject", static_cast<void (*)(lua::State *, BasePhysicsComponent &)>([](lua::State *l, BasePhysicsComponent &hEnt) {
		physics::PhysObj *phys = hEnt.GetPhysicsObject();
		if(phys == nullptr)
			return;
		luabind::object(l, phys->GetHandle()).push(l);
	}));
	def.def("InitializePhysics", static_cast<void (*)(lua::State *, BasePhysicsComponent &, uint32_t, uint32_t)>([](lua::State *l, BasePhysicsComponent &hEnt, uint32_t type, uint32_t physFlags) {
		physics::PhysObj *phys = hEnt.InitializePhysics(physics::PhysicsType(type), static_cast<BasePhysicsComponent::PhysFlags>(physFlags));
		if(phys != nullptr)
			luabind::object(l, phys->GetHandle()).push(l);
	}));
	def.def("InitializePhysics", static_cast<void (*)(lua::State *, BasePhysicsComponent &, uint32_t)>([](lua::State *l, BasePhysicsComponent &hEnt, uint32_t type) {
		physics::PhysObj *phys = hEnt.InitializePhysics(physics::PhysicsType(type));
		if(phys != nullptr)
			luabind::object(l, phys->GetHandle()).push(l);
	}));
	def.def("InitializePhysics",
	  static_cast<void (*)(lua::State *, BasePhysicsComponent &, util::TSharedHandle<physics::IConvexShape> &, uint32_t)>(
	    [](lua::State *l, BasePhysicsComponent &hEnt, util::TSharedHandle<physics::IConvexShape> &shape, uint32_t physFlags) { Lua::Physics::InitializePhysics(l, hEnt, shape, physFlags); }));
	def.def("InitializePhysics",
	  static_cast<void (*)(lua::State *, BasePhysicsComponent &, util::TSharedHandle<physics::IConvexShape> &, uint32_t)>(
	    [](lua::State *l, BasePhysicsComponent &hEnt, util::TSharedHandle<physics::IConvexShape> &shape, uint32_t physFlags) { Lua::Physics::InitializePhysics(l, hEnt, shape, physFlags); }));
	def.def("InitializePhysics", static_cast<void (*)(lua::State *, BasePhysicsComponent &, util::TSharedHandle<physics::IConvexShape> &)>([](lua::State *l, BasePhysicsComponent &hEnt, util::TSharedHandle<physics::IConvexShape> &shape) {
		Lua::Physics::InitializePhysics(l, hEnt, shape, 0);
	}));
	def.def("DestroyPhysicsObject", static_cast<void (*)(lua::State *, BasePhysicsComponent &)>([](lua::State *l, BasePhysicsComponent &hEnt) { hEnt.DestroyPhysicsObject(); }));
	def.def("DropToFloor", &BasePhysicsComponent::DropToFloor);
	def.def("IsTrigger", &BasePhysicsComponent::IsTrigger);
	def.def("SetKinematic", &BasePhysicsComponent::SetKinematic);
	def.def("IsKinematic", &BasePhysicsComponent::IsKinematic);

	def.def("GetCollisionCallbacksEnabled", &BasePhysicsComponent::GetCollisionCallbacksEnabled);
	def.def("SetCollisionCallbacksEnabled", static_cast<void (*)(lua::State *, BasePhysicsComponent &, bool)>([](lua::State *l, BasePhysicsComponent &hEnt, bool) { hEnt.SetCollisionCallbacksEnabled(true); }));
	def.def("GetCollisionContactReportEnabled", &BasePhysicsComponent::GetCollisionContactReportEnabled);
	def.def("SetCollisionContactReportEnabled", static_cast<void (*)(lua::State *, BasePhysicsComponent &, bool)>([](lua::State *l, BasePhysicsComponent &hEnt, bool) { hEnt.SetCollisionContactReportEnabled(true); }));
	def.def("SetCollisionFilterMask", &BasePhysicsComponent::SetCollisionFilterMask);
	def.def("GetCollisionFilterMask", &BasePhysicsComponent::GetCollisionFilterMask);
	def.def("SetCollisionFilterGroup", &BasePhysicsComponent::SetCollisionFilterGroup);
	def.def("GetCollisionFilterGroup", static_cast<physics::CollisionMask (BasePhysicsComponent::*)() const>(&BasePhysicsComponent::GetCollisionFilter));
	def.def("SetCollisionFilter", static_cast<void (BasePhysicsComponent::*)(physics::CollisionMask, physics::CollisionMask)>(&BasePhysicsComponent::SetCollisionFilter));
	def.def("EnableCollisions", &BasePhysicsComponent::EnableCollisions);
	def.def("DisableCollisions", &BasePhysicsComponent::DisableCollisions);
	def.def("SetCollisionsEnabled", static_cast<void (BasePhysicsComponent::*)(bool)>(&BasePhysicsComponent::SetCollisionsEnabled));
	def.def("SetCollisionsEnabled", static_cast<void (BasePhysicsComponent::*)(ecs::BaseEntity *, bool)>(&BasePhysicsComponent::SetCollisionsEnabled));
	def.def("EnableCollisions", static_cast<void (*)(lua::State *, BasePhysicsComponent &)>([](lua::State *l, BasePhysicsComponent &hEnt) { hEnt.SetCollisionsEnabled(true); }));
	def.def("DisableCollisions", static_cast<void (*)(lua::State *, BasePhysicsComponent &)>([](lua::State *l, BasePhysicsComponent &hEnt) { hEnt.SetCollisionsEnabled(false); }));
	def.def("SetSimulationEnabled", &BasePhysicsComponent::SetSimulationEnabled);
	def.def("IsSimulationEnabled", &BasePhysicsComponent::GetSimulationEnabled);
	def.def("ResetCollisions", &BasePhysicsComponent::ResetCollisions);

	def.def("GetPhysJointConstraints", static_cast<void (*)(lua::State *, BasePhysicsComponent &)>([](lua::State *l, BasePhysicsComponent &hEnt) {
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
	def.def("GetRotatedCollisionBounds", &BasePhysicsComponent::GetRotatedCollisionBounds, luabind::meta::join<luabind::out_value<2>, luabind::out_value<3>>::type {});

	def.def("GetPhysicsType", &BasePhysicsComponent::GetPhysicsType);
	def.def("GetCollisionRadius", &BasePhysicsComponent::GetCollisionRadius);
	def.def("IsPhysicsProp", static_cast<bool (*)(lua::State *, BasePhysicsComponent &)>([](lua::State *l, BasePhysicsComponent &hEnt) {
		auto physType = hEnt.GetPhysicsType();
		return (physType != physics::PhysicsType::None && physType != physics::PhysicsType::Static && physType != physics::PhysicsType::BoxController && physType != physics::PhysicsType::CapsuleController) ? true : false;
	}));

	def.def("GetAABBDistance", static_cast<void (*)(lua::State *, BasePhysicsComponent &, const Vector3 &)>([](lua::State *l, BasePhysicsComponent &hEnt, const Vector3 &p) { Lua::PushNumber(l, hEnt.GetAABBDistance(p)); }));
	def.def("GetAABBDistance",
	  static_cast<void (*)(lua::State *, BasePhysicsComponent &, BasePhysicsComponent &)>([](lua::State *l, BasePhysicsComponent &hEnt, BasePhysicsComponent &hOther) { Lua::PushNumber(l, hEnt.GetAABBDistance(hOther.GetEntity())); }));
	def.def("GetAABBDistance", static_cast<void (*)(lua::State *, BasePhysicsComponent &, ecs::BaseEntity &)>([](lua::State *l, BasePhysicsComponent &hEnt, ecs::BaseEntity &other) { Lua::PushNumber(l, hEnt.GetAABBDistance(other)); }));

	def.def("IsRagdoll", &BasePhysicsComponent::IsRagdoll);

	def.def("ApplyForce", static_cast<void (*)(lua::State *, BasePhysicsComponent &, const Vector3 &)>([](lua::State *l, BasePhysicsComponent &hEnt, const Vector3 &force) { hEnt.ApplyForce(force); }));
	def.def("ApplyForce", static_cast<void (*)(lua::State *, BasePhysicsComponent &, const Vector3 &, const Vector3 &)>([](lua::State *l, BasePhysicsComponent &hEnt, const Vector3 &force, const Vector3 &relPos) { hEnt.ApplyForce(force, relPos); }));
	def.def("ApplyImpulse", static_cast<void (*)(lua::State *, BasePhysicsComponent &, const Vector3 &)>([](lua::State *l, BasePhysicsComponent &hEnt, const Vector3 &impulse) { hEnt.ApplyImpulse(impulse); }));
	def.def("ApplyImpulse", static_cast<void (*)(lua::State *, BasePhysicsComponent &, const Vector3 &, const Vector3 &)>([](lua::State *l, BasePhysicsComponent &hEnt, const Vector3 &impulse, const Vector3 &relPos) { hEnt.ApplyImpulse(impulse, relPos); }));
	def.def("ApplyTorque", static_cast<void (*)(lua::State *, BasePhysicsComponent &, const Vector3 &)>([](lua::State *l, BasePhysicsComponent &hEnt, const Vector3 &torque) { hEnt.ApplyTorque(torque); }));
	def.def("ApplyTorqueImpulse", static_cast<void (*)(lua::State *, BasePhysicsComponent &, const Vector3 &)>([](lua::State *l, BasePhysicsComponent &hEnt, const Vector3 &torque) { hEnt.ApplyTorqueImpulse(torque); }));
	def.def("GetMass", &BasePhysicsComponent::GetMass);
	def.add_static_constant("EVENT_ON_PHYSICS_INITIALIZED", basePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED);
	def.add_static_constant("EVENT_ON_PHYSICS_DESTROYED", basePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED);
	def.add_static_constant("EVENT_ON_PHYSICS_UPDATED", basePhysicsComponent::EVENT_ON_PHYSICS_UPDATED);
	def.add_static_constant("EVENT_ON_DYNAMIC_PHYSICS_UPDATED", basePhysicsComponent::EVENT_ON_DYNAMIC_PHYSICS_UPDATED);
	def.add_static_constant("EVENT_ON_PRE_PHYSICS_SIMULATE", basePhysicsComponent::EVENT_ON_PRE_PHYSICS_SIMULATE);
	def.add_static_constant("EVENT_ON_POST_PHYSICS_SIMULATE", basePhysicsComponent::EVENT_ON_POST_PHYSICS_SIMULATE);
	def.add_static_constant("EVENT_ON_WAKE", basePhysicsComponent::EVENT_ON_WAKE);
	def.add_static_constant("EVENT_ON_SLEEP", basePhysicsComponent::EVENT_ON_SLEEP);
	def.add_static_constant("EVENT_HANDLE_RAYCAST", basePhysicsComponent::EVENT_HANDLE_RAYCAST);
	def.add_static_constant("EVENT_INITIALIZE_PHYSICS", basePhysicsComponent::EVENT_INITIALIZE_PHYSICS);

	def.add_static_constant("MOVETYPE_NONE", math::to_integral(physics::MoveType::None));
	def.add_static_constant("MOVETYPE_WALK", math::to_integral(physics::MoveType::Walk));
	def.add_static_constant("MOVETYPE_NOCLIP", math::to_integral(physics::MoveType::Noclip));
	def.add_static_constant("MOVETYPE_FLY", math::to_integral(physics::MoveType::Fly));
	def.add_static_constant("MOVETYPE_FREE", math::to_integral(physics::MoveType::Free));
	def.add_static_constant("MOVETYPE_PHYSICS", math::to_integral(physics::MoveType::Physics));
}

void pragma::LuaCore::base_render_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseRenderComponent>("BaseRenderComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetCastShadows", &BaseRenderComponent::SetCastShadows);
	def.def("GetCastShadows", &BaseRenderComponent::GetCastShadows);
}

void pragma::LuaCore::base_soft_body_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseSoftBodyComponent>("BaseSoftBodyComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

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
void pragma::LuaCore::base_sound_emitter_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseSoundEmitterComponent>("BaseSoundEmitterComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("CreateSound", &BaseSoundEmitterComponent::CreateSound);
	def.def("CreateSound", +[](BaseSoundEmitterComponent &c, std::string snd, audio::ALSoundType type) { return c.CreateSound(std::move(snd), type); });
	def.def("EmitSound", &BaseSoundEmitterComponent::EmitSound);
	def.def("EmitSound", +[](BaseSoundEmitterComponent &c, std::string snd, audio::ALSoundType type) { return c.EmitSound(std::move(snd), type); });
	def.def("StopSounds", &BaseSoundEmitterComponent::StopSounds);
	def.def("GetSounds", static_cast<void (*)(lua::State *, BaseSoundEmitterComponent &)>([](lua::State *l, BaseSoundEmitterComponent &hEnt) {
		std::vector<std::shared_ptr<audio::ALSound>> *sounds;
		hEnt.GetSounds(&sounds);
		Lua::CreateTable(l);
		int top = Lua::GetStackTop(l);
		for(int i = 0; i < sounds->size(); i++) {
			luabind::object(l, (*sounds)[i]).push(l);
			Lua::SetTableValue(l, top, i + 1);
		}
	}));
	def.add_static_constant("EVENT_ON_SOUND_CREATED", baseSoundEmitterComponent::EVENT_ON_SOUND_CREATED);
}

namespace Lua::Transform {
	void GetDirection(lua::State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther, bool bIgnoreYAxis) { Lua::Push<Vector3>(l, hEnt.GetDirection(hOther.GetEntity(), bIgnoreYAxis)); }

	void GetAngles(lua::State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther, bool bIgnoreYAxis) { Lua::Push<EulerAngles>(l, hEnt.GetAngles(hOther.GetEntity(), bIgnoreYAxis)); }

	void GetDotProduct(lua::State *l, pragma::BaseTransformComponent &hEnt, pragma::BaseTransformComponent &hOther, bool bIgnoreYAxis) { PushNumber(l, hEnt.GetDotProduct(hOther.GetEntity(), bIgnoreYAxis)); }
};
void pragma::LuaCore::base_transform_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseTransformComponent>("BaseTransformComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetOrigin", &BaseTransformComponent::GetOrigin);
	def.def("GetPos", static_cast<const Vector3 &(BaseTransformComponent::*)() const>(&BaseTransformComponent::GetPosition), luabind::copy_policy<0> {});
	def.def("GetPos", static_cast<Vector3 (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetPosition));
	def.def("SetPos", static_cast<void (BaseTransformComponent::*)(const Vector3 &)>(&BaseTransformComponent::SetPosition));
	def.def("SetPos", static_cast<void (BaseTransformComponent::*)(const Vector3 &, bool, CoordinateSpace)>(&BaseTransformComponent::SetPosition));
	def.def("SetPos", static_cast<void (BaseTransformComponent::*)(const Vector3 &, bool, CoordinateSpace)>(&BaseTransformComponent::SetPosition), luabind::default_parameter_policy<4, CoordinateSpace::World> {});
	def.def("GetRotation", static_cast<const Quat &(BaseTransformComponent::*)() const>(&BaseTransformComponent::GetRotation), luabind::copy_policy<0> {});
	def.def("GetRotation", static_cast<Quat (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetRotation));
	def.def("SetRotation", static_cast<void (BaseTransformComponent::*)(const Quat &)>(&BaseTransformComponent::SetRotation));
	def.def("SetRotation", static_cast<void (BaseTransformComponent::*)(const Quat &, CoordinateSpace)>(&BaseTransformComponent::SetRotation));
	def.def("GetAngles", static_cast<EulerAngles (BaseTransformComponent::*)(const Vector3 &, bool) const>(&BaseTransformComponent::GetAngles));
	def.def("GetAngles", static_cast<EulerAngles (BaseTransformComponent::*)(const Vector3 &, bool) const>(&BaseTransformComponent::GetAngles), luabind::default_parameter_policy<3, false> {});
	def.def("GetAngles", static_cast<EulerAngles (BaseTransformComponent::*)(const ecs::BaseEntity &, bool) const>(&BaseTransformComponent::GetAngles));
	def.def("GetAngles", static_cast<EulerAngles (BaseTransformComponent::*)(const ecs::BaseEntity &, bool) const>(&BaseTransformComponent::GetAngles), luabind::default_parameter_policy<3, false> {});
	def.def("GetAngles", static_cast<EulerAngles (BaseTransformComponent::*)() const>(&BaseTransformComponent::GetAngles));
	def.def("SetAngles", static_cast<void (BaseTransformComponent::*)(const EulerAngles &, CoordinateSpace)>(&BaseTransformComponent::SetAngles));
	def.def("SetAngles", static_cast<void (BaseTransformComponent::*)(const EulerAngles &)>(&BaseTransformComponent::SetAngles));
	def.def("GetForward", static_cast<Vector3 (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetForward));
	def.def("GetForward", static_cast<Vector3 (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetForward), luabind::default_parameter_policy<4, CoordinateSpace::World> {});
	def.def("GetRight", static_cast<Vector3 (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetRight));
	def.def("GetRight", static_cast<Vector3 (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetRight), luabind::default_parameter_policy<4, CoordinateSpace::World> {});
	def.def("GetUp", static_cast<Vector3 (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetUp));
	def.def("GetUp", static_cast<Vector3 (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetUp), luabind::default_parameter_policy<4, CoordinateSpace::World> {});

	def.def("GetEyePos", &BaseTransformComponent::GetEyePosition);
	def.def("GetEyeOffset", &BaseTransformComponent::GetEyeOffset);
	def.def("SetEyeOffset", &BaseTransformComponent::SetEyeOffset);

	def.def("LocalToWorld", static_cast<void (*)(lua::State *, BaseTransformComponent &, Vector3)>([](lua::State *l, BaseTransformComponent &hEnt, Vector3 origin) {
		hEnt.LocalToWorld(&origin);
		Lua::Push<Vector3>(l, origin);
	}));
	def.def("LocalToWorld", static_cast<void (*)(lua::State *, BaseTransformComponent &, Quat)>([](lua::State *l, BaseTransformComponent &hEnt, Quat rot) {
		hEnt.LocalToWorld(&rot);
		Lua::Push<Quat>(l, rot);
	}));
	def.def("LocalToWorld", static_cast<void (*)(lua::State *, BaseTransformComponent &, Vector3, Quat)>([](lua::State *l, BaseTransformComponent &hEnt, Vector3 origin, Quat rot) {
		hEnt.LocalToWorld(&origin, &rot);
		Lua::Push<Vector3>(l, origin);
		Lua::Push<Quat>(l, rot);
	}));
	def.def("WorldToLocal", static_cast<void (*)(lua::State *, BaseTransformComponent &, Vector3)>([](lua::State *l, BaseTransformComponent &hEnt, Vector3 origin) {
		hEnt.WorldToLocal(&origin);
		Lua::Push<Vector3>(l, origin);
	}));
	def.def("WorldToLocal", static_cast<void (*)(lua::State *, BaseTransformComponent &, Quat)>([](lua::State *l, BaseTransformComponent &hEnt, Quat rot) {
		hEnt.WorldToLocal(&rot);
		Lua::Push<Quat>(l, rot);
	}));
	def.def("WorldToLocal", static_cast<void (*)(lua::State *, BaseTransformComponent &, Vector3, Quat)>([](lua::State *l, BaseTransformComponent &hEnt, Vector3 origin, Quat rot) {
		hEnt.WorldToLocal(&origin, &rot);
		Lua::Push<Vector3>(l, origin);
		Lua::Push<Quat>(l, rot);
	}));

	def.def("GetPitch", static_cast<float (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetPitch));
	def.def("GetPitch", static_cast<float (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetPitch), luabind::default_parameter_policy<2, CoordinateSpace::World> {});
	def.def("GetYaw", static_cast<float (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetYaw));
	def.def("GetYaw", static_cast<float (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetYaw), luabind::default_parameter_policy<2, CoordinateSpace::World> {});
	def.def("GetRoll", static_cast<float (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetRoll));
	def.def("GetRoll", static_cast<float (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetRoll), luabind::default_parameter_policy<2, CoordinateSpace::World> {});
	def.def("SetPitch", static_cast<void (BaseTransformComponent::*)(float, CoordinateSpace)>(&BaseTransformComponent::SetPitch));
	def.def("SetPitch", static_cast<void (BaseTransformComponent::*)(float, CoordinateSpace)>(&BaseTransformComponent::SetPitch), luabind::default_parameter_policy<3, CoordinateSpace::World> {});
	def.def("SetYaw", static_cast<void (BaseTransformComponent::*)(float, CoordinateSpace)>(&BaseTransformComponent::SetYaw));
	def.def("SetYaw", static_cast<void (BaseTransformComponent::*)(float, CoordinateSpace)>(&BaseTransformComponent::SetYaw), luabind::default_parameter_policy<3, CoordinateSpace::World> {});
	def.def("SetRoll", static_cast<void (BaseTransformComponent::*)(float, CoordinateSpace)>(&BaseTransformComponent::SetRoll));
	def.def("SetRoll", static_cast<void (BaseTransformComponent::*)(float, CoordinateSpace)>(&BaseTransformComponent::SetRoll), luabind::default_parameter_policy<3, CoordinateSpace::World> {});

	def.def("GetLastMoveTime", &BaseTransformComponent::GetLastMoveTime);
	def.def("GetScale", static_cast<const Vector3 &(BaseTransformComponent::*)() const>(&BaseTransformComponent::GetScale), luabind::copy_policy<0> {});
	def.def("GetScale", static_cast<Vector3 (BaseTransformComponent::*)(CoordinateSpace) const>(&BaseTransformComponent::GetScale));
	def.def("GetMaxAxisScale", &BaseTransformComponent::GetMaxAxisScale);
	def.def("GetAbsMaxAxisScale", &BaseTransformComponent::GetAbsMaxAxisScale);
	def.def("SetScale", static_cast<void (BaseTransformComponent::*)(float)>(&BaseTransformComponent::SetScale));
	def.def("SetScale", static_cast<void (BaseTransformComponent::*)(const Vector3 &)>(&BaseTransformComponent::SetScale));
	def.def("SetScale", static_cast<void (BaseTransformComponent::*)(const Vector3 &, CoordinateSpace)>(&BaseTransformComponent::SetScale));

	def.def("GetDistance", static_cast<float (BaseTransformComponent::*)(const Vector3 &) const>(&BaseTransformComponent::GetDistance));
	def.def("GetDistance", static_cast<float (BaseTransformComponent::*)(const ecs::BaseEntity &) const>(&BaseTransformComponent::GetDistance));
	def.def("GetDistance", static_cast<float (*)(lua::State *, BaseTransformComponent &, BaseTransformComponent &)>([](lua::State *l, BaseTransformComponent &hEnt, BaseTransformComponent &hOther) { return hEnt.GetDistance(hOther.GetEntity()); }));

	def.def("GetDirection", static_cast<Vector3 (BaseTransformComponent::*)(const ecs::BaseEntity &, bool) const>(&BaseTransformComponent::GetDirection));
	def.def("GetDirection", static_cast<Vector3 (BaseTransformComponent::*)(const ecs::BaseEntity &, bool) const>(&BaseTransformComponent::GetDirection), luabind::default_parameter_policy<3, false> {});
	def.def("GetDirection", static_cast<Vector3 (BaseTransformComponent::*)(const Vector3 &, bool) const>(&BaseTransformComponent::GetDirection));
	def.def("GetDirection", static_cast<Vector3 (BaseTransformComponent::*)(const Vector3 &, bool) const>(&BaseTransformComponent::GetDirection), luabind::default_parameter_policy<3, false> {});
	def.def("GetDirection", static_cast<void (*)(lua::State *, BaseTransformComponent &, BaseTransformComponent &, bool)>([](lua::State *l, BaseTransformComponent &hEnt, BaseTransformComponent &hOther, bool bIgnoreYAxis) {
		Lua::Push<Vector3>(l, hEnt.GetDirection(hOther.GetEntity(), bIgnoreYAxis));
	}));

	def.def("GetAngles", static_cast<void (*)(lua::State *, BaseTransformComponent &, BaseTransformComponent &, bool)>([](lua::State *l, BaseTransformComponent &hEnt, BaseTransformComponent &hOther, bool bIgnoreYAxis) {
		Lua::Push<EulerAngles>(l, hEnt.GetAngles(hOther.GetEntity(), bIgnoreYAxis));
	}));
	def.def("GetAngles",
	  static_cast<void (*)(lua::State *, BaseTransformComponent &, ecs::BaseEntity &, bool)>([](lua::State *l, BaseTransformComponent &hEnt, ecs::BaseEntity &other, bool bIgnoreYAxis) { Lua::Push<EulerAngles>(l, hEnt.GetAngles(other, bIgnoreYAxis)); }));
	def.def("GetDotProduct", static_cast<void (*)(lua::State *, BaseTransformComponent &, const Vector3 &, bool)>([](lua::State *l, BaseTransformComponent &hEnt, const Vector3 &p, bool bIgnoreYAxis) { Lua::PushNumber(l, hEnt.GetDotProduct(p, bIgnoreYAxis)); }));
	def.def("GetDotProduct", static_cast<void (*)(lua::State *, BaseTransformComponent &, BaseTransformComponent &, bool)>([](lua::State *l, BaseTransformComponent &hEnt, BaseTransformComponent &hOther, bool bIgnoreYAxis) {
		Lua::PushNumber(l, hEnt.GetDotProduct(hOther.GetEntity(), bIgnoreYAxis));
	}));
	def.def("GetDotProduct",
	  static_cast<void (*)(lua::State *, BaseTransformComponent &, ecs::BaseEntity &, bool)>([](lua::State *l, BaseTransformComponent &hEnt, ecs::BaseEntity &other, bool bIgnoreYAxis) { Lua::PushNumber(l, hEnt.GetDotProduct(other, bIgnoreYAxis)); }));

	def.def("GetDirection", static_cast<void (*)(lua::State *, BaseTransformComponent &, BaseTransformComponent &)>([](lua::State *l, BaseTransformComponent &hEnt, BaseTransformComponent &hOther) { Lua::Transform::GetDirection(l, hEnt, hOther, false); }));
	def.def("GetAngles", static_cast<void (*)(lua::State *, BaseTransformComponent &, BaseTransformComponent &)>([](lua::State *l, BaseTransformComponent &hEnt, BaseTransformComponent &hOther) { Lua::Transform::GetAngles(l, hEnt, hOther, false); }));
	def.def("GetDotProduct", static_cast<void (*)(lua::State *, BaseTransformComponent &, BaseTransformComponent &)>([](lua::State *l, BaseTransformComponent &hEnt, BaseTransformComponent &hOther) { Lua::Transform::GetDotProduct(l, hEnt, hOther, false); }));
	def.def("GetDotProduct", static_cast<float (BaseTransformComponent::*)(const ecs::BaseEntity &, bool) const>(&BaseTransformComponent::GetDotProduct));
	def.def("GetDotProduct", static_cast<float (BaseTransformComponent::*)(const ecs::BaseEntity &, bool) const>(&BaseTransformComponent::GetDotProduct), luabind::default_parameter_policy<3, false> {});
	def.def("GetDotProduct", static_cast<float (BaseTransformComponent::*)(const Vector3 &, bool) const>(&BaseTransformComponent::GetDotProduct));
	def.def("GetDotProduct", static_cast<float (BaseTransformComponent::*)(const Vector3 &, bool) const>(&BaseTransformComponent::GetDotProduct), luabind::default_parameter_policy<3, false> {});
}

void pragma::LuaCore::base_color_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseColorComponent>("BaseColorComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetColorProperty", &BaseColorComponent::GetColorProperty);

	def.def("GetColor", &BaseColorComponent::GetColor, luabind::copy_policy<0> {});
	def.def("SetColor", static_cast<void (BaseColorComponent::*)(const Color &)>(&BaseColorComponent::SetColor));
	def.def("SetColor", static_cast<void (BaseColorComponent::*)(const Vector3 &)>(&BaseColorComponent::SetColor));
	def.def("SetColor", static_cast<void (BaseColorComponent::*)(const Vector4 &)>(&BaseColorComponent::SetColor));
	def.add_static_constant("EVENT_ON_COLOR_CHANGED", baseColorComponent::EVENT_ON_COLOR_CHANGED);
}

static std::optional<std::tuple<std::shared_ptr<pragma::geometry::ModelMesh>, std::shared_ptr<pragma::geometry::ModelSubMesh>, pragma::material::Material *>> FindAndAssignSurfaceMesh(pragma::BaseSurfaceComponent &c, luabind::object oFilter)
{
	std::function<int32_t(pragma::geometry::ModelMesh &, pragma::geometry::ModelSubMesh &, pragma::material::Material &, const std::string &)> filter = nullptr;
	if(oFilter) {
		filter = [&oFilter](pragma::geometry::ModelMesh &mesh, pragma::geometry::ModelSubMesh &subMesh, pragma::material::Material &mat, const std::string &shader) -> int32_t {
			auto res = oFilter(&mat, shader);
			return luabind::object_cast<int32_t>(res);
		};
	}
	auto res = c.FindAndAssignMesh(filter);
	if(!res.has_value())
		return {};
	return std::tuple<std::shared_ptr<pragma::geometry::ModelMesh>, std::shared_ptr<pragma::geometry::ModelSubMesh>, pragma::material::Material *> {res->mesh->shared_from_this(), res->subMesh->shared_from_this(), res->material};
}

void pragma::LuaCore::base_surface_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseSurfaceComponent>("BaseSurfaceComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetPlane", static_cast<void (BaseSurfaceComponent::*)(const math::Plane &)>(&BaseSurfaceComponent::SetPlane));
	def.def("SetPlane", static_cast<void (BaseSurfaceComponent::*)(const Vector3 &, float)>(&BaseSurfaceComponent::SetPlane));
	def.def("GetPlane", static_cast<const math::Plane &(BaseSurfaceComponent::*)() const>(&BaseSurfaceComponent::GetPlane), luabind::copy_policy<0> {});
	def.def("GetPlaneWs", static_cast<math::Plane (BaseSurfaceComponent::*)() const>(&BaseSurfaceComponent::GetPlaneWs));
	def.def("GetPlaneNormal", &BaseSurfaceComponent::GetPlaneNormal);
	def.def("SetPlaneNormal", &BaseSurfaceComponent::SetPlaneNormal);
	def.def("GetPlaneDistance", &BaseSurfaceComponent::GetPlaneDistance);
	def.def("SetPlaneDistance", &BaseSurfaceComponent::SetPlaneDistance);
	def.def("ProjectToSurface", &BaseSurfaceComponent::ProjectToSurface);
	def.def("Clear", &BaseSurfaceComponent::Clear);
	def.def("IsPointBelowSurface", &BaseSurfaceComponent::IsPointBelowSurface);
	def.def("GetPlaneRotation", &BaseSurfaceComponent::GetPlaneRotation);
	def.def(
	  "CalcLineSurfaceIntersection", +[](BaseSurfaceComponent &c, const Vector3 &lineOrigin, const Vector3 &lineDir) {
		  double t;
		  auto r = c.CalcLineSurfaceIntersection(lineOrigin, lineDir, &t);
		  return std::pair<bool, double> {r, t};
	  });
	def.def("GetMesh", static_cast<geometry::ModelSubMesh *(BaseSurfaceComponent::*)()>(&BaseSurfaceComponent::GetMesh), luabind::shared_from_this_policy<0> {});
	def.def("FindAndAssignSurfaceMesh", +[](BaseSurfaceComponent &c, luabind::object oFilter) -> std::optional<std::tuple<std::shared_ptr<geometry::ModelMesh>, std::shared_ptr<geometry::ModelSubMesh>, material::Material *>> { return FindAndAssignSurfaceMesh(c, oFilter); });
	def.def("FindAndAssignSurfaceMesh", +[](BaseSurfaceComponent &c) -> std::optional<std::tuple<std::shared_ptr<geometry::ModelMesh>, std::shared_ptr<geometry::ModelSubMesh>, material::Material *>> { return FindAndAssignSurfaceMesh(c, Lua::nil); });
	def.add_static_constant("EVENT_ON_SURFACE_PLANE_CHANGED", baseSurfaceComponent::EVENT_ON_SURFACE_PLANE_CHANGED);
}

void pragma::LuaCore::base_score_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseScoreComponent>("BaseScoreComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetScoreProperty", &BaseScoreComponent::GetScoreProperty);
	def.def("GetScore", &BaseScoreComponent::GetScore);
	def.def("SetScore", &BaseScoreComponent::SetScore);
	def.def("AddScore", &BaseScoreComponent::AddScore);
	def.def("SubtractScore", &BaseScoreComponent::SubtractScore);
	def.add_static_constant("EVENT_ON_SCORE_CHANGED", baseScoreComponent::EVENT_ON_SCORE_CHANGED);
}

void pragma::LuaCore::base_radius_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseRadiusComponent>("BaseRadiusComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetRadiusProperty", &BaseRadiusComponent::GetRadiusProperty);
	def.def("GetRadius", &BaseRadiusComponent::GetRadius);
	def.def("SetRadius", &BaseRadiusComponent::SetRadius);
	def.add_static_constant("EVENT_ON_RADIUS_CHANGED", baseRadiusComponent::EVENT_ON_RADIUS_CHANGED);
}

void pragma::LuaCore::base_field_angle_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFieldAngleComponent>("BaseFieldAngleComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetFieldAngleProperty", &BaseFieldAngleComponent::GetFieldAngleProperty);
	def.def("GetFieldAngle", &BaseFieldAngleComponent::GetFieldAngle);
	def.def("SetFieldAngle", &BaseFieldAngleComponent::SetFieldAngle);
	def.add_static_constant("EVENT_ON_FIELD_ANGLE_CHANGED", baseFieldAngleComponent::EVENT_ON_FIELD_ANGLE_CHANGED);
}

void pragma::LuaCore::base_env_sound_dsp_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvSoundDspComponent>("BaseEnvSoundDspComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_camera_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvCameraComponent>("BaseEnvCameraComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.add_static_constant("DEFAULT_NEAR_Z", baseEnvCameraComponent::DEFAULT_NEAR_Z);
	def.add_static_constant("DEFAULT_FAR_Z", baseEnvCameraComponent::DEFAULT_FAR_Z);
	def.add_static_constant("DEFAULT_FOV", baseEnvCameraComponent::DEFAULT_FOV);
	def.add_static_constant("DEFAULT_VIEWMODEL_FOV", baseEnvCameraComponent::DEFAULT_VIEWMODEL_FOV);
	def.scope[luabind::def("calc_projection_matrix", +[](math::Radian fov, float aspectRatio, float nearZ, float farZ, const rendering::Tile *optTile) -> Mat4 { return BaseEnvCameraComponent::CalcProjectionMatrix(fov, aspectRatio, nearZ, farZ, optTile); })];
	def.def("GetProjectionMatrix", &BaseEnvCameraComponent::GetProjectionMatrix, luabind::copy_policy<0> {});
	def.def("GetViewMatrix", &BaseEnvCameraComponent::GetViewMatrix, luabind::copy_policy<0> {});
	def.def("LookAt", static_cast<void (*)(lua::State *, BaseEnvCameraComponent &, const Vector3 &)>([](lua::State *l, BaseEnvCameraComponent &hComponent, const Vector3 &lookAtPos) {
		auto *trComponent = hComponent.GetEntity().GetTransformComponent();
		if(!trComponent)
			return;
		auto &camPos = trComponent->GetPosition();
		auto dir = lookAtPos - camPos;
		uvec::normalize(&dir);
		trComponent->SetRotation(uquat::create_look_rotation(dir, trComponent->GetUp()));
	}));
	def.def("UpdateMatrices", &BaseEnvCameraComponent::UpdateMatrices);
	def.def("UpdateViewMatrix", &BaseEnvCameraComponent::UpdateViewMatrix);
	def.def("UpdateProjectionMatrix", &BaseEnvCameraComponent::UpdateProjectionMatrix);
	def.def("SetFOV", &BaseEnvCameraComponent::SetFOV);
	def.def("GetProjectionMatrixProperty", &BaseEnvCameraComponent::GetProjectionMatrixProperty);
	def.def("GetViewMatrixProperty", &BaseEnvCameraComponent::GetViewMatrixProperty);
	def.def("GetNearZProperty", &BaseEnvCameraComponent::GetNearZProperty);
	def.def("GetFarZProperty", &BaseEnvCameraComponent::GetFarZProperty);
	def.def("GetFOVProperty", &BaseEnvCameraComponent::GetFOVProperty);
	def.def("GetAspectRatioProperty", &BaseEnvCameraComponent::GetAspectRatioProperty);
	def.def("SetAspectRatio", &BaseEnvCameraComponent::SetAspectRatio);
	def.def("SetNearZ", &BaseEnvCameraComponent::SetNearZ);
	def.def("SetFarZ", &BaseEnvCameraComponent::SetFarZ);
	def.def("GetFOV", &BaseEnvCameraComponent::GetFOV);
	def.def("GetFOVRad", &BaseEnvCameraComponent::GetFOVRad);
	def.def("GetAspectRatio", &BaseEnvCameraComponent::GetAspectRatio);
	def.def("GetNearZ", &BaseEnvCameraComponent::GetNearZ);
	def.def("GetFarZ", &BaseEnvCameraComponent::GetFarZ);
	def.def("GetFocalDistanceProperty", &BaseEnvCameraComponent::GetFocalDistanceProperty);
	def.def("GetFocalDistance", &BaseEnvCameraComponent::GetFocalDistance);
	def.def("SetFocalDistance", &BaseEnvCameraComponent::SetFocalDistance);
	def.def("GetFrustumPlanes", static_cast<std::vector<math::Plane> (*)(lua::State *, BaseEnvCameraComponent &)>([](lua::State *l, BaseEnvCameraComponent &hComponent) {
		std::vector<math::Plane> planes;
		hComponent.GetFrustumPlanes(planes);
		return planes;
	}));
	def.def("GetFarPlaneCenter", &BaseEnvCameraComponent::GetFarPlaneCenter);
	def.def("GetNearPlaneCenter", &BaseEnvCameraComponent::GetNearPlaneCenter);
	def.def("GetPlaneCenter", &BaseEnvCameraComponent::GetPlaneCenter);
	def.def("GetFarPlaneBoundaries", static_cast<std::array<Vector3, 4> (*)(lua::State *, BaseEnvCameraComponent &)>([](lua::State *l, BaseEnvCameraComponent &hComponent) -> std::array<Vector3, 4> {
		std::array<Vector3, 4> farBounds;
		hComponent.GetFarPlaneBoundaries(farBounds);
		return farBounds;
	}));
	def.def("GetNearPlaneBoundaries", static_cast<std::array<Vector3, 4> (*)(lua::State *, BaseEnvCameraComponent &)>([](lua::State *l, BaseEnvCameraComponent &hComponent) {
		std::array<Vector3, 4> nearBounds;
		hComponent.GetNearPlaneBoundaries(nearBounds);
		return nearBounds;
	}));
	def.def("GetPlaneBoundaries", static_cast<std::array<Vector3, 4> (*)(lua::State *, BaseEnvCameraComponent &, float)>([](lua::State *l, BaseEnvCameraComponent &hComponent, float z) {
		std::array<Vector3, 4> bounds;
		hComponent.GetPlaneBoundaries(z, bounds);
		return bounds;
	}));
	def.def("GetPlaneBoundaries", static_cast<Lua::mult<luabind::tableT<Vector3>, luabind::tableT<Vector3>> (*)(lua::State *, BaseEnvCameraComponent &)>([](lua::State *l, BaseEnvCameraComponent &hComponent) -> Lua::mult<luabind::tableT<Vector3>, luabind::tableT<Vector3>> {
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
	def.def("SetProjectionMatrix", &BaseEnvCameraComponent::SetProjectionMatrix);
	def.def("SetViewMatrix", &BaseEnvCameraComponent::SetViewMatrix);
	def.def("GetNearPlaneBounds", &BaseEnvCameraComponent::GetNearPlaneBounds, luabind::meta::join<luabind::out_value<2>, luabind::out_value<3>>::type {});
	def.def("GetFarPlaneBounds", &BaseEnvCameraComponent::GetFarPlaneBounds, luabind::meta::join<luabind::out_value<2>, luabind::out_value<3>>::type {});
	def.def("GetFarPlaneBounds", &BaseEnvCameraComponent::GetPlaneBounds, luabind::meta::join<luabind::out_value<3>, luabind::out_value<4>>::type {});
	def.def("GetFrustumPoints", static_cast<void (BaseEnvCameraComponent::*)(std::vector<Vector3> &) const>(&BaseEnvCameraComponent::GetFrustumPoints), luabind::out_value<2> {});
	def.def("GetNearPlanePoint", &BaseEnvCameraComponent::GetNearPlanePoint);
	def.def("GetFarPlanePoint", &BaseEnvCameraComponent::GetFarPlanePoint);
	def.def("GetPlanePoint", &BaseEnvCameraComponent::GetPlanePoint);
	def.def("GetFrustumNeighbors", static_cast<void (*)(lua::State *, BaseEnvCameraComponent &, int)>([](lua::State *l, BaseEnvCameraComponent &hComponent, int planeID) {
		if(planeID < 0 || planeID > 5)
			return;
		math::FrustumPlane neighborIDs[4];
		hComponent.GetFrustumNeighbors(math::FrustumPlane(planeID), &neighborIDs[0]);
		int table = Lua::CreateTable(l);
		for(unsigned int i = 0; i < 4; i++) {
			Lua::PushInt(l, i + 1);
			Lua::PushInt(l, static_cast<int>(neighborIDs[i]));
			Lua::SetTableValue(l, table);
		}
	}));
	def.def("GetFrustumPlaneCornerPoints", static_cast<void (*)(lua::State *, BaseEnvCameraComponent &, int, int)>([](lua::State *l, BaseEnvCameraComponent &hComponent, int planeA, int planeB) {
		if(planeA < 0 || planeB < 0 || planeA > 5 || planeB > 5)
			return;

		math::FrustumPoint cornerPoints[2];
		hComponent.GetFrustumPlaneCornerPoints(math::FrustumPlane(planeA), math::FrustumPlane(planeB), &cornerPoints[0]);

		Lua::PushInt(l, static_cast<int>(cornerPoints[0]));
		Lua::PushInt(l, static_cast<int>(cornerPoints[1]));
	}));
	def.def("CreateFrustumKDop", static_cast<void (BaseEnvCameraComponent::*)(const Vector2 &, const Vector2 &, std::vector<math::Plane> &) const>(&BaseEnvCameraComponent::CreateFrustumKDop), luabind::out_value<4> {});
	def.def("CreateFrustumMesh", static_cast<void (*)(lua::State *, BaseEnvCameraComponent &, const Vector2 &, const Vector2 &)>([](lua::State *l, BaseEnvCameraComponent &hComponent, const Vector2 &uvStart, const Vector2 &uvEnd) {
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
	def.def("ScreenSpaceToWorldSpace", static_cast<Vector3 (*)(lua::State *, BaseEnvCameraComponent &, const Vector2 &, float, float)>([](lua::State *l, BaseEnvCameraComponent &hComponent, const Vector2 &uv, float width, float height) -> Vector3 {
		auto &ent = hComponent.GetEntity();
		return uvec::calc_world_direction_from_2d_coordinates(ent.GetForward(), ent.GetRight(), ent.GetUp(), hComponent.GetFOVRad(), hComponent.GetNearZ(), hComponent.GetFarZ(), hComponent.GetAspectRatio(), width, height, uv);
	}));
	def.def("WorldSpaceToScreenSpace", static_cast<std::pair<Vector2, float> (*)(lua::State *, BaseEnvCameraComponent &, const Vector3 &)>([](lua::State *l, BaseEnvCameraComponent &hComponent, const Vector3 &point) -> std::pair<Vector2, float> {
		float dist;
		auto uv = uvec::calc_screenspace_uv_from_worldspace_position(point, hComponent.GetProjectionMatrix() * hComponent.GetViewMatrix(), hComponent.GetNearZ(), hComponent.GetFarZ(), dist);
		return {uv, dist};
	}));
	def.def("WorldSpaceToScreenSpaceDirection", static_cast<Vector2 (*)(lua::State *, BaseEnvCameraComponent &, const Vector3 &)>([](lua::State *l, BaseEnvCameraComponent &hComponent, const Vector3 &dir) -> Vector2 {
		return uvec::calc_screenspace_direction_from_worldspace_direction(dir, hComponent.GetProjectionMatrix() * hComponent.GetViewMatrix());
	}));
	def.def("CalcScreenSpaceDistance", static_cast<float (*)(lua::State *, BaseEnvCameraComponent &, const Vector3 &)>([](lua::State *l, BaseEnvCameraComponent &hComponent, const Vector3 &point) -> float {
		return uvec::calc_screenspace_distance_to_worldspace_position(point, hComponent.GetProjectionMatrix() * hComponent.GetViewMatrix(), hComponent.GetNearZ(), hComponent.GetFarZ());
	}));
	def.def("DepthToDistance",
	  static_cast<float (*)(lua::State *, BaseEnvCameraComponent &, double, float, float)>([](lua::State *l, BaseEnvCameraComponent &hComponent, double depth, float nearZ, float farZ) -> float { return uvec::depth_to_distance(depth, nearZ, farZ); }));
	def.def("CalcRayDirection", static_cast<Vector3 (*)(lua::State *, BaseEnvCameraComponent &, const Vector2 &)>([](lua::State *l, BaseEnvCameraComponent &hComponent, const Vector2 &uv) -> Vector3 {
		auto &ent = hComponent.GetEntity();
		return uvec::calc_world_direction_from_2d_coordinates(ent.GetForward(), ent.GetRight(), ent.GetUp(), hComponent.GetFOVRad(), hComponent.GetNearZ(), hComponent.GetFarZ(), hComponent.GetAspectRatio(), 0.f, 0.f, uv);
	}));
}

void pragma::LuaCore::base_env_explosion_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvExplosionComponent>("BaseEnvExplosionComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_fire_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvFireComponent>("BaseEnvFireComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_fog_controller_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvFogControllerComponent>("BaseEnvFogControllerComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_light_spot_vol_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvLightSpotVolComponent>("BaseEnvLightSpotVolComponent");
	def.def("SetIntensity", &BaseEnvLightSpotVolComponent::SetIntensityFactor);
	def.def("GetIntensity", &BaseEnvLightSpotVolComponent::GetIntensityFactor);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_microphone_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvMicrophoneComponent>("BaseEnvMicrophoneComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_quake_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvQuakeComponent>("BaseEnvQuakeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_smoke_trail_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvSmokeTrailComponent>("BaseEnvSmokeTrailComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_sound_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvSoundComponent>("BaseEnvSoundComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetSoundSource", &BaseEnvSoundComponent::SetSoundSource);
	def.def("SetPitch", &BaseEnvSoundComponent::SetPitch);
	def.def("SetGain", &BaseEnvSoundComponent::SetGain);
	def.def("SetRolloffFactor", &BaseEnvSoundComponent::SetRolloffFactor);
	def.def("SetMinGain", &BaseEnvSoundComponent::SetMinGain);
	def.def("SetMaxGain", &BaseEnvSoundComponent::SetMaxGain);
	def.def("SetInnerConeAngle", &BaseEnvSoundComponent::SetInnerConeAngle);
	def.def("SetOuterConeAngle", &BaseEnvSoundComponent::SetOuterConeAngle);
	def.def("SetOffset", &BaseEnvSoundComponent::SetOffset);
	def.def("GetOffset", &BaseEnvSoundComponent::GetOffset);
	def.def("SetTimeOffset", &BaseEnvSoundComponent::SetTimeOffset);
	def.def("GetTimeOffset", &BaseEnvSoundComponent::GetTimeOffset);
	def.def("SetReferenceDistance", &BaseEnvSoundComponent::SetReferenceDistance);
	def.def("SetMaxDistance", &BaseEnvSoundComponent::SetMaxDistance);
	def.def("SetRelativeToListener", &BaseEnvSoundComponent::SetRelativeToListener);
	def.def("SetPlayOnSpawn", &BaseEnvSoundComponent::SetPlayOnSpawn);
	def.def("SetLooping", &BaseEnvSoundComponent::SetLooping);
	def.def("SetSoundTypes", &BaseEnvSoundComponent::SetSoundType);
	def.def("Play", &BaseEnvSoundComponent::Play);
	def.def("Stop", &BaseEnvSoundComponent::Stop);
	def.def("Pause", &BaseEnvSoundComponent::Pause);
	def.def("GetSound", &BaseEnvSoundComponent::GetSound);
	def.def("IsPlaying", &BaseEnvSoundComponent::IsPlaying);
	def.def("IsPaused", &BaseEnvSoundComponent::IsPaused);
}

void pragma::LuaCore::base_env_soundscape_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvSoundScapeComponent>("BaseEnvSoundScapeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_sprite_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvSpriteComponent>("BaseEnvSpriteComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_timescale_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvTimescaleComponent>("BaseEnvTimescaleComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_weather_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvWeatherComponent>("BaseEnvWeatherComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_wind_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEnvWindComponent>("BaseEnvWindComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_env_filter_name_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFilterNameComponent>("BaseFilterNameComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("ShouldPass", &BaseFilterNameComponent::ShouldPass);
	def.add_static_constant("EVENT_ON_NAME_CHANGED", baseNameComponent::EVENT_ON_NAME_CHANGED);
}

void pragma::LuaCore::base_env_filter_class_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFilterClassComponent>("BaseFilterClassComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("ShouldPass", &BaseFilterClassComponent::ShouldPass);
}

void pragma::LuaCore::base_func_brush_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFuncBrushComponent>("BaseFuncBrushComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_func_button_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFuncButtonComponent>("BaseFuncButtonComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_func_kinematic_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFuncKinematicComponent>("BaseFuncKinematicComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_func_physics_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFuncPhysicsComponent>("BaseFuncPhysicsComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_func_portal_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFuncPortalComponent>("BaseFuncPortalComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_func_soft_physics_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFuncSoftPhysicsComponent>("BaseFuncSoftPhysicsComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_func_surface_material_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFuncSurfaceMaterialComponent>("BaseFuncSurfaceMaterialComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_logic_relay_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseLogicRelayComponent>("BaseLogicRelayComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_bot_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseBotComponent>("BaseBotComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_point_path_node_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePointPathNodeComponent>("BasePointPathNodeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_point_constraint_ball_socket_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePointConstraintBallSocketComponent>("BasePointConstraintBallSocketComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_point_constraint_cone_twist_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePointConstraintConeTwistComponent>("BasePointConstraintConeTwistComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_point_constraint_dof_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePointConstraintDoFComponent>("BasePointConstraintDoFComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_point_constraint_fixed_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePointConstraintFixedComponent>("BasePointConstraintFixedComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_point_constraint_hinge_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePointConstraintHingeComponent>("BasePointConstraintHingeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_point_constraint_slider_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePointConstraintSliderComponent>("BasePointConstraintSliderComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_point_render_target_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePointRenderTargetComponent>("BasePointRenderTargetComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_point_target_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePointTargetComponent>("BasePointTargetComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_prop_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePropComponent>("BasePropComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_prop_dynamic_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePropDynamicComponent>("BasePropDynamicComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_prop_physics_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePropPhysicsComponent>("BasePropPhysicsComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_trigger_hurt_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseTriggerHurtComponent>("BaseTriggerHurtComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_trigger_push_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseTriggerPushComponent>("BaseTriggerPushComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_trigger_remove_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseTriggerRemoveComponent>("BaseTriggerRemoveComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_trigger_teleport_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseTriggerTeleportComponent>("BaseTriggerTeleportComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_touch_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseTouchComponent>("BaseTouchComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetTriggerFlags", &BaseTouchComponent::SetTriggerFlags);
	def.def("GetTriggerFlags", &BaseTouchComponent::GetTriggerFlags);
	def.def("GetTouchingEntities", static_cast<luabind::object (*)(lua::State *, BaseTouchComponent &)>([](lua::State *l, BaseTouchComponent &hComponent) -> luabind::object {
		auto t = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &touchInfo : hComponent.GetTouchingInfo()) {
			if(touchInfo.touch.entity.valid() == false || touchInfo.triggered == false)
				continue;
			t[idx++] = touchInfo.touch.entity.get()->GetLuaObject();
		}
		return t;
	}));
	def.def("GetTouchingEntityCount", static_cast<uint32_t (*)(lua::State *, BaseTouchComponent &)>([](lua::State *l, BaseTouchComponent &hComponent) -> uint32_t {
		auto &touchingInfo = hComponent.GetTouchingInfo();
		return std::count_if(touchingInfo.begin(), touchingInfo.end(), [](const BaseTouchComponent::TouchInfo &touchInfo) -> bool { return touchInfo.triggered && touchInfo.touch.entity.valid(); });
	}));

	def.add_static_constant("EVENT_CAN_TRIGGER", baseTouchComponent::EVENT_CAN_TRIGGER);
	def.add_static_constant("EVENT_ON_START_TOUCH", baseTouchComponent::EVENT_ON_START_TOUCH);
	def.add_static_constant("EVENT_ON_END_TOUCH", baseTouchComponent::EVENT_ON_END_TOUCH);
	def.add_static_constant("EVENT_ON_TRIGGER", baseTouchComponent::EVENT_ON_TRIGGER);
	def.add_static_constant("EVENT_ON_TRIGGER_INITIALIZED", baseTouchComponent::EVENT_ON_TRIGGER_INITIALIZED);

	def.add_static_constant("TRIGGER_FLAG_NONE", math::to_integral(BaseTouchComponent::TriggerFlags::None));
	def.add_static_constant("TRIGGER_FLAG_BIT_PLAYERS", math::to_integral(BaseTouchComponent::TriggerFlags::Players));
	def.add_static_constant("TRIGGER_FLAG_BIT_NPCS", math::to_integral(BaseTouchComponent::TriggerFlags::NPCs));
	def.add_static_constant("TRIGGER_FLAG_BIT_PHYSICS", math::to_integral(BaseTouchComponent::TriggerFlags::Physics));
	def.add_static_constant("TRIGGER_FLAG_EVERYTHING", math::to_integral(BaseTouchComponent::TriggerFlags::Everything));
}

void pragma::LuaCore::base_trigger_gravity_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseEntityTriggerGravityComponent>("BaseEntityTriggerGravityComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_flashlight_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFlashlightComponent>("BaseFlashlightComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_flex_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseFlexComponent>("BaseFlexComponent");
	def.add_static_constant("EVENT_ON_FLEX_CONTROLLER_CHANGED", baseFlexComponent::EVENT_ON_FLEX_CONTROLLER_CHANGED);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_skybox_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseSkyboxComponent>("BaseSkyboxComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetSkyAngles", &BaseSkyboxComponent::SetSkyAngles);
	def.def("GetSkyAngles", &BaseSkyboxComponent::GetSkyAngles, luabind::copy_policy<0> {});
}

void pragma::LuaCore::base_world_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseWorldComponent>("BaseWorldComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

namespace Lua::AI {
	void GetMoveSpeed(lua::State *l, pragma::BaseAIComponent &hNPC, uint32_t animId) { PushNumber(l, hNPC.GetMoveSpeed(animId)); }

	void SetLookTarget(lua::State *l, pragma::BaseAIComponent &hNPC, const Vector3 &tgt, float t) { hNPC.SetLookTarget(tgt, t); }

	void SetLookTarget(lua::State *l, pragma::BaseAIComponent &hNPC, pragma::ecs::BaseEntity &ent, float t) { hNPC.SetLookTarget(ent, t); }
};
void pragma::LuaCore::base_ai_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseAIComponent>("BaseAIComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetMoveSpeed", static_cast<void (BaseAIComponent::*)(int32_t, float)>(&BaseAIComponent::SetMoveSpeed));
	def.def("SetMoveSpeed", static_cast<void (BaseAIComponent::*)(const std::string &, float)>(&BaseAIComponent::SetMoveSpeed));
	def.def("GetMoveSpeed", static_cast<void (*)(lua::State *, BaseAIComponent &, const std::string &)>([](lua::State *l, BaseAIComponent &hNPC, const std::string &anim) {
		auto &mdl = hNPC.GetEntity().GetModel();
		if(mdl == nullptr)
			return;
		auto animId = mdl->LookupAnimation(anim);
		if(animId < 0)
			return;
		Lua::AI::GetMoveSpeed(l, hNPC, animId);
	}));
	def.def("GetMoveSpeed", static_cast<void (*)(lua::State *, BaseAIComponent &, uint32_t)>([](lua::State *l, BaseAIComponent &hNPC, uint32_t animId) { Lua::AI::GetMoveSpeed(l, hNPC, animId); }));
	def.def("ClearMoveSpeed", static_cast<void (BaseAIComponent::*)(int32_t)>(&BaseAIComponent::ClearMoveSpeed));
	def.def("ClearMoveSpeed", static_cast<void (BaseAIComponent::*)(const std::string &)>(&BaseAIComponent::ClearMoveSpeed));
	def.def("ClearLookTarget", &BaseAIComponent::ClearLookTarget);
	def.def("SetLookTarget", static_cast<void (*)(lua::State *, BaseAIComponent &, const Vector3 &, float)>([](lua::State *l, BaseAIComponent &hNPC, const Vector3 &tgt, float t) { Lua::AI::SetLookTarget(l, hNPC, tgt, t); }));
	def.def("SetLookTarget", static_cast<void (*)(lua::State *, BaseAIComponent &, const Vector3 &)>([](lua::State *l, BaseAIComponent &hNPC, const Vector3 &tgt) { Lua::AI::SetLookTarget(l, hNPC, tgt, std::numeric_limits<float>::max()); }));
	def.def("SetLookTarget", static_cast<void (*)(lua::State *, BaseAIComponent &, ecs::BaseEntity &, float)>([](lua::State *l, BaseAIComponent &hNPC, ecs::BaseEntity &ent, float t) { Lua::AI::SetLookTarget(l, hNPC, ent, t); }));
	def.def("SetLookTarget", static_cast<void (*)(lua::State *, BaseAIComponent &, ecs::BaseEntity &)>([](lua::State *l, BaseAIComponent &hNPC, ecs::BaseEntity &ent) { Lua::AI::SetLookTarget(l, hNPC, ent, std::numeric_limits<float>::max()); }));
	def.def("GetLookTarget", &BaseAIComponent::GetLookTarget);
}

void pragma::LuaCore::base_character_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseCharacterComponent>("BaseCharacterComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	// Actor
	def.def("GetFrozenProperty", &BaseCharacterComponent::GetFrozenProperty);
	def.add_static_constant("EVENT_ON_KILLED", baseActorComponent::EVENT_ON_KILLED);
	def.add_static_constant("EVENT_ON_RESPAWN", baseActorComponent::EVENT_ON_RESPAWN);
	def.add_static_constant("EVENT_ON_DEATH", baseActorComponent::EVENT_ON_DEATH);

	// Character
	def.def("IsAlive", &BaseCharacterComponent::IsAlive);
	def.def("IsDead", &BaseCharacterComponent::IsDead);
	def.def("IsFrozen", &BaseCharacterComponent::IsFrozen);
	def.def("SetFrozen", &BaseCharacterComponent::SetFrozen);
	def.def("Kill", &BaseCharacterComponent::Kill);
	def.def("GetOrientationAxesRotation", &BaseCharacterComponent::GetOrientationAxesRotation, luabind::copy_policy<0> {});
	def.def("GetShootPos", &BaseCharacterComponent::GetShootPosition);
	def.def("GetSlopeLimit", &BaseCharacterComponent::GetSlopeLimit);
	def.def("GetStepOffset", &BaseCharacterComponent::GetStepOffset);
	def.def("GetTurnSpeed", &BaseCharacterComponent::GetTurnSpeed);
	def.def("GetViewAngles", &BaseCharacterComponent::GetViewAngles);
	def.def("GetViewForward", &BaseCharacterComponent::GetViewForward);
	def.def("GetViewRotation", static_cast<const Quat &(BaseCharacterComponent::*)() const>(&BaseCharacterComponent::GetViewOrientation), luabind::copy_policy<0> {});
	def.def("GetViewRight", &BaseCharacterComponent::GetViewRight);
	def.def("GetViewUp", &BaseCharacterComponent::GetViewUp);
	def.def("Ragdolize", &BaseCharacterComponent::Ragdolize);
	def.def("SetSlopeLimit", static_cast<void (*)(lua::State *, BaseCharacterComponent &, float)>([](lua::State *l, BaseCharacterComponent &hEntity, float slopeLimit) { hEntity.SetSlopeLimit(math::deg_to_rad(slopeLimit)); }));
	def.def("SetStepOffset", &BaseCharacterComponent::SetStepOffset);
	def.def("SetTurnSpeed", &BaseCharacterComponent::SetTurnSpeed);
	def.def("SetViewAngles", &BaseCharacterComponent::SetViewAngles);
	def.def("SetViewRotation", &BaseCharacterComponent::SetViewOrientation);
	def.def("NormalizeViewRotation", static_cast<void (BaseCharacterComponent::*)(Quat &)>(&BaseCharacterComponent::NormalizeViewOrientation));
	def.def("NormalizeViewRotation", static_cast<const Quat &(BaseCharacterComponent::*)()>(&BaseCharacterComponent::NormalizeViewOrientation), luabind::copy_policy<0> {});
	def.def("GetHitboxPhysicsObject", &BaseCharacterComponent::GetHitboxPhysicsObject);
	def.def("GetWeapons", static_cast<void (*)(lua::State *, BaseCharacterComponent &)>([](lua::State *l, BaseCharacterComponent &hEnt) {
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
	def.def("GetWeapons", static_cast<void (*)(lua::State *, BaseCharacterComponent &, const std::string &)>([](lua::State *l, BaseCharacterComponent &hEnt, const std::string &className) {
		auto weapons = hEnt.GetWeapons(className);
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto it = weapons.begin(); it != weapons.end(); ++it) {
			Lua::PushInt(l, idx++);
			(*it)->GetLuaObject().push(l);
			Lua::SetTableValue(l, t);
		}
	}));
	def.def("GetWeaponCount", static_cast<size_t (*)(lua::State *, BaseCharacterComponent &)>([](lua::State *l, BaseCharacterComponent &hEnt) -> size_t { return hEnt.GetWeapons().size(); }));
	def.def("GetActiveWeapon", &BaseCharacterComponent::GetActiveWeapon);
	def.def("HasWeapon", &BaseCharacterComponent::HasWeapon);
	def.def("GetAimRayData", static_cast<void (*)(lua::State *, BaseCharacterComponent &)>([](lua::State *l, BaseCharacterComponent &hEnt) { Lua::Push<physics::TraceData>(l, hEnt.GetAimTraceData()); }));
	def.def("GetAimRayData", static_cast<void (*)(lua::State *, BaseCharacterComponent &, float)>([](lua::State *l, BaseCharacterComponent &hEnt, float maxDist) { Lua::Push<physics::TraceData>(l, hEnt.GetAimTraceData(maxDist)); }));
	def.def("FootStep", &BaseCharacterComponent::FootStep);
	def.def("IsMoving", &BaseCharacterComponent::IsMoving);
	def.def("SetNeckControllers", &BaseCharacterComponent::SetNeckControllers);
	def.def("GetNeckYawController", &BaseCharacterComponent::GetNeckYawBlendController);
	def.def("GetNeckPitchController", &BaseCharacterComponent::GetNeckPitchBlendController);
	def.def("SetMoveController", static_cast<void (BaseCharacterComponent::*)(const std::string &)>(&BaseCharacterComponent::SetMoveController));
	def.def("SetMoveController", static_cast<void (BaseCharacterComponent::*)(const std::string &, const std::string &)>(&BaseCharacterComponent::SetMoveController));
	def.def("GetMoveController",
	  static_cast<luabind::mult<int32_t, int32_t> (*)(lua::State *, BaseCharacterComponent &)>([](lua::State *l, BaseCharacterComponent &hEnt) -> luabind::mult<int32_t, int32_t> { return {l, hEnt.GetMoveController(), hEnt.GetMoveControllerY()}; }));

	def.def("SetOrientation", &BaseCharacterComponent::SetCharacterOrientation);
	def.def("GetLocalOrientationAngles", &BaseCharacterComponent::GetLocalOrientationAngles);
	def.def("GetLocalOrientationRotation", &BaseCharacterComponent::GetLocalOrientationRotation);
	def.def("GetLocalOrientationViewAngles", &BaseCharacterComponent::GetLocalOrientationViewAngles);
	def.def("GetLocalOrientationViewRotation", &BaseCharacterComponent::GetLocalOrientationViewRotation);

	def.def("WorldToLocalOrientation", static_cast<Quat (BaseCharacterComponent::*)(const Quat &)>(&BaseCharacterComponent::WorldToLocalOrientation));
	def.def("WorldToLocalOrientation", static_cast<EulerAngles (BaseCharacterComponent::*)(const EulerAngles &)>(&BaseCharacterComponent::WorldToLocalOrientation));
	def.def("LocalOrientationToWorld", static_cast<Quat (BaseCharacterComponent::*)(const Quat &)>(&BaseCharacterComponent::LocalOrientationToWorld));
	def.def("LocalOrientationToWorld", static_cast<EulerAngles (BaseCharacterComponent::*)(const EulerAngles &)>(&BaseCharacterComponent::LocalOrientationToWorld));

	def.def("GetAmmoCount", static_cast<uint16_t (BaseCharacterComponent::*)(const std::string &) const>(&BaseCharacterComponent::GetAmmoCount));
	def.def("GetAmmoCount", static_cast<uint16_t (BaseCharacterComponent::*)(uint32_t) const>(&BaseCharacterComponent::GetAmmoCount));
	def.def("SetAmmoCount", static_cast<void (BaseCharacterComponent::*)(const std::string &, uint16_t)>(&BaseCharacterComponent::SetAmmoCount));
	def.def("SetAmmoCount", static_cast<void (BaseCharacterComponent::*)(uint32_t, uint16_t)>(&BaseCharacterComponent::SetAmmoCount));

	def.def("AddAmmo", static_cast<void (BaseCharacterComponent::*)(const std::string &, int16_t)>(&BaseCharacterComponent::AddAmmo));
	def.def("AddAmmo", static_cast<void (BaseCharacterComponent::*)(uint32_t, int16_t)>(&BaseCharacterComponent::AddAmmo));
	def.def("RemoveAmmo", static_cast<void (BaseCharacterComponent::*)(const std::string &, int16_t)>(&BaseCharacterComponent::RemoveAmmo));
	def.def("RemoveAmmo", static_cast<void (BaseCharacterComponent::*)(uint32_t, int16_t)>(&BaseCharacterComponent::RemoveAmmo));
	def.def("GetSlopeLimitProperty", &BaseCharacterComponent::GetSlopeLimitProperty);
	def.def("GetStepOffsetProperty", &BaseCharacterComponent::GetStepOffsetProperty);
	def.def("GetJumpPower", &BaseCharacterComponent::GetJumpPower);
	def.def("SetJumpPower", &BaseCharacterComponent::SetJumpPower);
	def.def("Jump", static_cast<bool (BaseCharacterComponent::*)()>(&BaseCharacterComponent::Jump));
	def.def("Jump", static_cast<bool (BaseCharacterComponent::*)(const Vector3 &)>(&BaseCharacterComponent::Jump));
	def.def("GetJumpPowerProperty", &BaseCharacterComponent::GetJumpPowerProperty);

	def.add_static_constant("EVENT_ON_FOOT_STEP", baseCharacterComponent::EVENT_ON_FOOT_STEP);
	def.add_static_constant("EVENT_ON_CHARACTER_ORIENTATION_CHANGED", baseCharacterComponent::EVENT_ON_CHARACTER_ORIENTATION_CHANGED);
	def.add_static_constant("EVENT_ON_DEPLOY_WEAPON", baseCharacterComponent::EVENT_ON_DEPLOY_WEAPON);
	def.add_static_constant("EVENT_ON_SET_ACTIVE_WEAPON", baseCharacterComponent::EVENT_ON_SET_ACTIVE_WEAPON);
	def.add_static_constant("EVENT_PLAY_FOOTSTEP_SOUND", baseCharacterComponent::EVENT_PLAY_FOOTSTEP_SOUND);
	def.add_static_constant("EVENT_IS_MOVING", baseCharacterComponent::EVENT_IS_MOVING);
	def.add_static_constant("EVENT_HANDLE_VIEW_ROTATION", baseCharacterComponent::EVENT_HANDLE_VIEW_ROTATION);
	def.add_static_constant("EVENT_ON_JUMP", baseCharacterComponent::EVENT_ON_JUMP);

	def.add_static_constant("FOOT_LEFT", math::to_integral(BaseCharacterComponent::FootType::Left));
	def.add_static_constant("FOOT_RIGHT", math::to_integral(BaseCharacterComponent::FootType::Right));
}

void pragma::LuaCore::base_vehicle_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseVehicleComponent>("BaseVehicleComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetSpeedKmh", &BaseVehicleComponent::GetSpeedKmh);
	def.def("GetSteeringFactor", &BaseVehicleComponent::GetSteeringFactor);
	def.def("GetSteeringWheel", &BaseVehicleComponent::GetSteeringWheel);
	def.def("HasDriver", &BaseVehicleComponent::HasDriver);
	def.def("GetDriver", &BaseVehicleComponent::GetDriver);
	def.def("SetDriver", &BaseVehicleComponent::SetDriver);
	def.def("ClearDriver", &BaseVehicleComponent::ClearDriver);
	def.def("SetupSteeringWheel", &BaseVehicleComponent::SetupSteeringWheel);
	def.def("GetPhysicsVehicle", static_cast<physics::IVehicle *(BaseVehicleComponent::*)()>(&BaseVehicleComponent::GetPhysicsVehicle));
	def.def("SetupPhysics", &BaseVehicleComponent::SetupVehicle);
	def.add_static_constant("EVENT_ON_DRIVER_ENTERED", baseVehicleComponent::EVENT_ON_DRIVER_ENTERED);
	def.add_static_constant("EVENT_ON_DRIVER_EXITED", baseVehicleComponent::EVENT_ON_DRIVER_EXITED);
}

namespace Lua::Weapon {
	void PrimaryAttack(lua::State *l, pragma::BaseWeaponComponent &hEnt, bool bOnce)
	{
		auto &wep = hEnt;
		wep.PrimaryAttack();
		if(bOnce == true)
			wep.EndPrimaryAttack();
	}

	void SecondaryAttack(lua::State *l, pragma::BaseWeaponComponent &hEnt, bool bOnce)
	{
		auto &wep = hEnt;
		wep.SecondaryAttack();
		if(bOnce == true)
			wep.EndSecondaryAttack();
	}
};
void pragma::LuaCore::base_weapon_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseWeaponComponent>("BaseWeaponComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetNextPrimaryAttack", &BaseWeaponComponent::SetNextPrimaryAttack);
	def.def("SetNextSecondaryAttack", &BaseWeaponComponent::SetNextSecondaryAttack);
	def.def("SetNextAttack", &BaseWeaponComponent::SetNextAttack);
	def.def("SetAutomaticPrimary", &BaseWeaponComponent::SetAutomaticPrimary);
	def.def("SetAutomaticSecondary", &BaseWeaponComponent::SetAutomaticSecondary);
	def.def("IsAutomaticPrimary", &BaseWeaponComponent::IsAutomaticPrimary);
	def.def("IsAutomaticSecondary", &BaseWeaponComponent::IsAutomaticSecondary);
	def.def("IsDeployed", &BaseWeaponComponent::IsDeployed);
	def.def("GetPrimaryAmmoType", &BaseWeaponComponent::GetPrimaryAmmoType);
	def.def("GetPrimaryAmmoTypeProperty", &BaseWeaponComponent::GetPrimaryAmmoTypeProperty);
	def.def("GetSecondaryAmmoType", &BaseWeaponComponent::GetSecondaryAmmoType);
	def.def("GetSecondaryAmmoTypeProperty", &BaseWeaponComponent::GetSecondaryAmmoTypeProperty);
	def.def("HasPrimaryAmmo", &BaseWeaponComponent::HasPrimaryAmmo);
	def.def("HasSecondaryAmmo", &BaseWeaponComponent::HasSecondaryAmmo);
	def.def("IsPrimaryClipEmpty", &BaseWeaponComponent::IsPrimaryClipEmpty);
	def.def("IsSecondaryClipEmpty", &BaseWeaponComponent::IsSecondaryClipEmpty);
	def.def("HasAmmo", &BaseWeaponComponent::HasAmmo);
	def.def("GetPrimaryClipSize", &BaseWeaponComponent::GetPrimaryClipSize);
	def.def("GetPrimaryClipSizeProperty", &BaseWeaponComponent::GetPrimaryClipSizeProperty);
	def.def("GetSecondaryClipSize", &BaseWeaponComponent::GetSecondaryClipSize);
	def.def("GetSecondaryClipSizeProperty", &BaseWeaponComponent::GetSecondaryClipSizeProperty);
	def.def("GetMaxPrimaryClipSize", &BaseWeaponComponent::GetMaxPrimaryClipSize);
	def.def("GetMaxPrimaryClipSizeProperty", &BaseWeaponComponent::GetMaxPrimaryClipSizeProperty);
	def.def("GetMaxSecondaryClipSize", &BaseWeaponComponent::GetMaxSecondaryClipSize);
	def.def("GetMaxSecondaryClipSizeProperty", &BaseWeaponComponent::GetMaxSecondaryClipSizeProperty);
	def.def("PrimaryAttack", static_cast<void (*)(lua::State *, BaseWeaponComponent &, bool)>([](lua::State *l, BaseWeaponComponent &hEnt, bool bOnce) { Lua::Weapon::PrimaryAttack(l, hEnt, bOnce); }));
	def.def("PrimaryAttack", static_cast<void (*)(lua::State *, BaseWeaponComponent &)>([](lua::State *l, BaseWeaponComponent &hEnt) { Lua::Weapon::PrimaryAttack(l, hEnt, false); }));
	def.def("SecondaryAttack", static_cast<void (*)(lua::State *, BaseWeaponComponent &, bool)>([](lua::State *l, BaseWeaponComponent &hEnt, bool bOnce) { Lua::Weapon::SecondaryAttack(l, hEnt, bOnce); }));
	def.def("SecondaryAttack", static_cast<void (*)(lua::State *, BaseWeaponComponent &)>([](lua::State *l, BaseWeaponComponent &hEnt) { Lua::Weapon::SecondaryAttack(l, hEnt, false); }));
	def.def("TertiaryAttack", &BaseWeaponComponent::TertiaryAttack);
	def.def("Attack4", &BaseWeaponComponent::Attack4);
	def.def("Reload", &BaseWeaponComponent::Reload);
	def.def("Deploy", &BaseWeaponComponent::Deploy);
	def.def("Holster", &BaseWeaponComponent::Holster);
	def.def("EndAttack", &BaseWeaponComponent::EndAttack);
	def.def("EndPrimaryAttack", &BaseWeaponComponent::EndPrimaryAttack);
	def.def("EndSecondaryAttack", &BaseWeaponComponent::EndSecondaryAttack);

	def.add_static_constant("EVENT_ON_DEPLOY", baseWeaponComponent::EVENT_ON_DEPLOY);
	def.add_static_constant("EVENT_ON_HOLSTER", baseWeaponComponent::EVENT_ON_HOLSTER);
	def.add_static_constant("EVENT_ON_PRIMARY_ATTACK", baseWeaponComponent::EVENT_ON_PRIMARY_ATTACK);
	def.add_static_constant("EVENT_ON_SECONDARY_ATTACK", baseWeaponComponent::EVENT_ON_SECONDARY_ATTACK);
	def.add_static_constant("EVENT_ON_TERTIARY_ATTACK", baseWeaponComponent::EVENT_ON_TERTIARY_ATTACK);
	def.add_static_constant("EVENT_ON_ATTACK4", baseWeaponComponent::EVENT_ON_ATTACK4);
	def.add_static_constant("EVENT_ON_END_PRIMARY_ATTACK", baseWeaponComponent::EVENT_ON_END_PRIMARY_ATTACK);
	def.add_static_constant("EVENT_ON_END_SECONDARY_ATTACK", baseWeaponComponent::EVENT_ON_END_SECONDARY_ATTACK);
	def.add_static_constant("EVENT_ON_RELOAD", baseWeaponComponent::EVENT_ON_RELOAD);
	def.add_static_constant("EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED", baseWeaponComponent::EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED);
	def.add_static_constant("EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED", baseWeaponComponent::EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED);
}

void pragma::LuaCore::base_player_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePlayerComponent>("BasePlayerComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetTimeConnected", &BasePlayerComponent::TimeConnected);
	def.def("IsKeyDown", &BasePlayerComponent::IsKeyDown);
	def.def("GetWalkSpeed", &BasePlayerComponent::GetWalkSpeed);
	def.def("GetRunSpeed", &BasePlayerComponent::GetRunSpeed);
	def.def("GetSprintSpeed", &BasePlayerComponent::GetSprintSpeed);
	def.def("SetWalkSpeed", &BasePlayerComponent::SetWalkSpeed);
	def.def("SetRunSpeed", &BasePlayerComponent::SetRunSpeed);
	def.def("SetSprintSpeed", &BasePlayerComponent::SetSprintSpeed);
	def.def("GetCrouchedWalkSpeed", &BasePlayerComponent::GetCrouchedWalkSpeed);
	def.def("SetCrouchedWalkSpeed", &BasePlayerComponent::SetCrouchedWalkSpeed);
	def.def("IsLocalPlayer", &BasePlayerComponent::IsLocalPlayer);
	def.def("GetStandHeight", &BasePlayerComponent::GetStandHeight);
	def.def("SetStandHeight", &BasePlayerComponent::SetStandHeight);
	def.def("GetCrouchHeight", &BasePlayerComponent::GetCrouchHeight);
	def.def("SetCrouchHeight", &BasePlayerComponent::SetCrouchHeight);
	def.def("GetStandEyeLevel", &BasePlayerComponent::GetStandEyeLevel);
	def.def("SetStandEyeLevel", &BasePlayerComponent::SetStandEyeLevel);
	def.def("GetCrouchEyeLevel", &BasePlayerComponent::GetCrouchEyeLevel);
	def.def("SetCrouchEyeLevel", &BasePlayerComponent::SetCrouchEyeLevel);
	def.def("SetFlashlightEnabled", &BasePlayerComponent::SetFlashlight);
	def.def("ToggleFlashlight", &BasePlayerComponent::ToggleFlashlight);
	def.def("IsFlashlightEnabled", &BasePlayerComponent::IsFlashlightOn);
	def.def("GetViewPos", &BasePlayerComponent::GetViewPos, luabind::copy_policy<0> {});

	def.def("ApplyViewRotationOffset", &BasePlayerComponent::ApplyViewRotationOffset);
	def.def("ApplyViewRotationOffset", static_cast<void (*)(lua::State *, BasePlayerComponent &, EulerAngles &)>([](lua::State *l, BasePlayerComponent &hPl, EulerAngles &ang) { hPl.ApplyViewRotationOffset(ang); }));
	def.def("PrintMessage", &BasePlayerComponent::PrintMessage);

	def.add_static_constant("MESSAGE_TYPE_CONSOLE", math::to_integral(console::MESSAGE::PRINTCONSOLE));
	def.add_static_constant("MESSAGE_TYPE_CHAT", math::to_integral(console::MESSAGE::PRINTCHAT));
}

void pragma::LuaCore::base_observer_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseObserverComponent>("BaseObserverComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetObserverMode", &BaseObserverComponent::SetObserverMode);
	def.def("GetObserverMode", &BaseObserverComponent::GetObserverMode);
	def.def("GetObserverModeProperty", &BaseObserverComponent::GetObserverModeProperty);
	def.def("SetObserverTarget", &BaseObserverComponent::SetObserverTarget);
	def.def("GetObserverTarget", &BaseObserverComponent::GetObserverTarget);

	def.add_static_constant("EVENT_ON_OBSERVATION_MODE_CHANGED", baseObserverComponent::EVENT_ON_OBSERVATION_MODE_CHANGED);

	// Enums
	def.add_static_constant("OBSERVERMODE_NONE", math::to_integral(ObserverMode::None));
	def.add_static_constant("OBSERVERMODE_FIRSTPERSON", math::to_integral(ObserverMode::FirstPerson));
	def.add_static_constant("OBSERVERMODE_THIRDPERSON", math::to_integral(ObserverMode::ThirdPerson));
	def.add_static_constant("OBSERVERMODE_SHOULDER", math::to_integral(ObserverMode::Shoulder));
	def.add_static_constant("OBSERVERMODE_ROAMING", math::to_integral(ObserverMode::Roaming));
	static_assert(math::to_integral(ObserverMode::Count) == 5, "Update this list when new modes are added!");
}

void pragma::LuaCore::base_gamemode_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseGamemodeComponent>("BaseGamemodeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetName", &BaseGamemodeComponent::GetName);
	def.def("GetIdentifier", &BaseGamemodeComponent::GetIdentifier);
	def.def("GetComponentName", &BaseGamemodeComponent::GetComponentName);
	def.def("GetAuthor", &BaseGamemodeComponent::GetAuthor);
	def.def("GetGamemodeVersion", &BaseGamemodeComponent::GetGamemodeVersion);

	// Enums
	def.add_static_constant("EVENT_ON_PLAYER_DEATH", baseGamemodeComponent::EVENT_ON_PLAYER_DEATH);
	def.add_static_constant("EVENT_ON_PLAYER_SPAWNED", baseGamemodeComponent::EVENT_ON_PLAYER_SPAWNED);
	def.add_static_constant("EVENT_ON_PLAYER_DROPPED", baseGamemodeComponent::EVENT_ON_PLAYER_DROPPED);
	def.add_static_constant("EVENT_ON_PLAYER_READY", baseGamemodeComponent::EVENT_ON_PLAYER_READY);
	def.add_static_constant("EVENT_ON_PLAYER_JOINED", baseGamemodeComponent::EVENT_ON_PLAYER_JOINED);
	def.add_static_constant("EVENT_ON_GAME_INITIALIZED", baseGamemodeComponent::EVENT_ON_GAME_INITIALIZED);
	def.add_static_constant("EVENT_ON_MAP_INITIALIZED", baseGamemodeComponent::EVENT_ON_MAP_INITIALIZED);
	def.add_static_constant("EVENT_ON_GAME_READY", baseGamemodeComponent::EVENT_ON_GAME_READY);
}

void pragma::LuaCore::base_generic_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseGenericComponent>("BaseGenericComponent");
	def.add_static_constant("EVENT_ON_COMPONENT_ADDED", baseGenericComponent::EVENT_ON_ENTITY_COMPONENT_ADDED);
	def.add_static_constant("EVENT_ON_COMPONENT_REMOVED", baseGenericComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED);
	def.add_static_constant("EVENT_ON_MEMBERS_CHANGED", baseGenericComponent::EVENT_ON_MEMBERS_CHANGED);
	def.add_static_constant("EVENT_ON_ACTIVE_STATE_CHANGED", baseEntityComponent::EVENT_ON_ACTIVE_STATE_CHANGED);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_info_landmark_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseInfoLandmarkComponent>("BaseInfoLandmarkComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_io_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseIOComponent>("BaseIOComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("Input",
	  static_cast<void (*)(lua::State *, BaseIOComponent &, std::string, ecs::BaseEntity &, ecs::BaseEntity &, std::string)>(
	    [](lua::State *l, BaseIOComponent &hIo, std::string input, ecs::BaseEntity &activator, ecs::BaseEntity &caller, std::string data) { hIo.Input(input, &activator, &caller, data); }));
	def.def("Input", static_cast<void (*)(lua::State *, BaseIOComponent &, std::string, ecs::BaseEntity &, std::string)>([](lua::State *l, BaseIOComponent &hIo, std::string input, ecs::BaseEntity &activator, std::string data) {
		hIo.Input(input, &activator, nullptr, data);
	}));
	def.def("Input", static_cast<void (*)(lua::State *, BaseIOComponent &, std::string, std::string)>([](lua::State *l, BaseIOComponent &hIo, std::string input, std::string data) { hIo.Input(input, nullptr, nullptr, data); }));
	def.def("Input", static_cast<void (*)(lua::State *, BaseIOComponent &, std::string)>([](lua::State *l, BaseIOComponent &hIo, std::string input) { hIo.Input(input, nullptr, nullptr, ""); }));
	def.def("StoreOutput",
	  static_cast<void (*)(lua::State *, BaseIOComponent &, const std::string &, const std::string &, const std::string &, const std::string &, float, int32_t)>(
	    [](lua::State *l, BaseIOComponent &hIo, const std::string &name, const std::string &entities, const std::string &input, const std::string &param, float delay, int32_t times) { hIo.StoreOutput(name, entities, input, param, delay, times); }));
	def.def("StoreOutput",
	  static_cast<void (*)(lua::State *, BaseIOComponent &, const std::string &, const std::string &, const std::string &, const std::string &, float)>(
	    [](lua::State *l, BaseIOComponent &hIo, const std::string &name, const std::string &entities, const std::string &input, const std::string &param, float delay) { hIo.StoreOutput(name, entities, input, param, delay); }));
	def.def("StoreOutput",
	  static_cast<void (*)(lua::State *, BaseIOComponent &, const std::string &, const std::string &, const std::string &, const std::string &)>(
	    [](lua::State *l, BaseIOComponent &hIo, const std::string &name, const std::string &entities, const std::string &input, const std::string &param) { hIo.StoreOutput(name, entities, input, param); }));
	def.def("StoreOutput", static_cast<void (*)(lua::State *, BaseIOComponent &, const std::string &, const std::string &)>([](lua::State *l, BaseIOComponent &hIo, const std::string &name, const std::string &info) { hIo.StoreOutput(name, info); }));
	def.def("FireOutput", static_cast<void (*)(lua::State *, BaseIOComponent &, const std::string &, ecs::BaseEntity &)>([](lua::State *l, BaseIOComponent &hIo, const std::string &name, ecs::BaseEntity &ent) { hIo.TriggerOutput(name, &ent); }));
	def.def("FireOutput", &BaseIOComponent::TriggerOutput);
	def.add_static_constant("EVENT_HANDLE_INPUT", baseIOComponent::EVENT_HANDLE_INPUT);

	def.add_static_constant("IO_FLAG_NONE", math::to_integral(BaseIOComponent::IoFlags::None));
	def.add_static_constant("IO_FLAG_BIT_FORCE_DELAYED_FIRE", math::to_integral(BaseIOComponent::IoFlags::ForceDelayedFire));
}

void pragma::LuaCore::base_model_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseModelComponent>("BaseModelComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetModel", static_cast<void (*)(lua::State *, BaseModelComponent &)>([](lua::State *l, BaseModelComponent &hModel) { hModel.SetModel(std::shared_ptr<asset::Model> {nullptr}); }));
	def.def("SetModel", static_cast<void (BaseModelComponent::*)(const std::string &)>(&BaseModelComponent::SetModel));
	def.def("SetModel", static_cast<void (BaseModelComponent::*)(const std::shared_ptr<asset::Model> &)>(&BaseModelComponent::SetModel));
	def.def("SetSkin", &BaseModelComponent::SetSkin);
	def.def("GetSkin", &BaseModelComponent::GetSkin);
	def.def("GetSkinProperty", &BaseModelComponent::GetSkinProperty);
	def.def("SetRandomSkin", static_cast<void (*)(lua::State *, BaseModelComponent &)>([](lua::State *l, BaseModelComponent &hModel) {
		auto &mdl = hModel.GetModel();
		if(mdl == nullptr)
			return;
		hModel.SetSkin(math::random(0, math::max(mdl->GetTextureGroups().size(), static_cast<size_t>(1)) - 1));
	}));
	def.def("GetModel", static_cast<luabind::optional<asset::Model> (*)(lua::State *, BaseModelComponent &)>([](lua::State *l, BaseModelComponent &hModel) -> luabind::optional<asset::Model> {
		auto mdl = hModel.GetModel();
		if(mdl == nullptr)
			return Lua::nil;
		return luabind::object {l, mdl};
	}));
	def.def("GetBodyGroup", &BaseModelComponent::GetBodyGroup);
	def.def("GetBodyGroups", &BaseModelComponent::GetBodyGroups);
	def.def("SetBodyGroup", static_cast<bool (BaseModelComponent::*)(uint32_t, uint32_t)>(&BaseModelComponent::SetBodyGroup));
	def.def("SetBodyGroup", static_cast<void (BaseModelComponent::*)(const std::string &, uint32_t)>(&BaseModelComponent::SetBodyGroup));
	def.def("SetBodyGroups", static_cast<void (*)(lua::State *, BaseModelComponent &, luabind::table<>)>([](lua::State *l, BaseModelComponent &hModel, luabind::table<> t) {
		auto bodyGroups = Lua::table_to_vector<uint32_t>(l, t, 2);
		for(auto i = decltype(bodyGroups.size()) {0u}; i < bodyGroups.size(); ++i)
			hModel.SetBodyGroup(i, bodyGroups[i]);
	}));
	def.def("LookupAnimation", &BaseModelComponent::LookupAnimation);
	def.def("SetModelName", &BaseModelComponent::SetModelName);
	def.def("GetModelName", &BaseModelComponent::GetModelName);
	def.def("LookupBlendController", &BaseModelComponent::LookupBlendController);
	def.def("LookupAttachment", &BaseModelComponent::LookupAttachment);
	def.def("GetHitboxCount", &BaseModelComponent::GetHitboxCount);
	def.def(
	  "GetHitboxBounds", +[](lua::State *l, BaseModelComponent &hEnt, uint32_t boneId) -> std::optional<std::tuple<Vector3, Vector3, Vector3, Quat>> {
		  Vector3 min, max, origin;
		  auto rot = uquat::identity();
		  if(!hEnt.GetHitboxBounds(boneId, min, max, origin, rot))
			  return {};
		  return std::tuple<Vector3, Vector3, Vector3, Quat> {min, max, origin, rot};
	  });
	def.def(
	  "GetHitboxBounds", +[](lua::State *l, BaseModelComponent &hEnt, uint32_t boneId, math::CoordinateSpace space) -> std::optional<std::tuple<Vector3, Vector3, Vector3, Quat>> {
		  Vector3 min, max, origin;
		  auto rot = uquat::identity();
		  if(!hEnt.GetHitboxBounds(boneId, min, max, origin, rot, space))
			  return {};
		  return std::tuple<Vector3, Vector3, Vector3, Quat> {min, max, origin, rot};
	  });
	def.def("LookupBone", &BaseModelComponent::LookupBone);
	def.def("GetAttachmentTransform", static_cast<luabind::optional<luabind::mult<Vector3, Quat>> (*)(lua::State *, BaseModelComponent &, std::string)>([](lua::State *l, BaseModelComponent &hEnt, std::string attachment) -> luabind::optional<luabind::mult<Vector3, Quat>> {
		Vector3 offset(0, 0, 0);
		auto rot = uquat::identity();
		if(hEnt.GetAttachment(attachment, &offset, &rot) == false)
			return Lua::nil;
		return luabind::mult<Vector3, Quat> {l, offset, rot};
	}));
	def.def("GetAttachmentTransform", static_cast<luabind::optional<luabind::mult<Vector3, Quat>> (*)(lua::State *, BaseModelComponent &, int)>([](lua::State *l, BaseModelComponent &hEnt, int attachment) -> luabind::optional<luabind::mult<Vector3, Quat>> {
		Vector3 offset(0, 0, 0);
		auto rot = uquat::identity();
		if(hEnt.GetAttachment(attachment, &offset, &rot) == false)
			return Lua::nil;
		return luabind::mult<Vector3, Quat> {l, offset, rot};
	}));

	def.add_static_constant("EVENT_ON_MODEL_CHANGED", baseModelComponent::EVENT_ON_MODEL_CHANGED);
	def.add_static_constant("EVENT_ON_MODEL_MATERIALS_LOADED", baseModelComponent::EVENT_ON_MODEL_MATERIALS_LOADED);
}

void pragma::LuaCore::base_time_scale_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseTimeScaleComponent>("BaseTimeScaleComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("GetTimeScale", &BaseTimeScaleComponent::GetTimeScale);
	def.def("SetTimeScale", &BaseTimeScaleComponent::GetTimeScale);
	def.def("GetEffectiveTimeScale", &BaseTimeScaleComponent::GetEffectiveTimeScale);
}

void pragma::LuaCore::base_ownable_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseOwnableComponent>("BaseOwnableComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetOwner", static_cast<void (*)(lua::State *, BaseOwnableComponent &, ecs::BaseEntity *)>([](lua::State *l, BaseOwnableComponent &hEnt, ecs::BaseEntity *owner) {
		auto &ownerComponent = hEnt;
		if(owner)
			ownerComponent.SetOwner(*owner);
		else
			ownerComponent.ClearOwner();
	}));
	def.def("SetOwner", &BaseOwnableComponent::ClearOwner);
	def.def("GetOwner", static_cast<ecs::BaseEntity *(BaseOwnableComponent::*)()>(&BaseOwnableComponent::GetOwner));
	def.add_static_constant("EVENT_ON_OWNER_CHANGED", baseOwnableComponent::EVENT_ON_OWNER_CHANGED);
}

void pragma::LuaCore::base_debug_text_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseDebugTextComponent>("BaseDebugTextComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetText", &BaseDebugTextComponent::SetText);
}

void pragma::LuaCore::base_debug_point_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseDebugPointComponent>("BaseDebugPointComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_debug_line_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseDebugLineComponent>("BaseDebugLineComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_debug_box_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseDebugBoxComponent>("BaseDebugBoxComponent");
	def.def("SetBounds", &BaseDebugBoxComponent::SetBounds);
	def.def("GetBounds", &BaseDebugBoxComponent::GetBounds);
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_debug_sphere_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseDebugSphereComponent>("BaseDebugSphereComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_debug_cone_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseDebugConeComponent>("BaseDebugConeComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_debug_cylinder_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseDebugCylinderComponent>("BaseDebugCylinderComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_debug_plane_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseDebugPlaneComponent>("BaseDebugPlaneComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
}

void pragma::LuaCore::base_point_at_target_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BasePointAtTargetComponent>("BasePointAtTargetComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("SetPointAtTarget", static_cast<void (*)(lua::State *, BasePointAtTargetComponent &, ecs::BaseEntity *)>([](lua::State *l, BasePointAtTargetComponent &hEnt, ecs::BaseEntity *target) {
		if(target)
			hEnt.SetPointAtTarget(*target);
		else
			hEnt.ClearPointAtTarget();
	}));
	def.def("SetPointAtTarget", &BasePointAtTargetComponent::ClearPointAtTarget);
	def.def("GetPointAtTarget", &BasePointAtTargetComponent::GetPointAtTarget);
}

void pragma::LuaCore::base_liquid_control_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseLiquidControlComponent>("BaseLiquidControlComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.def("CreateSplash", &BaseLiquidControlComponent::CreateSplash);
	def.def("GetStiffness", &BaseLiquidControlComponent::GetStiffness);
	def.def("SetStiffness", &BaseLiquidControlComponent::SetStiffness);
	def.def("GetPropagation", &BaseLiquidControlComponent::GetPropagation);
	def.def("SetPropagation", &BaseLiquidControlComponent::SetPropagation);
	def.def("GetWaterVelocity", &BaseLiquidControlComponent::GetLiquidVelocity, luabind::copy_policy<0> {});
	def.def("SetWaterVelocity", &BaseLiquidControlComponent::SetLiquidVelocity);
	def.def("GetDensity", &BaseLiquidControlComponent::GetDensity);
	def.def("SetDensity", &BaseLiquidControlComponent::SetDensity);
	def.def("GetLinearDragCoefficient", &BaseLiquidControlComponent::GetLinearDragCoefficient);
	def.def("SetLinearDragCoefficient", &BaseLiquidControlComponent::SetLinearDragCoefficient);
	def.def("GetTorqueDragCoefficient", &BaseLiquidControlComponent::GetTorqueDragCoefficient);
	def.def("SetTorqueDragCoefficient", &BaseLiquidControlComponent::SetTorqueDragCoefficient);
}

void pragma::LuaCore::base_liquid_surface_simulation_component::register_class(luabind::module_ &mod)
{
	auto def = Lua::create_base_entity_component_class<BaseLiquidSurfaceSimulationComponent>("BaseLiquidSurfaceSimulationComponent");
	util::ScopeGuard sgReg {[&mod, &def]() { mod[def]; }};
	def.add_static_constant("EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED", baseLiquidSurfaceSimulationComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED);
}

// --template-register-definition
