#include "pragma/lua/libraries/lfile.h"
#include <util_pragma_doc.hpp>
#include <luasystem.h>
#include <luainterface.hpp>

#include <luabind/detail/class_rep.hpp>

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

struct LuaFunctionInfo
{
	std::string name;
};

struct LuaClassInfo
{
	std::string name;
	std::vector<LuaFunctionInfo> methods;
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

static luabind::object get_class_names(lua_State* L)
{
	luabind::detail::class_registry* reg = luabind::detail::class_registry::get_registry(L);

	std::map<luabind::type_id, luabind::detail::class_rep*> const& classes = reg->get_classes();

	luabind::object result = luabind::newtable(L);
	std::size_t index = 1;

	for(const auto& cl : classes) {
		//result[index++] = cl.second->name();
		auto *name = cl.second->name();

		std::vector<pragma::doc::PCollection> collections {};
		// TODO

		//get_class_info(L,cl.second);
	}

	luabind::detail::class_registry* r = luabind::detail::class_registry::get_registry(L);
	//r->
	return result;
}
static void test()
{

}


namespace Lua::doc
{
	void register_library(Lua::Interface &lua);
};
void Lua::doc::register_library(Lua::Interface &lua)
{
	auto *l = lua.GetState();
	get_class_names(l);
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
			Lua::Push<pragma::doc::DerivedFrom*>(l,&const_cast<pragma::doc::DerivedFrom&>(df));
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
