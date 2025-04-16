/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/sh_lua_component.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/libraries/ludm.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/lua/base_lua_handle_method.hpp"
#include "pragma/lua/sh_lua_component_t.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/lua/ostream_operator_alias.hpp"
#include "pragma/lua/lua_util_component.hpp"
#include "pragma/lua/types/udm.hpp"
#include "pragma/lua/types/nil_type.hpp"
#include "pragma/logging.hpp"
#include <sharedutils/scope_guard.h>
#include <sharedutils/datastream.h>
#include <sharedutils/netpacket.hpp>
#include <udm.hpp>

#define ENABLE_CUSTOM_SETTER_GETTER 0

using namespace pragma;
struct ClassMembers {
	ClassMembers(const luabind::object &classObject) : classObject {classObject} {}
	luabind::object classObject;
	std::vector<BaseLuaBaseEntityComponent::MemberInfo> memberDeclarations;
};
static std::unordered_map<lua_State *, std::vector<std::shared_ptr<ClassMembers>>> s_classMembers {};
static std::vector<std::shared_ptr<ClassMembers>> &get_class_member_list(lua_State *l)
{
	auto it = s_classMembers.find(l);
	if(it == s_classMembers.end())
		it = s_classMembers.insert(std::make_pair(l, std::vector<std::shared_ptr<ClassMembers>> {})).first;
	return it->second;
}
static ClassMembers *get_class_member_declarations(const luabind::object &classObject)
{
	auto *l = classObject.interpreter();
	auto it = s_classMembers.find(l);
	if(it == s_classMembers.end())
		return nullptr;
	auto itClass = std::find_if(it->second.begin(), it->second.end(), [&classObject](const std::shared_ptr<ClassMembers> &classMember) { return classObject == classMember->classObject; });
	return (itClass != it->second.end()) ? itClass->get() : nullptr;
}

static std::string get_member_name(const std::string &funcName)
{
	if(funcName.empty())
		return "";
	std::stringstream ss {};
	ss << std::string(1, tolower(funcName.front())) << ustring::substr(funcName, 1ull);
	return ss.str();
}
static std::string get_member_variable_name(const std::string &funcName)
{
	if(funcName.empty())
		return "";
	return "m_" + get_member_name(funcName);
}
static std::any string_to_any(Game &game, const std::string &value, util::VarType type)
{
	static_assert(umath::to_integral(util::VarType::Count) == 23);
	switch(type) {
	case util::VarType::Bool:
		return static_cast<bool>(util::to_boolean(value));
	case util::VarType::Double:
		return static_cast<double>(util::to_float(value));
	case util::VarType::Float:
		return static_cast<float>(util::to_float(value));
	case util::VarType::Int8:
		return static_cast<int8_t>(util::to_int(value));
	case util::VarType::Int16:
		return static_cast<int16_t>(util::to_int(value));
	case util::VarType::Int32:
		return static_cast<int32_t>(util::to_int(value));
	case util::VarType::Int64:
		return static_cast<int64_t>(util::to_int(value));
	case util::VarType::LongDouble:
		return static_cast<long double>(util::to_int(value));
	case util::VarType::String:
		return value;
	case util::VarType::UInt8:
		return static_cast<uint8_t>(util::to_int(value));
	case util::VarType::UInt16:
		return static_cast<uint16_t>(util::to_int(value));
	case util::VarType::UInt32:
		return static_cast<uint32_t>(util::to_int(value));
	case util::VarType::UInt64:
		return static_cast<uint64_t>(util::to_int(value));
	case util::VarType::EulerAngles:
		return EulerAngles {value};
	case util::VarType::Color:
		return Color {value};
	case util::VarType::Vector:
		return uvec::create(value);
	case util::VarType::Vector2:
		{
			Vector2 r;
			ustring::string_to_array<Float, Double>(value, reinterpret_cast<float *>(&r), atof, 2);
			return r;
		}
	case util::VarType::Vector4:
		{
			Vector4 r;
			ustring::string_to_array<Float, Double>(value, reinterpret_cast<float *>(&r), atof, 4);
			return r;
		}
	case util::VarType::Entity:
		return pragma::EntityURef {value};
	case util::VarType::Quaternion:
		{
			Quat r;
			ustring::string_to_array<Float, Double>(value, reinterpret_cast<float *>(&r), atof, 4);
			return r;
		}
	case util::VarType::Transform:
	case util::VarType::ScaledTransform:
		return {}; // Not yet supported
	}
	return {};
}
static ClassMembers *get_class_members(const luabind::object &oClass)
{
	auto &members = get_class_member_list(oClass.interpreter());
	auto it = std::find_if(members.begin(), members.end(), [&oClass](const std::shared_ptr<ClassMembers> &classMembers) { return oClass == classMembers->classObject; });
	if(it == members.end())
		return nullptr;
	return it->get();
}
static uint32_t get_class_member_index(const luabind::object &oClass)
{
	auto &members = get_class_member_list(oClass.interpreter());
	auto it = std::find_if(members.begin(), members.end(), [&oClass](const std::shared_ptr<ClassMembers> &classMembers) { return oClass == classMembers->classObject; });
	if(it == members.end())
		return std::numeric_limits<uint32_t>::max();
	return it - members.begin();
}
static std::optional<BaseLuaBaseEntityComponent::MemberIndex> find_member_info_index(const luabind::object &oClass, const std::string &memberName)
{
	auto *members = get_class_members(oClass);
	if(!members)
		return {};
	auto itMember = std::find_if(members->memberDeclarations.begin(), members->memberDeclarations.end(), [&memberName](const BaseLuaBaseEntityComponent::MemberInfo &memberInfo) { return ustring::compare<std::string_view>(memberName, memberInfo.memberName, false); });
	if(itMember == members->memberDeclarations.end())
		return {};
	return itMember - members->memberDeclarations.begin();
}
const pragma::BaseLuaBaseEntityComponent::MemberInfo *BaseLuaBaseEntityComponent::GetLuaMemberInfo(ComponentMemberInfo &memberInfo) const
{
	if(!m_classMembers) {
		m_classMembers = get_class_member_declarations(*GetClassObject());
		assert(m_classMembers);
		if(!m_classMembers) {
			std::stringstream ss;
			ss << *this;
			throw std::runtime_error {"Missing class member declaration for Lua-based entity component " + ss.str() + "!"};
			return nullptr;
		}
	}
	if(memberInfo.userIndex >= m_classMembers->memberDeclarations.size() || m_classMembers->memberDeclarations[memberInfo.userIndex].memberNameHash != memberInfo.GetNameHash()) {
		auto it = std::find_if(m_classMembers->memberDeclarations.begin(), m_classMembers->memberDeclarations.end(), [&memberInfo](const BaseLuaBaseEntityComponent::MemberInfo &luaMemberInfo) { return luaMemberInfo.memberNameHash == memberInfo.GetNameHash(); });
		assert(it != m_classMembers->memberDeclarations.end());
		if(it == m_classMembers->memberDeclarations.end()) {
			throw std::runtime_error {"Member '" + std::string {*memberInfo.GetName()} + "' is missing Lua meta data!"};
			return nullptr;
		}
		memberInfo.userIndex = it - m_classMembers->memberDeclarations.begin();
	}
	return &m_classMembers->memberDeclarations[memberInfo.userIndex];
}

template<typename T>
static void init_pose_type_meta_data(const ComponentMemberInfo &memberInfo, BaseLuaBaseEntityComponent &component, BaseLuaBaseEntityComponent::MemberInfo &info)
{
	// Lazy initialization
	auto *poseMetaData = memberInfo.FindTypeMetaData<pragma::ents::PoseTypeMetaData>();
	assert(poseMetaData);
	auto idxPos = static_cast<BaseEntityComponent &>(component).GetMemberIndex(poseMetaData->posProperty);
	auto idxRot = static_cast<BaseEntityComponent &>(component).GetMemberIndex(poseMetaData->rotProperty);
	auto idxScale = std::is_same_v<T, udm::Transform> ? pragma::INVALID_COMPONENT_MEMBER_INDEX : static_cast<BaseEntityComponent &>(component).GetMemberIndex(poseMetaData->scaleProperty);
	auto &ncInfo = const_cast<BaseLuaBaseEntityComponent::MemberInfo &>(info);
	ncInfo.transformCompositeInfo = std::make_unique<BaseLuaBaseEntityComponent::MemberInfo::TransformCompositeInfo>();
	ncInfo.transformCompositeInfo->posIdx = idxPos ? *idxPos : pragma::INVALID_COMPONENT_MEMBER_INDEX;
	ncInfo.transformCompositeInfo->rotIdx = idxRot ? *idxRot : pragma::INVALID_COMPONENT_MEMBER_INDEX;
	ncInfo.transformCompositeInfo->scaleIdx = idxScale ? *idxScale : pragma::INVALID_COMPONENT_MEMBER_INDEX;
}

