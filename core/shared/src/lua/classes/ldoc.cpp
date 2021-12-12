/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#define ENABLE_LAD 1

#if ENABLE_LAD == 1
#include "pragma/lua/libraries/lfile.h"
#include "pragma/lua/util.hpp"
#include "pragma/lua/lua_doc.hpp"
#include "pragma/lua/lad/pdb_manager.hpp"
#include "pragma/lua/lad/symbol_handler.hpp"
#include "pragma/lua/lad/generator.hpp"
#include "pragma/lua/lad/repository_manager.hpp"
#include "pragma/lua/lad/type_name_manager.hpp"
#include "pragma/launchparameters.h"
#include <util_pragma_doc.hpp>
#include <pragma/util/util_game.hpp>
#include <luasystem.h>
#include <luainterface.hpp>
#include <unordered_set>
#include <udm.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/util_library.hpp>
#include <filesystem>

#include <luabind/detail/class_rep.hpp>
#include <luabind/detail/call.hpp>
#include <luabind/class_info.hpp>

#undef GetClassInfo

static const pragma::doc::Collection *find_class(const std::vector<pragma::doc::PCollection> &collections,const std::string &className)
{
	for(auto &col : collections)
	{
		if(col->GetName() == className)
			return col.get();
		auto *child = find_class(col->GetChildren(),className);
		if(child)
			return child;
	}
	return nullptr;
}

static void GetClassInfo(lua_State *L,luabind::detail::class_rep * crep)
{
	crep->get_table(L);
	luabind::object table(luabind::from_stack(L, -1));
	lua_pop(L, 1);

	std::size_t index = 1;

	for(luabind::iterator i(table), e; i != e; ++i)
	{
		std::string key;
		auto lkey = i.key();
		lkey.push(L);
		if(Lua::IsString(L,-1))
			key = Lua::CheckString(L,-1);
		Lua::Pop(L,1);

		auto type = luabind::type(*i);
		switch(type)
		{
		case LUA_TFUNCTION:
		{
			std::cout<<"FUNC: "<<key<<std::endl;
			
			/*auto *f = FindItem<pragma::doc::Function>(key,&pragma::doc::Collection::GetFunctions);
			if(f == nullptr)
			{
				std::stringstream ss;
				ss<<"Function '"<<key<<"' not found!";
				WriteToLog(ss);
			}*/
			break;
		}
		case LUA_TBOOLEAN:
			break;
		case LUA_TNUMBER:
			break;
		case LUA_TSTRING:
			break;
		case LUA_TTABLE:
			break;
		case LUA_TUSERDATA:
			break;
		case LUA_TLIGHTUSERDATA:
			break;
		}
		if(type != LUA_TFUNCTION)
			Con::cout<<"Type: "<<type<<Con::endl;
		if(type == LUA_TNUMBER)
		{
		}
		if(type != LUA_TFUNCTION)
			continue;

		// We have to create a temporary `object` here, otherwise the proxy
		// returned by operator->() will mess up the stack. This is a known
		// problem that probably doesn't show up in real code very often.
		luabind::object member(*i);
		member.push(L);
		luabind::detail::stack_pop pop(L, 1);

		luabind::detail::function_object const* impl_const = *(luabind::detail::function_object const**)lua_touserdata(L, -1);
		std::cout<<impl_const<<std::endl;

		if(lua_tocfunction(L, -1) == &luabind::detail::property_tag)
		{
			//result.attributes[index++] = i.key();
		} else
		{
			//result.methods[i.key()] = *i;
		}
	}
}

static std::unordered_map<std::string,std::string> g_typeTranslationTable {
	{"std::string","string"},
	{"short","int16"},
	{"unsigned char","uint8"},
	{"unsigned int","uint32"},
	{"double","float"},
	{"__int64","int64"},
	{"luabind::object","any"},
	{"File","file.File"},
	{"Vector","math.Vector"},
	{"Vertex","game.Model.Vertex"},
	{"Skeleton","game.Model.Skeleton"},
	{"Model","game.Model"},
	{"Sub","game.Model.Mesh.Sub"},
	{"ModelSubMesh","game.Model.Mesh.Sub"},
	{"FlexAnimation","game.Model.FlexAnimation"},
	{"CollisionMesh","game.Model.CollisionMesh"},
	{"MeshGroup","game.Model.MeshGroup"},
	{"Joint","game.Model.Joint"},
	{"Eyeball","game.Model.Eyeball"},
	{"VertexAnimation","game.Model.VertexAnimation"},
	{"MeshAnimation","game.Model.VertexAnimation.MeshAnimation"},
	{"VertexWeight","game.Model.VertexWeight"},
	{"Flex","game.Model.Flex"},
	{"Bone","game.Model.Skeleton.Bone"},
	{"char","char"},
	{"signed char","char"},
	{"unsigned short","uint16"},
	{"unsigned __int64","uint64"},
	{"int","int32"},
	{"float","float"},
	{"bool","bool"},
	{"FlexAnimationFrame","game.Model.FlexAnimation.Frame"},
	{"TimerHandle","time.Timer"},
	{"pragma::ai::Schedule","ai.Schedule"},
	{"LFile","file.File"},
	{"Version","util.Version"},
	{"std::basic_string","string"},
	{"glm::qua","Quaternion"},
	{"luabind::argument","any"},
	{"luabind::object","any"},
	{"CallbackHandle","Callback"},
	{"void","nil"},
	{"BaseGamemodeComponent",""},
	{"Material","game.Material"},
	{"ModelMesh","game.Model.Mesh"},
	{"pragma::nav::Mesh","nav.Mesh"},
	{"ConVar","console.Var"},
	{"pragma::animation::AnimatedPose","panima.Pose"},
	{"pragma::animation::AnimationSlice","panima.Slice"},
	{"pragma::animation::AnimationPlayer","panima.Player"},
	{"pragma::animation::AnimationChannel","panima.Channel"},
	{"pragma::physics::ISliderConstraint","phys.SliderConstraint"},
	{"pragma::physics::IRigidBody","phys.RigidBody"},
	{"pragma::physics::IConvexHullShape","phys.ConvexHullShape"},
	{"pragma::physics::IGhostObject","phys.GhostObj"},
	{"pragma::physics::IBoxShape","phys.BoxShape"},
	{"pragma::physics::ISoftBody","phys.SoftBody"},
	{"pragma::physics::IConeTwistConstraint","phys.ConeTwistConstraint"},
	{"pragma::physics::IController","phys.Controller"},
	{"pragma::physics::IConvexShape","phys.ConvexShape"},
	{"pragma::physics::IVehicle","phys.Vehicle"},
	{"pragma::physics::IConstraint","phys.Constraint"}
};

std::unordered_set<std::string> g_typeWarningCache;
std::unordered_map<std::string,std::string> g_classNameToFullName;

struct TypeInfo
{
	std::string typeName;
	std::vector<TypeInfo> templateParameterTypes;
};
static TypeInfo parse_type(std::string_view &str)
{
	TypeInfo typeInfo {};
	uint32_t idx = 0;
	while(idx < str.length() && str[idx] != '\0' && str[idx] != '<' && str[idx] != '>' && str[idx] != ',')
		++idx;
	typeInfo.typeName = str.substr(0,idx);
	str = (idx < str.length()) ? str.substr(idx) : std::string_view{};

	if(!str.empty())
	{
		if(str[0] == '<')
		{
			do
			{
				str = str.substr(1);
				typeInfo.templateParameterTypes.push_back(parse_type(str));
				ustring::remove_whitespace(str);
			}
			while(!str.empty() && str[0] == ',');
			if(!str.empty() && str[0] == '>')
				str = str.substr(1);
		}
	}
	
	return typeInfo;
}

