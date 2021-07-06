/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/lua/libraries/lfile.h"
#include "pragma/lua/util.hpp"
#include "pragma/lua/lua_doc.hpp"
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
#pragma optimize("",off)
#if 0
struct LUABIND_API class_base : scope
{
public:
	class_base(char const* name);

	struct base_desc
	{
		type_id type;
		int ptr_offset;
	};

	void init(
		type_id const& type, class_id id
		, type_id const& wrapped_type, class_id wrapper_id);

	void add_base(type_id const& base, cast_function cast);

	void add_member(registration* member);
	void add_default_member(registration* member);

	const char* name() const;

	void add_static_constant(const char* name, int val);
	void add_inner_scope(scope& s);

	void add_cast(class_id src, class_id target, cast_function cast);

private:
	class_registration* m_registration;
};
#endif

#undef GetClassInfo

struct LuaOverloadInfo
{
	std::vector<std::string> parameters {};
	std::string returnValue {};
};

struct LuaMethodInfo
{
	std::string name;
	std::vector<LuaOverloadInfo> overloads;
};

struct LuaClassInfo
{
	using EnumSet = std::unordered_map<std::string,int32_t>;
	luabind::detail::class_rep *classRep = nullptr;
	std::string name;
	std::vector<LuaMethodInfo> methods;
	std::vector<std::string> attributes;
	std::vector<luabind::detail::class_rep*> bases;
	std::unordered_map<std::string,EnumSet> enumSets;
};

class DocValidator
{
public:
private:
	template<class TItem>
		static const TItem *FindItem(const std::string &name,const std::vector<TItem>&(pragma::doc::Collection::*fPt)() const)
	{
		return nullptr;
	}
	template<class TItem>
		static const TItem *FindItem(const pragma::doc::Collection &collection,const std::string &name,const std::vector<TItem>&(pragma::doc::Collection::*fPt)() const);
	void GetClassInfo(lua_State *L,luabind::detail::class_rep *crep);
	void FindFunction(const std::string &name) {}
	void WriteToLog(const std::stringstream &log) {}
	void ValidateCollection(const pragma::doc::Collection &collection);
	std::vector<LuaClassInfo> m_classes;
};

template<class TItem>
	static const TItem *DocValidator::FindItem(const pragma::doc::Collection &collection,const std::string &name,const std::vector<TItem>&(pragma::doc::Collection::*fPt)() const)
{
	auto &items = (collection.*fPt)();
	auto it = std::find_if(items.begin(),items.end(),[&name](const TItem &el) {
		return el.GetName() == name;
	});
	if(it != items.end())
		return &*it;
	for(auto &child : collection.GetChildren())
	{
		auto *pItem = find_item(*child,name,fPt);
		if(pItem)
			return pItem;
	}
	return nullptr;
}

static bool find_function(const pragma::doc::Collection &collection,const std::string &name)
{

}

void DocValidator::ValidateCollection(const pragma::doc::Collection &collection)
{
	auto name = collection.GetName();
	auto it = std::find_if(m_classes.begin(),m_classes.end(),[&name](const LuaClassInfo &luaClass) {
		return luaClass.name == name;
	});
	if(it == m_classes.end())
		Con::cout<<"CLASS NOT FOUND!"<<Con::endl;
	//find_item<pragma::doc::Function>(collection,name,&pragma::doc::Collection::GetFunctions);
	for(auto &f : collection.GetFunctions())
	{

	}
}

void DocValidator::GetClassInfo(lua_State *L,luabind::detail::class_rep * crep)
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
			auto *f = FindItem<pragma::doc::Function>(key,&pragma::doc::Collection::GetFunctions);
			if(f == nullptr)
			{
				std::stringstream ss;
				ss<<"Function '"<<key<<"' not found!";
				WriteToLog(ss);
			}
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

		if(lua_tocfunction(L, -1) == &luabind::detail::property_tag)
		{
			//result.attributes[index++] = i.key();
		} else
		{
			//result.methods[i.key()] = *i;
		}
	}
}

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

static std::unordered_map<std::string,std::string> g_typeTranslationTable {
	{"std::string","string"},
	{"short","int16"},
	{"unsigned char","uint8"},
	{"unsigned int","uint32"},
	{"QuaternionInternal","Quaternion"},
	{"double","float"},
	{"__int64","int64"},
	{"luabind::object","any"},
	{"File","file.File"}
};