std::optional<ComponentMemberInfo> pragma::lua::get_component_member_info(lua_State *l, const std::string &functionName, ents::EntityMemberType memberType, const std::any &initialValue, BaseLuaBaseEntityComponent::MemberFlags memberFlags, const Lua::map<std::string, void> &attributes,
  luabind::object &outOnChange, bool dynamicMember)
{
	auto memberName = get_member_name(functionName);

	auto tmpMemberName = memberName;
	std::optional<ComponentMemberInfo> componentMemberInfo {};
	// TODO: Add support for properties
	auto &onChange = outOnChange;
	if(pragma::is_valid_component_property_type(memberType)) {
		onChange = attributes["onChange"];
		if(onChange) {
			onChange.push(l);
			Lua::CheckFunction(l, -1);
			Lua::Pop(l, 1);
		}
		auto vs = [&tmpMemberName, &functionName, &onChange, &attributes, memberType, dynamicMember](auto tag) -> pragma::ComponentMemberInfo {
			using T = typename decltype(tag)::type;
			if constexpr(pragma::is_valid_component_property_type_v<T>) {
				if constexpr(std::is_same_v<T, udm::Transform> || std::is_same_v<T, udm::ScaledTransform>) {
					// If a transform type has a PoseTypeMetaData attached to it, it will be treated as a special case.
					// In this case the property will act as a composite type, redirecting to a different position/rotation/scale property.
					auto oTypeMetaData = attributes["typeMetaData"];
					pragma::ents::PoseTypeMetaData *poseMetaData = nullptr;
					for(luabind::iterator it {oTypeMetaData}, end; it != end; ++it) {
						{
							auto typeData = luabind::object_cast<std::shared_ptr<pragma::ents::TypeMetaData>>(*it);
							if(typeid(*typeData) == typeid(pragma::ents::PoseTypeMetaData)) {
								poseMetaData = static_cast<pragma::ents::PoseTypeMetaData *>(typeData.get());
								break;
							}
						}
					}
					if(poseMetaData) {
						constexpr auto getter = +[](const ComponentMemberInfo &memberInfo, BaseLuaBaseEntityComponent &component, T &value) {
							auto *info = component.GetLuaMemberInfo(const_cast<ComponentMemberInfo &>(memberInfo));
							assert(info);
							if(!info->transformCompositeInfo)
								init_pose_type_meta_data<T>(memberInfo, component, const_cast<BaseLuaBaseEntityComponent::MemberInfo &>(*info));

							auto *posInfo = component.GetMemberInfo(info->transformCompositeInfo->posIdx);
							auto *rotInfo = component.GetMemberInfo(info->transformCompositeInfo->rotIdx);
							if(!posInfo || !rotInfo) {
								spdlog::trace("Transform property '{}' points to invalid pos or rot property!", std::string {*memberInfo.GetName()});
								return;
							}

							auto &pos = value.GetOrigin();
							posInfo->getterFunction(*posInfo, component, &pos);

							auto &rot = value.GetRotation();
							rotInfo->getterFunction(*rotInfo, component, &rot);

							if constexpr(std::is_same_v<T, udm::ScaledTransform>) {
								auto *scaleInfo = component.GetMemberInfo(info->transformCompositeInfo->scaleIdx);
								if(!scaleInfo) {
									spdlog::trace("Transform property '{}' points to invalid scale property!", std::string {*memberInfo.GetName()});
									return;
								}
								auto &scale = value.GetScale();
								scaleInfo->getterFunction(*scaleInfo, component, &scale);
							}
						};
						assert(!onChange);
						if(onChange)
							throw std::runtime_error {"onChange callback is not allowed for composite types!"};
						return create_component_member_info<BaseLuaBaseEntityComponent, T,
						  [](const ComponentMemberInfo &memberInfo, BaseLuaBaseEntityComponent &component, const T &value) {
							  auto *info = component.GetLuaMemberInfo(const_cast<ComponentMemberInfo &>(memberInfo));
							  assert(info);
							  if(!info->transformCompositeInfo)
								  init_pose_type_meta_data<T>(memberInfo, component, const_cast<BaseLuaBaseEntityComponent::MemberInfo &>(*info));

							  auto *posInfo = component.GetMemberInfo(info->transformCompositeInfo->posIdx);
							  auto *rotInfo = component.GetMemberInfo(info->transformCompositeInfo->rotIdx);
							  if(!posInfo || !rotInfo) {
								  spdlog::trace("Transform property '{}' points to invalid pos or rot property!", std::string {*memberInfo.GetName()});
								  return;
							  }

							  posInfo->setterFunction(*posInfo, component, &value.GetOrigin());
							  rotInfo->setterFunction(*rotInfo, component, &value.GetRotation());

							  if constexpr(std::is_same_v<T, udm::ScaledTransform>) {
								  auto *scaleInfo = component.GetMemberInfo(info->transformCompositeInfo->scaleIdx);
								  if(!scaleInfo) {
									  spdlog::trace("Transform property '{}' points to invalid scale property!", std::string {*memberInfo.GetName()});
									  return;
								  }
								  scaleInfo->setterFunction(*scaleInfo, component, &value.GetScale());
							  }
						  },
						  getter>(std::move(tmpMemberName));
					}
				}

				if(dynamicMember) {
					if constexpr(udm::is_udm_type<T>()) {
						constexpr auto getter = +[](const ComponentMemberInfo &memberInfo, BaseLuaBaseEntityComponent &component, T &value) {
							pragma::ents::EntityMemberType type;
							component.GetDynamicMemberValue<T>(memberInfo.userIndex, value, type);
						};
						if(!onChange) {
							return create_component_member_info<BaseLuaBaseEntityComponent, T, [](const ComponentMemberInfo &memberInfo, BaseLuaBaseEntityComponent &component, const T &value) { component.SetDynamicMemberValue<T>(memberInfo.userIndex, value); }, getter>(
							  std::move(tmpMemberName));
						}
						else {
							return create_component_member_info<BaseLuaBaseEntityComponent, T,
							  [](const ComponentMemberInfo &memberInfo, BaseLuaBaseEntityComponent &component, const T &value) {
								  component.SetDynamicMemberValue<T>(memberInfo.userIndex, value);

								  auto *info = component.GetDynamicMemberInfo(memberInfo.userIndex);
								  if(!info)
									  return;
								  auto &o = component.GetLuaObject();
								  if(info->onChange)
									  info->onChange(o);
							  },
							  getter>(std::move(tmpMemberName));
						}
					}
					else {
						// TODO: Add support for non-UDM types
						throw std::runtime_error {"Member " + functionName + " of type " + std::string {magic_enum::enum_name(memberType)} + " cannot be animated!"};
						return pragma::ComponentMemberInfo::CreateDummy();
					}
				}
				else {
					constexpr auto getter = +[](const ComponentMemberInfo &memberInfo, BaseLuaBaseEntityComponent &component, T &value) {
						auto *info = component.GetLuaMemberInfo(const_cast<ComponentMemberInfo &>(memberInfo));
						if constexpr(Lua::is_native_type<T>)
							value = luabind::object_cast_nothrow<T>(component.GetLuaObject()[info->memberVariableName], T {});
						else {
							auto *v = luabind::object_cast_nothrow<T *>(component.GetLuaObject()[info->memberVariableName], static_cast<T *>(nullptr));
							if(!v)
								value = {};
							else
								value = *v;
						}
					};
					if(!onChange) {
						return create_component_member_info<BaseLuaBaseEntityComponent, T,
						  [](const ComponentMemberInfo &memberInfo, BaseLuaBaseEntityComponent &component, const T &value) {
							  auto *info = component.GetLuaMemberInfo(const_cast<ComponentMemberInfo &>(memberInfo));
							  component.GetLuaObject()[info->memberVariableName] = value;
						  },
						  getter>(std::move(tmpMemberName));
					}
					else {
						return create_component_member_info<BaseLuaBaseEntityComponent, T,
						  [](const ComponentMemberInfo &memberInfo, BaseLuaBaseEntityComponent &component, const T &value) {
							  auto *info = component.GetLuaMemberInfo(const_cast<ComponentMemberInfo &>(memberInfo));
							  auto &o = component.GetLuaObject();
							  o[info->memberVariableName] = value;
							  if(info->onChange)
								  info->onChange(o);
						  },
						  getter>(std::move(tmpMemberName));
					}
				}
			}
			else {
				// Unreachable
				throw std::runtime_error {"Member " + functionName + " of type " + std::string {magic_enum::enum_name(memberType)} + " cannot be animated!"};
				return pragma::ComponentMemberInfo::CreateDummy();
			}
		};
		componentMemberInfo = std::move(ents::visit_member<false>(memberType, vs));

		auto specializationType = pragma::AttributeSpecializationType::None;
		auto oCustomSpecializationType = attributes["customSpecializationType"];
		if(oCustomSpecializationType)
			specializationType = pragma::AttributeSpecializationType::Custom;
		auto oSpecializationType = attributes["specializationType"];
		if(oSpecializationType)
			specializationType = luabind::object_cast<pragma::AttributeSpecializationType>(oSpecializationType);
		if(specializationType == pragma::AttributeSpecializationType::Custom) {
			std::string customType;
			if(oCustomSpecializationType)
				customType = luabind::object_cast<std::string>(oCustomSpecializationType);
			componentMemberInfo->SetSpecializationType(customType);
		}
		else
			componentMemberInfo->SetSpecializationType(specializationType);

		auto oFlags = attributes["flags"];
		if(oFlags) {
			auto flags = luabind::object_cast<ComponentMemberFlags>(oFlags);
			componentMemberInfo->SetFlags(flags);
		}

		auto oTypeMetaData = attributes["typeMetaData"];
		if(oTypeMetaData) {
			for(luabind::iterator it {oTypeMetaData}, end; it != end; ++it) {
				{
					auto typeData = luabind::object_cast<std::shared_ptr<pragma::ents::TypeMetaData>>(*it);
					componentMemberInfo->AddTypeMetaData(typeData);
				}
			}
		}

		auto oEnumValues = attributes["enumValues"];
		if(oEnumValues) {
			auto nameToValue = luabind::object_cast<std::unordered_map<std::string, int64_t>>(oEnumValues);
			std::unordered_map<int64_t, std::string> valueToName;
			std::vector<int64_t> values;
			valueToName.reserve(nameToValue.size());
			values.reserve(nameToValue.size());
			for(auto &pair : nameToValue) {
				valueToName[pair.second] = pair.first;
				values.push_back(pair.second);
			}
			componentMemberInfo->SetEnum(
			  [nameToValue = std::move(nameToValue)](const std::string &enumName) -> std::optional<int64_t> {
				  auto it = nameToValue.find(enumName);
				  return (it != nameToValue.end()) ? it->second : std::optional<int64_t> {};
			  },
			  [valueToName = std::move(valueToName)](int64_t enumValue) -> std::optional<std::string> {
				  auto it = valueToName.find(enumValue);
				  return (it != valueToName.end()) ? it->second : std::optional<std::string> {};
			  },
			  [values = std::move(values)]() -> std::vector<int64_t> { return values; });
		}

		auto oMetaData = attributes["metaData"];
		if(oMetaData) {
			auto &metaData = componentMemberInfo->AddMetaData();
			auto wrapper = static_cast<udm::LinkedPropertyWrapper>(metaData);
			Lua::udm::table_to_udm(Lua::tb<void> {oMetaData}, wrapper);
		}

		if(udm::is_numeric_type(ents::member_type_to_udm_type(memberType))) {
			for(luabind::iterator it {attributes}, end; it != end; ++it) {
				auto key = luabind::object_cast<std::string>(it.key());
				luabind::object val {*it};
				using namespace ustring::string_switch;
				switch(hash(key)) {
				case "min"_:
					componentMemberInfo->SetMin(luabind::object_cast<float>(val));
					break;
				case "max"_:
					componentMemberInfo->SetMax(luabind::object_cast<float>(val));
					break;
				case "stepSize"_:
					componentMemberInfo->SetStepSize(luabind::object_cast<float>(val));
					break;
				}
			}
		}
	}

	// Default value is currently only allowed for UDM types. Tag: component-member-udm-default
	if(pragma::ents::is_udm_member_type(memberType)) {
		udm::visit(pragma::ents::member_type_to_udm_type(memberType), [&componentMemberInfo, &initialValue](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(is_valid_component_property_type_v<T>)
				componentMemberInfo->SetDefault<T>(std::any_cast<T>(initialValue));
		});
	}
	return componentMemberInfo;
}

