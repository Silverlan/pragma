// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.ents;

//import pragma.scripting.lua;

enum class TypeMetaData : uint32_t { Range = 0, Coordinate, Pose, PoseComponent, Optional, Enabler, Parent, Count };
std::optional<std::type_index> type_meta_data_to_type_index(TypeMetaData eType)
{
	switch(eType) {
	case TypeMetaData::Range:
		return typeid(pragma::ents::RangeTypeMetaData);
	case TypeMetaData::Coordinate:
		return typeid(pragma::ents::CoordinateTypeMetaData);
	case TypeMetaData::Pose:
		return typeid(pragma::ents::PoseTypeMetaData);
	case TypeMetaData::PoseComponent:
		return typeid(pragma::ents::PoseComponentTypeMetaData);
	case TypeMetaData::Optional:
		return typeid(pragma::ents::OptionalTypeMetaData);
	case TypeMetaData::Enabler:
		return typeid(pragma::ents::EnablerTypeMetaData);
	case TypeMetaData::Parent:
		return typeid(pragma::ents::ParentTypeMetaData);
	}
	static_assert(pragma::math::to_integral(TypeMetaData::Count) == 7, "Update this implementation when adding news types!");
	return {};
}
luabind::object meta_data_type_to_lua_object(lua::State *l, const pragma::ents::TypeMetaData &metaData, TypeMetaData eType)
{
	switch(eType) {
	case TypeMetaData::Range:
		return luabind::object {l, static_cast<const pragma::ents::RangeTypeMetaData *>(&metaData)};
	case TypeMetaData::Coordinate:
		return luabind::object {l, static_cast<const pragma::ents::CoordinateTypeMetaData *>(&metaData)};
	case TypeMetaData::Pose:
		return luabind::object {l, static_cast<const pragma::ents::PoseTypeMetaData *>(&metaData)};
	case TypeMetaData::PoseComponent:
		return luabind::object {l, static_cast<const pragma::ents::PoseComponentTypeMetaData *>(&metaData)};
	case TypeMetaData::Optional:
		return luabind::object {l, static_cast<const pragma::ents::OptionalTypeMetaData *>(&metaData)};
	case TypeMetaData::Enabler:
		return luabind::object {l, static_cast<const pragma::ents::EnablerTypeMetaData *>(&metaData)};
	case TypeMetaData::Parent:
		return luabind::object {l, static_cast<const pragma::ents::ParentTypeMetaData *>(&metaData)};
	}
	static_assert(pragma::math::to_integral(TypeMetaData::Count) == 7, "Update this implementation when adding news types!");
	return Lua::nil;
}

static std::optional<pragma::ComponentId> find_component_id_by_class(pragma::Game &game, const luabind::object &o)
{
	auto &manager = game.GetLuaEntityManager();
	auto componentId = manager.FindComponentId(o);
	return componentId;
}
size_t Lua::ents::get_lua_component_member_count(pragma::Game &game, pragma::ComponentId componentId)
{
	auto &manager = game.GetLuaEntityManager();
	auto *o = manager.FindClassObject(componentId);
	if(!o)
		return 0;
	auto *memberInfos = pragma::BaseLuaBaseEntityComponent::GetMemberInfos(*o);
	if(!memberInfos)
		return 0;
	return memberInfos->size();
}
pragma::ComponentMemberInfo *Lua::ents::get_lua_component_member_info(pragma::Game &game, pragma::ComponentId componentId, pragma::ComponentMemberIndex memberIndex)
{
	auto &manager = game.GetLuaEntityManager();
	auto *o = manager.FindClassObject(componentId);
	if(!o)
		return nullptr;
	auto *memberInfos = pragma::BaseLuaBaseEntityComponent::GetMemberInfos(*o);
	if(!memberInfos || memberIndex >= memberInfos->size())
		return nullptr;
	return &*(*memberInfos)[memberIndex].componentMemberInfo;
}