static void normalize_type_name(pragma::lua::TypeNameManager &nameManager,std::string &type,const TypeInfo *typeInfo=nullptr)
{
	auto typeTranslated = nameManager.TranslateType(type);
	if(typeTranslated.has_value())
	{
		type = *typeTranslated;
		return;
	}
	Con::cout<<"Normalize Type Name: "<<type<<Con::endl;
	ustring::remove_whitespace(type);
	if(ustring::compare(type.data(),"[struct",true,7))
		type = type.substr(7);
	if(ustring::compare(type.data(),"[class",true,6))
		type = type.substr(6);

	//ustring::replace(paramName,"util::TSharedHandle<","");
	//ustring::replace(paramName,"util::WeakHandle<","");

	auto isRef = (type.find("&") != std::string::npos);
	auto isPtr = (type.find("*") != std::string::npos);
	auto isConst = (type.find(" const") != std::string::npos);
	//if(isRef)
	//	dtype.flags |= pragma::doc::Variant::Flags::Reference;
	//if(isPtr)
	//	dtype.flags |= pragma::doc::Variant::Flags::Pointer;
	//if(isConst)
	//	dtype.flags |= pragma::doc::Variant::Flags::Const;
	ustring::replace(type,"[","");
	ustring::replace(type,"]","");
	ustring::replace(type,"&","");
	ustring::replace(type,"*","");
	ustring::replace(type," const","");
	ustring::replace(type,"luabind::adl::","");
	ustring::replace(type,"custom","");
	ustring::replace(type,"enum","");
	//ustring::replace(paramName,">","");
	ustring::remove_whitespace(type);

	auto baseType = type;
	//auto p = baseType.rfind("::");
	//if(p != std::string::npos)
	//	baseType = baseType.substr(p +2);
	auto it = g_typeTranslationTable.find(baseType);
	if(it != g_typeTranslationTable.end())
		type = it->second;
	else
	{
		auto it = g_classNameToFullName.find(baseType);
		if(it != g_classNameToFullName.end())
			type = it->second;
		else
		{
			if(g_typeWarningCache.find(type) == g_typeWarningCache.end())
			{
				g_typeWarningCache.insert(type);
				Con::cwar<<"WARNING: Unknown type '"<<type<<"' ('"<<baseType<<"')!"<<Con::endl;
			}
		}
	}
}
static std::vector<pragma::doc::Variant> normalize_type_namee(const pragma::doc::Variant &type)
{
	std::vector<pragma::doc::Variant> newTypes;
	newTypes.push_back(type);
	auto &newType = newTypes.back();
	auto &paramName = newType.name;
	if(paramName.find("luabind::adl") != std::string::npos)
		std::cout<<"";
	ustring::replace(paramName,"custom ","");
	ustring::replace(paramName,"class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >","string");

	struct TypeInfo
	{
		std::string name;
		std::vector<TypeInfo> templateTypes;
		pragma::doc::Variant::Flags flags = pragma::doc::Variant::Flags::None;
	};
	auto normalize = [](std::string &type,TypeInfo &dtype) {
		ustring::remove_whitespace(type);
		if(ustring::compare(type.data(),"[struct",true,7))
			type = type.substr(7);
		if(ustring::compare(type.data(),"[class",true,6))
			type = type.substr(6);

		//ustring::replace(paramName,"util::TSharedHandle<","");
		//ustring::replace(paramName,"util::WeakHandle<","");

		auto isRef = (type.find("&") != std::string::npos);
		auto isPtr = (type.find("*") != std::string::npos);
		auto isConst = (type.find(" const") != std::string::npos);
		if(isRef)
			dtype.flags |= pragma::doc::Variant::Flags::Reference;
		if(isPtr)
			dtype.flags |= pragma::doc::Variant::Flags::Pointer;
		if(isConst)
			dtype.flags |= pragma::doc::Variant::Flags::Const;
		ustring::replace(type,"[","");
		ustring::replace(type,"]","");
		ustring::replace(type,"&","");
		ustring::replace(type,"*","");
		ustring::replace(type," const","");
		ustring::replace(type,"luabind::adl::","");
		//ustring::replace(paramName,">","");
		ustring::remove_whitespace(type);

		auto it = g_typeTranslationTable.find(type);
		if(it != g_typeTranslationTable.end())
			type = it->second;
		else
		{
			auto it = g_classNameToFullName.find(type);
			if(it != g_classNameToFullName.end())
				type = it->second;
			else
			{
				if(g_typeWarningCache.find(type) == g_typeWarningCache.end())
				{
					g_typeWarningCache.insert(type);
					Con::cwar<<"WARNING: Unknown type '"<<type<<"'!"<<Con::endl;
				}
			}
		}
	};

	auto fFindClosingTag = [](const std::string_view &str) -> size_t {
		if(str.empty())
			return std::string::npos;
		uint32_t idx = 0;
		uint32_t depth = 1;
		while(idx < str.length())
		{
			auto c = str[idx];
			if(c == '>')
			{
				if(--depth == 0)
					return idx;
			}
			else if(c == '<')
				++depth;
			++idx;
		}
		return std::string::npos;
	};

	std::function<void(TypeInfo&)> parseTemplateTypes = nullptr;
	std::string_view vpn {paramName};
	parseTemplateTypes = [&parseTemplateTypes,&vpn,&normalize,&fFindClosingTag](TypeInfo &type) {
		auto pStart = vpn.find('<');
		auto pEnd = vpn.find('>');
		if(pStart < pEnd)
		{
			type.name = vpn.substr(0,pStart);
			normalize(type.name,type);
			vpn = vpn.substr(pStart +1);
			
			if(type.name == "typehint") // We'll skip this one
				return parseTemplateTypes(type);
			uint32_t maxTypes = 1;
			if(type.name == "tableTT")
				maxTypes = 2;
			else if(type.name == "variant" || type.name == "mult")
				maxTypes = std::numeric_limits<decltype(maxTypes)>::max();
			for(;;)
			{
				type.templateTypes.push_back({});
				parseTemplateTypes(type.templateTypes.back());
				auto &t = type.templateTypes.back();
				if(t.name == "std::shared_ptr" && t.templateTypes.size() == 1)
				{
					auto tmp = std::move(t.templateTypes[0]);
					t = std::move(tmp);
				}
				pEnd = fFindClosingTag(vpn);
				if(--maxTypes == 0)
					break;
				auto c = vpn.find(',');
				if(c >= pEnd)
					break;
				pEnd = c;
			}
			if(pEnd == std::string::npos)
				vpn = {};
			else
				vpn = vpn.substr(pEnd +1);
			return;
		}
		type.name = vpn.substr(0,pEnd);
		normalize(type.name,type);
		if(pEnd == std::string::npos)
			vpn = {};
		else
			vpn = vpn.substr(pEnd +1);
	};

	std::string_view strView{paramName};
	auto typeInfo = parse_type(strView);


	return newTypes;
	/*TypeInfo typeInfo {};
	typeInfo.name = paramName;
	parseTemplateTypes(typeInfo);

	if(typeInfo.name == "optional")
	{
		if(typeInfo.templateTypes.empty())
		{
			newType.name = "nil";
			return newTypes;
		}
		newType.name = typeInfo.templateTypes[0].name;
		newType.flags |= pragma::doc::Type::Flags::Optional;
		return newTypes;
	}
	if(typeInfo.name == "table")
	{
		newType.name = "table";
		return newTypes;
	}
	if(typeInfo.name == "tableT")
	{
		newType.name = "table";
		if(typeInfo.templateTypes.size() > 0)
			newType.name += "[" +typeInfo.templateTypes[0].name +"]";
		return newTypes;
	}
	if(typeInfo.name == "tableTT")
	{
		newType.name = "table";
		if(typeInfo.templateTypes.size() > 0)
		{
			newType.name += "[" +typeInfo.templateTypes[0].name +"]";
			if(typeInfo.templateTypes.size() > 1)
				newType.name += "[" +typeInfo.templateTypes[1].name +"]";
		}
		return newTypes;
	}
	if(typeInfo.name == "variant")
	{
		auto tmp = std::move(newType);
		newTypes.clear();
		newTypes.reserve(typeInfo.templateTypes.size());
		for(auto &typeInfo : typeInfo.templateTypes)
		{

		}
		return newTypes;
	}

	return newTypes;*/
	/*
	if(typeInfo.name == "variant")
	{
		type.name
		type.name = "table";
		if(typeInfo.templateTypes.size() > 0)
			type.name += "[" +typeInfo.templateTypes[0].name +"]"; // TODO: Table of variants?
		return;
	}*/
}
static void apply(pragma::lua::TypeNameManager &nameManager,pragma::doc::Variant &variant,const TypeInfo &typeInfo)
{
	variant.name = typeInfo.typeName;
	normalize_type_name(nameManager,variant.name,&typeInfo);
	if(variant.name == "optional")
	{
		if(typeInfo.templateParameterTypes.empty())
		{
			variant.name = "nil";
			Con::cwar<<"WARNING: Missing optional template type"<<Con::endl;
		}
		else
		{
			auto tmp = std::move(typeInfo.templateParameterTypes.front());
			apply(nameManager,variant,tmp);
			variant.flags |= pragma::doc::Variant::Flags::Optional;
		}
		return;
	}
	if(variant.name == "typehint")
	{
		if(typeInfo.templateParameterTypes.empty())
		{
			variant.name = "nil";
			Con::cwar<<"WARNING: Missing typehint template type"<<Con::endl;
		}
		else
		{
			auto tmp = std::move(typeInfo.templateParameterTypes.front());
			apply(nameManager,variant,tmp);
		}
		return;
	}
	if(variant.name == "tableT")
	{
		variant.name = "table";
		if(typeInfo.templateParameterTypes.empty())
			Con::cwar<<"WARNING: Missing tableT template type"<<Con::endl;
		else
		{
			variant.typeParameters.push_back({});
			auto &p = variant.typeParameters.back();
			apply(nameManager,p,typeInfo.templateParameterTypes.front());
		}
		return;
	}
	if(variant.name == "tableTT")
	{
		variant.name = "table";
		if(typeInfo.templateParameterTypes.empty())
			Con::cwar<<"WARNING: Missing tableTT template type"<<Con::endl;
		else
		{
			variant.typeParameters.push_back({});
			apply(nameManager,variant.typeParameters.back(),typeInfo.templateParameterTypes.front());
			if(typeInfo.templateParameterTypes.size() > 1)
			{
				variant.typeParameters.push_back({});
				apply(nameManager,variant.typeParameters.back(),typeInfo.templateParameterTypes[1]);
			}
		}
		return;
	}
	if(variant.name == "variant")
	{
		variant.name = "variant";
		if(typeInfo.templateParameterTypes.empty())
			Con::cwar<<"WARNING: Missing variant template type"<<Con::endl;
		else
		{
			variant.typeParameters.reserve(typeInfo.templateParameterTypes.size());
			for(auto &type : typeInfo.templateParameterTypes)
			{
				variant.typeParameters.push_back({});
				apply(nameManager,variant.typeParameters.back(),type);
			}
		}
		return;
	}
	if(variant.name == "mult")
	{
		variant.name = "mult";
		if(typeInfo.templateParameterTypes.empty())
			Con::cwar<<"WARNING: Missing mult template type"<<Con::endl;
		else
		{
			variant.typeParameters.reserve(typeInfo.templateParameterTypes.size());
			for(auto &type : typeInfo.templateParameterTypes)
			{
				variant.typeParameters.push_back({});
				apply(nameManager,variant.typeParameters.back(),type);
			}
		}
		return;
	}
	std::cout<<"";
}
static void normalize_variant(pragma::lua::TypeNameManager &nameManager,pragma::doc::Variant &variant)
{
	auto &name = variant.name;
	if(name.empty())
		return;
	static const std::vector<std::pair<std::string,char>> templatePrefixes = {
		{"std::shared_ptr<",'>'},
		{"util::WeakHandle<",'>'},
		{"custom [",']'},
	};
	for(auto &prefix : templatePrefixes)
	{
		if(!ustring::compare(name.c_str(),prefix.first.c_str(),true,prefix.first.length()))
			continue;
		name = name.substr(prefix.first.length());
		name = name.substr(0,name.rfind(prefix.second));
	}
	auto typeTranslated = nameManager.TranslateType(name);
	if(typeTranslated.has_value())
	{
		name = *typeTranslated;
		return;
	}

	std::string_view v {variant.name};

	auto typeInfo = parse_type(v);
	//std::cout<<"TEST: "<<variant.name<<std::endl;
	//apply(nameManager,variant,typeInfo);
	if(typeInfo.typeName == "optional")
	{
		//apply(variant,typeInfo);
		/*if(typeInfo.templateParameterTypes.empty())
		{
			variant.name = "nil";
			Con::cwar<<"WARNING: Missing optional template type"<<Con::endl;
		}
		else
		{
			auto tmp = std::move(typeInfo.templateParameterTypes.front());
			variant.flags |= pragma::doc::Variant::Flags::Optional;
			variant.name = typeInfo.templateParameterTypes.front().typeName;
			normalize_variant(variant);
		}*/
		return;
	}
	/*if(typeInfo.typeName == "tableT")
	{
		variant.name = "table";
		if(typeInfo.templateParameterTypes.empty())
			Con::cwar<<"WARNING: Missing tableT template type"<<Con::endl;
		else
		{
			variant.typeParameters.push_back({});
			auto &p = variant.typeParameters.back();
			apply(p,typeInfo.templateParameterTypes.front());
		}
		return;
	}
	if(typeInfo.typeName == "tableTT")
	{
		variant.name = "table";
		if(typeInfo.templateParameterTypes.empty())
			Con::cwar<<"WARNING: Missing tableTT template type"<<Con::endl;
		else
		{
			variant.typeParameters.push_back({});
			auto &p = variant.typeParameters.back();
			p.name = typeInfo.templateParameterTypes.front().typeName;
			normalize_variant(p);
			if(typeInfo.templateParameterTypes.size() > 1)
			{
				variant.typeParameters.push_back({});
				auto &p = variant.typeParameters.back();
				p.name = typeInfo.templateParameterTypes[1].typeName;
				normalize_variant(p);
			}
		}
		return;
	}
	variant.name = typeInfo.typeName;*/ // Ignore template types
	/*if(typeInfo.typeName == "variant")
	{
		variant.name = "table";
		if(typeInfo.templateParameterTypes.empty())
			Con::cwar<<"WARNING: Missing tableT template type"<<Con::endl;
		else
		{
			variant.typeParameters.push_back({});
			auto &p = variant.typeParameters.back();
			p.name = typeInfo.templateParameterTypes.front().typeName;
			normalize_variant(p);
		}
		return;
	}*/

	/*for(auto it=variant.typeParameters.begin();it!=variant.typeParameters.end();)
	{
		auto &type = *it;
		auto newTypes = normalize_type_name(type);
		it = variant.typeParameters.erase(it);
		for(auto &newType : newTypes)
		{
			normalize_variant(newType);
			it = variant.typeParameters.insert(it,newType);
			++it;
		}
	}*/
}