static std::string name_to_keyvalue_name(const std::string &name)
{
	std::string kvName;
	size_t pos = 0;
	while(pos < name.length()) {
		auto c = name[pos];
		if(std::isupper(c)) {
			if(!kvName.empty())
				kvName += '_';
			kvName += std::tolower(c);
		}
		else
			kvName += c;
		++pos;
	}
	return kvName;
}

BaseLuaBaseEntityComponent::MemberIndex BaseLuaBaseEntityComponent::RegisterMember(const luabind::object &oClass, const std::string &functionName, ents::EntityMemberType memberType, const std::any &initialValue, MemberFlags memberFlags, const Lua::map<std::string, void> &attributes)
{
	if(functionName.empty())
		return INVALID_MEMBER;
	auto *l = oClass.interpreter();
	/*if((memberFlags &MemberFlags::StoreBit) != MemberFlags::None)
	{
		std::string err = "If store flag is set, version number mustn't be 0! Please check 'lua_help ents.BaseEntityComponent.RegisterMember' for more information!";
		lua_pushstring(l,err.c_str());
		lua_error(l);
		return INVALID_MEMBER;
	}*/
	auto memberName = get_member_name(functionName);
	auto memberVarName = get_member_variable_name(functionName);

	auto &members = get_class_member_list(l);
	auto it = std::find_if(members.begin(), members.end(), [&oClass](const std::shared_ptr<ClassMembers> &classMembers) { return oClass == classMembers->classObject; });
	if(it == members.end()) {
		members.push_back(std::make_shared<ClassMembers>(oClass));
		it = members.end() - 1;
	}
	auto itMember = std::find_if((*it)->memberDeclarations.begin(), (*it)->memberDeclarations.end(), [&functionName](const MemberInfo &memberInfo) { return ustring::compare<std::string_view>(functionName, memberInfo.functionName, false); });
	if(itMember == (*it)->memberDeclarations.end()) {
		luabind::object onChange;
		auto componentMemberInfo = pragma::lua::get_component_member_info(l, functionName, memberType, initialValue, memberFlags, attributes, onChange, false);

		if(componentMemberInfo.has_value()) {
			(*it)->memberDeclarations.push_back({functionName, memberName, get_component_member_name_hash(memberName), memberVarName, memberType, initialValue, memberFlags, std::move(onChange), std::move(componentMemberInfo)});
			itMember = (*it)->memberDeclarations.end() - 1;
		}
	}
	auto idx = itMember - (*it)->memberDeclarations.begin();

	auto oEnumValues = attributes["enumValues"];
	if(oEnumValues) {
		auto nameToValue = luabind::object_cast<std::unordered_map<std::string, int64_t>>(oEnumValues);
		for(auto &pair : nameToValue) {
			auto name = memberName + pair.first;
			name = name_to_keyvalue_name(name);
			for(auto &c : name)
				c = std::toupper(c);
			oClass[name] = pair.second;
		}
	}

	static_assert(umath::to_integral(pragma::ents::EntityMemberType::VersionIndex) == 0);
	std::string getterName = "Get";
	auto bProperty = (memberFlags & MemberFlags::PropertyBit) != MemberFlags::None;
	if((memberFlags & MemberFlags::GetterBit) != MemberFlags::None) {
		std::string getter;
#if ENABLE_CUSTOM_SETTER_GETTER == 1
		auto oGetter = attributes["getter"];
		if(oGetter) {
			auto getterFuncName = luabind::object_cast<std::string>(oGetter);
			getter = "function(self) return self:" + getterFuncName + "()";
		}
		else
#endif
		{
			getter = "function(self) return self." + memberVarName;
			if(memberType == ents::EntityMemberType::Entity)
				getter += ":GetEntity()";
			else if(memberType == ents::EntityMemberType::MultiEntity)
				throw std::runtime_error {"Not yet implemented!"};
			else if(bProperty)
				getter += ":Get()";
		}
		getter += " end";

		std::string err;
		if(Lua::PushLuaFunctionFromString(l, getter, "EntityComponentGetter", err) == false)
			Con::cwar << "Unable to register getter-function for member '" << functionName << "' for entity component: " << err << Con::endl;
		else {
			if((memberFlags & MemberFlags::UseHasGetterBit) != MemberFlags::None)
				getterName = "Has";
			else if((memberFlags & MemberFlags::UseIsGetterBit) != MemberFlags::None)
				getterName = "Is";

			auto idxFunc = Lua::GetStackTop(l);
			oClass.push(l); /* 2 */
			auto idxObject = Lua::GetStackTop(l);
			Lua::PushString(l, getterName + functionName); /* 3 */
			Lua::PushValue(l, idxFunc);                    /* 4 */
			Lua::SetTableValue(l, idxObject);              /* 2 */
			Lua::Pop(l, 2);                                /* 0 */
		}

		if(bProperty || memberType == ents::EntityMemberType::Entity) {
			std::string getterProperty = "function(self) return self." + memberVarName + " end";
			std::string err;
			if(Lua::PushLuaFunctionFromString(l, getterProperty, "EntityComponentGetterProperty", err) == false)
				Con::cwar << "Unable to register property-get-function for member '" << functionName << "' for entity component: " << err << Con::endl;
			else {
				auto idxFunc = Lua::GetStackTop(l);
				oClass.push(l); /* 2 */
				auto idxObject = Lua::GetStackTop(l);
				Lua::PushString(l, "Get" + functionName + (bProperty ? "Property" : "Reference")); /* 3 */
				Lua::PushValue(l, idxFunc);                                                        /* 4 */
				Lua::SetTableValue(l, idxObject);                                                  /* 2 */
				Lua::Pop(l, 2);                                                                    /* 0 */
			}
		}
	}
	if((memberFlags & MemberFlags::SetterBit) != MemberFlags::None) {
		std::string setter;
#if ENABLE_CUSTOM_SETTER_GETTER == 1
		auto oSetter = attributes["setter"];
		if(oSetter) {
			auto setterFuncName = luabind::object_cast<std::string>(oSetter);
			setter = "function(self,value) self:" + setterFuncName + "(value)";
		}
		else
#endif
		{
			auto bTransmit = (memberFlags & MemberFlags::TransmitOnChange) != MemberFlags::None;
			setter = "function(self,value) ";
			if(bTransmit)
				setter += "if(value == self:" + getterName + functionName + "()) then return end ";
			if(memberType == ents::EntityMemberType::Entity) {
				setter += "local t = util.get_type_name(value)\n"
				          "if(t ~= \"UniversalEntityReference\") then\n"
				          "	if(t == \"nil\") then value = ents.UniversalEntityReference()\n"
				          "	else value = ents.UniversalEntityReference(value) end\n"
				          "end\n";
			}
			else if(memberType == ents::EntityMemberType::ComponentProperty) {
				setter += "local t = util.get_type_name(value)\n"
				          "if(t ~= \"UniversalMemberReference\") then\n"
				          "	if(t == \"nil\") then value = ents.UniversalMemberReference()\n"
				          "	else value = ents.UniversalMemberReference(value) end\n"
				          "end\n";
			}
			else if(memberType == ents::EntityMemberType::MultiEntity)
				throw std::runtime_error {"Not yet implemented!"};
			setter += "self." + memberVarName;
			if(bProperty)
				setter += ":Set(value)";
			else
				setter += " = value";
		}
		if((memberFlags & MemberFlags::TransmitOnChange) == MemberFlags::TransmitOnChange || (memberFlags & MemberFlags::OutputBit) != MemberFlags::None || itMember->onChange)
			setter += " self:OnMemberValueChanged(" + std::to_string(idx) + ")";
		setter += " end";
		auto bSetterValid = true;
		std::string err;
		if(Lua::PushLuaFunctionFromString(l, setter, "EntityComponentSetter", err) == false) {
			bSetterValid = false;
			Con::cwar << "Unable to register setter-function for member '" << functionName << "' for entity component: " << err << Con::endl;
		} /* else 1 */
		if(bSetterValid) {
			auto idxFunc = Lua::GetStackTop(l);
			oClass.push(l); /* 2 */
			auto idxObject = Lua::GetStackTop(l);
			Lua::PushString(l, "Set" + functionName); /* 3 */
			Lua::PushValue(l, idxFunc);               /* 4 */
			Lua::SetTableValue(l, idxObject);         /* 2 */
			Lua::Pop(l, 2);                           /* 0 */
		}
	}
	return idx;
}
uint32_t BaseLuaBaseEntityComponent::GetStaticMemberCount() const
{
	auto *co = GetClassObject();
	if(!co)
		return 0;
	auto *infos = GetMemberInfos(*co);
	return infos ? infos->size() : 0;
}
void BaseLuaBaseEntityComponent::OnMemberValueChanged(uint32_t memberIdx)
{
	auto &members = GetMembers();
	if(memberIdx >= members.size())
		return;
	auto &member = members.at(memberIdx);
	if((member.flags & MemberFlags::OutputBit) != MemberFlags::None) {
		auto *pIoComponent = static_cast<pragma::BaseIOComponent *>(GetEntity().FindComponent("io").get());
		if(pIoComponent != nullptr)
			pIoComponent->TriggerOutput("OnSet" + std::string {member.functionName}, &GetEntity());
	}
	if(member.onChange)
		member.onChange(GetLuaObject());
}
std::vector<BaseLuaBaseEntityComponent::MemberInfo> *BaseLuaBaseEntityComponent::GetMemberInfos(const luabind::object &oClass)
{
	auto *p = get_class_member_declarations(oClass);
	return (p != nullptr) ? &p->memberDeclarations : nullptr;
}
void BaseLuaBaseEntityComponent::ClearMembers(lua_State *l)
{
	auto it = s_classMembers.find(l);
	if(it == s_classMembers.end())
		return;
	s_classMembers.erase(it);
}
void BaseLuaBaseEntityComponent::SetupLua(const luabind::object &o) { SetLuaObject(o); }

