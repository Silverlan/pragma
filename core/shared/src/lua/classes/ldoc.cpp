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
#include <Psapi.h>
#include <Dia2.h>

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
	std::optional<pragma::doc::Source> source {};
	std::optional<std::vector<std::string>> namedParameters {};
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
	{"Version","util.Version"}
};

std::unordered_set<std::string> g_typeWarningCache;
std::unordered_map<std::string,std::string> g_classNameToFullName;
static void normalize_param_name(std::string &paramName)
{
	auto isRef = (paramName.find("&") != std::string::npos);
	auto isPtr = (paramName.find("*") != std::string::npos);
	auto isConst = (paramName.find(" const") != std::string::npos);
	ustring::replace(paramName,"&","");
	ustring::replace(paramName,"*","");
	ustring::replace(paramName,"custom ","");
	ustring::replace(paramName," const","");
	ustring::replace(paramName,"std::shared_ptr<","");
	ustring::replace(paramName,"util::TSharedHandle<","");
	ustring::replace(paramName,"util::WeakHandle<","");
	ustring::replace(paramName,">","");

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
		auto it = g_classNameToFullName.find(paramName);
		if(it != g_classNameToFullName.end())
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

	auto sp = methodName.rfind(' ',paramStart);
	if(sp != std::string::npos)
		info.returnValue = methodName.substr(0,sp);
	return info;
}
static void *get_function_pointer_from_luabind_function_object(const luabind::detail::function_object &o)
{
	using T = luabind::detail::function_object_impl<Vector3 (__cdecl*)(void),luabind::meta::type_list<Vector3 >,luabind::meta::type_list<> >; // Template arguments are arbitrary; We only care about the function pointer, so they don't matter here
	return static_cast<void*>(static_cast<const T*>(&o)->f);
}
static void *get_function_pointer_from_luabind_function(lua_State *l,const luabind::object &lbFunc)
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

namespace pragma::os::windows
{
	using RVA = uint64_t;
};
class SymbolHandler
{
public:
	~SymbolHandler();
	bool Initialize();
	std::optional<std::string> FindModule(DWORD64 rva) const;
	std::optional<std::string> FindAddress(DWORD64 rva) const;
	std::optional<pragma::os::windows::RVA> FindSymbol(const std::string &symbol) const;
	std::optional<std::string> FindSource(DWORD64 rva,uint32_t &outLine) const;

	operator bool() const {return m_valid;}
private:
	HANDLE m_hProcess = nullptr;
	bool m_valid = false;
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

#include <span>

class PdbManager
{
public:
	struct SymbolInfo
	{
		struct Source
		{
			std::string fileName;
			uint32_t line;
		};
		std::string name;
		DWORD64 rva = 0;
		std::optional<Source> source {};
	};

	~PdbManager();
	bool Initialize();
	bool LoadPdb(const std::string &moduleName,const std::string &pdbPath);
	const std::vector<std::string> &GetModuleNames() const {return m_moduleNames;}

	std::optional<SymbolInfo> FindSymbolByRva(const std::string &moduleName,DWORD64 rva);
private:
	struct PdbSession
	{
		~PdbSession();
		std::string pdbPath;
		IDiaDataSource *source = nullptr;
		IDiaSession *session = nullptr;
		IDiaSymbol *globalSymbol = nullptr;
		IDiaEnumTables *enumTables = nullptr;
		IDiaEnumSymbolsByAddr *enumSymbolsByAddr = nullptr;
		operator bool() const {return m_valid;}
		void SetValid() {m_valid = true;}
	private:
		bool m_valid = false;
	};
	std::unordered_map<std::string,std::unique_ptr<PdbSession>> m_pdbSessions {};
	std::vector<std::string> m_moduleNames;
};

struct LuaDocGenerator
{
public:
	LuaDocGenerator(lua_State *l);
	void IterateLibraries(luabind::object o,pragma::doc::Collection &collection,pragma::doc::Collection &colTarget);
	void SetPdbManager(PdbManager *manager) {m_pdbManager = manager;}
	void SetSymbolHandler(SymbolHandler *symHandler) {m_symbolHandler = symHandler;}