std::unordered_set<std::string> g_typeWarningCache;
static void normalize_param_name(std::string &paramName)
{
	auto isRef = (paramName.find("&") != std::string::npos);
	auto isPtr = (paramName.find("*") != std::string::npos);
	auto isConst = (paramName.find(" const") != std::string::npos);
	ustring::replace(paramName,"&","");
	ustring::replace(paramName,"*","");
	ustring::replace(paramName,"custom ","");
	ustring::replace(paramName," const","");

	if(paramName.empty() == false && paramName.front() == '[' && paramName.back() == ']')
	{
		paramName.erase(paramName.begin());
		paramName.erase(paramName.end() -1);
		ustring::replace(paramName,"struct ","");
		ustring::replace(paramName,"class ","");
		// ustring::replace(paramName,"std::shared_ptr<","");
	}

	auto it = g_typeTranslationTable.find(paramName);
	if(it != g_typeTranslationTable.end())
		paramName = it->second;
	else
	{
		if(g_typeWarningCache.find(paramName) == g_typeWarningCache.end())
		{
			g_typeWarningCache.insert(paramName);
			Con::cwar<<"WARNING: Unknown type '"<<paramName<<"'!"<<Con::endl;
		}
	}
}

static void normalize_return_name(std::string &retName)
{
	normalize_param_name(retName);
}

static std::optional<LuaOverloadInfo> parse_function_overload(std::string &methodName,bool method)
{
	auto paramStart = methodName.find('(');
	auto paramEnd = methodName.rfind(')');
	if(paramStart == std::string::npos || paramEnd == std::string::npos)
		return {};

	LuaOverloadInfo info {};

	auto params = methodName.substr(paramStart +1,paramEnd -paramStart -1);
	if(params.empty() == false && params.front() == ',')
		params.erase(params.begin());

	auto &paramList = info.parameters;
	ustring::explode(params,",",paramList);
	for(auto &param : paramList)
		normalize_param_name(param);
	if(paramList.size() >= 2)
	{
		if(paramList.front() == "lua_State")
			paramList.erase(paramList.begin());
		if(method)
			paramList.erase(paramList.begin()); // Second arg is self
	}

	auto sp = methodName.rfind(' ',paramStart);
	if(sp != std::string::npos)
	{
		auto retType = methodName.substr(0,sp);
		normalize_return_name(retType);
		info.returnValue = retType;
	}
	return info;
}

static void parse_lua_property(lua_State *L,const std::string &name,const luabind::object &o,LuaClassInfo &result,bool isMethod)
{
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
				f->format_signature(L, function_name);
				luabind::detail::stack_pop pop(L, 1);

				if(Lua::IsString(L,-1))
				{
					std::string def = Lua::CheckString(L,-1);
					auto overloadInfo = parse_function_overload(def,isMethod);
					if(overloadInfo.has_value())
						method.overloads.push_back(*overloadInfo);
				}
			}
		}
	}
};

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
static LuaClassInfo get_class_info(lua_State *L,luabind::detail::class_rep *crep)
{
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
			parse_lua_property(L,name,*i,result,true);
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
					return overload.parameters == overloadInfoOther.parameters && overload.returnValue == overloadInfoOther.returnValue;
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
}