////////////////

BaseLuaBaseEntityComponent::BaseLuaBaseEntityComponent(BaseEntity &ent) : pragma::BaseEntityComponent(ent) {}

void BaseLuaBaseEntityComponent::InitializeLuaObject(lua_State *l) {}

luabind::object *BaseLuaBaseEntityComponent::GetClassObject()
{
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	auto &componentManager = game.GetEntityComponentManager();
	auto *pInfo = componentManager.GetComponentInfo(GetComponentId());
	if(!pInfo)
		return nullptr;
	auto &luaEntityManager = game.GetLuaEntityManager();
	return luaEntityManager.GetComponentClassObject(pInfo->name);
}

void BaseLuaBaseEntityComponent::Initialize()
{
	// The underlying handle for the lua object has not yet been assigned to our shared ptr, so we have to do it now.
	// This has to be done before any Lua member-functions are called, but can't be done inside the constructor, because
	// at that point the object hasn't been assigned to a shared ptr yet!
	//auto *pHandleWrapper = luabind::object_cast_nothrow<BaseLuaBaseEntityComponentHandleWrapper*>(GetLuaObject(),static_cast<BaseLuaBaseEntityComponentHandleWrapper*>(nullptr));
	//if(pHandleWrapper != nullptr)
	//	pHandleWrapper->handle = util::WeakHandle<pragma::BaseEntityComponent>(shared_from_this());

	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	auto &componentManager = game.GetEntityComponentManager();
	if((componentManager.GetComponentInfo(GetComponentId())->flags & pragma::ComponentFlags::Networked) != pragma::ComponentFlags::None)
		SetNetworked(true);

	auto *l = GetLuaState();
	InitializeLuaObject(l);

	auto *o = GetClassObject();
	if(o != nullptr) {
		m_classMemberIndex = get_class_member_index(*o);
		auto *p = get_class_member_declarations(*o);
		if(p != nullptr)
			InitializeMembers(p->memberDeclarations);
	}
	if(m_networkedMemberInfo != nullptr)
		m_networkedMemberInfo->netEvSetMember = SetupNetEvent("set_member_value");

	CallLuaMethod("Initialize");

	PushLuaObject(); /* 1 */
	auto t = Lua::GetStackTop(l);
	Lua::PushString(l, "Version"); /* 2 */
	Lua::GetTableValue(l, t);      /* 2 */
	if(Lua::IsSet(l, -1))
		m_version = Lua::CheckInt(l, -1);
	Lua::Pop(l, 2); /* 0 */

	auto &ent = GetEntity();
	CallLuaMethod("OnAttachedToEntity");
	pragma::BaseEntityComponent::Initialize();
}

void BaseLuaBaseEntityComponent::InitializeMembers(const std::vector<BaseLuaBaseEntityComponent::MemberInfo> &members)
{
	m_members = members;
	auto &o = GetLuaObject();
	auto *l = o.interpreter();
	o.push(l); /* 1 */
	auto t = Lua::GetStackTop(l);
	auto totalMemberFlags = MemberFlags::None;
	auto idxMember = 0u;
	for(auto &member : members) {
		auto &memberVarName = member.memberVariableName;
		Lua::PushString(l, memberVarName); /* 2 */

		if((member.flags & MemberFlags::PropertyBit) != MemberFlags::None)
			Lua::PushNewAnyProperty(l, detail::member_type_to_util_type(member.type), member.initialValue); /* 3 */
		else
			Lua::PushAny(l, detail::member_type_to_util_type(member.type), member.initialValue); /* 3 */
		if(Lua::IsNil(l, -1) && ents::is_udm_member_type(member.type))
			Con::cwar << "Invalid member type '" << magic_enum::enum_name(member.type) << "' for member '" << member.functionName << "' of entity component '" << GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetComponentInfo(GetComponentId())->name
			          << "'! Ignoring..." << Con::endl;
		Lua::SetTableValue(l, t); /* 1 */

		if((member.flags & MemberFlags::NetworkedBit) != MemberFlags::None) {
			if(m_networkedMemberInfo == nullptr)
				m_networkedMemberInfo = std::unique_ptr<NetworkedMemberInfo>(new NetworkedMemberInfo {});
			m_networkedMemberInfo->memberIndexToNetworkedIndex[idxMember] = m_networkedMemberInfo->networkedMembers.size();
			m_networkedMemberInfo->networkedMembers.push_back(idxMember);

			if((member.flags & MemberFlags::SnapshotData) != MemberFlags::None)
				m_networkedMemberInfo->snapshotMembers.push_back(idxMember);
		}

		totalMemberFlags |= member.flags;

		if(member.type == ents::EntityMemberType::Element)
			o[memberVarName] = ::udm::Property::Create<::udm::Element>();

		InitializeMember(member);

		if((member.flags & (MemberFlags::KeyValueBit | MemberFlags::InputBit)) != MemberFlags::None) {
			// We only need this for quick keyvalue or input lookups
			std::string lmemberName = member.functionName;
			ustring::to_lower(lmemberName);

			auto kvName = name_to_keyvalue_name(member.functionName);
			if(kvName != lmemberName)
				m_memberNameToIndex[std::move(kvName)] = idxMember; // Alternative kv name with underscores

			m_memberNameToIndex[std::move(lmemberName)] = idxMember;
		}

		++idxMember;
	}
	Lua::Pop(l, 1); /* 0 */

	if((totalMemberFlags & MemberFlags::KeyValueBit) != MemberFlags::None) {
		BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](const std::reference_wrapper<pragma::ComponentEvent> &evData) -> util::EventReply {
			auto &kvData = static_cast<pragma::CEKeyValueData &>(evData.get());
			auto it = m_memberNameToIndex.find(kvData.key);
			if(it == m_memberNameToIndex.end())
				return util::EventReply::Unhandled;
			auto &member = m_members.at(it->second);
			SetMemberValue(member, string_to_any(*GetEntity().GetNetworkState()->GetGameState(), kvData.value, detail::member_type_to_util_type(member.type)));
			return util::EventReply::Handled;
		});
	}
	if((totalMemberFlags & MemberFlags::InputBit) != MemberFlags::None) {
		BindEvent(pragma::BaseIOComponent::EVENT_HANDLE_INPUT, [this](const std::reference_wrapper<pragma::ComponentEvent> &evData) -> util::EventReply {
			auto &ioData = static_cast<pragma::CEInputData &>(evData.get());
			if(strncmp(ioData.input.data(), "set", 3u) != 0)
				return util::EventReply::Unhandled;
			auto input = ioData.input.substr(3u);
			auto it = m_memberNameToIndex.find(input);
			if(it == m_memberNameToIndex.end())
				return util::EventReply::Unhandled;
			auto &member = m_members.at(it->second);
			SetMemberValue(member, string_to_any(*GetEntity().GetNetworkState()->GetGameState(), ioData.data, detail::member_type_to_util_type(member.type)));
			return util::EventReply::Handled;
		});
	}
}

void BaseLuaBaseEntityComponent::OnTick(double dt) { CallLuaMethod<void, double>("OnTick", dt); }

void BaseLuaBaseEntityComponent::InitializeMember(const MemberInfo &memberInfo) {}

std::any *BaseLuaBaseEntityComponent::GetDynamicMemberValue(ComponentMemberIndex memberIndex, ents::EntityMemberType &outType)
{
	auto *info = GetMemberInfo(memberIndex);
	if(!info)
		return nullptr;
	if(memberIndex < m_dynamicMemberStartOffset)
		return nullptr;
	memberIndex -= m_dynamicMemberStartOffset;
	if(memberIndex >= m_dynamicMembers.size() || !m_dynamicMembers[memberIndex].enabled)
		return nullptr;
	outType = info->type;
	return &m_dynamicMembers[memberIndex].value;
}

void BaseLuaBaseEntityComponent::ReserveMembers(uint32_t count) { DynamicMemberRegister::ReserveMembers(count); }
BaseLuaBaseEntityComponent::DynamicMemberInfo *BaseLuaBaseEntityComponent::GetDynamicMemberInfo(ComponentMemberIndex memberIndex)
{
	if(memberIndex < m_dynamicMemberStartOffset)
		return nullptr;
	memberIndex -= m_dynamicMemberStartOffset;
	if(memberIndex >= m_dynamicMembers.size() || !m_dynamicMembers[memberIndex].enabled)
		return nullptr;
	return &m_dynamicMembers[memberIndex];
}
ComponentMemberIndex BaseLuaBaseEntityComponent::RegisterMember(ComponentMemberInfo &&memberInfo, luabind::object &onChange)
{
	auto idx = DynamicMemberRegister::RegisterMember(std::move(memberInfo));
	auto *info = GetDynamicMemberInfo(idx);
	if(info != nullptr)
		info->onChange = onChange;
	return idx;
}
ComponentMemberIndex BaseLuaBaseEntityComponent::RegisterMember(const ComponentMemberInfo &memberInfo, luabind::object &onChange)
{
	auto idx = DynamicMemberRegister::RegisterMember(memberInfo);
	auto *info = GetDynamicMemberInfo(idx);
	if(info != nullptr)
		info->onChange = onChange;
	return idx;
}
void BaseLuaBaseEntityComponent::ClearMembers() { DynamicMemberRegister::ClearMembers(); }
void BaseLuaBaseEntityComponent::RemoveMember(ComponentMemberIndex idx) { DynamicMemberRegister::RemoveMember(idx); }
void BaseLuaBaseEntityComponent::RemoveMember(const std::string &name) { DynamicMemberRegister::RemoveMember(name); }
void BaseLuaBaseEntityComponent::UpdateMemberNameMap() { DynamicMemberRegister::UpdateMemberNameMap(); }

