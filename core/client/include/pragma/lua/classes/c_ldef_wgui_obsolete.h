/*#include <pragma/lua/ldefinitions.h>
#include "wihandle.h"
class WGUI;
class DLLCLIENT WGUILuaMap
{
private:
	std::map<const std::type_info*,luabind::object*(*)(lua_State*,WIBase*)> m_factories;
public:
	void AddFactory(const std::type_info &info,luabind::object*(*fc)(lua_State*,WIBase*));
	luabind::object*(*FindFactory(const std::type_info &info))(lua_State*,WIBase*);
};

DLLCLIENT WGUILuaMap *GetWGUILuaMap();
DLLCLIENT void LinkWGUIToLuaFactory(const std::type_info &info,luabind::object*(*fc)(lua_State*,WIBase*));

class DLLCLIENT __reg_wgui_lua_factory
{
public:
	__reg_wgui_lua_factory(const std::type_info &info,luabind::object*(*fc)(lua_State*,WIBase*));
};

#define LINK_WGUI_TO_HANDLE_INHERIT(classname,baseclass) \
	__reg_wgui_lua_factory *__reg_wgui_lua_factory_##classname = new __reg_wgui_lua_factory(typeid(classname),&CreateWGUI##baseclass##LuaObject);

#define LINK_WGUI_TO_HANDLE(classname) \
	static luabind::object *CreateWGUI##classname##LuaObject(lua_State *lua,WIBase *p) \
	{ \
		return new luabind::object(lua,Ptr##classname##Handle(p->CreateHandle<##classname##Handle>())); \
	} \
	__reg_wgui_lua_factory *__reg_wgui_lua_factory_##classname = new __reg_wgui_lua_factory(typeid(classname),&CreateWGUI##classname##LuaObject);

#define DECLARE_LUA_WGUI_CLASS_INHERIT(gui,guibase) \
	class DLLCLIENT gui##Handle \
		: public guibase##Handle \
	{ \
	public: \
		friend WIBase; \
	protected: \
		gui##Handle(WIBase *gui); \
	}; \
	class DLLCLIENT Ptr##gui##Handle \
		: public Ptr##guibase##Handle \
	{ \
	public: \
		Ptr##gui##Handle(##gui##Handle *hGUI); \
		virtual gui##Handle *get() const; \
	};

#define DECLARE_LUA_WGUI_CLASS(gui) \
	DECLARE_LUA_WGUI_CLASS_INHERIT(gui,WI);

#define DEFINE_LUA_WGUI_CLASS_INHERIT(gui,guibase) \
	LINK_WGUI_TO_HANDLE(gui); \
	gui##Handle::##gui##Handle(WIBase *gui) \
		: guibase##Handle(gui) \
	{} \
	Ptr##gui##Handle::Ptr##gui##Handle(##gui##Handle *hRect) \
		: Ptr##guibase##Handle(hRect) \
	{} \
	gui##Handle *Ptr##gui##Handle::get() const { \
		return static_cast<##gui##Handle*>(Ptr##guibase##Handle::get()); \
	}

#define DEFINE_LUA_WGUI_CLASS(gui) \
	DEFINE_LUA_WGUI_CLASS_INHERIT(gui,WI);

class DLLCLIENT PtrWIHandle
	: public std::shared_ptr<WIHandle>
{
public:
	PtrWIHandle(WIHandle *hGUI);
	virtual WIHandle *get() const;
};

#include "wishape.h"
DECLARE_LUA_WGUI_CLASS(WIShape);
DECLARE_LUA_WGUI_CLASS_INHERIT(WITexturedShape,WIShape);

#include "witext.h"
DECLARE_LUA_WGUI_CLASS(WIText);

#define lua_checkgui(l,hGUI) \
	if(hGUI == NULL || !hGUI->IsValid()) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL GUI Element"); \
		lua_error(l); \
		return; \
	}

#define lua_pushgui(luastate,gui) \
	{ \
		WIHandle *hnd = gui->CreateHandle(); \
		luabind::object(l,std::shared_ptr<WIHandle>(hnd)).push(l); \
	}

lua_registercheck_inherited_ptr(GUIElement,WIHandle,PtrWIHandle);

DLLCLIENT Con::c_cout & operator<<(Con::c_cout &os,const WIHandle &handle);
DLLCLIENT std::ostream& operator<<(std::ostream &os,const WIHandle &handle);*/