static void add_function(lua_State *l,pragma::doc::Collection &collection,const LuaMethodInfo &method)
{
	auto fc = pragma::doc::Function::Create(collection,method.name);
	fc.SetFlags(pragma::doc::Function::Flags::AutoGenerated);
	for(auto &overloadInfo : method.overloads)
	{
		auto overload = pragma::doc::Overload::Create();
		uint32_t argIdx = 1;
		for(auto &type : overloadInfo.parameters)
		{
			auto param = pragma::doc::Parameter::Create("arg" +std::to_string(argIdx++));
			param.SetType(type);
			overload.AddParameter(param);
		}
		if(overloadInfo.returnValue != "void")
		{
			auto param = pragma::doc::Parameter::Create("ret1");
			param.SetType(overloadInfo.returnValue);
			overload.AddReturnValue(param);
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
	collection.AddFunction(fc);
}
static pragma::doc::PCollection class_info_to_collection(lua_State *l,const std::string &path,const LuaClassInfo &classInfo)
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

	for(auto &method : classInfo.methods)
		add_function(l,*collection,method);

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
	return collection;
}
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
struct ClassRegInfo
{
	std::string path;
	LuaClassInfo classInfo;
	bool cppDefined = true;
};
static void add_class(
	lua_State *l,luabind::detail::class_rep *crep,const std::string &path,std::unordered_map<luabind::detail::class_rep*,pragma::doc::Collection*> &iteratedClasses,
	std::unordered_map<luabind::detail::class_rep*,ClassRegInfo> &classRegs
)
{
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
		info.classInfo = get_class_info(l,crep);
		
		auto subPath = path;
		if(!subPath.empty())
			subPath += '.';
		subPath += info.classInfo.name;
		info.path = subPath;

		auto addNewClass = true;
		auto it = classRegs.find(crep);
		if(it != classRegs.end())
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
				classRegs.erase(it);
				addNewClass = true;
			}
			else
				addNewClass = false;
		}

		if(addNewClass)
		{
			classRegs[crep] = std::move(info);

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
						add_class(l,crepChild,subPath,iteratedClasses,classRegs);
					}
				}
			}
			Lua::Pop(l,1);
		}
	}
}
static void iterate_libraries(
	luabind::object o,const std::string &path,std::vector<luabind::object> &traversed,pragma::doc::Collection &collection,
	pragma::doc::Collection &colTarget,std::unordered_map<luabind::detail::class_rep*,pragma::doc::Collection*> &iteratedClasses,
	std::unordered_map<luabind::detail::class_rep*,ClassRegInfo> &classRegs
)
{
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
				iterate_libraries(val,subPath,traversed,*subCollection,*subCollection,iteratedClasses,classRegs);
			}
		}
		else if(type == LUA_TFUNCTION)
		{
			auto fcName = luabind::object_cast_nothrow<std::string>(it.key(),std::string{});
			luabind::detail::function_object * fobj = luabind::get_function_object(val);
			if(fobj)
			{
				std::cout<<"FUN: "<<fcName<<std::endl;
				parse_lua_property(o.interpreter(),fcName,val,classInfo,false);
			}
		}
		else if(type == LUA_TUSERDATA)
		{
			auto *crep = Lua::get_crep(val);
			if(crep)
				add_class(l,crep,path,iteratedClasses,classRegs);
		}
	}

	for(auto &methodInfo : classInfo.methods)
		add_function(l,colTarget,methodInfo);
}