/*static void *get_function_pointer_from_luabind_function(lua_State *l,const luabind::object &lbFunc)
{
	lbFunc.push(l);
	luabind::detail::stack_pop popFunc {l,1};

	auto *f = lua_tocfunction(l,-1);
	auto fidx = Lua::GetStackTop(l);

	auto *name = lua_getupvalue(l,fidx,1);
	if(!name)
		return nullptr;
	luabind::detail::stack_pop popUpValue {l,1};
	auto *tmp = *(luabind::detail::function_object**)lua_touserdata(l,-1);
	if(!tmp)
		return nullptr;
	return get_function_pointer_from_luabind_function_object(*tmp);
}

static void strip_base_class_methods(std::unordered_map<luabind::detail::class_rep*,LuaClassInfo> &classInfoList,LuaClassInfo &classInfo,LuaClassInfo &bc)
{
	for(auto *base : bc.bases)
	{
		auto it = classInfoList.find(base);
		if(it == classInfoList.end())
			continue;
		auto &baseClassInfo = it->second;
		strip_base_class_methods(classInfoList,classInfo,baseClassInfo);
		for(auto itMethod=classInfo.methods.begin();itMethod!=classInfo.methods.end();)
		{
			auto &methodInfo = *itMethod;
			auto it = std::find_if(baseClassInfo.methods.begin(),baseClassInfo.methods.end(),[&methodInfo](const LuaMethodInfo &methodInfoOther) {
				return methodInfo.name == methodInfoOther.name;
			});
			if(it == baseClassInfo.methods.end())
			{
				++itMethod;
				continue;
			}
			auto &baseClassMethod = *it;
			for(auto it=methodInfo.overloads.begin();it!=methodInfo.overloads.end();)
			{
				auto &overload = *it;
				auto itBase = std::find_if(baseClassMethod.overloads.begin(),baseClassMethod.overloads.end(),[&overload](const LuaOverloadInfo &overloadInfoOther) {
					return overload.parameters == overloadInfoOther.parameters && overload.returnValues == overloadInfoOther.returnValues;
				});
				if(itBase != baseClassMethod.overloads.end())
					it = methodInfo.overloads.erase(it);
				else
					++it;
			}
			if(methodInfo.overloads.empty())
				itMethod = classInfo.methods.erase(itMethod);
			else
				++itMethod;
		}
	}
}*/

#include "pragma/lua/libraries/lutil.hpp"

class class_rep_public
{
public:
	luabind::type_id m_type;
	std::vector<luabind::detail::class_rep::base_info> m_bases;
	const char* m_name;
	luabind::detail::lua_reference m_self_ref;
	luabind::handle m_table;
	luabind::handle m_default_table;
	luabind::detail::class_rep::class_type m_class_type;
	int m_instance_metatable;
	std::map<const char*, int, luabind::detail::ltstr> m_static_constants;
	int m_operator_cache;
	luabind::detail::cast_graph* m_casts;
	luabind::detail::class_id_map* m_classes;
};
static_assert(sizeof(luabind::detail::class_rep) == sizeof(class_rep_public));

static int static_class_gettable(lua_State* L) // Same as luabind::detail::class_rep::static_class_gettable, but without invoking an error if the key doesn't exist
{
	luabind::detail::class_rep* crep = static_cast<luabind::detail::class_rep*>(lua_touserdata(L, 1));

	// look in the static function table
	crep->get_default_table(L);
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if(!lua_isnil(L, -1)) return 1;
	else lua_pop(L, 2);

	const char* key = lua_tostring(L, 2);

	if(std::strlen(key) != lua_objlen(L, 2))
	{
		lua_pushnil(L);
		return 1;
	}

	std::map<const char*, int, luabind::detail::ltstr>::const_iterator j = reinterpret_cast<class_rep_public*>(crep)->m_static_constants.find(key);

	if(j != reinterpret_cast<class_rep_public*>(crep)->m_static_constants.end())
	{
		lua_pushnumber(L, j->second);
		return 1;
	}

	lua_pushnil(L);

	//return lua_type(interpreter, -1);

	return 1;
}

static void merge_function(pragma::doc::Function &a,const pragma::doc::Function &b)
{
	a.SetFlags(a.GetFlags() | b.GetFlags());
	a.SetGameStateFlags(a.GetGameStateFlags() | b.GetGameStateFlags());
	
	auto &overloadsA = a.GetOverloads();
	auto &overloadsB = b.GetOverloads();
	for(auto &overloadB : overloadsB)
	{
		auto it = std::find_if(overloadsA.begin(),overloadsA.end(),[&overloadB](const pragma::doc::Overload &overloadA) {
			return overloadA == overloadB;
		});
		if(it == overloadsA.end())
		{
			overloadsA.push_back(overloadB);
			continue;
		}
	}
}

static void merge_member(pragma::doc::Member &a,const pragma::doc::Member &b)
{
	a.SetFlags(a.GetFlags() | b.GetFlags());
	a.SetGameStateFlags(a.GetGameStateFlags() | b.GetGameStateFlags());
}

static void merge_enum(pragma::doc::Enum &a,const pragma::doc::Enum &b)
{
	a.SetGameStateFlags(a.GetGameStateFlags() | b.GetGameStateFlags());
}

static void merge_enum_set(pragma::doc::EnumSet &a,const pragma::doc::EnumSet &b)
{
	a.SetFlags(a.GetFlags() | b.GetFlags());
	
	auto &enumsA = a.GetEnums();
	auto &enumsB = b.GetEnums();
	for(auto &eB : enumsB)
	{
		auto it = std::find_if(enumsA.begin(),enumsA.end(),[&eB](const pragma::doc::Enum &eA) {
			return eA == eB;
		});
		if(it == enumsA.end())
		{
			enumsA.push_back(eB);
			enumsA.back().SetCollection(*a.GetCollection());
			continue;
		}
		merge_enum(*it,eB);
	}
}

static void merge_collection(pragma::doc::Collection &a,const pragma::doc::Collection &b)
{
	a.SetFlags(a.GetFlags() | b.GetFlags());
	
	auto &functionsA = a.GetFunctions();
	auto &functionsB = b.GetFunctions();
	for(auto &funcB : functionsB)
	{
		auto it = std::find_if(functionsA.begin(),functionsA.end(),[&funcB](const pragma::doc::Function &funcA) {
			return funcA == funcB;
		});
		if(it == functionsA.end())
		{
			functionsA.push_back(funcB);
			functionsA.back().SetCollection(a);
			continue;
		}
		merge_function(*it,funcB);
	}

	auto &membersA = a.GetMembers();
	auto &membersB = b.GetMembers();
	for(auto &memB : membersB)
	{
		auto it = std::find_if(membersA.begin(),membersA.end(),[&memB](const pragma::doc::Member &memA) {
			return memA == memB;
		});
		if(it == membersA.end())
		{
			membersA.push_back(memB);
			membersA.back().SetCollection(a);
			continue;
		}
		merge_member(*it,memB);
	}

	auto &enumSetA = a.GetEnumSets();
	auto &enumSetB = b.GetEnumSets();
	for(auto &esB : enumSetB)
	{
		auto it = std::find_if(enumSetA.begin(),enumSetA.end(),[&esB](const std::shared_ptr<pragma::doc::EnumSet> &esA) {
			return *esA == *esB;
		});
		if(it == enumSetA.end())
		{
			enumSetA.push_back(esB);
			enumSetA.back()->SetCollection(a);
			continue;
		}
		merge_enum_set(**it,*esB);
	}
	
	auto &derivedFromA = a.GetDerivedFrom();
	auto &derivedFromB = b.GetDerivedFrom();
	for(auto &derivedFrom : derivedFromB)
	{
		auto it = std::find_if(derivedFromA.begin(),derivedFromA.end(),[&derivedFrom](const std::shared_ptr<pragma::doc::DerivedFrom> &derivedFromOther) {
			return derivedFromOther->GetName() == derivedFrom->GetName();
		});
		if(it != derivedFromA.end())
			continue;
		derivedFromA.push_back(pragma::doc::DerivedFrom::Create((*it)->GetName(),(*it)->GetParent().get()));
	}

	auto &childrenA = a.GetChildren();
	auto &childrenB = b.GetChildren();
	for(auto &cB : childrenB)
	{
		auto it = std::find_if(childrenA.begin(),childrenA.end(),[&cB](const pragma::doc::PCollection &cA) {
			return *cA == *cB;
		});
		if(it == childrenA.end())
		{
			a.AddChild(cB);
			continue;
		}
		merge_collection(**it,*cB);
	}
}