//void test_lua_policies(lua::State *l);
void Lua::ents::register_library(lua::State *l)
{
	//test_lua_policies(l);
	auto entsMod = luabind::module(l, "ents");
	entsMod[(
		luabind::def("create",create),

		luabind::def("get_all",static_cast<tb<type<pragma::ecs::BaseEntity>>(*)(lua::State*)>(get_all)),
		luabind::def("get_all",static_cast<tb<type<pragma::ecs::BaseEntity>>(*)(lua::State*,func<type<pragma::ecs::BaseEntity>>)>(get_all)),
		luabind::def("get_all",static_cast<tb<type<pragma::ecs::BaseEntity>>(*)(lua::State*,pragma::ecs::EntityIterator::FilterFlags)>(get_all)),
		luabind::def("get_all",static_cast<tb<type<pragma::ecs::BaseEntity>>(*)(lua::State*,pragma::ecs::EntityIterator::FilterFlags,const tb<LuaEntityIteratorFilterBase>&)>(get_all)),
		luabind::def("get_all",static_cast<tb<type<pragma::ecs::BaseEntity>>(*)(lua::State*,const tb<LuaEntityIteratorFilterBase>&)>(get_all)),
		luabind::def("get_all_c",static_cast<tb<type<pragma::BaseEntityComponent>>(*)(lua::State*,func<type<pragma::BaseEntityComponent>>)>(get_all_c)),
		luabind::def("get_spawned",get_spawned),
		luabind::def("get_players",get_players),
		luabind::def("get_npcs",get_npcs),
		luabind::def("get_weapons",get_weapons),
		luabind::def("get_vehicles",get_vehicles),
		luabind::def("get_world",get_world),
		luabind::def("get_by_index",get_by_index),
		luabind::def("get_by_local_index",get_by_local_index),
		luabind::def("find_by_uuid",find_by_unique_index),
		luabind::def("get_null",get_null),
		luabind::def("find_by_filter",find_by_filter),
		luabind::def("find_by_class",find_by_class),
		luabind::def("find_by_name",find_by_name),
		luabind::def("find_by_component",find_by_component),
		luabind::def("find_in_sphere",find_in_sphere),
		luabind::def("find_in_aabb",find_in_aabb),
		luabind::def("find_in_cone",find_in_cone),
		luabind::def("create_trigger",static_cast<type<pragma::ecs::BaseEntity>(*)(lua::State*,const Vector3&,pragma::physics::IShape&)>(create_trigger)),
		luabind::def("create_trigger",static_cast<type<pragma::ecs::BaseEntity>(*)(lua::State*,const Vector3&,const Vector3&,const Vector3&,const EulerAngles&)>(create_trigger)),
		luabind::def("create_trigger",static_cast<type<pragma::ecs::BaseEntity>(*)(lua::State*,const Vector3&,float)>(create_trigger)),
		luabind::def("create_prop",&create_prop),
		luabind::def("create_prop",+[](lua::State *l,const std::string &mdl,const Vector3 *origin,const EulerAngles *angles) -> type<pragma::ecs::BaseEntity> {
			return create_prop(l,mdl,origin,angles,false);
		}),
		luabind::def("create_prop",+[](lua::State *l,const std::string &mdl,const Vector3 *origin) -> type<pragma::ecs::BaseEntity> {
			return create_prop(l,mdl,origin,nullptr,false);
		}),
		luabind::def("create_prop",+[](lua::State *l,const std::string &mdl) -> type<pragma::ecs::BaseEntity> {
			return create_prop(l,mdl,nullptr,nullptr,false);
		}),
		luabind::def("register",static_cast<void(*)(lua::State*,const std::string&,const classObject&)>(register_class)),
		luabind::def("register",static_cast<void(*)(lua::State*,const std::string&,const luabind::tableT<luabind::variant<std::string,pragma::ComponentId>>&,LuaEntityType)>(register_class)),
		luabind::def("register",+[](lua::State *l,const std::string &className,const luabind::tableT<luabind::variant<std::string,pragma::ComponentId>> &tComponents) {
			register_class(l,className,tComponents,LuaEntityType::Default);
		}),
		luabind::def("get_lua_component_member_count",get_lua_component_member_count),
		luabind::def("get_lua_component_member_info",get_lua_component_member_info),
		luabind::def("get_closest",get_closest),
		luabind::def("get_farthest",get_farthest),
		luabind::def("get_sorted_by_distance",get_sorted_by_distance),
		luabind::def("get_random",get_random),
		luabind::def("get_component_name",get_component_name),
		luabind::def("get_component_id",get_component_id),
		luabind::def("register_component_net_event",register_component_net_event),
		luabind::def("get_registered_component_types",+[](lua::State *l,pragma::Game &game) -> tb<pragma::ComponentId> {
			auto &manager = game.GetEntityComponentManager();
			auto t = luabind::newtable(l);
			for(uint32_t idx = 1;auto &componentInfo : manager.GetRegisteredComponentTypes())
			{
				if(componentInfo->id == pragma::INVALID_COMPONENT_ID)
					continue;
				t[idx++] = componentInfo->id;
			}
			return t;
		}),
		luabind::def("get_component_info",+[](lua::State *l,pragma::Game &game,pragma::ComponentId componentId) {
			auto &manager = game.GetEntityComponentManager();
			return manager.GetComponentInfo(componentId);
		}),
		luabind::def("find_component_id",+[](lua::State *l,pragma::Game &game,const std::string &name) -> std::optional<pragma::ComponentId> {
			auto &manager = game.GetEntityComponentManager();
			pragma::ComponentId componentId;
			if(manager.GetComponentTypeId(name,componentId) == false)
				return {};
			return componentId;
		}),
		luabind::def("load_component",&pragma::Game::LoadLuaComponentByName),
		luabind::def("find_installed_custom_components",+[](lua::State *l,pragma::Game &game) -> tb<std::string> {
			std::vector<std::string> dirs;
			std::string rootPath {SCRIPT_DIRECTORY +"/entities/components/"};
			pragma::fs::find_files(rootPath +'*',nullptr,&dirs);
			auto customComponents = luabind::newtable(l);
			auto nwStateDirName = game.GetLuaNetworkDirectoryName();
			auto luaFileName = game.GetLuaNetworkFileName();

			auto nwStateDirNameC = nwStateDirName;
			auto luaFileNameC = luaFileName;
			ufile::remove_extension_from_filename(nwStateDirNameC,std::vector<std::string>{FILE_EXTENSION});
		    ufile::remove_extension_from_filename(luaFileNameC, std::vector<std::string> {FILE_EXTENSION});
			nwStateDirNameC += DOT_FILE_EXTENSION_PRECOMPILED;
		    luaFileNameC += DOT_FILE_EXTENSION_PRECOMPILED;
			for(uint32_t idx=1;auto &dir : dirs)
			{
				auto cdir = rootPath +dir +'/';
				if(
					pragma::fs::exists(cdir +luaFileName) || pragma::fs::exists(cdir +nwStateDirName +'/' +luaFileName) ||
					pragma::fs::exists(cdir +luaFileNameC) || pragma::fs::exists(cdir +nwStateDirName +'/' +luaFileNameC)
				)
					customComponents[idx++] = dir;
			}
			return customComponents;
		}),
		luabind::def("get_registered_entity_types",+[](pragma::Game &game) -> std::vector<std::string> {
			std::vector<std::string> entities;
			game.GetRegisteredEntities(entities,entities);
			return entities;
		}),
		luabind::def("add_component_creation_listener",+[](pragma::Game &game,pragma::ComponentId id,func<void,pragma::BaseEntityComponent> callback) {
			pragma::ecs::EntityIterator it {game};
			it.AttachFilter<EntityIteratorFilterComponent>(id);
			auto entIt = it.begin();
			if(entIt != it.end())
			{
				auto *ent = *entIt;
				callback(ent->FindComponent(id)->GetLuaObject());
			}

			return game.GetEntityComponentManager().AddCreationCallback(id,[callback](std::reference_wrapper<pragma::BaseEntityComponent> c) mutable {
				callback(c.get().GetLuaObject());
			});
		}),
		luabind::def("add_component_creation_listener",+[](pragma::Game &game,const std::string &componentName,func<void,pragma::BaseEntityComponent> callback) {
			pragma::ecs::EntityIterator it {game};
			it.AttachFilter<EntityIteratorFilterComponent>(componentName);
			auto entIt = it.begin();
			if(entIt != it.end())
			{
				auto *ent = *entIt;
				callback(ent->FindComponent(componentName)->GetLuaObject());
			}

			return game.GetEntityComponentManager().AddCreationCallback(componentName,[callback](std::reference_wrapper<pragma::BaseEntityComponent> c) mutable {
				callback(c.get().GetLuaObject());
			});
		}),
		luabind::def("link_component_type",+[](pragma::Game &game,pragma::ComponentId linkFrom,pragma::ComponentId linkTo) {
			game.GetEntityComponentManager().LinkComponentType(linkFrom,linkTo);
		}),
		luabind::def("parse_uri",+[](const std::string &uriPath) -> std::optional<pragma::EntityUComponentMemberRef> {
			pragma::EntityUComponentMemberRef ref;
			if(!pragma::ecs::BaseEntity::ParseUri(uriPath,ref))
				return {};
			return ref;
		}),
		luabind::def("parse_uri",+[](const std::string &uriPath,const util::Uuid &uuid) -> std::optional<pragma::EntityUComponentMemberRef> {
			pragma::EntityUComponentMemberRef ref;
			if(!pragma::ecs::BaseEntity::ParseUri(uriPath,ref,&uuid.value))
				return {};
			return ref;
		}),
		luabind::def("create_uri",+[](const util::Uuid &uuid,const std::string &propName) -> std::string {
			return "pragma:game/entity/" +propName +"?entity_uuid=" +pragma::util::uuid_to_string(uuid.value);
		}),
		luabind::def("create_uri",+[](const std::string &uuid,const std::string &propName) -> std::string {
			return "pragma:game/entity/" +propName +"?entity_uuid=" +uuid;
		}),
		luabind::def("create_uri",+[](const std::string &propName) -> std::string {
			return "pragma:game/entity/" +propName;
		}),
		luabind::def("create_entity_uri",+[](const util::Uuid &uuid) -> std::string {
			return "pragma:game/entity?entity_uuid=" +pragma::util::uuid_to_string(uuid.value);
		}),
		luabind::def("create_component_uri",+[](const util::Uuid &uuid,const std::string &component) -> std::string {
			return "pragma:game/entity/ec/" +component +"?entity_uuid=" +pragma::util::uuid_to_string(uuid.value);
		}),
		luabind::def("is_member_type_animatable",static_cast<bool(*)(pragma::ents::EntityMemberType)>(&pragma::is_animatable_type)),
		luabind::def("is_member_type_udm_type",&pragma::ents::is_udm_member_type),
		luabind::def("member_type_to_string",+[](pragma::ents::EntityMemberType memberType) -> std::string {
		return std::string { magic_enum::enum_name(memberType)};
			}),
		luabind::def("string_to_member_type",+[](const std::string &memberType) -> std::optional<pragma::ents::EntityMemberType> {
			return magic_enum::enum_cast<pragma::ents::EntityMemberType>(memberType);
			})
	)];
	static_assert(pragma::math::to_integral(pragma::ents::EntityMemberType::VersionIndex) == 0);
	RegisterLibraryEnums(l, "ents",
	  {
	    {"MEMBER_TYPE_STRING", pragma::math::to_integral(pragma::ents::EntityMemberType::String)},
	    {"MEMBER_TYPE_INT8", pragma::math::to_integral(pragma::ents::EntityMemberType::Int8)},
	    {"MEMBER_TYPE_UINT8", pragma::math::to_integral(pragma::ents::EntityMemberType::UInt8)},
	    {"MEMBER_TYPE_INT16", pragma::math::to_integral(pragma::ents::EntityMemberType::Int16)},
	    {"MEMBER_TYPE_UINT16", pragma::math::to_integral(pragma::ents::EntityMemberType::UInt16)},
	    {"MEMBER_TYPE_INT32", pragma::math::to_integral(pragma::ents::EntityMemberType::Int32)},
	    {"MEMBER_TYPE_UINT32", pragma::math::to_integral(pragma::ents::EntityMemberType::UInt32)},
	    {"MEMBER_TYPE_INT64", pragma::math::to_integral(pragma::ents::EntityMemberType::Int64)},
	    {"MEMBER_TYPE_UINT64", pragma::math::to_integral(pragma::ents::EntityMemberType::UInt64)},
	    {"MEMBER_TYPE_FLOAT", pragma::math::to_integral(pragma::ents::EntityMemberType::Float)},
	    {"MEMBER_TYPE_DOUBLE", pragma::math::to_integral(pragma::ents::EntityMemberType::Double)},
	    {"MEMBER_TYPE_BOOLEAN", pragma::math::to_integral(pragma::ents::EntityMemberType::Boolean)},
	    {"MEMBER_TYPE_VECTOR2", pragma::math::to_integral(pragma::ents::EntityMemberType::Vector2)},
	    {"MEMBER_TYPE_VECTOR3", pragma::math::to_integral(pragma::ents::EntityMemberType::Vector3)},
	    {"MEMBER_TYPE_VECTOR4", pragma::math::to_integral(pragma::ents::EntityMemberType::Vector4)},
	    {"MEMBER_TYPE_QUATERNION", pragma::math::to_integral(pragma::ents::EntityMemberType::Quaternion)},
	    {"MEMBER_TYPE_EULER_ANGLES", pragma::math::to_integral(pragma::ents::EntityMemberType::EulerAngles)},
	    {"MEMBER_TYPE_SRGBA", pragma::math::to_integral(pragma::ents::EntityMemberType::Srgba)},
	    {"MEMBER_TYPE_HDR_COLOR", pragma::math::to_integral(pragma::ents::EntityMemberType::HdrColor)},
	    {"MEMBER_TYPE_TRANSFORM", pragma::math::to_integral(pragma::ents::EntityMemberType::Transform)},
	    {"MEMBER_TYPE_SCALED_TRANSFORM", pragma::math::to_integral(pragma::ents::EntityMemberType::ScaledTransform)},
	    {"MEMBER_TYPE_MAT4", pragma::math::to_integral(pragma::ents::EntityMemberType::Mat4)},
	    {"MEMBER_TYPE_MAT3X4", pragma::math::to_integral(pragma::ents::EntityMemberType::Mat3x4)},
	    {"MEMBER_TYPE_HALF", pragma::math::to_integral(pragma::ents::EntityMemberType::Half)},
	    {"MEMBER_TYPE_VECTOR2I", pragma::math::to_integral(pragma::ents::EntityMemberType::Vector2i)},
	    {"MEMBER_TYPE_VECTOR3I", pragma::math::to_integral(pragma::ents::EntityMemberType::Vector3i)},
	    {"MEMBER_TYPE_VECTOR4I", pragma::math::to_integral(pragma::ents::EntityMemberType::Vector4i)},
	    {"MEMBER_TYPE_ELEMENT", pragma::math::to_integral(pragma::ents::EntityMemberType::Element)},
	    {"MEMBER_TYPE_ENTITY", pragma::math::to_integral(pragma::ents::EntityMemberType::Entity)},
	    {"MEMBER_TYPE_MULTI_ENTITY", pragma::math::to_integral(pragma::ents::EntityMemberType::MultiEntity)},
	    {"MEMBER_TYPE_COMPONENT_PROPERTY", pragma::math::to_integral(pragma::ents::EntityMemberType::ComponentProperty)},
	    {"MEMBER_TYPE_COUNT", pragma::math::to_integral(pragma::ents::EntityMemberType::Count)},
	    {"MEMBER_TYPE_LAST", pragma::math::to_integral(pragma::ents::EntityMemberType::Last)},
	    {"MEMBER_TYPE_INVALID", pragma::math::to_integral(pragma::ents::EntityMemberType::Invalid)},

	    {"INVALID_COMPONENT_ID", pragma::INVALID_COMPONENT_ID},
	    {"INVALID_COMPONENT_MEMBER_INDEX", pragma::INVALID_COMPONENT_MEMBER_INDEX},
	  });

	auto componentInfoDef = luabind::class_<pragma::ComponentInfo>("ComponentInfo");
	componentInfoDef.def(
	  "__tostring", +[](const pragma::ComponentInfo &componentInfo) -> std::string {
		  std::stringstream ss;
		  ss << "ComponentInfo";
		  ss << "[" << componentInfo.id << "]";
		  ss << "[" << componentInfo.name << "]";
		  ss << "[Cat:" << componentInfo.category << "]";
		  ss << "[Flags:" << magic_enum::enum_flags_name(componentInfo.flags) << "]";
		  return ss.str();
	  });
	componentInfoDef.property("name", +[](lua::State *l, const pragma::ComponentInfo &componentInfo) { return std::string {*componentInfo.name}; });
	componentInfoDef.property("category", +[](lua::State *l, const pragma::ComponentInfo &componentInfo) { return std::string {*componentInfo.category}; });
	componentInfoDef.def_readonly("id", &pragma::ComponentInfo::id);
	componentInfoDef.def_readonly("flags", &pragma::ComponentInfo::flags);
	componentInfoDef.def("GetMemberCount", +[](const pragma::ComponentInfo &componentInfo) { return componentInfo.members.size(); });
	componentInfoDef.def(
	  "FindMemberIndex", +[](const pragma::ComponentInfo &componentInfo, const std::string &name) -> std::optional<pragma::ComponentMemberIndex> {
		  auto it = componentInfo.memberNameToIndex.find(name);
		  if(it == componentInfo.memberNameToIndex.end())
			  return {};
		  return it->second;
	  });
	componentInfoDef.def(
	  "GetMemberInfo", +[](const pragma::ComponentInfo &componentInfo, uint32_t memberIdx) -> const pragma::ComponentMemberInfo * {
		  if(memberIdx >= componentInfo.members.size())
			  return nullptr;
		  return &componentInfo.members[memberIdx];
	  });
	componentInfoDef.def(
	  "GetMemberInfo", +[](pragma::Game &game, const pragma::ComponentInfo &componentInfo, const std::string &name) -> const pragma::ComponentMemberInfo * {
		  auto lname = name;
		  if(pragma::math::is_flag_set(componentInfo.flags, pragma::ComponentFlags::LuaBased)) {
			  auto &manager = game.GetLuaEntityManager();
			  auto *o = manager.GetClassObject(componentInfo.name);
			  if(!o)
				  return nullptr;
			  auto *infos = pragma::BaseLuaBaseEntityComponent::GetMemberInfos(*o);
			  if(!infos)
				  return nullptr;
			  auto it = std::find_if(infos->begin(), infos->end(), [&lname](const pragma::BaseLuaBaseEntityComponent::MemberInfo &memberInfo) { return memberInfo.memberName == lname; });
			  if(it == infos->end() || !it->componentMemberInfo.has_value())
				  return nullptr;
			  return &*it->componentMemberInfo;
		  }
		  pragma::string::to_lower(lname);
		  auto it = componentInfo.memberNameToIndex.find(lname);
		  if(it == componentInfo.memberNameToIndex.end())
			  return nullptr;
		  auto memberIdx = it->second;
		  if(memberIdx >= componentInfo.members.size())
			  return nullptr;
		  return &componentInfo.members[memberIdx];
	  });

	auto memberInfoDef = luabind::class_<pragma::ComponentMemberInfo>("MemberInfo");
	memberInfoDef.add_static_constant("TYPE_META_DATA_RANGE", pragma::math::to_integral(TypeMetaData::Range));
	memberInfoDef.add_static_constant("TYPE_META_DATA_COORDINATE", pragma::math::to_integral(TypeMetaData::Coordinate));
	memberInfoDef.add_static_constant("TYPE_META_DATA_POSE", pragma::math::to_integral(TypeMetaData::Pose));
	memberInfoDef.add_static_constant("TYPE_META_DATA_POSE_COMPONENT", pragma::math::to_integral(TypeMetaData::PoseComponent));
	memberInfoDef.add_static_constant("TYPE_META_DATA_OPTIONAL", pragma::math::to_integral(TypeMetaData::Optional));
	memberInfoDef.add_static_constant("TYPE_META_DATA_ENABLER", pragma::math::to_integral(TypeMetaData::Enabler));
	memberInfoDef.add_static_constant("TYPE_META_DATA_PARENT", pragma::math::to_integral(TypeMetaData::Parent));
	static_assert(pragma::math::to_integral(TypeMetaData::Count) == 7, "Update this implementation when adding news types!");

	auto typeMetaDataDef = luabind::class_<pragma::ents::TypeMetaData>("TypeMetaData");
	memberInfoDef.scope[typeMetaDataDef];

	auto rangeTypeMetaDataDef = luabind::class_<pragma::ents::RangeTypeMetaData, pragma::ents::TypeMetaData>("RangeTypeMetaData");
	rangeTypeMetaDataDef.def_readwrite("min", &pragma::ents::RangeTypeMetaData::min);
	rangeTypeMetaDataDef.def_readwrite("max", &pragma::ents::RangeTypeMetaData::max);
	rangeTypeMetaDataDef.def_readwrite("stepSize", &pragma::ents::RangeTypeMetaData::stepSize);
	memberInfoDef.scope[rangeTypeMetaDataDef];

	auto coordinateTypeMetaDataDef = luabind::class_<pragma::ents::CoordinateTypeMetaData, pragma::ents::TypeMetaData>("CoordinateTypeMetaData");
	coordinateTypeMetaDataDef.def_readwrite("space", &pragma::ents::CoordinateTypeMetaData::space);
	coordinateTypeMetaDataDef.property(
	  "parentProperty", +[](lua::State *l, const pragma::ents::CoordinateTypeMetaData &metaData) { PushString(l, metaData.parentProperty.c_str()); }, +[](lua::State *l, pragma::ents::CoordinateTypeMetaData &metaData, const std::string &prop) { metaData.parentProperty = prop; });
	memberInfoDef.scope[coordinateTypeMetaDataDef];

	auto poseTypeMetaDataDef = luabind::class_<pragma::ents::PoseTypeMetaData, pragma::ents::TypeMetaData>("PoseTypeMetaData");
	poseTypeMetaDataDef.property("posProperty", +[](lua::State *l, const pragma::ents::PoseTypeMetaData &metaData) { PushString(l, metaData.posProperty.c_str()); }, +[](lua::State *l, pragma::ents::PoseTypeMetaData &metaData, const std::string &prop) { metaData.posProperty = prop; });
	poseTypeMetaDataDef.property("rotProperty", +[](lua::State *l, const pragma::ents::PoseTypeMetaData &metaData) { PushString(l, metaData.rotProperty.c_str()); }, +[](lua::State *l, pragma::ents::PoseTypeMetaData &metaData, const std::string &prop) { metaData.rotProperty = prop; });
	poseTypeMetaDataDef.property(
	  "scaleProperty", +[](lua::State *l, const pragma::ents::PoseTypeMetaData &metaData) { PushString(l, metaData.scaleProperty.c_str()); }, +[](lua::State *l, pragma::ents::PoseTypeMetaData &metaData, const std::string &prop) { metaData.scaleProperty = prop; });
	memberInfoDef.scope[poseTypeMetaDataDef];

	auto poseComponentTypeMetaDataDef = luabind::class_<pragma::ents::PoseComponentTypeMetaData, pragma::ents::TypeMetaData>("PoseComponentTypeMetaData");
	poseComponentTypeMetaDataDef.property(
	  "poseProperty", +[](lua::State *l, const pragma::ents::PoseComponentTypeMetaData &metaData) { PushString(l, metaData.poseProperty.c_str()); }, +[](lua::State *l, pragma::ents::PoseComponentTypeMetaData &metaData, const std::string &prop) { metaData.poseProperty = prop; });
	memberInfoDef.scope[poseComponentTypeMetaDataDef];

	auto optionalTypeMetaDataDef = luabind::class_<pragma::ents::OptionalTypeMetaData, pragma::ents::TypeMetaData>("OptionalTypeMetaData");
	optionalTypeMetaDataDef.property(
	  "enabledProperty", +[](lua::State *l, const pragma::ents::OptionalTypeMetaData &metaData) { PushString(l, metaData.enabledProperty.c_str()); }, +[](lua::State *l, pragma::ents::OptionalTypeMetaData &metaData, const std::string &prop) { metaData.enabledProperty = prop; });
	memberInfoDef.scope[optionalTypeMetaDataDef];

	auto parentTypeMetaDataDef = luabind::class_<pragma::ents::ParentTypeMetaData, pragma::ents::TypeMetaData>("ParentTypeMetaData");
	parentTypeMetaDataDef.property(
	  "parentProperty", +[](lua::State *l, const pragma::ents::ParentTypeMetaData &metaData) { PushString(l, metaData.parentProperty.c_str()); }, +[](lua::State *l, pragma::ents::ParentTypeMetaData &metaData, const std::string &prop) { metaData.parentProperty = prop; });
	memberInfoDef.scope[parentTypeMetaDataDef];

	auto enablerTypeMetaDataDef = luabind::class_<pragma::ents::EnablerTypeMetaData, pragma::ents::TypeMetaData>("EnablerTypeMetaData");
	enablerTypeMetaDataDef.property(
	  "targetProperty", +[](lua::State *l, const pragma::ents::EnablerTypeMetaData &metaData) { PushString(l, metaData.targetProperty.c_str()); }, +[](lua::State *l, pragma::ents::EnablerTypeMetaData &metaData, const std::string &prop) { metaData.targetProperty = prop; });
	memberInfoDef.scope[enablerTypeMetaDataDef];

	static_assert(pragma::math::to_integral(TypeMetaData::Count) == 7, "Update these bindings when adding news types!");

	memberInfoDef.add_static_constant("FLAG_NONE", pragma::math::to_integral(pragma::ComponentMemberFlags::None));
	memberInfoDef.add_static_constant("FLAG_HIDE_IN_INTERFACE_BIT", pragma::math::to_integral(pragma::ComponentMemberFlags::HideInInterface));
	memberInfoDef.add_static_constant("FLAG_CONTROLLER_BIT", pragma::math::to_integral(pragma::ComponentMemberFlags::Controller));
	memberInfoDef.add_static_constant("FLAG_WORLD_SPACE_BIT", pragma::math::to_integral(pragma::ComponentMemberFlags::WorldSpace));
	memberInfoDef.add_static_constant("FLAG_OBJECT_SPACE_BIT", pragma::math::to_integral(pragma::ComponentMemberFlags::ObjectSpace));
	memberInfoDef.add_static_constant("FLAG_READ_ONLY_BIT", pragma::math::to_integral(pragma::ComponentMemberFlags::ReadOnly));
	memberInfoDef.def(
	  "__tostring", +[](const pragma::ComponentMemberInfo &memberInfo) -> std::string {
		  std::stringstream ss;
		  ss << "MemberInfo";
		  ss << "[" << memberInfo.GetName() << "]";
		  ss << "[Type:" << magic_enum::enum_name(memberInfo.type) << "]";
		  ss << "[Spec:" << magic_enum::enum_name(memberInfo.GetSpecializationType());
		  auto *c = memberInfo.GetCustomSpecializationType();
		  if(c)
			  ss << "(" << *c << ")";
		  ss << "]";
		  if(pragma::ents::is_udm_member_type(memberInfo.type)) {
			  ::udm::visit_ng(pragma::ents::member_type_to_udm_type(memberInfo.type), [&memberInfo, &ss](auto tag) {
				  using T = typename decltype(tag)::type;
				  if constexpr(::udm::is_convertible<T, std::string>()) {
					  T def;
					  if(memberInfo.GetDefault<T>(def)) {
						  auto defStr = ::udm::convert<T, std::string>(def);
						  ss << "[Def:" << defStr << "]";
					  }
				  }
			  });
		  }
		  return ss.str();
	  });

	memberInfoDef.def("SetFlags", &pragma::ComponentMemberInfo::SetFlags);
	memberInfoDef.def("GetFlags", &pragma::ComponentMemberInfo::GetFlags);
	memberInfoDef.def("HasFlag", &pragma::ComponentMemberInfo::HasFlag);
	memberInfoDef.def("SetFlag", &pragma::ComponentMemberInfo::SetFlag);
	memberInfoDef.def("SetFlag", &pragma::ComponentMemberInfo::SetFlag, luabind::default_parameter_policy<3, true> {});
	memberInfoDef.def(
	  "FindTypeMetaData", +[](lua::State *l, const pragma::ComponentMemberInfo &info, TypeMetaData eType) -> luabind::object {
		  auto idx = type_meta_data_to_type_index(eType);
		  if(!idx)
			  return nil;
		  auto *metaData = info.FindTypeMetaData(*idx);
		  if(!metaData)
			  return nil;
		  return meta_data_type_to_lua_object(l, *metaData, eType);
	  });
	memberInfoDef.def("IsEnum", &pragma::ComponentMemberInfo::IsEnum);
	memberInfoDef.def("ValueToEnumName", &pragma::ComponentMemberInfo::ValueToEnumName);
	memberInfoDef.def("EnumNameToValue", &pragma::ComponentMemberInfo::EnumNameToValue);
	memberInfoDef.def(
	  "GetEnumValues", +[](const pragma::ComponentMemberInfo &memberInfo) -> std::optional<std::vector<int64_t>> {
		  std::vector<int64_t> values;
		  if(!memberInfo.GetEnumValues(values))
			  return {};
		  return values;
	  });
	memberInfoDef.def_readonly("type", &pragma::ComponentMemberInfo::type);
	memberInfoDef.property("name", +[](lua::State *l, const pragma::ComponentMemberInfo &memInfo) { return std::string {*memInfo.GetName()}; });
	memberInfoDef.property("nameHash", +[](lua::State *l, const pragma::ComponentMemberInfo &memInfo) { return memInfo.GetNameHash(); });
	memberInfoDef.property("specializationType", +[](lua::State *l, const pragma::ComponentMemberInfo &memInfo) { return memInfo.GetSpecializationType(); });
	memberInfoDef.property(
	  "customSpecializationType", +[](lua::State *l, const pragma::ComponentMemberInfo &memInfo) -> std::optional<std::string> {
		  auto *type = memInfo.GetCustomSpecializationType();
		  if(!type)
			  return {};
		  return *type;
	  });
	memberInfoDef.property("minValue", +[](lua::State *l, const pragma::ComponentMemberInfo &memInfo) { return memInfo.GetMin(); });
	memberInfoDef.property("maxValue", +[](lua::State *l, const pragma::ComponentMemberInfo &memInfo) { return memInfo.GetMax(); });
	memberInfoDef.property("stepSize", +[](lua::State *l, const pragma::ComponentMemberInfo &memInfo) { return memInfo.GetStepSize(); });
	memberInfoDef.property("metaData", +[](lua::State *l, const pragma::ComponentMemberInfo &memInfo) -> ::udm::PProperty { return memInfo.GetMetaData(); });
	memberInfoDef.property(
	  "default", +[](lua::State *l, const pragma::ComponentMemberInfo &memInfo) -> udm_type {
		  // Default value is currently only allowed for UDM types. Tag: component-member-udm-default
		  if(!pragma::ents::is_udm_member_type(memInfo.type))
			  return nil;
		  return ::udm::visit(pragma::ents::member_type_to_udm_type(memInfo.type), [&memInfo, l](auto tag) {
			  using T = typename decltype(tag)::type;
			  constexpr auto type = ::udm::type_to_enum<T>();
			  if constexpr(type != ::udm::Type::Element && !::udm::is_array_type(type)) {
				  T val;
				  if(!memInfo.GetDefault(val))
					  return nil;
				  return luabind::object {l, std::move(val)};
			  }
			  else
				  return nil;
		  });
	  });
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_NONE", pragma::math::to_integral(pragma::AttributeSpecializationType::None));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_COLOR", pragma::math::to_integral(pragma::AttributeSpecializationType::Color));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_DISTANCE", pragma::math::to_integral(pragma::AttributeSpecializationType::Distance));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_LIGHT_INTENSITY", pragma::math::to_integral(pragma::AttributeSpecializationType::LightIntensity));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_CUSTOM", pragma::math::to_integral(pragma::AttributeSpecializationType::Custom));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_ANGLE", pragma::math::to_integral(pragma::AttributeSpecializationType::Angle));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_FILE", pragma::math::to_integral(pragma::AttributeSpecializationType::File));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_DIRECTORY", pragma::math::to_integral(pragma::AttributeSpecializationType::Directory));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_PLANE", pragma::math::to_integral(pragma::AttributeSpecializationType::Plane));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_NORMAL", pragma::math::to_integral(pragma::AttributeSpecializationType::Normal));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_FRACTION", pragma::math::to_integral(pragma::AttributeSpecializationType::Fraction));
	static_assert(pragma::math::to_integral(pragma::AttributeSpecializationType::Count) == 11u);
	componentInfoDef.scope[memberInfoDef];

	entsMod[componentInfoDef];

	pragma::LuaCore::define_custom_constructor<pragma::ents::RangeTypeMetaData, +[]() -> std::shared_ptr<pragma::ents::RangeTypeMetaData> { return pragma::util::make_shared<pragma::ents::RangeTypeMetaData>(); }>(l);
	pragma::LuaCore::define_custom_constructor<pragma::ents::RangeTypeMetaData,
	  +[](std::optional<float> min, std::optional<float> max, std::optional<float> stepSize) -> std::shared_ptr<pragma::ents::RangeTypeMetaData> {
		  auto metaData = std::shared_ptr<pragma::ents::RangeTypeMetaData> {new pragma::ents::RangeTypeMetaData {}};
		  metaData->min = min;
		  metaData->max = max;
		  metaData->stepSize = stepSize;
		  return metaData;
	  },
	  std::optional<float>, std::optional<float>, std::optional<float>>(l);

	pragma::LuaCore::define_custom_constructor<pragma::ents::CoordinateTypeMetaData, +[]() -> std::shared_ptr<pragma::ents::CoordinateTypeMetaData> { return pragma::util::make_shared<pragma::ents::CoordinateTypeMetaData>(); }>(l);
	pragma::LuaCore::define_custom_constructor<pragma::ents::CoordinateTypeMetaData,
	  +[](pragma::math::CoordinateSpace space, const std::string &parentProperty) -> std::shared_ptr<pragma::ents::CoordinateTypeMetaData> {
		  auto metaData = std::shared_ptr<pragma::ents::CoordinateTypeMetaData> {new pragma::ents::CoordinateTypeMetaData {}};
		  metaData->space = space;
		  metaData->parentProperty = parentProperty;
		  return metaData;
	  },
	  pragma::math::CoordinateSpace, const std::string &>(l);

	pragma::LuaCore::define_custom_constructor<pragma::ents::PoseTypeMetaData, +[]() -> std::shared_ptr<pragma::ents::PoseTypeMetaData> { return pragma::util::make_shared<pragma::ents::PoseTypeMetaData>(); }>(l);
	pragma::LuaCore::define_custom_constructor<pragma::ents::PoseTypeMetaData,
	  +[](const std::string &posProperty, const std::string &rotProperty, const std::string &scaleProperty) -> std::shared_ptr<pragma::ents::PoseTypeMetaData> {
		  auto metaData = std::shared_ptr<pragma::ents::PoseTypeMetaData> {new pragma::ents::PoseTypeMetaData {}};
		  metaData->posProperty = posProperty;
		  metaData->rotProperty = rotProperty;
		  metaData->scaleProperty = scaleProperty;
		  return metaData;
	  },
	  const std::string &, const std::string &, const std::string &>(l);

	pragma::LuaCore::define_custom_constructor<pragma::ents::PoseComponentTypeMetaData, +[]() -> std::shared_ptr<pragma::ents::PoseComponentTypeMetaData> { return pragma::util::make_shared<pragma::ents::PoseComponentTypeMetaData>(); }>(l);
	pragma::LuaCore::define_custom_constructor<pragma::ents::PoseComponentTypeMetaData,
	  +[](const std::string &poseProperty) -> std::shared_ptr<pragma::ents::PoseComponentTypeMetaData> {
		  auto metaData = std::shared_ptr<pragma::ents::PoseComponentTypeMetaData> {new pragma::ents::PoseComponentTypeMetaData {}};
		  metaData->poseProperty = poseProperty;
		  return metaData;
	  },
	  const std::string &>(l);

	pragma::LuaCore::define_custom_constructor<pragma::ents::OptionalTypeMetaData, +[]() -> std::shared_ptr<pragma::ents::OptionalTypeMetaData> { return pragma::util::make_shared<pragma::ents::OptionalTypeMetaData>(); }>(l);
	pragma::LuaCore::define_custom_constructor<pragma::ents::OptionalTypeMetaData,
	  +[](const std::string &enabledProperty) -> std::shared_ptr<pragma::ents::OptionalTypeMetaData> {
		  auto metaData = std::shared_ptr<pragma::ents::OptionalTypeMetaData> {new pragma::ents::OptionalTypeMetaData {}};
		  metaData->enabledProperty = enabledProperty;
		  return metaData;
	  },
	  const std::string &>(l);
	pragma::LuaCore::define_custom_constructor<pragma::ents::EnablerTypeMetaData, +[]() -> std::shared_ptr<pragma::ents::EnablerTypeMetaData> { return pragma::util::make_shared<pragma::ents::EnablerTypeMetaData>(); }>(l);
	pragma::LuaCore::define_custom_constructor<pragma::ents::EnablerTypeMetaData,
	  +[](const std::string &targetProperty) -> std::shared_ptr<pragma::ents::EnablerTypeMetaData> {
		  auto metaData = std::shared_ptr<pragma::ents::EnablerTypeMetaData> {new pragma::ents::EnablerTypeMetaData {}};
		  metaData->targetProperty = targetProperty;
		  return metaData;
	  },
	  const std::string &>(l);
	pragma::LuaCore::define_custom_constructor<pragma::ents::ParentTypeMetaData, +[]() -> std::shared_ptr<pragma::ents::ParentTypeMetaData> { return pragma::util::make_shared<pragma::ents::ParentTypeMetaData>(); }>(l);
	pragma::LuaCore::define_custom_constructor<pragma::ents::ParentTypeMetaData,
	  +[](const std::string &parentProperty) -> std::shared_ptr<pragma::ents::ParentTypeMetaData> {
		  auto metaData = std::shared_ptr<pragma::ents::ParentTypeMetaData> {new pragma::ents::ParentTypeMetaData {}};
		  metaData->parentProperty = parentProperty;
		  return metaData;
	  },
	  const std::string &>(l);

	static_assert(pragma::math::to_integral(TypeMetaData::Count) == 7, "Update this implementation when adding news types!");
}