#include <span>
static void iterate_libraries(luabind::object o,pragma::doc::Collection &collection,pragma::doc::Collection &colTarget,std::unordered_map<luabind::detail::class_rep*,pragma::doc::Collection*> &iteratedClasses)
{
	auto *l = o.interpreter();
	std::vector<luabind::object> traversed {};
	std::unordered_map<luabind::detail::class_rep*,ClassRegInfo> classRegs;
	iterate_libraries(o,"",traversed,collection,colTarget,iteratedClasses,classRegs);

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
	for(auto &pair : classRegs)
	{
		std::vector<std::string> pathComponents;
		ustring::explode(pair.second.path,".",pathComponents);
		if(pathComponents.empty())
			pathComponents.push_back("_G");
		addItem(pair.second,root,pathComponents);
	}
	
	std::function<void(pragma::doc::Collection&,const ClassRegTreeItem&)> classRegToCollection = nullptr;
	classRegToCollection = [l,&classRegToCollection,&iteratedClasses,&collection](pragma::doc::Collection &parentCollection,const ClassRegTreeItem &item) {
		for(auto &child : item.children)
		{
			pragma::doc::PCollection childCollection = nullptr;
			auto newCollection = true;
			if(child->regInfo)
			{
				childCollection = class_info_to_collection(l,child->regInfo->path,child->regInfo->classInfo);
				if(child->regInfo->cppDefined)
					childCollection->SetFlags(childCollection->GetFlags() | pragma::doc::Collection::Flags::DefinedInCpp);
				iteratedClasses[child->regInfo->classInfo.classRep] = childCollection.get();
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

class RepositoryManager
{
public:
	static constexpr auto REPOSITORY_CHECKOUT_URL = "https://github.com/Silverlan/pragma.git";
	static constexpr auto REPOSITORY_URL = "https://github.com/Silverlan/pragma/blob/";
	static constexpr auto REPOSITORY_BRANCH = "master";
	static std::unique_ptr<RepositoryManager> Create(lua_State *l,std::string &outErr);
	bool LoadRepositoryReferences(std::string &outErr);
private:
	RepositoryManager()=default;
	void ClearLocalRepositoryDir();
	static void CollectSourceCodeFiles(const std::string &sourceCodeLocation,std::vector<std::string> &outFiles);

	using GitClone = bool(*)(
		const std::string&,const std::string&,
		const std::vector<std::string>&,const std::string&,std::string&,
		std::string*
	);

	GitClone m_gitClone = nullptr;
	std::string m_repositoryRootDir = "core/";
	std::string m_tmpRepositoryLocation = "temp/plad_repo/";
	std::shared_ptr<util::Library> m_gitLib = nullptr;
};

std::unique_ptr<RepositoryManager> RepositoryManager::Create(lua_State *l,std::string &outErr)
{
	constexpr auto MODULE_NAME = "git/pr_git";
	auto manager = std::unique_ptr<RepositoryManager>{new RepositoryManager{}};
	manager->m_gitLib = pragma::get_engine()->GetNetworkState(l)->InitializeLibrary(MODULE_NAME);
	if(!manager->m_gitLib)
	{
		outErr = "Unable to load module '" +std::string{MODULE_NAME} +"'!";
		return nullptr;
	}
	constexpr auto GIT_CLONE_FUNC_NAME = "pr_git_clone";
	manager->m_gitClone = manager->m_gitLib->FindSymbolAddress<GitClone>(GIT_CLONE_FUNC_NAME);
	if(!manager->m_gitClone)
	{
		outErr = "Could not locate symbol '" +std::string{GIT_CLONE_FUNC_NAME} +"' in module '" +std::string{MODULE_NAME} +"'!";
		return nullptr;
	}
	return manager->m_gitClone ? std::move(manager) : nullptr;
}

void RepositoryManager::CollectSourceCodeFiles(const std::string &sourceCodeLocation,std::vector<std::string> &outFiles)
{
	auto offset = outFiles.size();
	FileManager::FindSystemFiles((sourceCodeLocation +"*.cpp").c_str(),&outFiles,nullptr,true);
	for(auto i=offset;i<outFiles.size();++i)
		outFiles[i] = sourceCodeLocation +outFiles[i];

	std::vector<std::string> dirs;
	FileManager::FindSystemFiles((sourceCodeLocation +"*").c_str(),nullptr,&dirs,true);
	for(auto &dir : dirs)
		CollectSourceCodeFiles(sourceCodeLocation +dir +'/',outFiles);
}

void RepositoryManager::ClearLocalRepositoryDir()
{
	// Git-files are read-only by default; We'll have to change permission flags to remove them
	auto absRepositoryDir = FileManager::GetProgramPath() +'/' +m_tmpRepositoryLocation;
	try
	{
		for(auto &path : std::filesystem::recursive_directory_iterator(absRepositoryDir +".git"))
		{
			try {
				std::filesystem::permissions(path,std::filesystem::perms::all);
			}
			catch (std::exception& e) {
			}           
		}
	}
	catch(const std::filesystem::filesystem_error &err)
	{
	}
	filemanager::remove_directory(m_tmpRepositoryLocation);
}

bool RepositoryManager::LoadRepositoryReferences(std::string &outErr)
{
	std::string rootDir = "core/shared/";
	std::string localRepositoryDir = "temp/plad_repo/";
	auto absRepositoryDir = FileManager::GetProgramPath() +'/' +localRepositoryDir;

	ClearLocalRepositoryDir();
	FileManager::CreatePath(localRepositoryDir.c_str());

	std::string commitId;
	auto result = m_gitClone(
		REPOSITORY_CHECKOUT_URL,REPOSITORY_BRANCH,
		{rootDir +"*.cpp"},absRepositoryDir,outErr,&commitId
	);
	util::ScopeGuard sgClearTmpRepo {[this]() {ClearLocalRepositoryDir();}};
	if(!result)
		return false;

	std::vector<std::string> files;
	auto &sourceCodeDir = absRepositoryDir;
	CollectSourceCodeFiles(sourceCodeDir +rootDir,files);
	for(auto &fname : files)
	{
		if(ufile::get_file_from_filename(fname) == "ldoc.cpp")
			continue; // Skip this file
		auto f = FileManager::OpenSystemFile(fname.c_str(),"r");
		if(!f)
			continue;
		auto contents = f->ReadString();
		f = nullptr;
		auto pos = contents.find("#PLAD");
		if(pos == std::string::npos)
			continue;
		auto e = contents.find('\n',pos);
		auto sub = contents.substr(pos +5,e -(pos +5));
		ustring::remove_whitespace(sub);

		uint32_t lineIdx = 0;
		size_t c = 0;
		while(c < pos)
		{
			c = contents.find('\n',c);
			++lineIdx;
			if(c != std::string::npos)
				++c;
		}

		Con::cout<<"Found PLAD ID: "<<sub<<" (Line "<<lineIdx<<")"<<Con::endl;

		auto relPath = fname.substr(sourceCodeDir.length());
		std::string url = std::string{REPOSITORY_URL} +std::string{REPOSITORY_BRANCH} +"/" +relPath +"#L" +std::to_string(lineIdx);
		std::string urlCommit = std::string{REPOSITORY_URL} +commitId +"/" +relPath +"#L" +std::to_string(lineIdx);
		Con::cout<<"URL: "<<url<<Con::endl;
		Con::cout<<"URL Commit: "<<urlCommit<<Con::endl;
	}
	return true;
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

static void autogenerate()
{
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

	for(auto &stateInfo : luaStates)
	{
		auto *L = stateInfo.l;
		auto* reg = luabind::detail::class_registry::get_registry(L);
		auto& classes = reg->get_classes();

		auto rootCol = pragma::doc::Collection::Create();
		std::string rootColName = "root";
		rootCol->SetName(rootColName);
		auto gCol = pragma::doc::Collection::Create();
		gCol->SetName("_G");
		gCol->SetFlags(gCol->GetFlags() | pragma::doc::Collection::Flags::AutoGenerated);
		rootCol->AddChild(gCol);
		std::unordered_map<luabind::detail::class_rep*,pragma::doc::Collection*> iteratedClasses;
		iterate_libraries(luabind::globals(L),*rootCol,*gCol,iteratedClasses);

		for(auto &pair : iteratedClasses)
		{
			auto &crep = *pair.first;
			auto &bases = crep.bases();
			for(auto &base : bases)
			{
				auto it = iteratedClasses.find(base.base);
				assert(it != iteratedClassesRev.end());
				if(it == iteratedClasses.end())
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

	// Merge collections from all Lua states
	auto &rootCol = luaStates.front().collection;
	for(auto it=luaStates.begin() +1;it!=luaStates.end();++it)
	{
		auto &stateInfo = *it;
		merge_collection(*rootCol,*it->collection);
	}
	//
	
	std::string err;
	auto repoMan = RepositoryManager::Create(luaStates.front().l,err);
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
	udmData->SaveAscii(docLocation);
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

void Lua::doc::register_library(Lua::Interface &lua)
{
	auto *l = lua.GetState();

	const auto *libName = "doc";
	auto &docLib = lua.RegisterLibrary(libName);
	Lua::RegisterLibrary(l,libName,{
		{"load",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			std::string fileName = Lua::CheckString(l,1);
			if(Lua::file::validate_write_operation(l,fileName) == false)
				return 0;
			std::string err;
			auto udmData = ::util::load_udm_asset(fileName,&err);
			if(udmData == nullptr)
				return 0;

			auto col = pragma::doc::Collection::Load(udmData->GetAssetData(),err);
			if(!col)
				return 0;
			Lua::Push(l,col);
			return 1;
		})},
		{"autogenerate",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			autogenerate();
			return 0;
		})},
		{"generate_zerobrane_autocomplete_script",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			Lua::doc::generate_autocomplete_script();
			return 0;
		})}
	});
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
	docLib[cdefFunction];

	auto cdefExCode = luabind::class_<pragma::doc::Function::ExampleCode>("ExampleCode");
	cdefExCode.def_readwrite("description",&pragma::doc::Function::ExampleCode::description);
	cdefExCode.def_readwrite("code",&pragma::doc::Function::ExampleCode::code);
	cdefFunction.scope[cdefFunction];

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
	cdefMember.def("GetType",static_cast<void(*)(lua_State*,pragma::doc::Member&)>([](lua_State *l,pragma::doc::Member &member) {
		Lua::PushString(l,member.GetType());
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
#pragma optimize("",on)