FARPROC customGetProcAddress(HMODULE module, const PSTR functionName) 
{ 
  PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(module); 
  if(!pDosHeader || pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) 
    throw std::runtime_error("Process::customGetProcAddress Error : DOS PE header is invalid."); 

  PIMAGE_NT_HEADERS pNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<PCHAR>(module) + pDosHeader->e_lfanew); 
  if(pNtHeader->Signature != IMAGE_NT_SIGNATURE) 
    throw std::runtime_error("Process::customGetProcAddress Error : NT PE header is invalid."); 

  PVOID pExportDirTemp        = reinterpret_cast<PBYTE>(module) + pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress; 
  PIMAGE_EXPORT_DIRECTORY pExportDir  = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(pExportDirTemp); 
  if(pExportDir->AddressOfNames == NULL) 
    throw std::runtime_error("Process::customGetProcAddress Error : Symbol names missing entirely."); 

  PDWORD pNamesRvas      = reinterpret_cast<PDWORD>(reinterpret_cast<PBYTE>(module) + pExportDir->AddressOfNames); 
  PWORD pNameOrdinals      = reinterpret_cast<PWORD>(reinterpret_cast<PBYTE>(module) + pExportDir->AddressOfNameOrdinals); 
  PDWORD pFunctionAddresses  = reinterpret_cast<PDWORD>( reinterpret_cast<PBYTE>(module) + pExportDir->AddressOfFunctions); 

  for (DWORD n = 0; n < pExportDir->NumberOfNames; n++) 
  { 
    PSTR CurrentName = reinterpret_cast<PSTR>(reinterpret_cast<PBYTE>(module) + pNamesRvas[n]);   
    if(strcmp(functionName, CurrentName) == 0) 
    { 
      WORD Ordinal = pNameOrdinals[n]; 
      return reinterpret_cast<FARPROC>(reinterpret_cast<PBYTE>(module) + pFunctionAddresses[Ordinal]); 
    } 
  } 

  return 0; 
}

#include <Tlhelp32.h>
MODULEENTRY32 GetModuleInfo(std::uint32_t ProcessID, const char* ModuleName)
{
    void* hSnap = nullptr;
    MODULEENTRY32 Mod32 = {0};

    if ((hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID)) == INVALID_HANDLE_VALUE)
        return Mod32;

    Mod32.dwSize = sizeof(MODULEENTRY32);
    while (Module32Next(hSnap, &Mod32))
    {
        if (!strcmp(ModuleName, Mod32.szModule))
        {
            CloseHandle(hSnap);
            return Mod32;
        }
    }

    CloseHandle(hSnap);
    return {0};
}

namespace dbg
{
    class Help
    {
    public:
        bool Initialize();
        void LoadSymbolModule();
    private:
        void EnumerateSymbols();
        static BOOL CALLBACK EnumSymProc( 
            PSYMBOL_INFO pSymInfo,   
            ULONG SymbolSize,      
            PVOID UserContext);
        HANDLE m_hProcess;
    };
};

bool dbg::Help::Initialize()
{
    DWORD  error;

    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

    m_hProcess = GetCurrentProcess();

    if (!SymInitialize(m_hProcess, NULL, TRUE))
    {
        // SymInitialize failed
        error = GetLastError();
        printf("SymInitialize returned error : %d\n", error);
        return FALSE;
    }
    return TRUE;
}

void dbg::Help::EnumerateSymbols()
{

}

BOOL CALLBACK dbg::Help::EnumSymProc( 
    PSYMBOL_INFO pSymInfo,   
    ULONG SymbolSize,      
    PVOID UserContext)
{
    UNREFERENCED_PARAMETER(UserContext);
    
    printf("%08X %4u %s\n", 
           pSymInfo->Address, SymbolSize, pSymInfo->Name);

    {
        IMAGEHLP_LINE64 imgHlp;
        auto *hlp = static_cast<Help*>(UserContext);
        DWORD displacement;
        if(SymGetLineFromAddr64(hlp->m_hProcess,pSymInfo->Address,&displacement,&imgHlp) == TRUE)
        {
            std::cout<<"Filename: "<<imgHlp.FileName<<std::endl;
        }
    }


    return TRUE;
}

#include <Winbase.h>
void dbg::Help::LoadSymbolModule()
{
    std::string szImageName = "E:/projects/pragma/build_winx64/install/bin/shared.dll";
    DWORD64 dwBaseAddr = 0;
	AddDllDirectory(L"E:/projects/pragma/build_winx64/install/bin/");
    auto dllBase = SymLoadModuleEx(m_hProcess,    // target process 
                        NULL,        // handle to image - not used
                        szImageName.c_str(), // name of image file
                        NULL,        // name of module - not required
                        dwBaseAddr,  // base address - not required
                        0,           // size of image - not required
                        NULL,        // MODLOAD_DATA used for special cases 
                        0);
    if (dllBase)          // flags - not required
    {
        // SymLoadModuleEx returned success
    }
    else
    {
        // SymLoadModuleEx failed
        DWORD error = GetLastError();
        //printf("SymLoadModuleEx returned error : %d\n", error);
		std::cout<<"ERR: "<<pragma::os::windows::get_last_error_as_string()<<std::endl;
    }

    // Enum
    const char *Mask = "?testt@@YA?AV?$shared_ptr@VSchedule@ai@pragma@@@std@@XZ";
    if (SymEnumSymbols(m_hProcess,     // Process handle from SymInitialize.
                        dllBase,   // Base address of module.
                        NULL,        // Name of symbols to match.
                        EnumSymProc, // Symbol handler procedure.
                        this))       // User context.
    {
        // SymEnumSymbols succeeded
    }
    else
    {
        // SymEnumSymbols failed
        //printf("SymEnumSymbols failed: %d\n", GetLastError());
        std::cout<<pragma::os::windows::get_last_error_as_string()<<std::endl;
    }
}

class DLLNETWORK ThisIsATestClass {
public:
	ThisIsATestClass()=default;
};
ThisIsATestClass test_test_test() {return ThisIsATestClass{};}

#pragma comment(lib,"Dbghelp.lib")

#include <luabind/make_function.hpp>
#include "pragma/lua/libraries/lasset.hpp"

