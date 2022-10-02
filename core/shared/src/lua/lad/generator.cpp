/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */
#ifdef _MSC_VER
#include "stdafx_shared.h"
#include "pragma/lua/lad/lad.hpp"
#include "pragma/lua/lad/generator.hpp"
#include "pragma/lua/lad/symbol_handler.hpp"
#include "pragma/lua/lad/pdb_manager.hpp"
#include "pragma/lua/lad/type_name_manager.hpp"
#include "pragma/lua/util.hpp"
#include <util_pragma_doc.hpp>
#include <span>
#include <Psapi.h>
#include <luabind/class_info.hpp>
#include <sharedutils/magic_enum.hpp>

#pragma comment(lib,"Psapi.lib")

using namespace pragma::lua;

namespace luabind {

	LUABIND_API class_info get_class_info(argument const& o);
	detail::function_object* get_function_object(object const& fn) {
		lua_State * L = fn.interpreter();
		{
			fn.push(L);
			detail::stack_pop pop(L, 1);
			if(!detail::is_luabind_function(L, -1)) {
				return NULL;
			}
		}
		return *touserdata<detail::function_object*>(std::get<1>(getupvalue(fn, 1)));
	}

	std::string get_function_name(object const& fn) {
		detail::function_object * f = get_function_object(fn);
		if(!f) {
			return "";
		}
		return f->name;
	}
};

// We have to access some members of luabind::detail::class_rep which are inaccessable,
// so we'll force them to be accessable.
class Luaclass_rep
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
static_assert(sizeof(Luaclass_rep) == sizeof(luabind::detail::class_rep));
static Luaclass_rep &access_class_rep(luabind::detail::class_rep &rep) {return reinterpret_cast<Luaclass_rep&>(rep);}

static std::string_view lcp(const std::string_view &str0,const std::string_view &str1)
{
	size_t len = 0;
	size_t i = 0;
	while(i < str0.length() && i < str1.length())
	{
		if(str0[i] != str1[i])
		{
			++i;
			break;
		}
		++len;
		++i;
	}
	return std::string_view{str0.data(),len};
}

static std::vector<std::string> lcp(std::vector<std::string> &strings,std::string &outPrefix)
{
	outPrefix = "";
	if(strings.empty())
		return {};
	std::string subStr {};
	uint32_t n = 0;
	for(auto i=decltype(strings.size()){0u};i<strings.size();++i)
	{
		for(auto j=i +1;j<strings.size();++j)
		{
			auto l = lcp(strings[i],strings[j]);

			auto p = l.rfind("_ON_");
			if(p != std::string::npos)
				l = l.substr(0,p);
			else
			{
				p = l.rfind('_');
				if(p != std::string::npos)
					l = l.substr(0,p);
			}
			if(l.length() > subStr.length())
				subStr = l;
		}
	}

	if(subStr.empty())
	{
		auto b = std::move(strings.back());
		strings.pop_back();
		outPrefix = b;
		return {std::move(b)};
	}

	std::vector<std::string> list {};
	list.reserve(strings.size());
	for(auto it=strings.begin();it!=strings.end();)
	{
		auto &str = *it;
		if(ustring::compare(str.c_str(),subStr.data(),true,subStr.length()) == false)
		{
			++it;
			continue;
		}
		list.push_back(std::move(str));
		it = strings.erase(it);
	}
	outPrefix = subStr;
	return list;
}

static std::unordered_map<std::string,std::vector<std::string>> group_by_prefix(std::vector<std::string> &strings)
{
	std::unordered_map<std::string,std::vector<std::string>> groupedByPrefix;
	while(!strings.empty())
	{
		std::string prefix;
		auto list = lcp(strings,prefix);
		groupedByPrefix[prefix] = std::move(list);
	}
	return groupedByPrefix;
}

////////////////////////////////

LuaDocGenerator::LuaDocGenerator(lua_State *l)
	: m_luaState{l}
{
	InitializeRepositoryUrls();
}

std::optional<std::string> LuaDocGenerator::SourceToUrl(const pragma::doc::Source &source)
{
	auto &moduleName = source.moduleName;
	auto it = m_moduleToRepositoryMap.find(moduleName);
	if(it != m_moduleToRepositoryMap.end())
	{
		auto &repoName = it->second;
		auto url = repoName +source.fileName +"#L" +std::to_string(source.line);
		return url;
	}
	else
		Con::cwar<<"WARNING: No repository specified for module '"<<moduleName<<"'!"<<Con::endl;
	return {};
}

std::optional<std::string> LuaDocGenerator::GetCommitId(const std::string &repositoryUrl)
{
	std::string output;
	if(!util::start_and_wait_for_command(("git ls-remote " +repositoryUrl +" HEAD").c_str(),nullptr,nullptr,&output))
		return {};
	ustring::remove_whitespace(output);
	if(output.empty())
		return {};
	auto t = output.find('\t');
	if(t == std::string::npos)
		return {};
	return output.substr(0,t);
}

std::string LuaDocGenerator::BuildRepositoryUrl(const std::string &baseUrl,const std::string &commitUrl)
{
	auto commitId = GetCommitId(baseUrl);
	if(!commitId.has_value())
		commitId = "master";
	auto url = commitUrl;
	ustring::replace(url,"%commitid%",*commitId);
	return baseUrl +url;
}

