/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/lua/libraries/lfile.h"
#include <util_pragma_doc.hpp>
#include <luasystem.h>
#include <luainterface.hpp>

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

struct LuaMethodInfo
{
	std::string name;
	std::vector<std::string> overloads;
};

struct LuaClassInfo
{
	luabind::detail::class_rep *classRep = nullptr;
	std::string name;
	std::vector<LuaMethodInfo> methods;
	std::vector<std::string> attributes;
	std::vector<luabind::detail::class_rep*> bases;
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

		if(lua_tocfunction(L, -1) == &luabind::detail::property_tag)
		{
			auto attr = i.key();
			attr.push(L);
			if(Lua::IsString(L,-1))
				result.attributes.push_back(Lua::CheckString(L,-1));
			Lua::Pop(L,1);
		} else
		{
			auto method = i.key();
			method.push(L);
			if(Lua::IsString(L,-1))
			{
				result.methods.push_back({});
				auto &method = result.methods.back();
				method.name = Lua::CheckString(L,-1);

				auto &omethod = *i;
				luabind::detail::function_object * fobj = luabind::get_function_object(omethod);
				if(fobj) {
					luabind::object overloadTable(luabind::newtable(L));
					const char* function_name = fobj->name.c_str();
					for(luabind::detail::function_object const* f = fobj; f != 0; f = f->next)
					{
						f->format_signature(L, function_name);
						luabind::detail::stack_pop pop(L, 1);

						if(Lua::IsString(L,-1))
							method.overloads.push_back(Lua::CheckString(L,-1));
					}
				}
				Lua::Pop(L,1);
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
				auto itBase = std::find_if(baseClassMethod.overloads.begin(),baseClassMethod.overloads.end(),[&overload](const std::string &overloadInfoOther) {
					return overload == overloadInfoOther;
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

static std::unordered_map<std::string,std::string> g_typeTranslationTable {
	{"std::string","string"},
	{"short","int"},
	{"unsigned char","int"},
	{"unsigned int","int"},
	{"QuaternionInternal","Quaternion"},
	{"double","float"},
};

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

	for(auto &pair : g_typeTranslationTable)
		ustring::replace(paramName,pair.first,pair.second);
}

static void normalize_return_name(std::string &retName)
{
	normalize_param_name(retName);
}

static void normalize_method_name(std::string &methodName)
{
	auto paramStart = methodName.find('(');
	auto paramEnd = methodName.rfind(')');
	if(paramStart == std::string::npos || paramEnd == std::string::npos)
		return;

	auto params = methodName.substr(paramStart +1,paramEnd -paramStart -1);
	if(params.empty() == false && params.front() == ',')
		params.erase(params.begin());

	std::vector<std::string> paramList;
	ustring::explode(params,",",paramList);
	for(auto &param : paramList)
		normalize_param_name(param);
	if(paramList.size() >= 2)
	{
		if(paramList.front() == "lua_State")
			paramList.erase(paramList.begin());
		paramList.erase(paramList.begin()); // Second arg is self
	}

	params.clear();
	for(auto i=decltype(paramList.size()){0u};i<paramList.size();++i)
	{
		if(i > 0)
			params += ',';
		params += paramList.at(i);
	}

	methodName = methodName.substr(0,paramStart) +'(' +params +')';

	auto sp = methodName.rfind(' ',paramStart);
	if(sp != std::string::npos)
	{
		auto retType = methodName.substr(0,sp);
		normalize_return_name(retType);
		methodName = retType +' ' +methodName.substr(sp +1);
	}
}

static void autogenerate(lua_State *L)
{
	auto* reg = luabind::detail::class_registry::get_registry(L);
	auto& classes = reg->get_classes();

	std::unordered_map<luabind::detail::class_rep*,LuaClassInfo> classInfo {};
	for(const auto &cl : classes)
		classInfo[cl.second] = get_class_info(L,cl.second);

	for(auto &pair : classInfo)
		strip_base_class_methods(classInfo,pair.second,pair.second);

	for(auto &pair : classInfo)
	{
		for(auto &methodInfo : pair.second.methods)
		{
			for(auto &overload : methodInfo.overloads)
				normalize_method_name(overload);
		}
	}

	// Generate pragma doc
	for(auto &pair : classInfo)
	{
		auto &classInfo = pair.second;
		auto collection = pragma::doc::Collection::Create();
		for(auto *base : classInfo.bases)
		{
			// TODO: Base class of base class?
			auto derivedFrom = pragma::doc::DerivedFrom::Create(base->name());
			collection->AddDerivedFrom(*derivedFrom);
		}

		for(auto &attr : classInfo.attributes)
		{
			auto member = pragma::doc::Member::Create(*collection,attr);
			collection->AddMember(member);
		}

		for(auto &method : classInfo.methods)
		{

			//method.name;
			//method.overloads;

		}
		//classInfo.methods
	}

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
			auto f = FileManager::OpenFile(fileName.c_str(),"rb");
			if(f == nullptr)
				return 0;
			std::vector<pragma::doc::PCollection> collections {};
			if(pragma::doc::load_collections(f,collections) == false)
				return 0;
			auto t = Lua::CreateTable(l);
			auto idx = 1;
			for(auto &col : collections)
			{
				Lua::PushInt(l,idx++);
				Lua::Push<pragma::doc::PCollection>(l,col);
				Lua::SetTableValue(l,t);
			}
			return 1;
		})},
		{"autogenerate",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			autogenerate(l);
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