void BaseLuaBaseEntityComponent::OnMemberRegistered(const ComponentMemberInfo &memberInfo, ComponentMemberIndex index)
{
	DynamicMemberRegister::OnMemberRegistered(memberInfo, index);

	if(pragma::ents::is_udm_member_type(memberInfo.type)) {
		const_cast<ComponentMemberInfo &>(memberInfo).userIndex = index;
		if(m_dynamicMembers.empty())
			m_dynamicMemberStartOffset = index;
		index -= m_dynamicMemberStartOffset;
		if(index >= m_dynamicMembers.size())
			m_dynamicMembers.resize(index + 1);
		auto &dynMember = m_dynamicMembers[index];
		dynMember.enabled = true;
		udm::visit(pragma::ents::member_type_to_udm_type(memberInfo.type), [&memberInfo, &dynMember](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(is_valid_component_property_type_v<T>) {
				T def;
				memberInfo.GetDefault<T>(def);
				dynMember.value = def;
			}
		});
	}
}
void BaseLuaBaseEntityComponent::OnMemberRemoved(const ComponentMemberInfo &memberInfo, ComponentMemberIndex index)
{
	DynamicMemberRegister::OnMemberRemoved(memberInfo, index);
	if(index < m_dynamicMemberStartOffset)
		return;
	index -= m_dynamicMemberStartOffset;
	if(index >= m_dynamicMembers.size())
		return;
	m_dynamicMembers[index].enabled = false;
	m_dynamicMembers[index].value = {};
}

void BaseLuaBaseEntityComponent::OnActiveStateChanged(bool active)
{
	BaseEntityComponent::OnActiveStateChanged(active);
	CallLuaMethod<void, bool>("OnActiveStateChanged", active);
}

std::optional<ComponentMemberIndex> BaseLuaBaseEntityComponent::DoGetMemberIndex(const std::string &name) const
{
	auto *o = GetClassObject();
	if(!o)
		return {};
	auto idx = find_member_info_index(*o, name);
	if(idx.has_value())
		return idx;
	idx = DynamicMemberRegister::GetMemberIndex(name);
	if(idx.has_value())
		return *idx; // +GetStaticMemberCount();
	return std::optional<ComponentMemberIndex> {};
}
const ComponentMemberInfo *BaseLuaBaseEntityComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto &classMembers = get_class_member_list(GetLuaObject().interpreter());
	if(m_classMemberIndex < classMembers.size()) {
		auto &members = classMembers[m_classMemberIndex];
		if(idx < members->memberDeclarations.size()) {
			auto &memberInfo = members->memberDeclarations[idx];
			return memberInfo.componentMemberInfo.has_value() ? &*memberInfo.componentMemberInfo : nullptr;
		}
	}
	return DynamicMemberRegister::GetMemberInfo(idx); // -GetStaticMemberCount());
}

const luabind::object &BaseLuaBaseEntityComponent::GetLuaObject() const { return pragma::BaseEntityComponent::GetLuaObject(); }
luabind::object &BaseLuaBaseEntityComponent::GetLuaObject() { return pragma::BaseEntityComponent::GetLuaObject(); }

CallbackHandle BaseLuaBaseEntityComponent::BindInitComponentEvent(ComponentId componentId, const std::function<void(std::reference_wrapper<pragma::BaseEntityComponent>)> &callback)
{
	auto it = m_initComponentCallbacks.find(componentId);
	if(it != m_initComponentCallbacks.end()) {
		if(it->second.IsValid())
			it->second.Remove();
		m_initComponentCallbacks.erase(it);
	}
	it = m_initComponentCallbacks.insert(std::make_pair(componentId, FunctionCallback<void, std::reference_wrapper<pragma::BaseEntityComponent>>::Create(callback))).first;
	auto pComponent = GetEntity().FindComponent(componentId);
	if(pComponent.valid())
		callback(std::reference_wrapper<pragma::BaseEntityComponent>(*pComponent));
	return it->second;
}

util::EventReply BaseLuaBaseEntityComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == pragma::BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED) {
		auto &componentAddedData = static_cast<pragma::CEOnEntityComponentAdded &>(evData);
		auto it = m_initComponentCallbacks.find(componentAddedData.component.GetComponentId());
		if(it != m_initComponentCallbacks.end()) {
			auto &hCb = it->second;
			if(hCb.IsValid() == false)
				m_initComponentCallbacks.erase(it);
			else
				hCb.Call<void, std::reference_wrapper<pragma::BaseEntityComponent>>(componentAddedData.component);
		}
	}

	// Deprecated: Events always have to be explicitely registered by Lua-components
	// to avoid the overhead of pushing arguments for events that probably won't even
	// be used by the Lua-script.
	/*
	// CallLuaMember<void,uint32_t>("HandleEvent",eventId);
	// CallLuaMember does not work with a variable number of arguments,
	// so we have to call the method manually.
	// See also Lua::register_base_entity_component -> BindEvent
	auto o = LuaObjectBase::GetLuaObject();
	auto r = o["HandleEvent"];
	if(r)
	{
		auto *l = o.interpreter();
		o.push(l);
		Lua::PushString(l,"HandleEvent");
		Lua::GetTableValue(l,-2);
		if(Lua::IsFunction(l,-1))
		{
			// duplicate the self-object
			Lua::PushValue(l,-2);
			// remove the bottom self-object
			Lua::RemoveValue(l,-3);

			auto n = Lua::GetStackTop(l);
			Lua::PushInt(l,eventId);
			evData.PushArguments(l);
			auto numArgs = (Lua::GetStackTop(l) -n) +1u; // +1 because of self-object
			auto c = Lua::CallFunction(l,numArgs,evData.GetReturnCount());
			if(c != Lua::StatusCode::Ok)
				Lua::HandleLuaError(l,c);
			else
				evData.HandleReturnValues(l);
		}
	}*/
	return util::EventReply::Unhandled;
}

void BaseLuaBaseEntityComponent::OnAttached(BaseEntity &ent)
{
	pragma::BaseEntityComponent::OnAttached(ent);
	CallLuaMethod("OnAttachedToEntity");
}
void BaseLuaBaseEntityComponent::OnDetached(BaseEntity &ent)
{
	pragma::BaseEntityComponent::OnDetached(ent);
	CallLuaMethod("OnDetachedToEntity");
}

std::any BaseLuaBaseEntityComponent::GetMemberValue(const MemberInfo &memberInfo) const
{
	auto &o = const_cast<BaseLuaBaseEntityComponent *>(this)->GetLuaObject();
	auto *l = o.interpreter();
	o.push(l); /* 1 */
	auto t = Lua::GetStackTop(l);
	Lua::PushString(l, memberInfo.memberVariableName); /* 2 */
	Lua::GetTableValue(l, t);                          /* 2 */
	if((memberInfo.flags & MemberFlags::PropertyBit) == MemberFlags::None) {
		auto value = Lua::GetAnyValue(l, detail::member_type_to_util_type(memberInfo.type), -1);
		Lua::Pop(l, 2); /* 0 */
		return value;
	}
	auto value = Lua::GetAnyPropertyValue(l, -1, detail::member_type_to_util_type(memberInfo.type));
	Lua::Pop(l, 2); /* 0 */
	return value;
}

void BaseLuaBaseEntityComponent::SetMemberValue(const MemberInfo &memberInfo, const std::any &value) const
{
	auto &o = const_cast<BaseLuaBaseEntityComponent *>(this)->GetLuaObject();
	auto *l = o.interpreter();
	o.push(l); /* 1 */
	auto t = Lua::GetStackTop(l);
	Lua::PushString(l, memberInfo.memberVariableName); /* 2 */
	if((memberInfo.flags & MemberFlags::PropertyBit) != MemberFlags::None) {
		Lua::GetTableValue(l, t);                                                                  /* 2 */
		Lua::SetAnyPropertyValue(l, -1, detail::member_type_to_util_type(memberInfo.type), value); /* 2 */
		Lua::Pop(l, 1);                                                                            /* 1 */
	}
	else {
		Lua::PushAny(l, detail::member_type_to_util_type(memberInfo.type), value); /* 3 */
		Lua::SetTableValue(l, t);                                                  /* 1 */
	}
	Lua::Pop(l, 1); /* 0 */
	if(memberInfo.onChange)
		memberInfo.onChange(o);
}

static void write_value(udm::LinkedPropertyWrapperArg udm, const std::any &value, util::VarType type)
{
	switch(type) {
	case util::VarType::Bool:
		udm = std::any_cast<udm::Boolean>(value);
		break;
	case util::VarType::Double:
		udm = std::any_cast<udm::Double>(value);
		break;
	case util::VarType::Float:
		udm = std::any_cast<udm::Float>(value);
		break;
	case util::VarType::Int8:
		udm = std::any_cast<udm::Int8>(value);
		break;
	case util::VarType::Int16:
		udm = std::any_cast<udm::Int16>(value);
		break;
	case util::VarType::Int32:
		udm = std::any_cast<udm::Int32>(value);
		break;
	case util::VarType::Int64:
		udm = std::any_cast<udm::Int64>(value);
		break;
	case util::VarType::LongDouble:
		udm = static_cast<udm::Double>(std::any_cast<long double>(value));
		break;
	case util::VarType::String:
		udm = std::any_cast<udm::String>(value);
		break;
	case util::VarType::UInt8:
		udm = std::any_cast<udm::UInt8>(value);
		break;
	case util::VarType::UInt16:
		udm = std::any_cast<udm::UInt16>(value);
		break;
	case util::VarType::UInt32:
		udm = std::any_cast<udm::UInt32>(value);
		break;
	case util::VarType::UInt64:
		udm = std::any_cast<udm::UInt64>(value);
		break;
	case util::VarType::EulerAngles:
		udm = std::any_cast<udm::EulerAngles>(value);
		break;
	case util::VarType::Color:
		{
			auto color = std::any_cast<Color>(value);
			udm = Vector4 {color.r, color.g, color.b, color.a};
			break;
		}
	case util::VarType::Vector:
		udm = std::any_cast<udm::Vector3>(value);
		break;
	case util::VarType::Vector2:
		udm = std::any_cast<udm::Vector2>(value);
		break;
	case util::VarType::Vector4:
		udm = std::any_cast<udm::Vector4>(value);
		break;
	case util::VarType::Entity:
		{
			auto ref = std::any_cast<pragma::EntityURef>(value);
			auto *ident = ref.GetIdentifier();
			if(!ident)
				udm = "";
			else
				udm = std::visit([&udm](auto &val) { return BaseEntity::GetUri(val); }, *ident);
			break;
		}
	case util::VarType::Quaternion:
		udm = std::any_cast<udm::Quaternion>(value);
		break;
	}
}