void LuaDocGenerator::InitializeRepositoryUrls()
{
	m_moduleToRepositoryMap = {
		{"shared",BuildRepositoryUrl("https://github.com/Silverlan/pragma/","blob/%commitid%/core/shared/")},
		{"server",BuildRepositoryUrl("https://github.com/Silverlan/pragma/","blob/%commitid%/core/server/")},
		{"client",BuildRepositoryUrl("https://github.com/Silverlan/pragma/","blob/%commitid%/core/client/")},
		{"luabind",BuildRepositoryUrl("https://github.com/Silverlan/luabind-deboostified/","blob/%commitid%/")},
		{"pr_steamworks",BuildRepositoryUrl("https://github.com/Silverlan/pr_steamworks/","blob/%commitid%/")},
		{"mathutil",BuildRepositoryUrl("https://github.com/Silverlan/mathutil/","blob/%commitid%/")},
		{"sharedutils",BuildRepositoryUrl("https://github.com/Silverlan/sharedutils/","blob/%commitid%/")},
		{"util_udm",BuildRepositoryUrl("https://github.com/Silverlan/util_udm/","blob/%commitid%/")},
		{"glm",BuildRepositoryUrl("https://github.com/g-truc/glm/","blob/%commitid%/")},
		{"prosper",BuildRepositoryUrl("https://github.com/Silverlan/prosper/","blob/%commitid%/")},
		{"vfilesystem",BuildRepositoryUrl("https://github.com/Silverlan/vfilesystem/","blob/%commitid%/")},
		{"alsoundsystem",BuildRepositoryUrl("https://github.com/Silverlan/alsoundsystem/","blob/%commitid%/")},
		{"datasystem",BuildRepositoryUrl("https://github.com/Silverlan/datasystem/","blob/%commitid%/")},
		{"iglfw",BuildRepositoryUrl("https://github.com/Silverlan/iglfw/","blob/%commitid%/")},
		{"materialsystem",BuildRepositoryUrl("https://github.com/Silverlan/materialsystem/","blob/%commitid%/")},
		{"cmaterialsystem",BuildRepositoryUrl("https://github.com/Silverlan/materialsystem/","blob/%commitid%/")},
		{"util_image",BuildRepositoryUrl("https://github.com/Silverlan/util_image/","blob/%commitid%/")},
		{"util_pragma_doc",BuildRepositoryUrl("https://github.com/Silverlan/util_pragma_doc/","blob/%commitid%/")},
		{"util_sound",BuildRepositoryUrl("https://github.com/Silverlan/util_sound/","blob/%commitid%/")},
		{"util_udm",BuildRepositoryUrl("https://github.com/Silverlan/util_udm/","blob/%commitid%/")},
		{"wgui",BuildRepositoryUrl("https://github.com/Silverlan/wgui/","blob/%commitid%/")},
		{"pr_dmx",BuildRepositoryUrl("https://github.com/Silverlan/pr_dmx/","blob/%commitid%/")},
		{"pr_cycles",BuildRepositoryUrl("https://github.com/Silverlan/pr_cycles/","blob/%commitid%/")},
		{"pr_openvr",BuildRepositoryUrl("https://github.com/Silverlan/pr_openvr/","blob/%commitid%/")},
		{"pr_steamworks",BuildRepositoryUrl("https://github.com/Silverlan/pr_steamworks/","blob/%commitid%/")},
		{"panima",BuildRepositoryUrl("https://github.com/Silverlan/panima/","blob/%commitid%/")}
	};
}

void LuaDocGenerator::IterateLibraries(
	luabind::object o,const std::string &path,pragma::doc::Collection &collection,
	pragma::doc::Collection &colTarget
)
{
	auto &traversed = m_traversed;
	auto &iteratedClasses = m_iteratedClasses;
	auto &classRegs = m_classRegs;
	auto *l = o.interpreter();
	if(std::find(traversed.begin(),traversed.end(),o) != traversed.end())
		return; // Prevent infinite recursion
	LuaClassInfo classInfo {};
	traversed.push_back(o);
	for(luabind::iterator it{o},end;it!=end;++it)
	{
		auto val = *it;
		auto type = luabind::type(val);
		if(type == LUA_TTABLE)
		{
			auto libName = luabind::object_cast_nothrow<std::string>(it.key(),std::string{});
			if(path == "package" && libName == "loaded")
				continue;
			std::cout<<"LIB: "<<libName<<std::endl;
			if(libName.empty())
				std::cout<<"";
			else
			{
				auto subCollection = pragma::doc::Collection::Create();
				subCollection->SetName(libName);
				auto subPath = path;
				if(!subPath.empty())
					subPath += '.';
				subPath += libName;
				collection.AddChild(subCollection);
				subCollection->SetFlags(subCollection->GetFlags() | pragma::doc::Collection::Flags::Library | pragma::doc::Collection::Flags::AutoGenerated);
				IterateLibraries(val,subPath,*subCollection,*subCollection);
			}
		}
		else if(type == LUA_TFUNCTION)
		{
			auto fcName = luabind::object_cast_nothrow<std::string>(it.key(),std::string{});
			luabind::detail::function_object * fobj = luabind::get_function_object(val);
			if(fobj)
			{
				std::cout<<"FUN: "<<fcName<<std::endl;
				ParseLuaProperty(fcName,val,classInfo,false);
			}
		}
		else if(type == LUA_TUSERDATA)
		{
			auto *crep = Lua::get_crep(val);
			if(crep)
				AddClass(crep,path);
		}
	}
	if(classInfo.classRep)
		m_classRepToCollection[classInfo.classRep] = &colTarget;
	m_collectionClassInfo[&colTarget] = std::move(classInfo);
}

void LuaDocGenerator::PopulateMethods(lua_State *l)
{
	auto &map = GetCollectionToLuaClassInfoMap();
	for(auto &pair : map)
	{
		auto &col = *pair.first;
		auto &classInfo = pair.second;
		auto isMethod = umath::is_flag_set(col.GetFlags(),pragma::doc::Collection::Flags::Class);
		for(auto &method : classInfo.methods)
			AddFunction(l,col,method,isMethod);
	}
}