Lua::type<pragma::ecs::BaseEntity> Lua::ents::create(lua::State *l, const std::string &classname)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();

	auto *ent = game->CreateEntity(classname);
	if(ent == nullptr)
		return nil;
	return ent->GetLuaObject();
}

Lua::type<pragma::ecs::BaseEntity> Lua::ents::create_prop(lua::State *l, const std::string &mdl, const Vector3 *origin, const EulerAngles *angles, bool physicsProp = false)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();
	auto *ent = game->CreateEntity(physicsProp ? "prop_physics" : "prop_dynamic");
	if(ent == nullptr)
		return nil;
	if(origin)
		ent->SetPosition(*origin);
	if(angles)
		ent->SetRotation(uquat::create(*angles));
	ent->SetModel(mdl);
	ent->Spawn();
	return ent->GetLuaObject();
}

namespace Lua::ents {
	type<pragma::ecs::BaseEntity> create_trigger(lua::State *l, const Vector3 &origin, const EulerAngles *angles, pragma::physics::IConvexShape *shape)
	{
		auto *state = pragma::Engine::Get()->GetNetworkState(l);
		auto *game = state->GetGameState();
		auto *ent = game->CreateEntity("trigger_touch");
		if(ent == nullptr)
			return nil;
		auto pTrComponent = ent->GetTransformComponent();
		if(pTrComponent != nullptr) {
			if(angles != nullptr)
				pTrComponent->SetAngles(*angles);
			pTrComponent->SetPosition(origin);
		}
		ent->SetKeyValue("spawnflags", "1144"); // TODO: Use enums
		ent->Spawn();
		if(shape != nullptr) {
			auto pPhysComponent = ent->GetPhysicsComponent();
			if(pPhysComponent != nullptr)
				pPhysComponent->InitializePhysics(*shape);
		}
		return ent->GetLuaObject();
	}
};
Lua::type<pragma::ecs::BaseEntity> Lua::ents::create_trigger(lua::State *l, const Vector3 &origin, float radius)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *phys = game->GetPhysicsEnvironment();
	auto shape = phys->CreateSphereShape(radius, phys->GetGenericMaterial());
	return create_trigger(l, origin, nullptr, dynamic_cast<pragma::physics::IConvexShape *>(shape.get()));
}
Lua::type<pragma::ecs::BaseEntity> Lua::ents::create_trigger(lua::State *l, const Vector3 &origin, pragma::physics::IShape &shape)
{
	if(shape.IsConvex() == false) {
		Con::CWAR << "Cannot create trigger_touch entity with non-convex physics shape!" << Con::endl;
		return nil;
	}
	auto cvShape = std::dynamic_pointer_cast<pragma::physics::IConvexShape>(shape.shared_from_this());
	return create_trigger(l, origin, nullptr, cvShape.get());
}