static void autogenerate()
{
	{
		auto *cl = pragma::get_engine()->GetClientState();
		if(cl)
		{
			cl->InitializeLibrary("unirender/pr_unirender");
			cl->InitializeLibrary("openvr/pr_openvr");

			cl->InitializeLibrary("pr_dmx");
			cl->InitializeLibrary("steamworks/pr_steamworks");
		}

		auto *sv = pragma::get_engine()->GetServerNetworkState();
		if(sv)
		{
			sv->InitializeLibrary("pr_dmx");
			sv->InitializeLibrary("steamworks/pr_steamworks");
		}
	}

	g_typeWarningCache.clear();
	struct LuaStateInfo
	{
		lua_State *l;
		pragma::doc::PCollection collection;
	};
	std::vector<LuaStateInfo> luaStates;
	auto &en = *pragma::get_engine();
	auto *sv = en.GetServerNetworkState();
	if(sv)
	{
		auto *lsv = sv->GetLuaState();
		if(lsv)
			luaStates.push_back({lsv});
	}
	auto *cl = en.GetClientState();
	if(cl)
	{
		auto *lcl = cl->GetLuaState();
		if(lcl)
			luaStates.push_back({lcl});
	}
	if(luaStates.empty())
		return;

	const std::unordered_map<std::string,std::string> pdbModules = {
		{"server","E:/projects/pragma/build_winx64/core/server/RelWithDebInfo/server.pdb"},
		{"client","E:/projects/pragma/build_winx64/core/client/RelWithDebInfo/client.pdb"},
		{"shared","E:/projects/pragma/build_winx64/core/shared/RelWithDebInfo/shared.pdb"},
		{"mathutil","E:/projects/pragma/build_winx64/external_libs/mathutil/RelWithDebInfo/mathutil.pdb"},
		{"sharedutils","E:/projects/pragma/build_winx64/external_libs/sharedutils/RelWithDebInfo/sharedutils.pdb"},
		{"prosper","E:/projects/pragma/build_winx64/external_libs/prosper/RelWithDebInfo/prosper.pdb"},
		{"vfilesystem","E:/projects/pragma/build_winx64/external_libs/vfilesystem/RelWithDebInfo/vfilesystem.pdb"},
		{"alsoundsystem","E:/projects/pragma/build_winx64/external_libs/alsoundsystem/RelWithDebInfo/alsoundsystem.pdb"},
		{"datasystem","E:/projects/pragma/build_winx64/external_libs/datasystem/RelWithDebInfo/datasystem.pdb"},
		{"iglfw","E:/projects/pragma/build_winx64/external_libs/iglfw/RelWithDebInfo/iglfw.pdb"},
		{"materialsystem","E:/projects/pragma/build_winx64/external_libs/materialsystem/RelWithDebInfo/materialsystem.pdb"},
		{"cmaterialsystem","E:/projects/pragma/build_winx64/external_libs/materialsystem/RelWithDebInfo/cmaterialsystem.pdb"},
		{"util_image","E:/projects/pragma/build_winx64/external_libs/util_image/RelWithDebInfo/util_image.pdb"},
		{"util_pragma_doc","E:/projects/pragma/build_winx64/external_libs/util_pragma_doc/RelWithDebInfo/util_pragma_doc.pdb"},
		{"util_sound","E:/projects/pragma/build_winx64/external_libs/util_sound/RelWithDebInfo/util_sound.pdb"},
		{"util_udm","E:/projects/pragma/build_winx64/external_libs/util_udm/RelWithDebInfo/util_udm.pdb"},
		{"wgui","E:/projects/pragma/build_winx64/external_libs/wgui/RelWithDebInfo/wgui.pdb"},
		{"pr_dmx","E:/projects/pragma/build_winx64/modules/pr_dmx/RelWithDebInfo/pr_dmx.pdb"},
		{"pr_cycles","E:/projects/pragma/build_winx64/modules/pr_cycles/RelWithDebInfo/pr_unirender.pdb"},
		{"pr_openvr","E:/projects/pragma/build_winx64/modules/pr_openvr/RelWithDebInfo/pr_openvr.pdb"},
		{"pr_steamworks","E:/projects/pragma/build_winx64/modules/pr_steamworks/RelWithDebInfo/pr_steamworks.pdb"},
		{"panima","E:/projects/pragma/build_winx64/external_libs/panima/RelWithDebInfo/panima.pdb"}
	};
	pragma::lua::PdbManager pdbManager {};
	if(pdbManager.Initialize())
	{
		// TODO: Determine the paths automatically somehow?
		for(auto &pair : pdbModules)
			pdbManager.LoadPdb(pair.first,pair.second);
	}
	
	pragma::lua::TypeNameManager typeManager {};
	for(auto &stateInfo : luaStates)
	{
		auto *L = stateInfo.l;

		auto *reg = luabind::detail::class_registry::get_registry(L);
		auto &classes = reg->get_classes();
		for(auto &pair : classes)
		{
			static_assert(sizeof(luabind::type_id) == sizeof(std::type_info*));
			auto &typeInfo = **(reinterpret_cast<std::type_info* const *>(&pair.second->type()));
			typeManager.RegisterType(typeInfo,*pair.second);
		}
	}

	pragma::lua::SymbolHandler symHandler {};
	symHandler.Initialize();
	for(auto &stateInfo : luaStates)
	{
		//
		/*for(auto &pair : pdbModules)
		{
			auto begin = pdbManager.begin(pair.first);
			auto end = pdbManager.end();
			for(auto it=begin;it!=end;++it)
			{
				auto &symbol = *it;
				if(symbol.GetTag() != pragma::lua::SymTag::PublicSymbol)
					continue;
				auto name = symbol.GetName();
				if(!name.has_value())
					continue;
				auto itType = typeList.find(*name);
				if(itType == typeList.end())
					continue;
				itType->second = "LuaType";
			}
		}



		Con::cout<<"Found:"<<Con::endl;
		for(auto &pair : typeList)
		{
			Con::cout<<pair.first<<": ";
			if(!pair.second.has_value())
				Con::cout<<"n/a";
			else
				Con::cout<<*pair.second;
			Con::cout<<Con::endl;
		}
		for(auto &pair : typeList)
			Con::cout<<pair.first<<Con::endl;
		Con::cout<<Con::endl;*/
		//

		
		auto *L = stateInfo.l;

		auto rootCol = pragma::doc::Collection::Create();
		std::string rootColName = "root";
		rootCol->SetName(rootColName);
		auto gCol = pragma::doc::Collection::Create();
		gCol->SetName("_G");
		gCol->SetFlags(gCol->GetFlags() | pragma::doc::Collection::Flags::AutoGenerated);
		rootCol->AddChild(gCol);
		
		pragma::lua::LuaDocGenerator docGenerator {L};
		docGenerator.SetPdbManager(&pdbManager);
		docGenerator.SetSymbolHandler(&symHandler);
		docGenerator.SetTypeManager(&typeManager);
		docGenerator.IterateLibraries(luabind::globals(L),*rootCol,*gCol);

		// All class information has now been collected, so we have the full type names for all registered types now, which we
		// need to generate the correct parameters and return values for our methods.
		docGenerator.PopulateMethods(L);
		//

		auto &classMap = docGenerator.GetClassMap();
		for(auto &pair : classMap)
		{
			auto &crep = *pair.first;
			auto &bases = crep.bases();
			for(auto &base : bases)
			{
				auto it = classMap.find(base.base);
				if(it == classMap.end())
				{
					Con::cwar<<"WARNING: Missing class '"<<base.base->name()<<"'"<<Con::endl;
					continue;
				}
				auto fullName = it->second->GetFullName();
				fullName = fullName.substr(rootColName.length() +1);
				auto derivedFrom = pragma::doc::DerivedFrom::Create(fullName);
				pair.second->AddDerivedFrom(*derivedFrom);
			}
		}
		rootCol->StripBaseDefinitionsFromDerivedCollections();
		stateInfo.collection = rootCol;
	}

	auto &assignedTypes = typeManager.GetAssignedTypes();
	for(auto &pair : typeManager.GetTypes())
	{
		if(pair.second->bestMatch.has_value())
			continue;
		Con::cwar<<"WARNING: Unassigned type '"<<pair.second->luaName<<"' ("<<pair.second->name<<")"<<Con::endl;
	}
	for(auto &pair : assignedTypes)
	{
		if(!pair.second)
			continue;
		auto *ti = typeManager.GetTypeInfo(pair.second);
		Con::cout<<pair.first<<" = "<<ti->luaName<<" ("<<ti->name<<")"<<Con::endl;
	}

	// Merge collections from all Lua states
	auto &rootCol = luaStates.front().collection;
	for(auto it=luaStates.begin() +1;it!=luaStates.end();++it)
	{
		auto &stateInfo = *it;
		merge_collection(*rootCol,*it->collection);
	}
	//

	// Merge base types into derived types (where applicable)
	auto *colEnts = rootCol->FindChildCollection("ents");
	std::vector<pragma::doc::PCollection> removeQueue;
	if(colEnts)
	{
		auto &children = colEnts->GetChildren();
		for(auto it=children.begin();it!=children.end();++it)
		{
			auto &child = *it;
			auto isComponentType = false;
			auto *derivedFrom = &child->GetDerivedFrom();
			if(derivedFrom && !derivedFrom->empty())
			{
				auto *df = derivedFrom->front().get();
				while(df)
				{
					if(df->GetName() == "ents.EntityComponent")
					{
						isComponentType = true;
						break;
					}
					auto *c = rootCol->FindChildCollection(df->GetName());
					if(!c || c->GetDerivedFrom().empty())
						break;
					df = c->GetDerivedFrom().front().get();
				}
			}
			if(!isComponentType)
				continue;
			auto df = (*derivedFrom)[0];
			if(df->GetName() == "ents.EntityComponent")
				continue; // Directly derived from BaseEntityComponent
			derivedFrom->clear();
			auto *colDerived = rootCol->FindChildCollection(df->GetName());
			if(colDerived)
			{
				merge_collection(*child,*colDerived);
				removeQueue.push_back(colDerived->shared_from_this());
			}
		}
	}
	for(auto &col : removeQueue)
	{
		auto *colParent = col->GetParent();
		if(!colParent)
			continue;
		auto &children = colParent->GetChildren();
		auto it = std::find(children.begin(),children.end(),col);
		if(it != children.end())
			children.erase(it);
	}
	//

	// Get class names
	g_classNameToFullName.clear();
	std::unordered_map<std::string,uint32_t> classNameCount;
	std::function<void(pragma::doc::Collection&)> getClassNames = nullptr;
	getClassNames = [&getClassNames,&classNameCount](pragma::doc::Collection &col) {
		auto &name = col.GetName();
		auto fullName = col.GetFullName();
		if(ustring::compare(fullName.c_str(),"root",true,4))
			fullName = fullName.substr(4);
		if(ustring::compare(fullName.c_str(),".",true,1))
			fullName = fullName.substr(1);
		if(fullName.empty() == false)
		{
			g_classNameToFullName[name] = std::move(fullName);
			auto it = classNameCount.find(name);
			if(it == classNameCount.end())
				it = classNameCount.insert(std::make_pair(name,0)).first;
			++it->second;
		}

		for(auto &child : col.GetChildren())
			getClassNames(*child);
	};
	getClassNames(*rootCol);
	for(auto &pair : classNameCount)
	{
		if(pair.second == 1)
			continue;
		auto it = g_classNameToFullName.find(pair.first);
		if(it != g_classNameToFullName.end())
			g_classNameToFullName.erase(it);
	}
	//

	// Translate parameter and return types
	std::function<void(pragma::doc::Collection&)> translateCollectionTypes = nullptr;
	translateCollectionTypes = [&translateCollectionTypes,&typeManager](pragma::doc::Collection &col) {
		auto isClass = umath::is_flag_set(col.GetFlags(),pragma::doc::Collection::Flags::Class);
		auto &functions = col.GetFunctions();
		for(auto &f : functions)
		{
			auto &overloads = f.GetOverloads();
			for(auto &overload : overloads)
			{
				auto &params = overload.GetParameters();
				for(auto it=params.begin();it!=params.end();)
				{
					auto &param = *it;
					auto variant = param.GetType();
					if(variant.name == "lua_State")
					{
						it = params.erase(it);
						continue;
					}
					normalize_variant(typeManager,variant);
					param.SetType(std::move(variant));
					++it;
				}
				if(!params.empty() && params.front().GetType().name == "lua_State")
					params.erase(params.begin());
				auto isMethod = isClass;
				if(isMethod && !params.empty())
					params.erase(params.begin());
				for(auto &ret : overload.GetReturnValues())
				{
					auto variant = ret.GetType();
					normalize_variant(typeManager,variant);
					ret.SetType(std::move(variant));
				}
			}
		}

		auto &members = col.GetMembers();
		for(auto &member : members)
		{
			auto type = member.GetType();
			normalize_type_name(typeManager,type.name);
			member.SetType(std::move(type));
		}

		for(auto &child : col.GetChildren())
			translateCollectionTypes(*child);
	};
	translateCollectionTypes(*rootCol);
	//
	
	std::string err;
	auto repoMan = pragma::lua::RepositoryManager::Create(luaStates.front().l,err);
	if(repoMan)
	{
		if(!repoMan->LoadRepositoryReferences(err))
			Con::cwar<<"WARNING: LAD repository manager failed: "<<err<<Con::endl;
	}
	else
		Con::cwar<<"WARNING: Unable to create LAD repository manager: "<<err<<Con::endl;

	// Merge with old documentation
	std::string docLocation = std::string{Lua::doc::FILE_LOCATION} +"pragma." +std::string{Lua::doc::FILE_EXTENSION_ASCII};
	auto udmData = util::load_udm_asset(docLocation,&err);
	if(udmData)
	{
		auto docOld = pragma::doc::Collection::Load(udmData->GetAssetData(),err);
		if(docOld)
		{
			std::function<void(pragma::doc::Collection&,pragma::doc::Collection&)> mergeOldDocIntoNew = nullptr;
			mergeOldDocIntoNew = [&mergeOldDocIntoNew](pragma::doc::Collection &colNew,pragma::doc::Collection &colOld) {
				auto &newFunctions = colNew.GetFunctions();
				for(auto &f : colOld.GetFunctions())
				{
					if(umath::is_flag_set(f.GetFlags(),pragma::doc::Function::Flags::AutoGenerated))
						continue;
					auto &name = f.GetName();
					auto it = std::find_if(newFunctions.begin(),newFunctions.end(),[&name](const pragma::doc::Function &f) {
						return f.GetName() == name;
					});
					if(it == newFunctions.end())
					{
						newFunctions.push_back(f);
						it = newFunctions.end() -1;
					}
					else
						*it = f;
					it->SetCollection(colNew);
				}

				auto &newMembers = colNew.GetMembers();
				for(auto &m : colOld.GetMembers())
				{
					if(umath::is_flag_set(m.GetFlags(),pragma::doc::Member::Flags::AutoGenerated))
						continue;
					auto &name = m.GetName();
					auto it = std::find_if(newMembers.begin(),newMembers.end(),[&name](const pragma::doc::Member &m) {
						return m.GetName() == name;
					});
					if(it == newMembers.end())
					{
						newMembers.push_back(m);
						it = newMembers.end() -1;
					}
					else
						*it = m;
					it->SetCollection(colNew);
				}

				auto &newEnumSets = colNew.GetEnumSets();
				for(auto &es : colOld.GetEnumSets())
				{
					if(umath::is_flag_set(es->GetFlags(),pragma::doc::EnumSet::Flags::AutoGenerated))
						continue;
					auto &name = es->GetName();
					auto it = std::find_if(newEnumSets.begin(),newEnumSets.end(),[&name](const pragma::doc::PEnumSet &es) {
						return es->GetName() == name;
					});
					if(it == newEnumSets.end())
					{
						newEnumSets.push_back(es);
						it = newEnumSets.end() -1;
					}
					else
						*it = es;
					(*it)->SetCollection(colNew);
					for(auto &e : (*it)->GetEnums())
						e.SetEnumSet(**it);
				}

				auto &newChildren = colNew.GetChildren();
				for(auto &child : colOld.GetChildren())
				{
					auto &name = child->GetName();
					auto it = std::find_if(newChildren.begin(),newChildren.end(),[&name](const pragma::doc::PCollection &c) {
						return c->GetName() == name;
					});
					if(it == newChildren.end())
					{
						auto c = pragma::doc::Collection::Create();
						newChildren.push_back(c);
						*c = *child;
						c->SetParent(&colNew);
						continue;
					}
					mergeOldDocIntoNew(**it,*child);
				}
			};
			mergeOldDocIntoNew(*rootCol,*docOld);
		}
	}
	

#if 0
	/*for(auto *base : classInfo.bases)
	{
		// TODO: Base class of base class?
		std::string dfName = base->name();
		if(!path.empty())
			dfName = path +'.' +dfName;
		auto derivedFrom = pragma::doc::DerivedFrom::Create(dfName);
		collection->AddDerivedFrom(*derivedFrom);
	}*/
#endif
	/*for(auto &pair : classInfo)
	{
		auto it = iteratedClasses.find(pair.first);
		if(it != iteratedClasses.end())
			continue;
		Con::cout<<"Undefined class: "<<pair.first->name()<<std::endl;
	}*/

#if 0
	std::unordered_map<
	for(const auto &cl : classes)
	{
		std::cout<<"CL: "<<cl.second->name()<<std::endl;

		if(ustring::compare(cl.second->name(),"AnimatedComponent"))
		{
			luabind::object ents = luabind::globals(L)["ents"];
			if(ents)
			{
				luabind::object test = ents["AnimatedComponent"];
				if(test)
				{
					//DLLNETWORK luabind::detail::class_rep *get_crep(luabind::object o);
					auto *crep = Lua::get_crep(test);
					if(crep)
					{
						if(cl.second == crep)
						{
							std::cout<<"MATCH!"<<std::endl;
						}
					}
				}
			}
		}
		classInfo[cl.second] = get_class_info(L,cl.second);
	}
#endif

	filemanager::create_path(Lua::doc::FILE_LOCATION);
	udmData = udm::Data::Create();
	rootCol->Save(udmData->GetAssetData(),err);
	try
	{
	udmData->SaveAscii(docLocation);
	}
	catch(const udm::Exception &e)
	{
		std::cout<<e.what()<<std::endl;
	}
	catch(const std::exception &e)
	{
		std::cout<<e.what()<<std::endl;
	}
	catch(...)
	{
		std::cout<<"Unknown!"<<std::endl;
	}
	Lua::doc::reset();

#if 0
	std::stringstream ss;
	for(auto &pair : classInfo)
	{
		auto &classInfo = pair.second;
		ss<<"Class: "<<classInfo.name<<"\n";
		for(auto &attr : classInfo.attributes)
			ss<<"\t"<<attr<<"\n";
		for(auto &method : classInfo.methods)
		{
			for(auto &overload : method.overloads)
				ss<<"\t"<<overload<<"\n";
		}

		//std::vector<pragma::doc::PCollection> collections {};
		// TODO

		//get_class_info(L,cl.second);
	}
	auto f = FileManager::OpenFile<VFilePtrReal>("testdoc.txt","w");
	f->WriteString(ss.str());
	f = nullptr;
#endif
}