void LuaDocGenerator::GenerateDocParameters(const luabind::detail::TypeInfo &input,std::vector<pragma::doc::Parameter> &outputs,const std::optional<std::string> &pargName,bool keepTuples)
{
	std::string argName = "unknown"; // TODO
	if(pargName.has_value())
		argName = *pargName;
	auto applyFlags = [&input](pragma::doc::Variant &variant) {
		umath::set_flag(variant.flags,pragma::doc::Variant::Flags::Const,input.isConst);
		umath::set_flag(variant.flags,pragma::doc::Variant::Flags::Pointer,input.isPointer);
		umath::set_flag(variant.flags,pragma::doc::Variant::Flags::Reference,input.isReference);
		umath::set_flag(variant.flags,pragma::doc::Variant::Flags::SmartPtr,input.isSmartPtr);
		umath::set_flag(variant.flags,pragma::doc::Variant::Flags::Enum,input.isEnum);
		umath::set_flag(variant.flags,pragma::doc::Variant::Flags::Optional,input.isOptional);
	};

	if(input.typeIdentifier.has_value())
	{
		auto createUnknown = [&outputs,&argName]() {
			auto param = pragma::doc::Parameter::Create(argName);
			auto &variant = param.GetType();
			variant.name = "unknown";
			outputs.push_back(std::move(param));
		};
		if(input.templateTypes.empty())
		{
			Con::cwar<<"WARNING: Got template type without template parameters!"<<Con::endl;
			createUnknown();
		}
		else
		{
			std::vector<pragma::doc::Parameter> subParams;
			for(auto &tempType : input.templateTypes)
				GenerateDocParameters(tempType,subParams);
			auto checkParamCount = [&subParams,&createUnknown](uint32_t expected,bool atLeast=false) -> bool {
				if((!atLeast && subParams.size() != expected) || (atLeast && subParams.size() < expected))
				{
					Con::cwar<<"WARNING: Unexpected number of template parameters for template type!"<<Con::endl;
					createUnknown();
					return false;
				}
				return true;
			};
			
			auto createTuple = [&argName,&subParams,&outputs]() {
				auto param = pragma::doc::Parameter::Create(argName);
				auto &variant = param.GetType();
				variant.name = "tuple";
				variant.typeParameters.reserve(subParams.size());
				for(auto &subParam : subParams)
					variant.typeParameters.push_back(std::move(subParam.GetType()));
				outputs.push_back(std::move(param));
			};

			if(input.typeIdentifier == "shared_ptr" || input.typeIdentifier == "unique_ptr")
			{
				if(checkParamCount(1))
				{
					auto param = std::move(subParams.front());
					param.SetName(argName);
					auto &variant = param.GetType();
					variant.flags |= pragma::doc::Variant::Flags::SmartPtr;
					outputs.push_back(std::move(param));
				}
			}
			else if(input.typeIdentifier == "typehint")
			{
				if(checkParamCount(1))
				{
					auto param = std::move(subParams.front());
					param.SetName(argName);
					outputs.push_back(std::move(param));
				}
			}
			else if(input.typeIdentifier == "vector" || input.typeIdentifier == "array" || input.typeIdentifier == "tableT")
			{
				uint32_t numExpected = 1;
				if(checkParamCount(numExpected,true))
				{
					auto param = pragma::doc::Parameter::Create(argName);
					auto &variant = param.GetType();
					variant.name = "table";

					variant.typeParameters.push_back(std::move(subParams.front().GetType()));
					outputs.push_back(std::move(param));
				}
			}
			else if(input.typeIdentifier == "tableTT")
			{
				if(checkParamCount(2))
				{
					auto param = pragma::doc::Parameter::Create(argName);
					auto &variant = param.GetType();
					variant.name = "table";

					variant.typeParameters.push_back(std::move(subParams[0].GetType()));
					variant.typeParameters.push_back(std::move(subParams[1].GetType()));
					outputs.push_back(std::move(param));
				}
			}
			else if(input.typeIdentifier == "map")
			{
				if(checkParamCount(2,true))
				{
					auto param = pragma::doc::Parameter::Create(argName);
					auto &variant = param.GetType();
					variant.name = "map";

					variant.typeParameters.push_back(std::move(subParams[0].GetType()));
					variant.typeParameters.push_back(std::move(subParams[1].GetType()));
					outputs.push_back(std::move(param));
				}
			}
			else if(input.typeIdentifier == "pair")
			{
				if(checkParamCount(2))
				{
					if(keepTuples == false)
					{
						auto param0 = std::move(subParams[0]);
						auto param1 = std::move(subParams[1]);
						param0.SetName(argName +"_1");
						param1.SetName(argName +"_2");
						outputs.push_back(std::move(param0));
						outputs.push_back(std::move(param1));
					}
					else
						createTuple();
				}
			}
			else if(input.typeIdentifier == "tuple" || input.typeIdentifier == "mult")
			{
				if(checkParamCount(1,true))
				{
					if(keepTuples == false)
					{
						outputs.reserve(outputs.size() +subParams.size());
						uint32_t idx = 0;
						for(auto &param : subParams)
						{
							auto tmp = std::move(param);
							tmp.SetName(argName +"_" +std::to_string(++idx));
							outputs.push_back(std::move(tmp));
						}
					}
					else
						createTuple();
				}
			}
			else if(input.typeIdentifier == "optional")
			{
				if(checkParamCount(1,true))
				{
					outputs.reserve(outputs.size() +subParams.size());
					uint32_t idx = 0;
					auto multiple = subParams.size() > 1;
					for(auto &param : subParams)
					{
						auto tmpName = argName;
						if(multiple)
							tmpName += "_" +std::to_string(++idx);
						outputs.push_back(param);
						outputs.back().SetName(tmpName);
						outputs.back().GetType().flags |= pragma::doc::Variant::Flags::Optional;
					}
				}
			}
			else if(input.typeIdentifier == "userData" || input.typeIdentifier == "classObject")
			{
				auto param = pragma::doc::Parameter::Create(argName);
				auto &variant = param.GetType();
				variant.name = *input.typeIdentifier;
				outputs.push_back(std::move(param));
			}
			else if(input.typeIdentifier == "functype")
			{
				createUnknown();
				/*auto param = pragma::doc::Parameter::Create(argName);
				auto &variant = param.GetType();
				variant.name = "function";

				
				variant.typeParameters.push_back(std::move(subParams[0].GetType()));
				variant.typeParameters.push_back(std::move(subParams[1].GetType()));*/
				// TODO
			}
			else if(input.typeIdentifier == "variant")
			{
				if(checkParamCount(1,true))
				{
					auto param = pragma::doc::Parameter::Create(argName);
					auto &variant = param.GetType();
					variant.name = "variant";

					variant.typeParameters.reserve(subParams.size());
					for(auto &subParam : subParams)
						variant.typeParameters.push_back(std::move(subParam.GetType()));
					outputs.push_back(std::move(param));
				}
			}
			else if(input.typeIdentifier == "variadic")
			{
				if(checkParamCount(1,true))
				{
					auto param = pragma::doc::Parameter::Create(argName);
					auto &variant = param.GetType();
					variant.name = "variadic";

					variant.typeParameters.reserve(subParams.size());
					for(auto &subParam : subParams)
						variant.typeParameters.push_back(std::move(subParam.GetType()));
					outputs.push_back(std::move(param));
				}
			}
			else
				Con::cwar<<"WARNING: Unknown template type!"<<Con::endl;
		}
		return;
	}

	// Generic type
	auto param = pragma::doc::Parameter::Create(argName);

	auto createFundamentalVariant = [&applyFlags,&param](const char *name) {
		auto &variant = param.GetType();
		variant.name = name;
		applyFlags(variant);
	};
    if(input.typeInfo == &typeid(std::int8_t) || input.type == luabind::detail::TypeInfo::FundamentalType::Char)
		createFundamentalVariant("int8");
    else if(input.typeInfo == &typeid(std::uint8_t) || input.type == luabind::detail::TypeInfo::FundamentalType::UChar)
		createFundamentalVariant("uint8");
    else if(input.typeInfo == &typeid(std::int16_t) || input.type == luabind::detail::TypeInfo::FundamentalType::Short)
		createFundamentalVariant("int16");
    else if(input.typeInfo == &typeid(std::uint16_t) || input.type == luabind::detail::TypeInfo::FundamentalType::UShort)
		createFundamentalVariant("uint16");
    else if(input.typeInfo == &typeid(std::int32_t) || input.type == luabind::detail::TypeInfo::FundamentalType::Int)
		createFundamentalVariant("int32");
    else if(input.typeInfo == &typeid(std::uint32_t) || input.type == luabind::detail::TypeInfo::FundamentalType::UInt)
		createFundamentalVariant("uint32");
    else if(input.typeInfo == &typeid(std::int64_t) || input.type == luabind::detail::TypeInfo::FundamentalType::Long)
        createFundamentalVariant("int64");
    else if(input.typeInfo == &typeid(std::uint64_t) || input.type == luabind::detail::TypeInfo::FundamentalType::ULong)
        createFundamentalVariant("uint64");
	else if(input.typeInfo == &typeid(void) || input.type == luabind::detail::TypeInfo::FundamentalType::Void)
		createFundamentalVariant("nil");
	else if(input.typeInfo == &typeid(bool) || input.type == luabind::detail::TypeInfo::FundamentalType::Bool)
		createFundamentalVariant("bool");
	else if(input.typeInfo == &typeid(float) || input.typeInfo == &typeid(double) || input.type == luabind::detail::TypeInfo::FundamentalType::Float)
		createFundamentalVariant("float");
	else if(input.typeInfo == &typeid(std::string) || input.type == luabind::detail::TypeInfo::FundamentalType::String)
		createFundamentalVariant("string");
	else if(input.typeInfo == &typeid(luabind::object) || input.type == luabind::detail::TypeInfo::FundamentalType::LuabindObject)
		createFundamentalVariant("any");
	else if(input.typeInfo == &typeid(luabind::argument) || input.type == luabind::detail::TypeInfo::FundamentalType::LuabindArgument)
		createFundamentalVariant("any");
	else if(input.type == luabind::detail::TypeInfo::FundamentalType::LuaState)
		createFundamentalVariant("lua_State");
	else if(!input.crep)
	{
		std::string typeName;
		if(input.typeInfo)
		{
			param.GetType().name = input.typeInfo->name();
			typeName = input.typeInfo->name();
		}
		else
			typeName = magic_enum::enum_name(input.type);
		param.GetType().flags |= pragma::doc::Variant::Flags::UnknownType;
		applyFlags(param.GetType());
		Con::cwar<<"WARNING: Unknown type '"<<typeName<<"'!"<<Con::endl;
	}
	else
	{
		auto it = m_classRepToCollection.find(input.crep);
		if(it != m_classRepToCollection.end())
		{
			auto *typeCol = it->second;
			auto &variant = param.GetType();
			variant.name = typeCol->GetFullName();
			const char *prefix = "root.";
			if(ustring::compare(variant.name.c_str(),prefix,true,strlen(prefix)))
				variant.name = variant.name.substr(strlen(prefix));
			applyFlags(variant);
		}
	}
	outputs.push_back(param);
}