	const std::unordered_map<luabind::detail::class_rep*,pragma::doc::Collection*> &GetClassMap() const {return m_iteratedClasses;}
private:
	std::string *LoadFileContents(const std::string &fileName);
	void IterateLibraries(
		luabind::object o,const std::string &path,pragma::doc::Collection &collection,
		pragma::doc::Collection &colTarget
	);
	void ParseLuaProperty(const std::string &name,const luabind::object &o,LuaClassInfo &result,bool isMethod);
	LuaClassInfo GetClassInfo(luabind::detail::class_rep *crep);
	void AddClass(luabind::detail::class_rep *crep,const std::string &path);
	void InitializeRepositoryUrls();
	static std::optional<std::string> GetCommitId(const std::string &repositoryUrl);
	static std::string BuildRepositoryUrl(const std::string &baseUrl,const std::string &commitUrl);
	std::optional<std::string> SourceToUrl(const pragma::doc::Source &source);
	pragma::doc::PCollection ClassInfoToCollection(lua_State *l,const std::string &path,const LuaClassInfo &classInfo);
	void AddFunction(lua_State *l,pragma::doc::Collection &collection,const LuaMethodInfo &method);
	
	void TranslateFunctionLineDefinition(const std::string &fileName,uint32_t &inOutLine);
	std::optional<std::vector<std::string>> FindFunctionArguments(const std::string &fileName,uint32_t line);

	lua_State *m_luaState = nullptr;
	std::unordered_map<std::string,std::string> m_cachedFileContents {};
	std::vector<luabind::object> m_traversed {};
	std::unordered_map<luabind::detail::class_rep*,ClassRegInfo> m_classRegs;
	std::unordered_map<luabind::detail::class_rep*,pragma::doc::Collection*> m_iteratedClasses;
	std::unordered_map<std::string,std::string> m_moduleToRepositoryMap;
	PdbManager *m_pdbManager = nullptr;
	SymbolHandler *m_symbolHandler = nullptr;
};

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
		{"pr_steamworks",BuildRepositoryUrl("https://github.com/Silverlan/pr_steamworks/","blob/%commitid%/")}
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

	for(auto &methodInfo : classInfo.methods)
		AddFunction(l,colTarget,methodInfo);
}