Lua::type<pragma::ecs::BaseEntity> Lua::ents::create_trigger(lua::State *l, const Vector3 &origin, const Vector3 &min, const Vector3 &max, const EulerAngles &angles)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *phys = game->GetPhysicsEnvironment();

	auto extents = max - min;
	auto center = (max + min) * 0.5f;
	auto centerOrigin = origin + center;
	auto shape = phys->CreateBoxShape(extents * 0.5f, phys->GetGenericMaterial());
	return create_trigger(l, centerOrigin, &angles, shape.get());
}

static Lua::tb<Lua::type<pragma::ecs::BaseEntity>> entities_to_table(lua::State *l, std::vector<pragma::ecs::BaseEntity *> &ents)
{
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto *ent : ents) {
		if(!ent)
			continue;
		t[idx++] = ent->GetLuaObject();
	}
	return t;
}

static Lua::tb<Lua::type<pragma::ecs::BaseEntity>> entities_to_table(lua::State *l, pragma::ecs::EntityIterator &entIt)
{
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto *ent : entIt)
		t[idx++] = ent->GetLuaObject();
	return t;
}

static void iterate_entities(lua::State *l, const std::function<void(pragma::ecs::BaseEntity *)> &fCallback)
{
	auto fcIterator = 1;
	Lua::CheckFunction(l, fcIterator);
	pragma::ecs::BaseEntity *ent = nullptr;
	do {
		Lua::PushValue(l, fcIterator); /* 1 */
		Lua::Call(l, 0, 1);            /* 1 */
		if(Lua::IsSet(l, -1)) {
			ent = luabind::object_cast<pragma::ecs::BaseEntity *>(luabind::object {luabind::from_stack(l, -1)});
			if(ent != nullptr)
				fCallback(ent);
		}
		else
			ent = nullptr;
		Lua::Pop(l, 1); /* 0 */
	} while(ent != nullptr);
}