void LuaDocGenerator::AddFunction(lua_State *l,pragma::doc::Collection &collection,const LuaMethodInfo &method,bool isMethod)
{
	auto fc = pragma::doc::Function::Create(collection,method.name);
	fc.SetFlags(pragma::doc::Function::Flags::AutoGenerated);
	for(auto &overloadInfo : method.overloads)
	{
		auto overload = pragma::doc::Overload::Create();
		auto &params = overload.GetParameters();
		for(auto argIdx=decltype(overloadInfo.parameters.size()){0u};argIdx<overloadInfo.parameters.size();++argIdx)
		{
			std::string argName = "arg" +std::to_string(argIdx);
			if(overloadInfo.namedParameters.has_value())
				argName = (*overloadInfo.namedParameters)[argIdx];
			auto &type = overloadInfo.parameters[argIdx];
			GenerateDocParameters(type,params,argName);
		}

		auto &returnValues = overload.GetReturnValues();
		for(auto argIdx=decltype(overloadInfo.returnValues.size()){0u};argIdx<overloadInfo.returnValues.size();++argIdx)
		{
			std::string argName = "ret" +std::to_string(argIdx);
			auto &type = overloadInfo.returnValues[argIdx];
			GenerateDocParameters(type,returnValues,argName);
		}
		if(overloadInfo.source.has_value())
		{
			overload.SetSource(*overloadInfo.source);
			auto url = SourceToUrl(*overloadInfo.source);
			if(url.has_value())
				overload.SetURL(*url);
		}
		fc.AddOverload(overload);
	}
	auto *en = pragma::get_engine();
	auto *cl = en->GetClientState();
	auto *sv = en->GetServerNetworkState();
	auto *lcl = cl ? cl->GetLuaState() : nullptr;
	auto *lsv = sv ? sv->GetLuaState() : nullptr;
	if(l == lcl)
		fc.SetGameStateFlags(fc.GetGameStateFlags() | pragma::doc::GameStateFlags::Client);
	if(l == lsv)
		fc.SetGameStateFlags(fc.GetGameStateFlags() | pragma::doc::GameStateFlags::Server);
	if(isMethod)
		fc.SetType(pragma::doc::Function::Type::Method);
	collection.AddFunction(fc);
}