static void read_value(Game &game, udm::LinkedPropertyWrapperArg udm, std::any &value, util::VarType type)
{
	switch(type) {
	case util::VarType::Bool:
		{
			auto val = udm.ToValue<udm::Boolean>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::Double:
		{
			auto val = udm.ToValue<udm::Double>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::Float:
		{
			auto val = udm.ToValue<udm::Float>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::Int8:
		{
			auto val = udm.ToValue<udm::Int8>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::Int16:
		{
			auto val = udm.ToValue<udm::Int16>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::Int32:
		{
			auto val = udm.ToValue<udm::Int32>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::Int64:
		{
			auto val = udm.ToValue<udm::Int64>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::LongDouble:
		{
			auto val = udm.ToValue<udm::Double>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::String:
		{
			auto val = udm.ToValue<udm::String>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::UInt8:
		{
			auto val = udm.ToValue<udm::UInt8>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::UInt16:
		{
			auto val = udm.ToValue<udm::UInt16>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::UInt32:
		{
			auto val = udm.ToValue<udm::UInt32>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::UInt64:
		{
			auto val = udm.ToValue<udm::UInt64>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::EulerAngles:
		{
			auto val = udm.ToValue<udm::EulerAngles>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::Color:
		{
			auto val = udm.ToValue<udm::Vector4>();
			if(val.has_value())
				value = Color {static_cast<int16_t>(val->x), static_cast<int16_t>(val->y), static_cast<int16_t>(val->z), static_cast<int16_t>(val->w)};
			break;
		}
	case util::VarType::Vector:
		{
			auto val = udm.ToValue<udm::Vector3>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::Vector2:
		{
			auto val = udm.ToValue<udm::Vector2>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::Vector4:
		{
			auto val = udm.ToValue<udm::Vector4>();
			if(val.has_value())
				value = *val;
			break;
		}
	case util::VarType::Entity:
		{
			auto val = udm.ToValue<udm::String>();
			if(val.has_value()) {
				pragma::EntityUComponentMemberRef ref;
				if(BaseEntity::ParseUri(std::move(*val), ref))
					value = pragma::EntityURef {std::move(ref)};
			}
			break;
		}
	case util::VarType::Quaternion:
		{
			auto val = udm.ToValue<udm::Quaternion>();
			if(val.has_value())
				value = *val;
			break;
		}
	}
}

void BaseLuaBaseEntityComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	// CallLuaMember<void,luabind::object>("OnEntityComponentAdded",component.GetLuaObject()); // Unused: Already covered by EVENT_ON_COMPONENT_ADDED
}
void BaseLuaBaseEntityComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	// CallLuaMember<void,luabind::object>("OnEntityComponentRemoved",component.GetLuaObject()); // Unused: Already covered by EVENT_ON_COMPONENT_REMOVED
}

const std::vector<BaseLuaBaseEntityComponent::MemberInfo> &BaseLuaBaseEntityComponent::GetMembers() const { return m_members; }
void BaseLuaBaseEntityComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	pragma::BaseEntityComponent::Save(udm);
	for(auto &member : m_members) {
		if((member.flags & MemberFlags::StoreBit) == MemberFlags::None)
			continue;
		auto value = GetMemberValue(member);
		write_value(udm["members." + std::string {member.functionName}], value, detail::member_type_to_util_type(member.type));
	}
	CallLuaMethod<void, udm::LinkedPropertyWrapper>("Save", udm);
}
void BaseLuaBaseEntityComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	pragma::BaseEntityComponent::Load(udm, version);
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	for(auto &member : m_members) {
		if((member.flags & MemberFlags::StoreBit) == MemberFlags::None)
			continue;
		std::any value;
		auto udmMember = udm["members." + std::string {member.functionName}];
		if(udmMember) {
			read_value(game, udmMember, value, detail::member_type_to_util_type(member.type));
			SetMemberValue(member, value);
		}
	}
	CallLuaMethod<void, udm::LinkedPropertyWrapper, uint32_t>("Load", udm, version);
}
uint32_t BaseLuaBaseEntityComponent::GetVersion() const { return m_version; }

void BaseLuaBaseEntityComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	CallLuaMethod("OnEntitySpawn");
}

void BaseLuaBaseEntityComponent::OnEntityPostSpawn()
{
	BaseEntityComponent::OnEntityPostSpawn();
	CallLuaMethod("OnEntityPostSpawn");
}

void BaseLuaBaseEntityComponent::OnRemove()
{
	pragma::BaseEntityComponent::OnRemove();
	CallLuaMethod("OnRemove");
}

void BaseLuaBaseEntityComponent::SetNetworked(bool b) { m_bShouldTransmitNetData = b; }
bool BaseLuaBaseEntityComponent::IsNetworked() const { return m_bShouldTransmitNetData; }
void BaseLuaBaseEntityComponent::SetShouldTransmitSnapshotData(bool b) { m_bShouldTransmitSnapshotData = b; }
bool BaseLuaBaseEntityComponent::ShouldTransmitSnapshotData() const { return m_bShouldTransmitSnapshotData; }

CallbackHandle BaseLuaBaseEntityComponent::BindInitComponentEvent(lua_State *l, pragma::ComponentId componentId, luabind::object methodNameOrFunction)
{
	methodNameOrFunction.push(l);
	util::ScopeGuard sg([l]() { Lua::Pop(l, 1); });

	auto argTarget = -1;
	if(Lua::IsString(l, argTarget)) {
		std::string methodName = Lua::CheckString(l, argTarget);
		auto hCb = BindInitComponentEvent(componentId, [this, methodName](std::reference_wrapper<pragma::BaseEntityComponent> component) { CallLuaMethod<void, luabind::object>(methodName, component.get().GetLuaObject()); });
		Lua::Push<CallbackHandle>(l, hCb);
		return hCb;
	}
	Lua::CheckFunction(l, argTarget);
	auto hCb = BindInitComponentEvent(componentId, [methodNameOrFunction](std::reference_wrapper<pragma::BaseEntityComponent> component) {
		auto &fc = methodNameOrFunction;
		auto *l = fc.interpreter();
		auto c = Lua::CallFunction(
		  l,
		  [&fc, &component](lua_State *l) -> Lua::StatusCode {
			  fc.push(l);
			  component.get().PushLuaObject(l);
			  return Lua::StatusCode::Ok;
		  },
		  0);
	});
	Lua::Push<CallbackHandle>(l, hCb);
	return hCb;
}
CallbackHandle BaseLuaBaseEntityComponent::BindNetEvent(pragma::NetEventId eventId, const std::function<void(std::reference_wrapper<NetPacket>, pragma::BasePlayerComponent *)> &callback)
{
	auto it = m_boundNetEvents.find(eventId);
	if(it != m_boundNetEvents.end()) {
		if(it->second.IsValid())
			it->second.Remove();
		m_boundNetEvents.erase(it);
	}
	auto hCb = FunctionCallback<void, std::reference_wrapper<NetPacket>, pragma::BasePlayerComponent *>::Create(callback);
	m_boundNetEvents.insert(std::make_pair(eventId, hCb));
	return hCb;
}
CallbackHandle BaseLuaBaseEntityComponent::BindNetEvent(lua_State *l, pragma::NetEventId eventId, luabind::object methodNameOrFunction)
{
	methodNameOrFunction.push(l);
	util::ScopeGuard sg([l]() { Lua::Pop(l, 1); });

	auto argTarget = -1;
	if(Lua::IsString(l, argTarget)) {
		std::string methodName = Lua::CheckString(l, argTarget);
		auto hCb = BindNetEvent(eventId, [this, methodName](std::reference_wrapper<NetPacket> packet, pragma::BasePlayerComponent *pl) { InvokeNetEventHandle(methodName, packet, pl); });
		Lua::Push<CallbackHandle>(l, hCb);
		return hCb;
	}
	Lua::CheckFunction(l, argTarget);
	auto hCb = BindNetEvent(eventId, [methodNameOrFunction](std::reference_wrapper<NetPacket> packet, pragma::BasePlayerComponent *pl) {
		auto &fc = methodNameOrFunction;
		auto *l = fc.interpreter();
		auto c = Lua::CallFunction(
		  l,
		  [&fc, &packet, &pl](lua_State *l) mutable -> Lua::StatusCode {
			  fc.push(l);
			  if(pl != nullptr)
				  pl->PushLuaObject(l);
			  Lua::Push<NetPacket>(l, packet.get());
			  return Lua::StatusCode::Ok;
		  },
		  0);
	});
	Lua::Push<CallbackHandle>(l, hCb);
	return hCb;
}
CallbackHandle BaseLuaBaseEntityComponent::BindEvent(lua_State *l, pragma::ComponentId eventId, luabind::object methodNameOrFunction)
{
	methodNameOrFunction.push(l);
	util::ScopeGuard sg([l]() { Lua::Pop(l, 1); });

	auto argTarget = -1;
	if(Lua::IsString(l, argTarget)) {
		std::string methodName = Lua::CheckString(l, argTarget);
		auto hCb = BindEvent(eventId, [this, methodName](std::reference_wrapper<pragma::ComponentEvent> ev) -> ::util::EventReply {
			// CallLuaMember does not work with a variable number of arguments,
			// so we have to call the method manually.
			// See also BaseLuaBaseEntityComponent::HandleEvent
			auto o = GetLuaObject();
			auto r = o[methodName];
			if(r && luabind::type(r) == LUA_TFUNCTION) {
				auto *l = o.interpreter();
				auto numRet = ev.get().GetReturnCount();
				auto c = Lua::CallFunction(
				  l,
				  [&ev, &o, &r, &methodName](lua_State *l) -> Lua::StatusCode {
					  r.push(l);
					  o.push(l);
					  ev.get().PushArguments(l);
					  return Lua::StatusCode::Ok;
				  },
				  1 + numRet);
				if(c == Lua::StatusCode::Ok) {
					auto r = ::util::EventReply::Unhandled;
					auto idxFirst = -static_cast<int32_t>(numRet + 1);
					if(Lua::IsSet(l, idxFirst) && Lua::IsNumber(l, idxFirst))
						r = static_cast<::util::EventReply>(Lua::CheckInt(l, idxFirst));
					Lua::RemoveValue(l, idxFirst); /* pop handled value */

					ev.get().HandleReturnValues(l);
					Lua::Pop(l, numRet);
					return r;
				}
			}
			return ::util::EventReply::Unhandled;
		});
		Lua::Push<CallbackHandle>(l, hCb);
		return hCb;
	}
	Lua::CheckFunction(l, argTarget);
	auto hCb = BindEvent(eventId, [methodNameOrFunction](std::reference_wrapper<pragma::ComponentEvent> ev) -> ::util::EventReply {
		auto &fc = methodNameOrFunction;
		auto *l = fc.interpreter();

		auto numRet = ev.get().GetReturnCount();
		auto c = Lua::CallFunction(
		  l,
		  [&fc, &ev](lua_State *l) -> Lua::StatusCode {
			  fc.push(l);
			  ev.get().PushArguments(l);
			  return Lua::StatusCode::Ok;
		  },
		  1 + numRet);
		if(c == Lua::StatusCode::Ok) {
			auto r = ::util::EventReply::Unhandled;
			if(Lua::IsSet(l, -1))
				r = static_cast<::util::EventReply>(Lua::CheckInt(l, -1));
			Lua::Pop(l, 1); /* pop handled value */

			ev.get().HandleReturnValues(l);
			Lua::Pop(l, numRet);
			return r;
		}
		return ::util::EventReply::Unhandled;
	});
	Lua::Push<CallbackHandle>(l, hCb);
	return hCb;
}

static pragma::BaseLuaBaseEntityComponent::MemberFlags string_to_member_flag(lua_State *l, const std::string_view &str)
{
	using namespace ustring::string_switch;
	switch(hash(str)) {
	case "prop"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::PropertyBit;
	case "get"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::GetterBit;
	case "set"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::SetterBit;
	case "sav"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::StoreBit;
	case "kv"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::KeyValueBit;
	case "in"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::InputBit;
	case "out"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::OutputBit;
	case "io"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::InputBit | pragma::BaseLuaBaseEntityComponent::MemberFlags::OutputBit;
	case "net"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::NetworkedBit;
	case "has"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::UseHasGetterBit;
	case "is"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::UseIsGetterBit;
	case "trans"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::TransmitOnChange;
	case "snap"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::SnapshotData;
	case "def"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::Default;
	case "defnw"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultNetworked;
	case "deftrans"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultTransmit;
	case "defsnap"_:
		return pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultSnapshot;
	}
	Lua::Error(l, "Invalid definition of component member flags: Flag '" + std::string {str} + "' is not a recognized flag! Valid flags are: prop, get, set, sav, kv, in, out, io, net, has, is, trans, snap, def, defnw, deftrans, defsnap.");
	return pragma::BaseLuaBaseEntityComponent::MemberFlags::None;
}

pragma::BaseLuaBaseEntityComponent::MemberFlags pragma::lua::string_to_member_flags(lua_State *l, std::string_view strFlags)
{
	auto flags = pragma::BaseLuaBaseEntityComponent::MemberFlags::None;
	auto flagsRem = pragma::BaseLuaBaseEntityComponent::MemberFlags::None;

	size_t start = strFlags.empty() ? std::string::npos : 0;
	while(start != std::string::npos) {
		auto end = strFlags.find_first_of("+-", start + 1);
		auto sub = (end != std::string::npos) ? strFlags.substr(start, end - start) : strFlags.substr(start);
		if(sub.length() > 1) {
			// Very first sign of string may be omitted
			auto hasSign = (sub[0] == '+' || sub[0] == '-');
			auto sign = hasSign ? sub[0] : '+';
			auto val = sub.substr(hasSign ? 1 : 0);
			auto flag = string_to_member_flag(l, val);
			if(flag != pragma::BaseLuaBaseEntityComponent::MemberFlags::None) {
				switch(sign) {
				case '+':
					flags |= flag;
					break;
				case '-':
					flagsRem |= flag;
					break;
				default:
					Lua::Error(l, "Invalid definition of component member flags (" + std::string {strFlags} + ")! Every flag has to be prefixed by '+' or '-'!");
				}
			}
		}
		if(end == std::string::npos)
			break;
		strFlags = strFlags.substr(end);
		start = 0;
	}
	flags &= ~flagsRem;
	return flags;
}

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseEntityComponent);
#endif

void Lua::register_base_entity_component(luabind::module_ &modEnts)
{
	auto classDef = pragma::lua::create_entity_component_class<pragma::BaseLuaBaseEntityComponent, pragma::BaseEntityComponent>("BaseBaseEntityComponent");
	classDef.def("SetNetworked", &pragma::BaseLuaBaseEntityComponent::SetNetworked);
	classDef.def("IsNetworked", &pragma::BaseLuaBaseEntityComponent::IsNetworked);
	classDef.def("SetShouldTransmitSnapshotData", &pragma::BaseLuaBaseEntityComponent::SetShouldTransmitSnapshotData);
	classDef.def("ShouldTransmitSnapshotData", &pragma::BaseLuaBaseEntityComponent::ShouldTransmitSnapshotData);
	classDef.def("GetVersion", &pragma::BaseLuaBaseEntityComponent::GetVersion);
	classDef.def("FlagCallbackForRemoval",
	  static_cast<void (*)(lua_State *, pragma::BaseLuaBaseEntityComponent &, CallbackHandle &, pragma::BaseEntityComponent::CallbackType, pragma::BaseLuaBaseEntityComponent &)>(
	    [](lua_State *l, pragma::BaseLuaBaseEntityComponent &hComponent, CallbackHandle &hCb, pragma::BaseEntityComponent::CallbackType callbackType, pragma::BaseLuaBaseEntityComponent &hComponentOther) {
		    if(hCb.IsValid() == false)
			    return;
		    hComponent.FlagCallbackForRemoval(hCb, callbackType, &hComponentOther);
	    }));
	classDef.def("FlagCallbackForRemoval",
	  static_cast<void (*)(lua_State *, pragma::BaseLuaBaseEntityComponent &, CallbackHandle &, pragma::BaseEntityComponent::CallbackType)>([](lua_State *l, pragma::BaseLuaBaseEntityComponent &hComponent, CallbackHandle &hCb, pragma::BaseEntityComponent::CallbackType callbackType) {
		  if(hCb.IsValid() == false)
			  return;
		  hComponent.FlagCallbackForRemoval(hCb, callbackType);
	  }));
	classDef.def("BindComponentInitEvent", static_cast<CallbackHandle (*)(lua_State *, pragma::BaseLuaBaseEntityComponent &, uint32_t, luabind::object)>([](lua_State *l, pragma::BaseLuaBaseEntityComponent &hComponent, uint32_t componentId, luabind::object methodNameOrFunction) {
		return hComponent.BindInitComponentEvent(l, componentId, methodNameOrFunction);
	}));
	classDef.def("BindEvent", static_cast<CallbackHandle (*)(lua_State *, pragma::BaseLuaBaseEntityComponent &, uint32_t, luabind::object)>([](lua_State *l, pragma::BaseLuaBaseEntityComponent &hComponent, uint32_t eventId, luabind::object methodNameOrFunction) {
		return hComponent.BindEvent(l, eventId, methodNameOrFunction);
	}));
	classDef.def(
	  "GetEntityComponent", +[](lua_State *l, pragma::BaseLuaBaseEntityComponent &hComponent, Lua::nil_type) { return; });
	classDef.def("GetEntityComponent",
	  static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (*)(lua_State *, pragma::BaseLuaBaseEntityComponent &, uint32_t)>(
	    [](lua_State *l, pragma::BaseLuaBaseEntityComponent &hComponent, uint32_t componentId) -> pragma::ComponentHandle<pragma::BaseEntityComponent> { return hComponent.GetEntity().FindComponent(componentId); }));
	classDef.def("GetEntityComponent",
	  static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (*)(lua_State *, pragma::BaseLuaBaseEntityComponent &, const std::string &)>(
	    [](lua_State *l, pragma::BaseLuaBaseEntityComponent &hComponent, const std::string &componentName) -> pragma::ComponentHandle<pragma::BaseEntityComponent> { return hComponent.GetEntity().FindComponent(componentName); }));
	classDef.def("AddEntityComponent",
	  static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (*)(lua_State *, pragma::BaseLuaBaseEntityComponent &, uint32_t)>(
	    [](lua_State *l, pragma::BaseLuaBaseEntityComponent &hComponent, uint32_t componentId) -> pragma::ComponentHandle<pragma::BaseEntityComponent> { return hComponent.GetEntity().AddComponent(componentId); }));
	classDef.def("AddEntityComponent",
	  static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (*)(lua_State *, pragma::BaseLuaBaseEntityComponent &, uint32_t, luabind::object)>(
	    [](lua_State *l, pragma::BaseLuaBaseEntityComponent &hComponent, uint32_t componentId, luabind::object methodNameOrFunction) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
		    hComponent.BindInitComponentEvent(l, componentId, methodNameOrFunction);
		    return hComponent.GetEntity().AddComponent(componentId);
	    }));
	classDef.def("AddEntityComponent",
	  static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (*)(lua_State *, pragma::BaseLuaBaseEntityComponent &, const std::string &)>(
	    [](lua_State *l, pragma::BaseLuaBaseEntityComponent &hComponent, const std::string &name) -> pragma::ComponentHandle<pragma::BaseEntityComponent> { return hComponent.GetEntity().AddComponent(name); }));
	classDef.def("AddEntityComponent",
	  static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (*)(lua_State *, pragma::BaseLuaBaseEntityComponent &, const std::string &, luabind::object)>(
	    [](lua_State *l, pragma::BaseLuaBaseEntityComponent &hComponent, const std::string &name, luabind::object methodNameOrFunction) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
		    auto hNewComponent = hComponent.GetEntity().AddComponent(name);
		    if(hNewComponent.expired())
			    return hNewComponent;
		    hComponent.BindInitComponentEvent(l, hNewComponent->GetComponentId(), methodNameOrFunction);
		    return hNewComponent;
	    }));
	classDef.def("OnMemberValueChanged", &pragma::BaseLuaBaseEntityComponent::OnMemberValueChanged);
	classDef.scope[luabind::def(
	  "RegisterMember", +[](lua_State *l, const Lua::classObject &o, const std::string &memberName, pragma::ents::EntityMemberType memberType, Lua::udm_type oDefault, const Lua::map<std::string, void> &attributes, pragma::BaseLuaBaseEntityComponent::MemberFlags memberFlags) {
		  auto anyInitialValue = Lua::GetAnyValue(l, detail::member_type_to_util_type(memberType), 4);
		  pragma::BaseLuaBaseEntityComponent::RegisterMember(o, memberName, memberType, anyInitialValue, memberFlags, attributes);
	  })];
	classDef.scope[luabind::def(
	  "RegisterMember", +[](lua_State *l, const Lua::classObject &o, const std::string &memberName, pragma::ents::EntityMemberType memberType, Lua::udm_type oDefault, const std::string &memberFlags) {
		  auto anyInitialValue = Lua::GetAnyValue(l, detail::member_type_to_util_type(memberType), 4);
		  pragma::BaseLuaBaseEntityComponent::RegisterMember(o, memberName, memberType, anyInitialValue, pragma::lua::string_to_member_flags(l, memberFlags), luabind::newtable(l));
	  })];
	classDef.scope[luabind::def(
	  "RegisterMember", +[](lua_State *l, const Lua::classObject &o, const std::string &memberName, pragma::ents::EntityMemberType memberType, Lua::udm_type oDefault, const Lua::map<std::string, void> &attributes, const std::string &memberFlags) {
		  auto anyInitialValue = Lua::GetAnyValue(l, detail::member_type_to_util_type(memberType), 4);
		  pragma::BaseLuaBaseEntityComponent::RegisterMember(o, memberName, memberType, anyInitialValue, pragma::lua::string_to_member_flags(l, memberFlags), attributes);
	  })];
	classDef.scope[luabind::def(
	  "RegisterMember", +[](lua_State *l, const Lua::classObject &o, const std::string &memberName, pragma::ents::EntityMemberType memberType, Lua::udm_type oDefault, const Lua::map<std::string, void> &attributes) {
		  auto anyInitialValue = Lua::GetAnyValue(l, detail::member_type_to_util_type(memberType), 4);
		  pragma::BaseLuaBaseEntityComponent::RegisterMember(o, memberName, memberType, anyInitialValue, pragma::BaseLuaBaseEntityComponent::MemberFlags::Default, attributes);
	  })];
	classDef.scope[luabind::def(
	  "RegisterMember", +[](lua_State *l, const Lua::classObject &o, const std::string &memberName, pragma::ents::EntityMemberType memberType, Lua::udm_type oDefault) {
		  auto anyInitialValue = Lua::GetAnyValue(l, detail::member_type_to_util_type(memberType), 4);
		  pragma::BaseLuaBaseEntityComponent::RegisterMember(o, memberName, memberType, anyInitialValue, pragma::BaseLuaBaseEntityComponent::MemberFlags::Default, luabind::newtable(l));
	  })];
	classDef.scope[luabind::def("RegisterNetEvent", &Game::SetupNetEvent)];

	classDef.add_static_constant("MEMBER_FLAG_NONE", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::None));
	classDef.add_static_constant("MEMBER_FLAG_BIT_PROPERTY", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::PropertyBit));
	classDef.add_static_constant("MEMBER_FLAG_BIT_GETTER", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::GetterBit));
	classDef.add_static_constant("MEMBER_FLAG_BIT_SETTER", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::SetterBit));
	classDef.add_static_constant("MEMBER_FLAG_BIT_STORE", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::StoreBit));
	classDef.add_static_constant("MEMBER_FLAG_BIT_KEY_VALUE", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::KeyValueBit));
	classDef.add_static_constant("MEMBER_FLAG_BIT_INPUT", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::InputBit));
	classDef.add_static_constant("MEMBER_FLAG_BIT_OUTPUT", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::OutputBit));
	classDef.add_static_constant("MEMBER_FLAG_BIT_NETWORKED", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::NetworkedBit));
	classDef.add_static_constant("MEMBER_FLAG_BIT_USE_HAS_GETTER", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::UseHasGetterBit));
	classDef.add_static_constant("MEMBER_FLAG_BIT_USE_IS_GETTER", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::UseIsGetterBit));

	classDef.add_static_constant("MEMBER_FLAG_TRANSMIT_ON_CHANGE", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::TransmitOnChange));
	classDef.add_static_constant("MEMBER_FLAG_SNAPSHOT_DATA", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::SnapshotData));
	classDef.add_static_constant("MEMBER_FLAG_DEFAULT", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::Default));
	classDef.add_static_constant("MEMBER_FLAG_DEFAULT_NETWORKED", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultNetworked));
	classDef.add_static_constant("MEMBER_FLAG_DEFAULT_TRANSMIT", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultTransmit));
	classDef.add_static_constant("MEMBER_FLAG_DEFAULT_SNAPSHOT", umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultSnapshot));

	classDef.def("Initialize", &pragma::BaseLuaBaseEntityComponent::Lua_Initialize, &pragma::BaseLuaBaseEntityComponent::default_Lua_Initialize);
	classDef.def("OnTick", &pragma::BaseLuaBaseEntityComponent::Lua_OnTick, &pragma::BaseLuaBaseEntityComponent::default_Lua_OnTick);
	classDef.def("OnRemove", &pragma::BaseLuaBaseEntityComponent::Lua_OnRemove, &pragma::BaseLuaBaseEntityComponent::default_Lua_OnRemove);
	classDef.def("OnEntitySpawn", &pragma::BaseLuaBaseEntityComponent::Lua_OnEntitySpawn, &pragma::BaseLuaBaseEntityComponent::default_Lua_OnEntitySpawn);
	classDef.def("OnEntityPostSpawn", &pragma::BaseLuaBaseEntityComponent::Lua_OnEntityPostSpawn, &pragma::BaseLuaBaseEntityComponent::default_Lua_OnEntityPostSpawn);
	classDef.def("OnActiveStateChanged", &pragma::BaseLuaBaseEntityComponent::Lua_OnActiveStateChanged, &pragma::BaseLuaBaseEntityComponent::default_Lua_OnActiveStateChanged);
	classDef.def("OnAttachedToEntity", &pragma::BaseLuaBaseEntityComponent::Lua_OnAttachedToEntity, &pragma::BaseLuaBaseEntityComponent::default_Lua_OnAttachedToEntity);
	classDef.def("OnDetachedFromEntity", &pragma::BaseLuaBaseEntityComponent::Lua_OnDetachedToEntity, &pragma::BaseLuaBaseEntityComponent::default_Lua_OnDetachedToEntity);
	// classDef.def("OnEntityComponentAdded",&pragma::BaseLuaBaseEntityComponent::Lua_OnEntityComponentAdded,&pragma::BaseLuaBaseEntityComponent::default_Lua_OnEntityComponentAdded);
	// classDef.def("OnEntityComponentRemoved",&pragma::BaseLuaBaseEntityComponent::Lua_OnEntityComponentRemoved,&pragma::BaseLuaBaseEntityComponent::default_Lua_OnEntityComponentRemoved);
	// HandleEvent is variadic and can't be defined like this in luabind!
	//classDef.def("HandleEvent",&pragma::BaseLuaBaseEntityComponent::Lua_HandleEvent,&pragma::BaseLuaBaseEntityComponent::default_Lua_HandleEvent);
	classDef.def("Save", &pragma::BaseLuaBaseEntityComponent::Lua_Save, &pragma::BaseLuaBaseEntityComponent::default_Lua_Save);
	classDef.def("Load", &pragma::BaseLuaBaseEntityComponent::Lua_Load, &pragma::BaseLuaBaseEntityComponent::default_Lua_Load);
	modEnts[classDef];
}