Lua::opt<Lua::mult<Lua::type<pragma::ecs::BaseEntity>, double>> Lua::ents::get_closest(lua::State *l, const Vector3 &origin)
{
	auto dClosest = std::numeric_limits<float>::max();
	pragma::ecs::BaseEntity *entClosest = nullptr;
	iterate_entities(l, [&dClosest, &entClosest, &origin](pragma::ecs::BaseEntity *ent) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(!pTransformComponent)
			return;
		auto d = uvec::distance_sqr(pTransformComponent->GetPosition(), origin);
		if(d >= dClosest)
			return;
		dClosest = d;
		entClosest = ent;
	});
	if(entClosest == nullptr)
		return nil;
	return Lua::mult<type<pragma::ecs::BaseEntity>, double> {l, entClosest->GetLuaObject(), dClosest};
}
Lua::opt<Lua::mult<Lua::type<pragma::ecs::BaseEntity>, double>> Lua::ents::get_farthest(lua::State *l, const Vector3 &origin)
{
	auto dFarthest = -1.f;
	pragma::ecs::BaseEntity *entClosest = nullptr;
	iterate_entities(l, [&dFarthest, &entClosest, &origin](pragma::ecs::BaseEntity *ent) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(!pTransformComponent)
			return;
		auto d = uvec::distance_sqr(pTransformComponent->GetPosition(), origin);
		if(d <= dFarthest)
			return;
		dFarthest = d;
		entClosest = ent;
	});
	if(entClosest == nullptr)
		return nil;
	return Lua::mult<type<pragma::ecs::BaseEntity>, double> {l, entClosest->GetLuaObject(), dFarthest};
}
Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_sorted_by_distance(lua::State *l, const Vector3 &origin)
{
	std::vector<std::pair<pragma::ecs::BaseEntity *, float>> ents {};
	iterate_entities(l, [&ents, &origin](pragma::ecs::BaseEntity *ent) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(!pTransformComponent)
			return;
		auto d = uvec::distance_sqr(pTransformComponent->GetPosition(), origin);
		ents.push_back({ent, d});
	});
	std::sort(ents.begin(), ents.end(), [](const std::pair<pragma::ecs::BaseEntity *, float> &a, const std::pair<pragma::ecs::BaseEntity *, float> &b) { return a.second < b.second; });
	auto t = luabind::newtable(l);
	auto idx = 1;
	for(auto &pair : ents)
		t[idx++] = pair.first->GetLuaObject();
	return t;
}
Lua::type<pragma::ecs::BaseEntity> Lua::ents::get_random(lua::State *l)
{
	std::vector<pragma::ecs::BaseEntity *> ents {};
	iterate_entities(l, [&ents](pragma::ecs::BaseEntity *ent) { ents.push_back(ent); });
	if(ents.empty())
		return nil;
	auto r = pragma::math::random(0, ents.size() - 1);
	auto *ent = ents.at(r);
	return ent->GetLuaObject();
}