pragma::doc::PCollection LuaDocGenerator::ClassInfoToCollection(lua_State *l,const std::string &path,const LuaClassInfo &classInfo)
{
	auto collection = pragma::doc::Collection::Create();
	collection->SetName(classInfo.name);
	collection->SetFlags(pragma::doc::Collection::Flags::Class);

	/*for(auto *base : classInfo.bases)
	{
		// TODO: Base class of base class?
		std::string dfName = base->name();
		if(!path.empty())
			dfName = path +'.' +dfName;
		auto derivedFrom = pragma::doc::DerivedFrom::Create(dfName);
		collection->AddDerivedFrom(*derivedFrom);
	}*/

	for(auto &attr : classInfo.attributes)
	{
		auto member = pragma::doc::Member::Create(*collection,attr);
		member.SetFlags(pragma::doc::Member::Flags::AutoGenerated);
		collection->AddMember(member);
	}

	for(auto &ces : classInfo.enumSets)
	{
		auto es = pragma::doc::EnumSet::Create(ces.first,collection.get());
		es->ReserveEnums(ces.second.size());
		for(auto &pair : ces.second)
		{
			auto e = pragma::doc::Enum::Create(*es);
			e.SetName(pair.first);
			e.SetValue(std::to_string(pair.second));
			if(pair.first.find("_BIT_"))
				e.SetType(pragma::doc::Enum::Type::Bit);
			else
				e.SetType(pragma::doc::Enum::Type::Regular);
			es->AddEnum(std::move(e));
		}
		es->SetFlags(pragma::doc::EnumSet::Flags::AutoGenerated);
		collection->AddEnumSet(es);
	}
	m_collectionClassInfo[collection.get()] = classInfo;
	if(classInfo.classRep)
		m_classRepToCollection[classInfo.classRep] = collection.get();
	return collection;
}

void LuaDocGenerator::IterateLibraries(luabind::object o,pragma::doc::Collection &collection,pragma::doc::Collection &colTarget)
{
	IterateLibraries(o,"",collection,colTarget);

	struct ClassRegTreeItem
	{
		ClassRegInfo *regInfo = nullptr;
		std::string name;
		std::vector<std::unique_ptr<ClassRegTreeItem>> children;
	};
	std::function<void(ClassRegInfo&,ClassRegTreeItem&,const std::span<std::string>&)> addItem = nullptr;
	addItem = [&addItem](ClassRegInfo &classRegInfo,ClassRegTreeItem &parent,const std::span<std::string> &path) {
		auto it = std::find_if(parent.children.begin(),parent.children.end(),[&path](const std::unique_ptr<ClassRegTreeItem> &child) {
			return child->name == path.front();
		});
		if(it == parent.children.end())
		{
			parent.children.push_back(std::make_unique<ClassRegTreeItem>());
			auto &child = parent.children.back();
			child->name = path.front();
			it = parent.children.end() -1;
		}
		if(path.size() == 1)
		{
			(*it)->regInfo = &classRegInfo;
			return;
		}
		addItem(classRegInfo,**it,path.subspan(1));
	};
	ClassRegTreeItem root {};
	for(auto &pair : m_classRegs)
	{
		std::vector<std::string> pathComponents;
		ustring::explode(pair.second.path,".",pathComponents);
		if(pathComponents.empty())
			pathComponents.push_back("_G");
		addItem(pair.second,root,pathComponents);
	}
	
	std::function<void(pragma::doc::Collection&,const ClassRegTreeItem&)> classRegToCollection = nullptr;
	classRegToCollection = [this,&classRegToCollection,&collection](pragma::doc::Collection &parentCollection,const ClassRegTreeItem &item) {
		for(auto &child : item.children)
		{
			pragma::doc::PCollection childCollection = nullptr;
			auto newCollection = true;
			if(child->regInfo)
			{
				childCollection = ClassInfoToCollection(m_luaState,child->regInfo->path,child->regInfo->classInfo);
				if(child->regInfo->cppDefined)
					childCollection->SetFlags(childCollection->GetFlags() | pragma::doc::Collection::Flags::DefinedInCpp);
				m_iteratedClasses[child->regInfo->classInfo.classRep] = childCollection.get();
			}
			else
			{
				auto *c = collection.FindChildCollection(child->name);
				if(c)
				{
					newCollection = false;
					childCollection = c->shared_from_this();
				}
				else
				{
					childCollection = pragma::doc::Collection::Create();
					childCollection->SetName(child->name);
					childCollection->SetFlags(pragma::doc::Collection::Flags::Library);
				}
			}
			if(newCollection)
			{
				childCollection->SetFlags(childCollection->GetFlags() | pragma::doc::Collection::Flags::AutoGenerated);
				parentCollection.AddChild(childCollection);
			}

			classRegToCollection(*childCollection,*child);
		}
	};
	classRegToCollection(collection,root);
}