BaseLuaBaseEntityComponent::MemberInfo::MemberInfo(const std::string &functionName, const std::string &memberName, size_t memberNameHash, const std::string &memberVariableName, ents::EntityMemberType type, const std::any &initialValue, BaseLuaBaseEntityComponent::MemberFlags flags,
  const luabind::object &onChange, const std::optional<ComponentMemberInfo> &componentMemberInfo)
    : functionName(functionName), memberName(memberName), memberNameHash(memberNameHash), memberVariableName(memberVariableName), type(type), initialValue(initialValue), flags(flags), onChange(onChange), componentMemberInfo(componentMemberInfo)
{
}

BaseLuaBaseEntityComponent::MemberInfo::MemberInfo(const MemberInfo &other)
    : functionName(other.functionName), memberName(other.memberName), memberNameHash(other.memberNameHash), memberVariableName(other.memberVariableName), type(other.type), initialValue(other.initialValue), flags(other.flags), onChange(other.onChange),
      transformCompositeInfo(other.transformCompositeInfo ? std::make_unique<TransformCompositeInfo>(*other.transformCompositeInfo) : nullptr), componentMemberInfo(other.componentMemberInfo)
{
}

BaseLuaBaseEntityComponent::MemberInfo::MemberInfo(MemberInfo &&other)
    : functionName(std::move(other.functionName)), memberName(std::move(other.memberName)), memberNameHash(std::move(other.memberNameHash)), memberVariableName(std::move(other.memberVariableName)), type(std::move(other.type)), initialValue(std::move(other.initialValue)),
      flags(std::move(other.flags)), onChange(std::move(other.onChange)), transformCompositeInfo(std::move(other.transformCompositeInfo)), componentMemberInfo(std::move(other.componentMemberInfo))
{
	other.transformCompositeInfo = nullptr;
}