Lua::opt<std::string> Lua::ents::get_component_name(lua::State *l, pragma::ComponentId componentId)
{
	auto *info = pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->GetEntityComponentManager().GetComponentInfo(componentId);
	if(info == nullptr)
		return nil;
	return {l, info->name};
}
Lua::opt<uint32_t> Lua::ents::get_component_id(lua::State *l, const std::string &componentName)
{
	pragma::ComponentId componentId;
	if(pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->GetEntityComponentManager().GetComponentTypeId(componentName, componentId) == false)
		return nil;
	return {l, componentId};
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_all(lua::State *l)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();
	std::vector<pragma::ecs::BaseEntity *> *ents;
	game->GetEntities(&ents);
	return entities_to_table(l, *ents);
}
Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_all(lua::State *l, func<type<pragma::ecs::BaseEntity>> func)
{
	std::vector<pragma::ecs::BaseEntity *> ents {};
	iterate_entities(l, [&ents](pragma::ecs::BaseEntity *ent) { ents.push_back(ent); });
	if(ents.empty())
		return luabind::newtable(l);
	auto t = luabind::newtable(l);
	auto idx = 1;
	for(auto *ent : ents) {
		auto &o = ent->GetLuaObject();
		//if(!luabind::object_cast<bool>(func(o)))
		//	continue;
		t[idx++] = o;
	}
	return t;
}
static void iterate_entity_components(lua::State *l, const std::function<void(pragma::BaseEntityComponent *)> &fCallback)
{
	auto fcIterator = 1;
	Lua::CheckFunction(l, fcIterator);
	pragma::BaseEntityComponent *ent = nullptr;
	do {
		Lua::PushValue(l, fcIterator); /* 1 */
		Lua::Call(l, 0, 2);            /* 1 */
		if(Lua::IsSet(l, -1)) {
			ent = luabind::object_cast<pragma::BaseEntityComponent *>(luabind::object {luabind::from_stack(l, -1)});
			if(ent != nullptr)
				fCallback(ent);
		}
		else
			ent = nullptr;
		Lua::Pop(l, 2); /* 0 */
	} while(ent != nullptr);
}
Lua::tb<Lua::type<pragma::BaseEntityComponent>> Lua::ents::get_all_c(lua::State *l, func<type<pragma::BaseEntityComponent>> func)
{
	std::vector<pragma::BaseEntityComponent *> ents {};
	iterate_entity_components(l, [&ents](pragma::BaseEntityComponent *ent) { ents.push_back(ent); });
	if(ents.empty())
		return luabind::newtable(l);
	auto t = luabind::newtable(l);
	auto idx = 1;
	for(auto *ent : ents) {
		auto &o = ent->GetLuaObject();
		//if(!luabind::object_cast<bool>(func(o)))
		//	continue;
		t[idx++] = o;
	}
	return t;
}
Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_all(lua::State *l, pragma::ecs::EntityIterator::FilterFlags filterFlags)
{
	auto it = create_lua_entity_iterator(l, nil, filterFlags);

	auto t = luabind::newtable(l);
	auto idx = 1u;
	for(auto o : it)
		t[idx++] = o;
	return t;
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_all(lua::State *l, pragma::ecs::EntityIterator::FilterFlags filterFlags, const tb<LuaEntityIteratorFilterBase> &filters)
{
	auto it = create_lua_entity_iterator(l, filters, filterFlags);
	auto t = luabind::newtable(l);
	auto idx = 1u;
	for(auto o : it)
		t[idx++] = o;
	return t;
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_all(lua::State *l, const tb<LuaEntityIteratorFilterBase> &filters) { return get_all(l, pragma::ecs::EntityIterator::FilterFlags::Default, filters); }

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_spawned(lua::State *l)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();
	std::vector<pragma::ecs::BaseEntity *> ents;
	game->GetSpawnedEntities(&ents);
	return entities_to_table(l, ents);
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_players(lua::State *l)
{
	auto &game = *pragma::Engine::Get()->GetNetworkState(l)->GetGameState();
	auto t = luabind::newtable(l);
	auto idx = 1u;
	pragma::ecs::EntityIterator it {game, pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending | pragma::ecs::EntityIterator::FilterFlags::Player};
	for(auto *ent : it)
		t[idx++] = ent->GetPlayerComponent().get()->GetLuaObject();
	return t;
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_npcs(lua::State *l)
{
	auto &game = *pragma::Engine::Get()->GetNetworkState(l)->GetGameState();
	auto t = luabind::newtable(l);
	auto idx = 1u;
	pragma::ecs::EntityIterator it {game, pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending | pragma::ecs::EntityIterator::FilterFlags::NPC};
	for(auto *ent : it)
		t[idx++] = ent->GetAIComponent().get()->GetLuaObject();
	return t;
}
Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_weapons(lua::State *l)
{
	auto &game = *pragma::Engine::Get()->GetNetworkState(l)->GetGameState();
	auto t = luabind::newtable(l);
	auto idx = 1u;
	pragma::ecs::EntityIterator it {game, pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending | pragma::ecs::EntityIterator::FilterFlags::Weapon};
	for(auto *ent : it)
		t[idx++] = ent->GetWeaponComponent().get()->GetLuaObject();
	return t;
}
Lua::tb<Lua::type<pragma::BaseVehicleComponent>> Lua::ents::get_vehicles(lua::State *l)
{
	auto &game = *pragma::Engine::Get()->GetNetworkState(l)->GetGameState();
	auto t = luabind::newtable(l);
	auto idx = 1u;
	pragma::ecs::EntityIterator it {game, pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending | pragma::ecs::EntityIterator::FilterFlags::Vehicle};
	for(auto *ent : it)
		t[idx++] = ent->GetVehicleComponent().get()->GetLuaObject();
	return t;
}

Lua::opt<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_world(lua::State *l)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();
	auto *pWorld = game->GetWorld();
	if(pWorld == nullptr)
		return nil;
	pragma::ecs::BaseEntity &world = pWorld->GetEntity();
	return world.GetLuaObject();
}

Lua::opt<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_by_index(lua::State *l, uint32_t idx)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();
	pragma::ecs::BaseEntity *ent = game->GetEntity(idx);
	if(ent == nullptr)
		return nil;
	return ent->GetLuaObject();
}

Lua::opt<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::get_by_local_index(lua::State *l, uint32_t idx)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();
	pragma::ecs::BaseEntity *ent = game->GetEntityByLocalIndex(idx);
	if(ent == nullptr)
		return nil;
	return ent->GetLuaObject();
}