void LuaDocGenerator::AddFunction(lua_State *l,pragma::doc::Collection &collection,const LuaMethodInfo &method)
{
	auto fc = pragma::doc::Function::Create(collection,method.name);
	fc.SetFlags(pragma::doc::Function::Flags::AutoGenerated);
	for(auto &overloadInfo : method.overloads)
	{
		auto overload = pragma::doc::Overload::Create();
		for(auto argIdx=decltype(overloadInfo.parameters.size()){0u};argIdx<overloadInfo.parameters.size();++argIdx)
		{
			auto &type = overloadInfo.parameters[argIdx];
			std::string argName = "arg" +std::to_string(argIdx);
			if(overloadInfo.namedParameters.has_value())
				argName = (*overloadInfo.namedParameters)[argIdx];
			auto param = pragma::doc::Parameter::Create(argName);
			param.SetType(type);
			overload.AddParameter(param);
		}
		if(overloadInfo.returnValue != "void")
		{
			auto param = pragma::doc::Parameter::Create("ret1");
			param.SetType(overloadInfo.returnValue);
			overload.AddReturnValue(param);
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

	for(auto &method : classInfo.methods)
		AddFunction(l,*collection,method);

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
				auto *fptr = get_function_pointer_from_luabind_function_object(*f);
				f->format_signature(L, function_name);
				luabind::detail::stack_pop pop(L, 1);

				if(Lua::IsString(L,-1))
				{
					std::string def = Lua::CheckString(L,-1);
					auto overloadInfo = parse_function_overload(def,isMethod);
					if(overloadInfo.has_value())
					{
						if(fptr)
						{
							auto fAddress = reinterpret_cast<DWORD64>(fptr);
							uint32_t line;
							auto fileName = m_symbolHandler->FindSource(fAddress,line);
							if(!fileName.has_value())
							{
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
									auto symbol = m_pdbManager->FindSymbolByRva(*moduleName,fAddress);
									if(symbol.has_value() && symbol->source.has_value())
									{
										fileName = symbol->source->fileName;
										line = symbol->source->line;
									}

									if(!fileName.has_value())
										Con::cwar<<"WARNING: Unable to determine function source for "<<fptr<<" ("<<function_name<<") in module "<<*moduleName<<"!"<<Con::endl;
								}
								else
									Con::cwar<<"WARNING: Unable to determine module for function "<<fptr<<" ("<<function_name<<")"<<"!"<<Con::endl;
							}
							if(fileName.has_value())
							{
								auto args = FindFunctionArguments(*fileName,line);
								TranslateFunctionLineDefinition(*fileName,line);
								if(!args.has_value())
									std::cout<<"";
								else if(args->size() != overloadInfo->parameters.size())
									std::cout<<"";
								else
									overloadInfo->namedParameters = args;

								auto normalizedFileName = *fileName;
								ustring::replace(normalizedFileName,"\\","/");
								auto p = normalizedFileName.rfind("/src/");
								if(p == std::string::npos)
									p = normalizedFileName.rfind("/include/");
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
									overloadInfo->source = std::move(source);
								}
							}
						}
						method.overloads.push_back(*overloadInfo);
					}
				}
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

namespace pragma::ai {class Schedule;}
__declspec(dllexport) std::shared_ptr<pragma::ai::Schedule> testt() {return nullptr;}

__declspec(dllexport) void test_function() {}

static std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }
    
    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    
    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);
    
    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);
            
    return message;
}

SymbolHandler::~SymbolHandler()
{

}

bool SymbolHandler::Initialize()
{
    SymSetOptions(/*SYMOPT_UNDNAME | */SYMOPT_DEFERRED_LOADS);
    m_hProcess = GetCurrentProcess();
    if(!SymInitialize(m_hProcess,nullptr,TRUE))
	{
		Con::cwar<<"WARNING: Unable to initialize symbol handler: "<<GetLastErrorAsString()<<Con::endl;
		m_valid = false;
        return false;
	}
	m_valid = true;
	return true;
}

/*void SymbolHandler::AddModule(const std::string &mod)
{
    DWORD64 dwBaseAddr = 0;
    auto baseAddress = SymLoadModuleEx(
		m_hProcess, // target process 
		nullptr, // handle to image - not used
		mod.c_str(), // name of image file
		nullptr, // name of module - not required
		dwBaseAddr, // base address - not required
		0, // size of image - not required
		nullptr, // MODLOAD_DATA used for special cases 
		0
	);
	if(baseAddress)
		m_modules.push_back(baseAddress);
}*/