namespace Lua::doc
{
	void register_library(Lua::Interface &lua);
};

static void generate_launch_param_doc()
{
	// TODO
}

enum class CvarStateFlag : uint32_t
{
	None = 0,
	Engine,
	Client,
	Server
};
REGISTER_BASIC_BITWISE_OPERATORS(CvarStateFlag)
static std::string generate_convar_doc()
{
	auto *en = pragma::get_engine();
	if(!en)
		return "";
	struct CvarInfo
	{
		std::string name;
		CvarStateFlag stateFlags = CvarStateFlag::None;
		std::shared_ptr<ConConf> cvar;
	};
	std::unordered_map<std::string,CvarInfo> uniqueCvarList;
	auto getAllVars = [&uniqueCvarList](ConVarMap &cvMap,CvarStateFlag flag) {
		auto &cvars = cvMap.GetConVars();
		uniqueCvarList.reserve(uniqueCvarList.size() +cvars.size());
		for(auto &pair : cvars)
		{
			auto it = uniqueCvarList.find(pair.first);
			if(it == uniqueCvarList.end())
				it = uniqueCvarList.insert(std::make_pair(pair.first,CvarInfo{})).first;
			it->second.name = pair.first;
			it->second.stateFlags |= flag;
			it->second.cvar = pair.second;
		}
	};
	auto *cvMap = en->GetConVarMap();
	if(cvMap)
		getAllVars(*cvMap,CvarStateFlag::Engine);

	auto *sv = en->GetServerNetworkState();
	auto *svMap = sv ? sv->GetConVarMap() : nullptr;
	if(svMap)
		getAllVars(*svMap,CvarStateFlag::Server);
	
	auto *cl = en->GetClientState();
	auto *clMap = cl ? cl->GetConVarMap() : nullptr;
	if(clMap)
		getAllVars(*clMap,CvarStateFlag::Client);


	std::vector<std::string> cvarNames;
	std::vector<std::string> cmdNames;
	for(auto &pair : uniqueCvarList)
	{
		auto &cvarInfo = pair.second;
		if(cvarInfo.cvar->GetType() == ConType::Variable)
			cvarNames.push_back(cvarInfo.name);
		else if(cvarInfo.cvar->GetType() == ConType::Command)
			cmdNames.push_back(cvarInfo.name);
	}
	std::sort(cvarNames.begin(),cvarNames.end());
	std::sort(cmdNames.begin(),cmdNames.end());

	auto getConVar = [&uniqueCvarList](const std::string &name) -> std::shared_ptr<ConConf> {
		auto it = uniqueCvarList.find(name);
		assert(it != uniqueCvarList.end());
		return it->second.cvar;
	};

	auto replaceSpecialChars = [](std::string &str) {
		ustring::replace(str,"<","&lt;");
		ustring::replace(str,">","&gt;");
	};
	auto listToStr = [&getConVar,&replaceSpecialChars](const std::vector<std::string> &list,std::stringstream &ss,bool isVar) {
		for(auto name : list)
		{
			auto cf = getConVar(name);
			replaceSpecialChars(name);
			auto help = cf->GetUsageHelp();
			if(isVar && help.empty())
				help = "<value>";
			replaceSpecialChars(help);
			ss<<"<h5>"<<name<<" "<<help<<"</h5>\n";
			ss<<"<p>";
			if(cf->GetType() == ConType::Variable)
			{
				auto &cv = *static_cast<ConVar*>(cf.get());
				ss<<"<strong>Type:</strong> "<<magic_enum::enum_name(cv.GetVarType())<<"</p>\n";
			}
			auto helpText = cf->GetHelpText();
			replaceSpecialChars(helpText);
			ss<<"<p>"<<helpText<<"</p>\n";
		}
	};
	std::stringstream ss;
	ss<<"<h1>Variables</h1>\n";
	listToStr(cvarNames,ss,true);

	ss<<"\n<h1>Commands</h1>\n";
	listToStr(cmdNames,ss,false);
	return ss.str();
}