Lua::opt<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::find_by_unique_index(lua::State *l, const std::string &uuid)
{
	auto uniqueIndex = pragma::util::uuid_string_to_bytes(uuid);
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *ent = game->FindEntityByUniqueId(pragma::util::uuid_string_to_bytes(uuid));
	if(!ent)
		return nil;
	return ent->GetLuaObject();
}

namespace luabind::detail {
	template<typename ValueType, typename BaseType>
	void make_null_value(lua::State *L, ValueType &&val)
	{
		// See luabind/detail/make_instance.hpp
		class_registry *registry = class_registry::get_registry(L);
		auto typeId = type_id {typeid(BaseType)};
		auto *cls = registry->find_class(typeId);
		if(!cls) {
			Lua::PushNil(L);
			return;
		}
		auto &classIdMap = cls->classes();
		auto classId = classIdMap.get(typeId);
		if(classId == unknown_class) {
			Lua::PushNil(L);
			return;
		}

		object_rep *instance = push_new_instance(L, cls);

		using value_type = typename std::remove_reference<ValueType>::type;
		using holder_type = pointer_like_holder<value_type>;

		void *storage = instance->allocate(sizeof(holder_type));

		try {
			new(storage) holder_type(L, std::forward<ValueType>(val), classId, nullptr);
		}
		catch(...) {
			instance->deallocate(storage);
			Lua::Pop(L, 1);
			throw;
		}

		instance->set_instance(static_cast<holder_type *>(storage));
	}
};