static void *get_fptr()
{
	if(true)
		return &testt;
	auto &en = *pragma::get_engine();
	auto *sv = en.GetServerNetworkState();
	if(sv)
	{
		auto *lsv = sv->GetLuaState();

		luabind::object o = luabind::globals(lsv)["ai"];
		o = o["create_schedule"];
		o.push(lsv);

		lua_CFunction f = lua_tocfunction(lsv,-1);
		return f;
	}
	return nullptr;
}
std::optional<std::string> SymbolHandler::FindModule(DWORD64 rva) const
{
	if(!*this)
		return {};
	IMAGEHLP_MODULE moduleInfo;
	moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
	if(SymGetModuleInfo(m_hProcess,rva,&moduleInfo))
		return moduleInfo.ModuleName;
	return {};
}
std::optional<std::string> SymbolHandler::FindAddress(DWORD64 rva) const
{
	if(!*this)
		return {};
	DWORD64 dwDisplacement;
	constexpr uint32_t maxNameLen = 4'096;
	auto *ptr = new uint8_t[sizeof(SYMBOL_INFO) +maxNameLen +1]; // See https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/ns-dbghelp-symbol_info?redirectedfrom=MSDN
	util::ScopeGuard sg {[ptr]() {delete[] ptr;}};
	auto &sym = *reinterpret_cast<SYMBOL_INFO*>(ptr);
	sym.MaxNameLen = maxNameLen;
	sym.SizeOfStruct = sizeof(SYMBOL_INFO);
	if(SymFromAddr(m_hProcess,rva,&dwDisplacement,&sym))
		return sym.Name;
	return {};
}
std::optional<std::string> SymbolHandler::FindSource(DWORD64 rva,uint32_t &outLine) const
{
	if(!*this)
		return {};
	IMAGEHLP_LINE64 line;
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	DWORD dwDisplacement;
	if(SymGetLineFromAddr(m_hProcess,rva,&dwDisplacement,&line) == TRUE)
	{
		outLine = line.LineNumber;
		return line.FileName;
	}
	else
		Con::cwar<<"WARNING: "<<GetLastErrorAsString()<<Con::endl;
	return {};
}
std::optional<pragma::os::windows::RVA> SymbolHandler::FindSymbol(const std::string &symbol) const
{
	if(!*this)
		return {};
	struct SymbolInfo
	{
		const SymbolHandler &symbolHandler;
		std::optional<pragma::os::windows::RVA> rva {};
	} symbolInfo {*this};
	auto mask = "*!" +symbol; // See https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-symenumsymbols
    SymEnumSymbols(m_hProcess, // Process handle from SymInitialize.
		0, // Base address of module.
		mask.c_str(), // Name of symbols to match.
		[](
			PSYMBOL_INFO pSymInfo,   
			ULONG SymbolSize,      
			PVOID UserContext
		) -> BOOL {
			auto &symbolInfo = *static_cast<SymbolInfo*>(UserContext);
			symbolInfo.rva = pSymInfo->Address -pSymInfo->ModBase;

			IMAGEHLP_LINE64 line;
			DWORD dwDisplacement;
			if(SymGetLineFromAddr(symbolInfo.symbolHandler.m_hProcess,(DWORD64)testt/*pSymInfo->Address*/,&dwDisplacement,&line) == TRUE)
			{
				std::cout<<"LINE: "<<line.FileName<<": "<<line.LineNumber<<std::endl;
			}
			else
				std::cout<<GetLastErrorAsString()<<std::endl;
			return TRUE;
		}, // Symbol handler procedure.
		&symbolInfo
	);
	return symbolInfo.rva;
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
		std::cout<<"ERR: "<<GetLastErrorAsString()<<std::endl;
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
        std::cout<<GetLastErrorAsString()<<std::endl;
    }
}

#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"C:/Program Files (x86)/Microsoft Visual Studio 14.0/DIA SDK/lib/amd64/diaguids.lib")

std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen)
{
    int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

    std::string dblstr(len, '\0');
    len = ::WideCharToMultiByte(CP_ACP, 0 /* no flags */,
                                pstr, wslen /* not necessary NULL-terminated */,
                                &dblstr[0], len,
                                NULL, NULL /* no default char */);

    return dblstr;
}

std::string ConvertBSTRToMBS(BSTR bstr)
{
    int wslen = ::SysStringLen(bstr);
    return ConvertWCSToMBS((wchar_t*)bstr, wslen);
}

PdbManager::PdbSession::~PdbSession()
{
	if(enumSymbolsByAddr)
		enumSymbolsByAddr->Release();
	if(enumTables)
		enumTables->Release();
	if(globalSymbol)
		globalSymbol->Release();
	if(session)
		session->Release();
	if(source)
		source->Release();
}
PdbManager::~PdbManager()
{
	m_pdbSessions.clear();
	CoUninitialize();
}
bool PdbManager::Initialize()
{
	CoInitializeEx(nullptr,COINIT_MULTITHREADED); // If failed, was probably already initialized, so we're ignoring the return value (we still have to call CoUnInitialize either way)
	return true;
}