void LuaDocGenerator::AddClass(luabind::detail::class_rep *crep,const std::string &path)
{
	auto *l = m_luaState;
	if(crep->get_class_type() == luabind::detail::class_rep::class_type::lua_class)
	{
		/*auto *l = val.interpreter();

		auto x = luabind::detail::is_class_rep(l, -1);
		auto y = luabind::detail::get_instance(l, -1);

		val.push(l);
		luabind::detail::stack_pop sp(l,1);
		luabind::argument a {luabind::from_stack(l,-1)};
		auto classInfo = luabind::get_class_info(a);
		std::cout<<"";*/
	}
	else
	{
		ClassRegInfo info {};
		info.classInfo = GetClassInfo(crep);
		
		auto subPath = path;
		if(!subPath.empty())
			subPath += '.';
		subPath += info.classInfo.name;
		info.path = subPath;

		auto addNewClass = true;
		auto it = m_classRegs.find(crep);
		if(it != m_classRegs.end())
		{
			// Found class duplicate; Prioritize the one with the lower number of parents
			uint32_t numParentsCur = std::count_if(it->second.path.begin(),it->second.path.end(),[](char c) {return c == '.';});
			if(it->second.path.find('.') == std::string::npos) // Global namespace; Avoid if possible (and choose alternatives instead)
				numParentsCur = std::numeric_limits<uint32_t>::max();
			uint32_t numParentsNew = std::count_if(subPath.begin(),subPath.end(),[](char c) {return c == '.';});
			if(subPath.find('.') == std::string::npos)
				numParentsNew = std::numeric_limits<uint32_t>::max();
			if(numParentsNew < numParentsCur)
			{
				m_classRegs.erase(it);
				addNewClass = true;
			}
			else
				addNewClass = false;
		}

		if(addNewClass)
		{
			m_classRegs[crep] = std::move(info);

			crep->get_default_table(l);
			luabind::object t {luabind::from_stack{l,-1}};
			for(luabind::iterator it{t},end;it!=end;++it)
			{
				auto type = luabind::type(*it);
				if(type == LUA_TUSERDATA)
				{
					auto *crepChild = Lua::get_crep(*it);
					if(crepChild && crepChild != crep)
					{
						auto cpy = it.key();
						AddClass(crepChild,subPath);
					}
				}
			}
			Lua::Pop(l,1);
		}
	}
}

static void normalize_enum_type(std::string &enumType)
{
	std::vector<std::string> subStrings;
	ustring::explode(enumType,"_",subStrings);
	if(subStrings.empty())
		return;
	if(subStrings.back().empty())
		subStrings.pop_back();
	enumType.clear();
	for(auto &subStr : subStrings)
	{
		ustring::to_lower(subStr);
		subStr[0] = std::toupper(subStr[0]);
		enumType += subStr;
	}
}

LuaClassInfo LuaDocGenerator::GetClassInfo(luabind::detail::class_rep *crep)
{
	auto *L = m_luaState;
	crep->get_table(L);
	luabind::object table(luabind::from_stack(L, -1));
	lua_pop(L, 1);

	LuaClassInfo result {};
	result.classRep = crep;
	result.name = crep->name();

	for(auto &baseInfo : crep->bases())
		result.bases.push_back(baseInfo.base);

	for(luabind::iterator i(table), e; i != e; ++i)
	{
		if(luabind::type(*i) != LUA_TFUNCTION)
			continue;

		// We have to create a temporary `object` here, otherwise the proxy
		// returned by operator->() will mess up the stack. This is a known
		// problem that probably doesn't show up in real code very often.
		luabind::object member(*i);
		member.push(L);
		luabind::detail::stack_pop pop(L, 1);
		
		auto attr = i.key();
		attr.push(L);
		if(Lua::IsString(L,-1))
		{
			std::string name = Lua::CheckString(L,-1);
			Lua::Pop(L,1);
			ParseLuaProperty(name,*i,result,true);
		}
	}

	auto &staticConstants = access_class_rep(*crep).m_static_constants;
	if(!staticConstants.empty())
	{
		std::vector<std::string> enums;
		enums.reserve(staticConstants.size());
		for(auto &pair : staticConstants)
			enums.push_back(pair.first);
		auto prefixedEnumSets = group_by_prefix(enums);
		for(auto &pair : prefixedEnumSets)
		{
			auto esName = pair.first;
			normalize_enum_type(esName);
			auto &es = result.enumSets.insert(std::make_pair(esName,LuaClassInfo::EnumSet{})).first->second;
			for(auto &enumName : pair.second)
			{
				auto it = staticConstants.find(enumName.c_str());
				assert(it != staticConstants.end());
				es[enumName] = it->second;
			}
		}
	}
	return result;
}

static std::optional<size_t> find_beginning_of_function_definition(const std::string &contents,uint32_t line)
{
	size_t p = -1;
	while(line > 0)
	{
		p = contents.find('\n',p +1);
		if(p == std::string::npos)
			return {};
		if(--line == 0)
			break;
	}
	if(p == -1)
		p = 0;

    std::string_view view = contents;
    view = view.substr(0,p);
    auto pe = contents.rfind('\n',p -1);
	if(pe == std::string::npos)
		pe = 0;
    auto strLine = contents.substr(pe +1,p -(pe +1));
    ustring::remove_whitespace(strLine);
    if(strLine.back() != '{')
    {
        p = std::string::npos;
        uint32_t depth = 0;
        p = view.find_last_of("{}");
        if(p != std::string::npos && view[p] == '{')
            ++depth;
        do
        {
            if(p == std::string::npos)
                return {};
            if(view[p] == '{')
            {
                view = view.substr(0,p);
                if(depth == 0)
                    return {};
                if(depth == 1)
                    break;
                --depth;
				p = view.find_last_of("{}");
                continue;
            }
            if(view[p] == '}')
                ++depth;
            view = view.substr(0,p);
            p = view.find_last_of("{}");
        }
        while(true);
    }
	p = view.find_last_of(")");
	if(p == std::string::npos)
		return {};
	return p;
}