Lua::type<EntityHandle> Lua::ents::get_null(lua::State *l)
{
	luabind::detail::make_null_value<EntityHandle, pragma::ecs::BaseEntity>(l, EntityHandle {});
	luabind::object o {luabind::from_stack(l, -1)};
	Pop(l, 1);
	return o;
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::find_by_filter(lua::State *l, const std::string &name)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();

	pragma::ecs::EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterEntity>(name);
	return entities_to_table(l, entIt);
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::find_by_class(lua::State *l, const std::string &className)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();

	pragma::ecs::EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterClass>(className);
	return entities_to_table(l, entIt);
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::find_by_name(lua::State *l, const std::string &name)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();

	pragma::ecs::EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterName>(name);
	return entities_to_table(l, entIt);
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::find_by_component(lua::State *l, const std::string &componentName)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();

	auto t = luabind::newtable(l);
	auto idx = 1u;
	for(auto *ent : pragma::ecs::EntityIterator {*game, componentName})
		t[idx++] = ent->GetLuaObject();
	return t;
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::find_in_aabb(lua::State *l, const Vector3 &min, const Vector3 &max) { return find_in_box(l, min, max); }

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::find_in_sphere(lua::State *l, const Vector3 &origin, float radius)
{
	std::vector<pragma::ecs::BaseEntity *> ents;
	auto *state = pragma::Engine::Get()->GetNetworkState(l);

	pragma::ecs::EntityIterator entIt {*state->GetGameState()};
	entIt.AttachFilter<EntityIteratorFilterSphere>(origin, radius);
	return entities_to_table(l, entIt);
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::find_in_box(lua::State *l, const Vector3 &min, const Vector3 &max)
{
	std::vector<pragma::ecs::BaseEntity *> ents;
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();

	pragma::ecs::EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterBox>(min, max);
	return entities_to_table(l, entIt);
}

Lua::tb<Lua::type<pragma::ecs::BaseEntity>> Lua::ents::find_in_cone(lua::State *l, const Vector3 &origin, const Vector3 &dir, float radius, float angle)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();

	pragma::ecs::EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterCone>(origin, dir, radius, angle);
	return entities_to_table(l, entIt);
}

Lua::opt<pragma::ComponentEventId> Lua::ents::get_event_id(lua::State *l, const std::string &name)
{
	pragma::ComponentEventId eventId;
	if(pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->GetEntityComponentManager().GetEventId(name, eventId) == false)
		return nil;
	return {l, eventId};
}

void Lua::ents::register_class(lua::State *l, const std::string &className, const classObject &classObject)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &manager = game->GetLuaEntityManager();
	manager.RegisterEntity(className, const_cast<Lua::classObject &>(classObject), {});
}
void Lua::ents::register_class(lua::State *l, const std::string &className, const luabind::tableT<luabind::variant<std::string, pragma::ComponentId>> &tComponents, LuaEntityType type)
{
	std::vector<pragma::ComponentId> components;
	auto numComponents = GetObjectLength(l, tComponents);
	components.reserve(numComponents);
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	for(auto i = decltype(numComponents) {0}; i < numComponents; ++i) {
		auto o = tComponents[i + 1];
		if(static_cast<Type>(luabind::type(o)) == Type::String) {
			std::string name = luabind::object_cast<std::string>(o);
			auto id = pragma::INVALID_COMPONENT_ID;
			if(componentManager.GetComponentTypeId(name, id, false))
				components.push_back(id);
			else {
				if(game->LoadLuaComponentByName(name) == true && componentManager.GetComponentTypeId(name, id, false) == true)
					components.push_back(id);
				else
					Con::CWAR << "Attempted to add unknown entity component '" << name << "' to registration of entity '" << className << "'! Skipping..." << Con::endl;
			}
		}
		else {
			auto id = luabind::object_cast<uint32_t>(o);
			if(id != pragma::INVALID_COMPONENT_ID)
				components.push_back(id);
			else
				Con::CWAR << "Attempted to add unknown entity component to registration of entity '" << className << "'! Skipping..." << Con::endl;
		}
	}

	const char *cLuaClassName = "__TMP_ENTITY_CLASS";
	std::string luaClassName = cLuaClassName;
	std::stringstream ss;
	ss << "util.register_class(\"" << luaClassName << "\",BaseEntity);\n";
	ss << "function " << luaClassName << ":__init()\n";
	ss << "    BaseEntity.__init(self)\n";
	ss << "end\n";

	auto r = pragma::scripting::lua_core::run_string(l, ss.str(), "register_class");
	if(r == StatusCode::Ok) {
		auto o = luabind::object(luabind::globals(l)[cLuaClassName]);
		if(o) {
			o["Type"] = pragma::math::to_integral(type);

			auto &manager = game->GetLuaEntityManager();
			manager.RegisterEntity(className, o, components);
		}
	}

	PushNil(l);
	SetGlobal(l, luaClassName);
}

Lua::opt<pragma::NetEventId> Lua::ents::register_component_net_event(lua::State *l, pragma::ComponentId componentId, const std::string &name)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	auto *componentInfo = componentManager.GetComponentInfo(componentId);
	if(componentInfo == nullptr) {
		Con::CWAR << "Attempted to register component net event '" << name << "' to unknown component type " << componentId << "!" << Con::endl;
		return nil;
	}

	std::string componentName = componentInfo->name;
	auto netName = componentName + '_' + std::string {name};
	return {l, game->SetupNetEvent(netName)};
}

Lua::opt<pragma::ComponentEventId> Lua::ents::register_component_event(lua::State *l, pragma::ComponentId componentId, const std::string &name)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	auto *componentInfo = componentManager.GetComponentInfo(componentId);
	if(componentInfo == nullptr) {

		Con::CWAR << "Attempted to register component event '" << name << "' to unknown component type " << componentId << "!" << Con::endl;
		return nil;
	}

	std::string componentName = componentInfo->name;
	auto netName = componentName + '_' + std::string {name};
	auto eventId = componentManager.RegisterEventById(netName, componentId, pragma::ComponentEventInfo::Type::Broadcast);
	return {l, eventId};
}