bool PdbManager::LoadPdb(const std::string &moduleName,const std::string &pdbPath)
{
	auto sessionInfo = std::unique_ptr<PdbSession>{new PdbSession{}};
	auto hr = CoCreateInstance(
		CLSID_DiaSource,
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(IDiaDataSource),
		(void**)&sessionInfo->source
	);
	if(FAILED(hr))
		return false;

	wchar_t wszFilename[_MAX_PATH];
	mbstowcs(wszFilename,pdbPath.data(),sizeof(wszFilename) /sizeof(wszFilename[0]));
	if(FAILED(sessionInfo->source->loadDataFromPdb(wszFilename)))
	{
		if(FAILED(sessionInfo->source->loadDataForExe(wszFilename,nullptr,nullptr)))
			return false;
	}

	if(FAILED(sessionInfo->source->openSession(&sessionInfo->session))) 
		return false;
	if(FAILED(sessionInfo->session->get_globalScope(&sessionInfo->globalSymbol)))
		return false;
	if(FAILED(sessionInfo->session->getEnumTables(&sessionInfo->enumTables)))
		return false;
	if(FAILED(sessionInfo->session->getSymbolsByAddr(&sessionInfo->enumSymbolsByAddr)))
		return false;
	sessionInfo->SetValid();
	m_pdbSessions[moduleName] = std::move(sessionInfo);
	m_moduleNames.push_back(moduleName);
	return true;
}

std::optional<PdbManager::SymbolInfo> PdbManager::FindSymbolByRva(const std::string &moduleName,DWORD64 rva)
{
	auto it = m_pdbSessions.find(moduleName);
	if(it == m_pdbSessions.end() || !*it->second)
		return {};
	IDiaSymbol *symbol;
	auto &sessionInfo = *it->second;
	if(sessionInfo.session->findSymbolByRVA(rva,SymTagEnum::SymTagFunction,&symbol) != S_OK)
		return {};
	DWORD symTag;
	if(symbol->get_symTag(&symTag) != S_OK || symTag != SymTagFunction)
		return {};
	SymbolInfo symbolInfo {};

	BSTR name;
	if(symbol->get_name(&name) == S_OK)
		symbolInfo.name = ConvertBSTRToMBS(name);
	symbolInfo.rva = rva;

	ULONGLONG length = 0;
	if(symbol->get_length(&length) == S_OK)
	{
		IDiaEnumLineNumbers *lineNums[100];
		if(sessionInfo.session->findLinesByRVA(rva,length,lineNums) == S_OK)
		{
			auto &l = lineNums[0];
			IDiaLineNumber *lineNum;
			ULONG fetched = 0;
			for(uint8_t i=0;i<5;++i)
			{
				if(l->Next(i,&lineNum,&fetched) == S_OK && fetched == 1)
				{
					DWORD l;
					IDiaSourceFile *srcFile;
					if(lineNum->get_sourceFile(&srcFile) == S_OK)
					{
						BSTR fileName;
						srcFile->get_fileName(&fileName);
						if(lineNum->get_lineNumber(&l) == S_OK)
						{
							symbolInfo.source = SymbolInfo::Source{};
							symbolInfo.source->fileName = ConvertBSTRToMBS(fileName);
							symbolInfo.source->line = l;
							break;
						}
					}
				}
			}
		}
	}
	symbol->Release();
	return symbolInfo;
}