BaseLuaBaseEntityComponent::MemberInfo &BaseLuaBaseEntityComponent::MemberInfo::operator=(const MemberInfo &other)
{
	functionName = other.functionName;
	memberName = other.memberName;
	memberNameHash = other.memberNameHash;
	memberVariableName = other.memberVariableName;
	type = other.type;
	initialValue = other.initialValue;
	flags = other.flags;
	onChange = other.onChange;
	transformCompositeInfo = other.transformCompositeInfo ? std::make_unique<TransformCompositeInfo>(*other.transformCompositeInfo) : nullptr;
	componentMemberInfo = other.componentMemberInfo;
	return *this;
}

BaseLuaBaseEntityComponent::MemberInfo &BaseLuaBaseEntityComponent::MemberInfo::operator=(MemberInfo &&other)
{
	functionName = std::move(other.functionName);
	memberName = std::move(other.memberName);
	memberNameHash = std::move(other.memberNameHash);
	memberVariableName = std::move(other.memberVariableName);
	type = std::move(other.type);
	initialValue = std::move(other.initialValue);
	flags = std::move(other.flags);
	onChange = std::move(other.onChange);
	transformCompositeInfo = std::move(other.transformCompositeInfo);
	componentMemberInfo = std::move(other.componentMemberInfo);
	other.transformCompositeInfo = nullptr;
	return *this;
}