void Lua::doc::register_library(Lua::Interface &lua)
{
	auto *l = lua.GetState();

	const auto *libName = "doc";
	auto &docLib = lua.RegisterLibrary(libName);
	docLib[
		luabind::def("load",static_cast<luabind::optional<pragma::doc::Collection>(*)(lua_State*,const std::string&)>([](lua_State *l,const std::string &fileName) -> luabind::optional<pragma::doc::Collection> {
			auto fname = fileName;
			if(Lua::file::validate_write_operation(l,fname) == false)
				return nil;
			std::string err;
			auto udmData = ::util::load_udm_asset(fname,&err);
			if(udmData == nullptr)
				return nil;

			auto col = pragma::doc::Collection::Load(udmData->GetAssetData(),err);
			if(!col)
				return nil;
			return luabind::object{l,col};
		})),
		luabind::def("autogenerate",&autogenerate),
		luabind::def("generate_lad_assets",static_cast<void(*)(lua_State*)>([](lua_State *l) {
			Lua::RunString(l,"doc.autogenerate() local el = udm.load('doc/lua/pragma.ldoc'):GetAssetData():GetData() local js = udm.to_json(el) file.write('doc/lua/web_api.json',js) doc.generate_zerobrane_autocomplete_script()","internal");
		})),
		luabind::def("generate_zerobrane_autocomplete_script",static_cast<void(*)(lua_State*)>([](lua_State *l) {
			Lua::doc::generate_autocomplete_script();
		})),
		luabind::def("generate_convar_documentation",generate_convar_doc)
	];
	Lua::RegisterLibraryEnums(l,libName,{
		{"GAME_STATE_FLAG_NONE",umath::to_integral(pragma::doc::GameStateFlags::None)},
		{"GAME_STATE_FLAG_BIT_SERVER",umath::to_integral(pragma::doc::GameStateFlags::Server)},
		{"GAME_STATE_FLAG_BIT_CLIENT",umath::to_integral(pragma::doc::GameStateFlags::Client)},
		{"GAME_STATE_FLAG_BIT_GUI",umath::to_integral(pragma::doc::GameStateFlags::GUI)},
		{"GAME_STATE_FLAG_SHARED",umath::to_integral(pragma::doc::GameStateFlags::Shared)},
		{"GAME_STATE_FLAG_ANY",umath::to_integral(pragma::doc::GameStateFlags::Any)}
	});

	auto classDefBase = luabind::class_<pragma::doc::BaseCollectionObject>("BaseCollectionObject");
	classDefBase.def("GetFullName",static_cast<void(*)(lua_State*,pragma::doc::BaseCollectionObject&)>([](lua_State *l,pragma::doc::BaseCollectionObject &o) {
		Lua::PushString(l,o.GetFullName());
	}));
	classDefBase.def("GetWikiURL",static_cast<void(*)(lua_State*,pragma::doc::BaseCollectionObject&)>([](lua_State *l,pragma::doc::BaseCollectionObject &o) {
		Lua::PushString(l,o.GetWikiURL());
	}));
	docLib[classDefBase];

	auto cdefGroup = luabind::class_<pragma::doc::Group>("Group");
	cdefGroup.def("GetName",static_cast<void(*)(lua_State*,pragma::doc::Group&)>([](lua_State *l,pragma::doc::Group &group) {
		Lua::PushString(l,group.GetName());
	}));
	docLib[cdefGroup];

	auto cdefOverload = luabind::class_<pragma::doc::Overload>("Overload");
	cdefOverload.def("GetParameters",static_cast<void(*)(lua_State*,pragma::doc::Overload&)>([](lua_State *l,pragma::doc::Overload &overload) {
		auto &params = overload.GetParameters();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &param : params)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<pragma::doc::Parameter*>(l,&const_cast<pragma::doc::Parameter&>(param));
			Lua::SetTableValue(l,t);
		}
	}));
	cdefOverload.def("GetParameters",static_cast<void(*)(lua_State*,pragma::doc::Overload&)>([](lua_State *l,pragma::doc::Overload &overload) {
		auto &retVals = overload.GetReturnValues();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &retVal : retVals)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<pragma::doc::Parameter*>(l,&const_cast<pragma::doc::Parameter&>(retVal));
			Lua::SetTableValue(l,t);
		}
	}));
	docLib[cdefOverload];
	
	auto cdefFunction = luabind::class_<pragma::doc::Function>("Function");
	cdefFunction.add_static_constant("TYPE_FUNCTION",umath::to_integral(pragma::doc::Function::Type::Function));
	cdefFunction.add_static_constant("TYPE_METHOD",umath::to_integral(pragma::doc::Function::Type::Method));
	cdefFunction.add_static_constant("TYPE_HOOK",umath::to_integral(pragma::doc::Function::Type::Hook));

	cdefFunction.add_static_constant("FLAG_NONE",umath::to_integral(pragma::doc::Function::Flags::None));
	cdefFunction.add_static_constant("FLAG_BIT_DEBUG",umath::to_integral(pragma::doc::Function::Flags::Debug));
	cdefFunction.add_static_constant("FLAG_VANILLA",umath::to_integral(pragma::doc::Function::Flags::Vanilla));
	cdefFunction.add_static_constant("FLAG_DEPRECATED",umath::to_integral(pragma::doc::Function::Flags::Deprecated));
	cdefFunction.def("GetName",static_cast<void(*)(lua_State*,pragma::doc::Function&)>([](lua_State *l,pragma::doc::Function &fc) {
		Lua::PushString(l,fc.GetName());
	}));
	cdefFunction.def("GetFullName",static_cast<void(*)(lua_State*,pragma::doc::Function&)>([](lua_State *l,pragma::doc::Function &fc) {
		Lua::PushString(l,fc.GetFullName());
	}));
	cdefFunction.def("GetDescription",static_cast<void(*)(lua_State*,pragma::doc::Function&)>([](lua_State *l,pragma::doc::Function &fc) {
		Lua::PushString(l,fc.GetDescription());
	}));
	cdefFunction.def("GetURL",static_cast<void(*)(lua_State*,pragma::doc::Function&)>([](lua_State *l,pragma::doc::Function &fc) {
		Lua::PushString(l,fc.GetURL());
	}));
	cdefFunction.def("GetType",static_cast<void(*)(lua_State*,pragma::doc::Function&)>([](lua_State *l,pragma::doc::Function &fc) {
		Lua::PushInt(l,umath::to_integral(fc.GetType()));
	}));
	cdefFunction.def("GetFlags",static_cast<void(*)(lua_State*,pragma::doc::Function&)>([](lua_State *l,pragma::doc::Function &fc) {
		Lua::PushInt(l,umath::to_integral(fc.GetFlags()));
	}));
	cdefFunction.def("GetGameStateFlags",static_cast<void(*)(lua_State*,pragma::doc::Function&)>([](lua_State *l,pragma::doc::Function &fc) {
		Lua::PushInt(l,umath::to_integral(fc.GetGameStateFlags()));
	}));
	cdefFunction.def("GetExampleCode",static_cast<void(*)(lua_State*,pragma::doc::Function&)>([](lua_State *l,pragma::doc::Function &fc) {
		auto &exampleCode = fc.GetExampleCode();
		if(exampleCode.has_value() == false)
			return;
		Lua::Push<pragma::doc::Function::ExampleCode*>(l,&const_cast<pragma::doc::Function::ExampleCode&>(*exampleCode));
	}));
	cdefFunction.def("GetOverloads",static_cast<void(*)(lua_State*,pragma::doc::Function&)>([](lua_State *l,pragma::doc::Function &fc) {
		auto &overloads = fc.GetOverloads();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &overload : overloads)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<pragma::doc::Overload*>(l,&const_cast<pragma::doc::Overload&>(overload));
			Lua::SetTableValue(l,t);
		}
	}));
	cdefFunction.def("GetRelated",static_cast<void(*)(lua_State*,pragma::doc::Function&)>([](lua_State *l,pragma::doc::Function &fc) {
		auto &related = fc.GetRelated();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &r : related)
		{
			Lua::PushInt(l,idx++);
			Lua::PushString(l,r);
			Lua::SetTableValue(l,t);
		}
	}));
	cdefFunction.def("GetGroups",static_cast<void(*)(lua_State*,pragma::doc::Function&)>([](lua_State *l,pragma::doc::Function &fc) {
		auto &groups = fc.GetGroups();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &group : groups)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<pragma::doc::Group*>(l,&const_cast<pragma::doc::Group&>(group));
			Lua::SetTableValue(l,t);
		}
	}));

	auto cdefExCode = luabind::class_<pragma::doc::Function::ExampleCode>("ExampleCode");
	cdefExCode.def_readwrite("description",&pragma::doc::Function::ExampleCode::description);
	cdefExCode.def_readwrite("code",&pragma::doc::Function::ExampleCode::code);
	cdefFunction.scope[cdefExCode];
	docLib[cdefFunction];

	auto cdefMember = luabind::class_<pragma::doc::Member>("Member");
	cdefMember.add_static_constant("MODE_NONE",umath::to_integral(pragma::doc::Member::Mode::None));
	cdefMember.add_static_constant("MODE_READ",umath::to_integral(pragma::doc::Member::Mode::Read));
	cdefMember.add_static_constant("MODE_WRITE",umath::to_integral(pragma::doc::Member::Mode::Write));
	cdefMember.def("GetName",static_cast<void(*)(lua_State*,pragma::doc::Member&)>([](lua_State *l,pragma::doc::Member &member) {
		Lua::PushString(l,member.GetName());
	}));
	cdefMember.def("GetFullName",static_cast<void(*)(lua_State*,pragma::doc::Member&)>([](lua_State *l,pragma::doc::Member &member) {
		Lua::PushString(l,member.GetFullName());
	}));
	cdefMember.def("GetType",static_cast<pragma::doc::Variant*(*)(lua_State*,pragma::doc::Member&)>([](lua_State *l,pragma::doc::Member &member) {
		return &const_cast<pragma::doc::Variant&>(member.GetType());
	}));
	cdefMember.def("GetWikiURL",static_cast<void(*)(lua_State*,pragma::doc::Member&)>([](lua_State *l,pragma::doc::Member &member) {
		Lua::PushString(l,member.GetWikiURL());
	}));
	cdefMember.def("GetDescription",static_cast<void(*)(lua_State*,pragma::doc::Member&)>([](lua_State *l,pragma::doc::Member &member) {
		Lua::PushString(l,member.GetDescription());
	}));
	cdefMember.def("GetGameStateFlags",static_cast<void(*)(lua_State*,pragma::doc::Member&)>([](lua_State *l,pragma::doc::Member &member) {
		Lua::PushInt(l,umath::to_integral(member.GetGameStateFlags()));
	}));
	cdefMember.def("GetMode",static_cast<void(*)(lua_State*,pragma::doc::Member&)>([](lua_State *l,pragma::doc::Member &member) {
		Lua::PushInt(l,umath::to_integral(member.GetMode()));
	}));
	cdefMember.def("GetDefault",static_cast<void(*)(lua_State*,pragma::doc::Member&)>([](lua_State *l,pragma::doc::Member &member) {
		auto &def = member.GetDefault();
		if(def.has_value() == false)
			return;
		Lua::PushString(l,*def);
	}));
	docLib[cdefMember];

	auto cdefEnum = luabind::class_<pragma::doc::Enum>("Enum");
	cdefEnum.add_static_constant("TYPE_REGULAR",umath::to_integral(pragma::doc::Enum::Type::Regular));
	cdefEnum.add_static_constant("TYPE_BIT",umath::to_integral(pragma::doc::Enum::Type::Bit));
	cdefEnum.def("GetName",static_cast<void(*)(lua_State*,pragma::doc::Enum&)>([](lua_State *l,pragma::doc::Enum &e) {
		Lua::PushString(l,e.GetName());
	}));
	cdefEnum.def("GetFullName",static_cast<void(*)(lua_State*,pragma::doc::Enum&)>([](lua_State *l,pragma::doc::Enum &e) {
		Lua::PushString(l,e.GetFullName());
	}));
	cdefEnum.def("GetValue",static_cast<void(*)(lua_State*,pragma::doc::Enum&)>([](lua_State *l,pragma::doc::Enum &e) {
		Lua::PushString(l,e.GetValue());
	}));
	cdefEnum.def("GetDescription",static_cast<void(*)(lua_State*,pragma::doc::Enum&)>([](lua_State *l,pragma::doc::Enum &e) {
		Lua::PushString(l,e.GetDescription());
	}));
	cdefEnum.def("GetWikiURL",static_cast<void(*)(lua_State*,pragma::doc::Enum&)>([](lua_State *l,pragma::doc::Enum &e) {
		Lua::PushString(l,e.GetWikiURL());
	}));
	cdefEnum.def("GetType",static_cast<void(*)(lua_State*,pragma::doc::Enum&)>([](lua_State *l,pragma::doc::Enum &e) {
		Lua::PushInt(l,umath::to_integral(e.GetType()));
	}));
	cdefEnum.def("GetGameStateFlags",static_cast<void(*)(lua_State*,pragma::doc::Enum&)>([](lua_State *l,pragma::doc::Enum &e) {
		Lua::PushInt(l,umath::to_integral(e.GetGameStateFlags()));
	}));
	docLib[cdefEnum];

	auto cdefEnumSet = luabind::class_<pragma::doc::EnumSet>("EnumSet");
	cdefEnumSet.def("GetEnums",static_cast<void(*)(lua_State*,pragma::doc::EnumSet&)>([](lua_State *l,pragma::doc::EnumSet &eSet) {
		auto &enums = eSet.GetEnums();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &e : enums)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<pragma::doc::Enum*>(l,&const_cast<pragma::doc::Enum&>(e));
			Lua::SetTableValue(l,t);
		}
	}));
	cdefEnumSet.def("GetName",static_cast<void(*)(lua_State*,pragma::doc::EnumSet&)>([](lua_State *l,pragma::doc::EnumSet &eSet) {
		Lua::PushString(l,eSet.GetName());
	}));
	cdefEnumSet.def("GetFullName",static_cast<void(*)(lua_State*,pragma::doc::EnumSet&)>([](lua_State *l,pragma::doc::EnumSet &eSet) {
		Lua::PushString(l,eSet.GetFullName());
	}));
	cdefEnumSet.def("GetUnderlyingType",static_cast<void(*)(lua_State*,pragma::doc::EnumSet&)>([](lua_State *l,pragma::doc::EnumSet &eSet) {
		Lua::PushString(l,eSet.GetUnderlyingType());
	}));
	cdefEnumSet.def("GetWikiURL",static_cast<void(*)(lua_State*,pragma::doc::EnumSet&)>([](lua_State *l,pragma::doc::EnumSet &eSet) {
		Lua::PushString(l,eSet.GetWikiURL());
	}));
	docLib[cdefEnumSet];

	auto cdefModule = luabind::class_<pragma::doc::Module>("Module");
	cdefModule.def("GetWikiURL",static_cast<void(*)(lua_State*,pragma::doc::Module&)>([](lua_State *l,pragma::doc::Module &mod) {
		Lua::PushString(l,mod.GetName());
	}));
	cdefModule.def("GetTarget",static_cast<void(*)(lua_State*,pragma::doc::Module&)>([](lua_State *l,pragma::doc::Module &mod) {
		Lua::PushString(l,mod.GetTarget());
	}));
	docLib[cdefModule];

	auto cdefDerivedFrom = luabind::class_<pragma::doc::DerivedFrom>("DerivedFrom");
	cdefDerivedFrom.def("GetName",static_cast<void(*)(lua_State*,pragma::doc::DerivedFrom&)>([](lua_State *l,pragma::doc::DerivedFrom &df) {
		Lua::PushString(l,df.GetName());
	}));
	cdefDerivedFrom.def("GetParent",static_cast<void(*)(lua_State*,pragma::doc::DerivedFrom&)>([](lua_State *l,pragma::doc::DerivedFrom &df) {
		auto &parent = df.GetParent();
		if(parent == nullptr)
			return;
		Lua::Push<pragma::doc::DerivedFrom*>(l,parent.get());
	}));
	docLib[cdefDerivedFrom];

	auto cdefCollection = luabind::class_<pragma::doc::Collection>("Collection");
	cdefCollection.add_static_constant("FLAG_NONE",umath::to_integral(pragma::doc::Collection::Flags::None));
	cdefCollection.add_static_constant("FLAG_BIT_BASE",umath::to_integral(pragma::doc::Collection::Flags::Base));
	cdefCollection.add_static_constant("FLAG_BIT_LIBRARY",umath::to_integral(pragma::doc::Collection::Flags::Library));
	cdefCollection.add_static_constant("FLAG_BIT_CLASS",umath::to_integral(pragma::doc::Collection::Flags::Class));
	cdefCollection.def("GetFunctions",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		auto &fcs = collection.GetFunctions();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &f : fcs)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<pragma::doc::Function*>(l,&const_cast<pragma::doc::Function&>(f));
			Lua::SetTableValue(l,t);
		}
	}));
	cdefCollection.def("GetMembers",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		auto &members = collection.GetMembers();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &m : members)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<pragma::doc::Member*>(l,&const_cast<pragma::doc::Member&>(m));
			Lua::SetTableValue(l,t);
		}
	}));
	cdefCollection.def("GetEnumSets",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		auto &eSets = collection.GetEnumSets();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &eSet : eSets)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<pragma::doc::EnumSet*>(l,&const_cast<pragma::doc::EnumSet&>(*eSet));
			Lua::SetTableValue(l,t);
		}
	}));
	cdefCollection.def("GetRelated",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		auto &related = collection.GetRelated();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &r : related)
		{
			Lua::PushInt(l,idx++);
			Lua::PushString(l,r);
			Lua::SetTableValue(l,t);
		}
	}));
	cdefCollection.def("GetChildren",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		auto &children = collection.GetChildren();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &child : children)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<pragma::doc::Collection*>(l,&const_cast<pragma::doc::Collection&>(*child));
			Lua::SetTableValue(l,t);
		}
	}));
	cdefCollection.def("GetDerivedFrom",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		auto &derivedFrom = collection.GetDerivedFrom();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &df : derivedFrom)
		{
			Lua::PushInt(l,idx++);
			Lua::Push(l,const_cast<std::shared_ptr<pragma::doc::DerivedFrom>&>(df));
			Lua::SetTableValue(l,t);
		}
	}));
	cdefCollection.def("GetName",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		Lua::PushString(l,collection.GetName());
	}));
	cdefCollection.def("GetFullName",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		Lua::PushString(l,collection.GetFullName());
	}));
	cdefCollection.def("GetDescription",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		Lua::PushString(l,collection.GetDescription());
	}));
	cdefCollection.def("GetURL",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		Lua::PushString(l,collection.GetURL());
	}));
	cdefCollection.def("GetWikiURL",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		Lua::PushString(l,collection.GetWikiURL());
	}));
	cdefCollection.def("GetFlags",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		Lua::PushInt(l,umath::to_integral(collection.GetFlags()));
	}));
	cdefCollection.def("GetParent",static_cast<void(*)(lua_State*,pragma::doc::Collection&)>([](lua_State *l,pragma::doc::Collection &collection) {
		auto *parent = collection.GetParent();
		if(parent == nullptr)
			return;
		Lua::Push<pragma::doc::Collection*>(l,const_cast<pragma::doc::Collection*>(parent));
	}));
	docLib[cdefCollection];
}

#else
namespace Lua::doc
{
	void register_library(Lua::Interface &lua) {}
};
#endif