static BOOL test_pdb(const std::string &pdbFilePath,uint64_t rva)
{
    IDiaDataSource  *pSource;
    if(FAILED(CoInitializeEx(NULL,COINIT_MULTITHREADED)))
        return FALSE;
    auto hr = CoCreateInstance(
        CLSID_DiaSource,
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IDiaDataSource),
        (void **) &pSource
    );
    if(FAILED(hr))
        return FALSE;

    wchar_t wszFilename[_MAX_PATH];
    mbstowcs(wszFilename,pdbFilePath.data(),sizeof(wszFilename) /sizeof(wszFilename[0]));
    if(FAILED(pSource->loadDataFromPdb(wszFilename)))
    {
        if(FAILED(pSource->loadDataForExe(wszFilename,NULL,NULL)))
            return FALSE;
    }

    IDiaSession *session;
    IDiaSymbol *globalSymbol = nullptr;
    IDiaEnumTables *enumTables = nullptr;
    IDiaEnumSymbolsByAddr *enumSymbolsByAddr = nullptr;
    if(FAILED(pSource->openSession(&session))) 
        return FALSE;

    if(FAILED(session->get_globalScope(&globalSymbol)))
        return FALSE;

    if(FAILED(session->getEnumTables(&enumTables)))
        return FALSE;

    if(FAILED(session->getSymbolsByAddr(&enumSymbolsByAddr)))
        return FALSE;

    IDiaSymbol *symbol;
    if(session->findSymbolByVA(rva,SymTagEnum::SymTagFunction,&symbol) == S_OK)
    {
        BSTR name;
        symbol->get_name(&name);
        std::cout<<"Name: "<<ConvertBSTRToMBS(name)<<std::endl;

        ULONGLONG length = 0;
        if(symbol->get_length(&length) == S_OK)
        {
            uint32_t lineId;
            //auto sourceFile = symbol.GetSourceFile(lineId);

            IDiaEnumLineNumbers *lineNums[100];
            if(session->findLinesByRVA(rva,length,lineNums) == S_OK)
            {
                auto &l = lineNums[0];
                IDiaLineNumber *line;
                IDiaLineNumber *lineNum;
                ULONG fetched = 0;
                for(uint8_t i=0;i<5;++i) {
                if(l->Next(i,&lineNum,&fetched) == S_OK && fetched == 1)
                {
                    DWORD l;
                    IDiaSourceFile *srcFile;
                    if(lineNum->get_sourceFile(&srcFile) == S_OK)
                    {
                        BSTR fileName;
                        srcFile->get_fileName(&fileName);
                        std::cout<<"File: "<<ConvertBSTRToMBS(fileName)<<std::endl;
                    }
                    if(lineNum->get_lineNumber(&l) == S_OK)
                        std::cout<<"Line: "<<l<<std::endl;
                    if(lineNum->get_lineNumberEnd(&l) == S_OK)
                        std::cout<<"Line End: "<<l<<std::endl;
                    
                }
                }
            }
        }
    }

    return TRUE;
}

#include <luabind/make_function.hpp>
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

#if 0
		luabind::object o = luabind::globals(lsv)["ai"];
		o = o["create_schedule"];
		o.push(lsv);
		Lua::StackDump(lsv);

		auto base = GetModuleInfo(GetCurrentProcessId(),"server.dll");
		std::cout<<"Addr: "<<base.modBaseAddr<<std::endl;
		
		lua_CFunction f = lua_tocfunction(lsv,-1);
		std::cout<<"F: "<<std::hex<<(reinterpret_cast<uint64_t>(f))<<std::endl;
		std::cout<<"Rel Ptr: "<<std::hex<<(reinterpret_cast<uint64_t>(f) -reinterpret_cast<uint64_t>(base.modBaseAddr))<<std::endl;


    dbg::Help help {};
    if(help.Initialize())
    {
        help.LoadSymbolModule();
    }

		testt();
		base = GetModuleInfo(GetCurrentProcessId(),"shared.dll");
		std::cout<<"&testt: "<<std::hex<<reinterpret_cast<uint64_t>(&testt)<<","<<GetProcAddress(base.hModule,"?testt@@YA?AV?$shared_ptr@VSchedule@ai@pragma@@@std@@XZ")<<std::endl;
		std::cout<<"&modBaseAddr: "<<std::hex<<reinterpret_cast<uint64_t>(base.modBaseAddr)<<std::endl;
		std::cout<<"Diff: "<<std::hex<<(reinterpret_cast<uint64_t>(&testt) -reinterpret_cast<uint64_t>(base.modBaseAddr))<<std::endl;

		
		std::cout<<"A: "<<typeid(f).name()<<std::endl;
		std::cout<<"B: "<<typeid(decltype(f)).name()<<std::endl;

		auto mod = GetModuleHandle("server.dll");
		CHAR name[] = {'a','s','d','f','\0'};
		customGetProcAddress(mod,name);
		//auto *x = luabind::touserdata<luabind::detail::function_object*>(o);
		//auto *y = luabind::object_cast<luabind::detail::function_object*>(o);
		//luabind::detail::function_object const* impl_const = *(luabind::detail::function_object const**)lua_touserdata(lsv, lua_upvalueindex(1));
		//luabind::detail::function_object const* impl_const2 = (luabind::detail::function_object const*)lua_touserdata(lsv, lua_upvalueindex(1));
		std::cout<<f<<std::endl;
		//luabind::detail::function_object_impl const* impl_const = *(luabind::detail::function_object_impl const**)lua_touserdata(lsv, lua_upvalueindex(1));

		//luabind::detail::function_object_impl* impl = const_cast<luabind::detail::function_object_impl*>(impl_const);
		//impl->f