void LuaDocGenerator::TranslateFunctionLineDefinition(const std::string &fileName,uint32_t &inOutLine)
{
	auto *contents = LoadFileContents(fileName);
	if(!contents)
		return;
	auto pos = find_beginning_of_function_definition(*contents,inOutLine);
	if(!pos.has_value())
		return;
	size_t p = -1;
	uint32_t line = 0;
	do
	{
		p = contents->find('\n',p +1);
		if(p < *pos)
			++line;
	}
	while(p < *pos);
	inOutLine = line +1;
}
std::string *LuaDocGenerator::LoadFileContents(const std::string &fileName)
{
	auto it = m_cachedFileContents.find(fileName);
	if(it == m_cachedFileContents.end())
	{
		auto f = FileManager::OpenSystemFile(fileName.c_str(),"r");
		if(!f)
			return nullptr;
		auto contents = f->ReadString();
		m_cachedFileContents[fileName] = std::move(contents);
		f = nullptr;

		it = m_cachedFileContents.find(fileName);
	}
	return &it->second;
}

static std::optional<std::string_view> find_function_argument_string(const std::string &contents,uint32_t line)
{
	auto op = find_beginning_of_function_definition(contents,line);
	if(!op.has_value())
		return {};
	auto p = *op;
	std::string_view view = contents;
	view = view.substr(0,p);
	uint32_t depth = 0;
	auto argEnd = p;
	while(true)
	{
		p = view.find_last_of("()");
        
		if(p == std::string::npos)
			return {};
		if(view[p] == '(')
		{
			if(depth == 0)
				break;
			--depth;
			view = view.substr(0,p);
			continue;
		}
		if(view[p] == ')')
			++depth;
		view = view.substr(0,p);
	}
	view = view.substr(p +1,argEnd -p -1);
	return view;
}

static std::vector<std::string> parse_function_argument_names(const std::string_view &argStr)
{
	std::vector<std::string> args;
	ustring::explode(std::string{argStr},",",args);
	std::vector<std::string> argNames;
	argNames.reserve(args.size());
	for(auto &arg : args)
	{
		ustring::remove_whitespace(arg);
		auto pos = arg.find_last_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
		if(pos == std::string::npos)
			argNames.push_back(arg);
		else
			argNames.push_back(arg.substr(pos +1));
	}
	return argNames;
}

std::optional<std::vector<std::string>> LuaDocGenerator::FindFunctionArguments(const std::string &fileName,uint32_t line)
{
	auto *contents = LoadFileContents(fileName);
	if(!contents)
		return {};
	auto view = find_function_argument_string(*contents,line);
	if(!view.has_value())
		return {};
	return parse_function_argument_names(*view);
}

static void *get_function_pointer_from_luabind_function_object(const luabind::detail::function_object &o)
{
	using T = luabind::detail::function_object_impl<Vector3 (__cdecl*)(void),luabind::meta::type_list<Vector3 >,luabind::meta::type_list<> >; // Template arguments are arbitrary; We only care about the function pointer, so they don't matter here
	return static_cast<void*>(static_cast<const T*>(&o)->f);
}

/*static std::optional<LuaOverloadInfo> parse_function_overload(std::string &methodName,bool method)
{
	auto paramStart = methodName.find('(');
	auto paramEnd = methodName.rfind(')');
	if(paramStart == std::string::npos || paramEnd == std::string::npos)
		return {};

	LuaOverloadInfo info {};

	auto params = methodName.substr(paramStart +1,paramEnd -paramStart -1);
	if(params.empty() == false && params.front() == ',')
		params.erase(params.begin());

	std::vector<std::string> paramList;
	ustring::explode(params,",",paramList);
	info.parameters.reserve(paramList.size());
	for(auto &param : paramList)
	{
		info.parameters.push_back({});
		info.parameters.back().name = std::move(param);
	}

	auto sp = methodName.rfind(' ',paramStart);
	if(sp != std::string::npos)
	{
		info.returnValues.push_back({});
		info.returnValues.back().name = methodName.substr(0,sp);
	}
	return info;
}*/