#endif
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

	PdbManager pdbManager {};
	if(pdbManager.Initialize())
	{
		// TODO: Determine the paths automatically somehow?
		pdbManager.LoadPdb("server","E:/projects/pragma/build_winx64/core/server/RelWithDebInfo/server.pdb");
		pdbManager.LoadPdb("client","E:/projects/pragma/build_winx64/core/client/RelWithDebInfo/client.pdb");
		pdbManager.LoadPdb("shared","E:/projects/pragma/build_winx64/core/shared/RelWithDebInfo/shared.pdb");
		pdbManager.LoadPdb("mathutil","E:/projects/pragma/build_winx64/external_libs/mathutil/RelWithDebInfo/mathutil.pdb");
		pdbManager.LoadPdb("sharedutils","E:/projects/pragma/build_winx64/external_libs/sharedutils/RelWithDebInfo/sharedutils.pdb");
	}

	SymbolHandler symHandler {};
	symHandler.Initialize();
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
		
		LuaDocGenerator docGenerator {L};
		docGenerator.SetPdbManager(&pdbManager);
		docGenerator.SetSymbolHandler(&symHandler);
		docGenerator.IterateLibraries(luabind::globals(L),*rootCol,*gCol);

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

	// Merge collections from all Lua states
	auto &rootCol = luaStates.front().collection;
	for(auto it=luaStates.begin() +1;it!=luaStates.end();++it)
	{
		auto &stateInfo = *it;
		merge_collection(*rootCol,*it->collection);
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
	translateCollectionTypes = [&translateCollectionTypes](pragma::doc::Collection &col) {
		auto isClass = umath::is_flag_set(col.GetFlags(),pragma::doc::Collection::Flags::Class);
		auto &functions = col.GetFunctions();
		for(auto &f : functions)
		{
			auto &overloads = f.GetOverloads();
			for(auto &overload : overloads)
			{
				auto &params = overload.GetParameters();
				for(auto &param : params)
				{
					auto type = param.GetType();
					normalize_param_name(type);
					param.SetType(type);
				}
				if(!params.empty() && params.front().GetType() == "lua_State")
					params.erase(params.begin());
				auto isMethod = isClass;
				if(isMethod && !params.empty())
					params.erase(params.begin());
				for(auto &ret : overload.GetReturnValues())
				{
					auto type = ret.GetType();
					normalize_param_name(type);
					ret.SetType(type);
				}
			}
		}

		auto &members = col.GetMembers();
		for(auto &member : members)
		{
			auto type = member.GetType();
			normalize_param_name(type);
			member.SetType(type);
		}

		for(auto &child : col.GetChildren())
			translateCollectionTypes(*child);
	};
	translateCollectionTypes(*rootCol);
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