void LuaDocGenerator::ParseLuaProperty(const std::string &name,const luabind::object &o,LuaClassInfo &result,bool isMethod)
{
	auto *L = m_luaState;
	o.push(L);
	luabind::detail::stack_pop opop(L, 1);
	if(lua_tocfunction(L, -1) == &luabind::detail::property_tag)
		result.attributes.push_back(name);
	else
	{
		result.methods.push_back({});
		auto &method = result.methods.back();
		method.name = name;

		auto &omethod = o;
		luabind::detail::function_object * fobj = luabind::get_function_object(omethod);
		if(fobj) {
			luabind::object overloadTable(luabind::newtable(L));
			const char* function_name = fobj->name.c_str();
			for(luabind::detail::function_object const* f = fobj; f != 0; f = f->next)
			{
				std::vector<luabind::detail::TypeInfo> types;
				f->get_signature_info(L,function_name,types);
				LuaOverloadInfo overloadInfo {};
				assert(!types.empty()); // Should always contain at least the return type, even if it's void
				if(!types.empty())
				{
					overloadInfo.returnValues.push_back(std::move(types.front()));
					types.erase(types.begin());
					overloadInfo.parameters = std::move(types);
				}

				auto *fptr = get_function_pointer_from_luabind_function_object(*f);
				// f->format_signature(L, function_name);
				// luabind::detail::stack_pop pop(L, 1);
				// if(Lua::IsString(L,-1)) std::string def = Lua::CheckString(L,-1);
				// auto overloadInfo = parse_function_overload(def,isMethod);

				if(fptr)
				{
					auto fAddress = reinterpret_cast<DWORD64>(fptr);
					uint32_t line;
					auto fileName = m_symbolHandler->FindSource(fAddress,line);

					std::optional<std::string> moduleName {};
					HMODULE hModule = NULL;
					if(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
							GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
							(LPCTSTR)fAddress, &hModule))
					{
						TCHAR name[MAX_PATH];
						if(GetModuleBaseName(GetCurrentProcess(),hModule,name,sizeof(name) /sizeof(TCHAR)) != 0)
						{
							moduleName = name;
							ufile::remove_extension_from_filename(*moduleName,std::vector<std::string>{"dll"});

							fAddress -= reinterpret_cast<DWORD64>(hModule);
						}
					}

					if(moduleName.has_value())
					{
#ifdef ENABLE_PDB_MANAGER
						auto symbol = m_pdbManager->FindSymbolByRva(*moduleName,fAddress);
						if(symbol.has_value())
						{
							if(!fileName.has_value() && symbol->source.has_value())
							{
								fileName = symbol->source->fileName;
								line = symbol->source->line;
							}

							/*auto pdbParamInfoToType = [](pragma::lua::ParameterInfo &paramInfo,pragma::doc::Variant &param) {
								if(umath::is_flag_set(paramInfo.flags,ParameterInfo::Flags::Const))
									param.flags |= pragma::doc::Variant::Flags::Const;
								if(umath::is_flag_set(paramInfo.flags,ParameterInfo::Flags::Reference))
									param.flags |= pragma::doc::Variant::Flags::Reference;
								if(umath::is_flag_set(paramInfo.flags,ParameterInfo::Flags::Pointer))
									param.flags |= pragma::doc::Variant::Flags::Pointer;
								if(umath::is_flag_set(paramInfo.flags,ParameterInfo::Flags::Volatile))
									param.flags |= pragma::doc::Variant::Flags::Volatile;
								if(umath::is_flag_set(paramInfo.flags,ParameterInfo::Flags::Unaligned))
									param.flags |= pragma::doc::Variant::Flags::Unaligned;
								if(umath::is_flag_set(paramInfo.flags,ParameterInfo::Flags::Struct))
									param.flags |= pragma::doc::Variant::Flags::Struct;
								if(umath::is_flag_set(paramInfo.flags,ParameterInfo::Flags::Class))
									param.flags |= pragma::doc::Variant::Flags::Class;
								if(umath::is_flag_set(paramInfo.flags,ParameterInfo::Flags::Union))
									param.flags |= pragma::doc::Variant::Flags::Union;
								if(umath::is_flag_set(paramInfo.flags,ParameterInfo::Flags::Interface))
									param.flags |= pragma::doc::Variant::Flags::Interface;
								if(umath::is_flag_set(paramInfo.flags,ParameterInfo::Flags::Enum))
									param.flags |= pragma::doc::Variant::Flags::Enum;
								param.name = paramInfo.typeName;
							};
							if(symbol->parameters.has_value())
							{
								// We'll use the PDB parameter information instead of our parsed ones
								overloadInfo->parameters.clear();
								overloadInfo->parameters.reserve(symbol->parameters->size());
								for(auto &paramInfo : *symbol->parameters)
								{
									overloadInfo->parameters.push_back({});
									auto &param = overloadInfo->parameters.back();
									if(!paramInfo.typeName.empty())
										m_typeNameManager->AssignType(paramInfo.typeName);
									pdbParamInfoToType(paramInfo,param);
								}
							}
							
							if(symbol->returnValue.has_value())
							{
								overloadInfo->returnValues.clear();
								overloadInfo->returnValues.push_back({});
								auto &ret = overloadInfo->returnValues.back();
								if(!symbol->returnValue->typeName.empty())
									m_typeNameManager->AssignType(symbol->returnValue->typeName);
								pdbParamInfoToType(*symbol->returnValue,ret);
							}*/
						}
#endif

						if(!fileName.has_value())
							Con::cwar<<"WARNING: Unable to determine function source for "<<fptr<<" ("<<function_name<<") in module "<<*moduleName<<"!"<<Con::endl;
					}
					else
						Con::cwar<<"WARNING: Unable to determine module for function "<<fptr<<" ("<<function_name<<")"<<"!"<<Con::endl;

					if(fileName.has_value())
					{
						auto args = FindFunctionArguments(*fileName,line);
						TranslateFunctionLineDefinition(*fileName,line);
						if(!args.has_value())
							std::cout<<"";
						else if(args->size() != overloadInfo.parameters.size())
							std::cout<<"";
						else
							overloadInfo.namedParameters = args;

						auto normalizedFileName = *fileName;
						ustring::replace(normalizedFileName,"\\","/");
						auto p = normalizedFileName.rfind("/src/");
						if(p == std::string::npos)
							p = normalizedFileName.rfind("/include/");
						if(p == std::string::npos)
							p = normalizedFileName.rfind("/luabind/");
						if(p == std::string::npos)
							p = normalizedFileName.rfind("/glm/");
						if(p != std::string::npos)
						{
							p = normalizedFileName.rfind("/",p -1);
							normalizedFileName = normalizedFileName.substr(p);
						}
						else
							std::cout<<"!";

						//auto modName = m_symbolHandler.FindModule(reinterpret_cast<DWORD64>(fptr));
						//if(modName.has_value())
						{
							auto path = util::Path::CreateFile(normalizedFileName);
							auto modName = path.GetFront();
							path.PopFront();
							pragma::doc::Source source {};
							source.fileName = path.GetString();
							source.line = line;
							source.moduleName = std::move(modName);
							overloadInfo.source = std::move(source);
						}
					}
				}
				method.overloads.push_back(overloadInfo);
			}
		}
#if 0
/*
* static BOOL test_dbg_help(const std::string &szImageName,const std::string &functionName,uint64_t &outRva)
{
	SymbolHandler handler {};
	if(handler.Initialize())
	{

	{
	auto &en = *pragma::get_engine();
	auto *sv = en.GetServerNetworkState();
		auto *lsv = sv->GetLuaState();
		luabind::object o = luabind::globals(lsv)["ai"];
		o = o["create_schedule"];

		auto *fptr = get_function_pointer_from_luabind_function(lsv,o);
		if(fptr)
		{
			uint32_t line;
			auto fileName = handler.FindSource(reinterpret_cast<DWORD64>(fptr),line);
			if(fileName.has_value())
			{
				std::cout<<"FileName: "<<*fileName<<std::endl;
			}
		}
		
	}
}
*/
#endif
	}
};
#endif
