#include "lchromium.hpp"
#include "wiweb.hpp"
#include "wvmodule.hpp"
#include "wiweb.hpp"
#include <luasystem.h>
#include <wiluahandlewrapper.h>
#include <luainterface.hpp>

#define lua_checkgui(l,hGUI) \
	if(!hGUI.IsValid()) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL GUI Element"); \
		lua_error(l); \
		return; \
	}

namespace Lua
{
	namespace WIWeb
	{
		static void LoadURL(lua_State *l,WIWebHandle &hPanel,const std::string &url);
		static void SetBrowserViewSize(lua_State *l,WIWebHandle &hPanel,const Vector2i &size);
		static void GetBrowserViewSize(lua_State *l,WIWebHandle &hPanel);

		static void CanGoBack(lua_State *l,WIWebHandle &hPanel);
		static void CanGoForward(lua_State *l,WIWebHandle &hPanel);
		static void GoBack(lua_State *l,WIWebHandle &hPanel);
		static void GoForward(lua_State *l,WIWebHandle &hPanel);
		static void HasDocument(lua_State *l,WIWebHandle &hPanel);
		static void IsLoading(lua_State *l,WIWebHandle &hPanel);
		static void Reload(lua_State *l,WIWebHandle &hPanel);
		static void ReloadIgnoreCache(lua_State *l,WIWebHandle &hPanel);
		static void StopLoad(lua_State *l,WIWebHandle &hPanel);
		static void Copy(lua_State *l,WIWebHandle &hPanel);
		static void Cut(lua_State *l,WIWebHandle &hPanel);
		static void Delete(lua_State *l,WIWebHandle &hPanel);
		static void Paste(lua_State *l,WIWebHandle &hPanel);
		static void Redo(lua_State *l,WIWebHandle &hPanel);
		static void SelectAll(lua_State *l,WIWebHandle &hPanel);
		static void Undo(lua_State *l,WIWebHandle &hPanel);

		static void SetZoomLevel(lua_State *l,WIWebHandle &hPanel,float lv);
		static void GetZoomLevel(lua_State *l,WIWebHandle &hPanel);
	};
};

static void register_wiweb_class(Lua::Interface &l)
{
	auto &modGUI = l.RegisterLibrary("gui");
	auto classDefWeb = luabind::class_<WIWebHandle COMMA luabind::bases<WITexturedShapeHandle COMMA WIShapeHandle COMMA WIHandle>>("Web");
	classDefWeb.def("LoadURL",&Lua::WIWeb::LoadURL);
	classDefWeb.def("SetBrowserViewSize",&Lua::WIWeb::SetBrowserViewSize);
	classDefWeb.def("GetBrowserViewSize",&Lua::WIWeb::GetBrowserViewSize);

	classDefWeb.def("CanGoBack",&Lua::WIWeb::CanGoBack);
	classDefWeb.def("CanGoForward",&Lua::WIWeb::CanGoForward);
	classDefWeb.def("GoBack",&Lua::WIWeb::GoBack);
	classDefWeb.def("GoForward",&Lua::WIWeb::GoForward);
	classDefWeb.def("HasDocument",&Lua::WIWeb::HasDocument);
	classDefWeb.def("IsLoading",&Lua::WIWeb::IsLoading);
	classDefWeb.def("Reload",&Lua::WIWeb::Reload);
	classDefWeb.def("ReloadIgnoreCache",&Lua::WIWeb::ReloadIgnoreCache);
	classDefWeb.def("StopLoad",&Lua::WIWeb::StopLoad);
	classDefWeb.def("Copy",&Lua::WIWeb::Copy);
	classDefWeb.def("Cut",&Lua::WIWeb::Cut);
	classDefWeb.def("Delete",&Lua::WIWeb::Delete);
	classDefWeb.def("Paste",&Lua::WIWeb::Paste);
	classDefWeb.def("Redo",&Lua::WIWeb::Redo);
	classDefWeb.def("SelectAll",&Lua::WIWeb::SelectAll);
	classDefWeb.def("Undo",&Lua::WIWeb::Undo);

	classDefWeb.def("SetZoomLevel",&Lua::WIWeb::SetZoomLevel);
	classDefWeb.def("GetZoomLevel",&Lua::WIWeb::GetZoomLevel);
	modGUI[classDefWeb];
}

void Lua::chromium::register_library(Lua::Interface &l)
{
	static auto bGuiFactoryInitialized = false;
	if(bGuiFactoryInitialized == false)
	{
		bGuiFactoryInitialized = true;
		IState::add_gui_lua_wrapper_factory([](lua_State *l,WIBase *p) -> luabind::object* {
			if(typeid(*p) == typeid(::WIWeb))
				return new luabind::object(l,WIWebHandle(WITexturedShapeHandle(WIShapeHandle(p->GetHandle()))));
			return nullptr;
		});
	}
	register_wiweb_class(l);
	/*Lua::RegisterLibrary(lua,"xml",{
		{"parse",Lua::xml::lib::parse},
		{"load",Lua::xml::lib::load},
		{"create",Lua::xml::lib::create}
	});*/
}

void Lua::WIWeb::LoadURL(lua_State *l,WIWebHandle &hPanel,const std::string &url)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->LoadURL(url);
}

void Lua::WIWeb::SetBrowserViewSize(lua_State *l,WIWebHandle &hPanel,const Vector2i &size)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->SetBrowserViewSize(size);
}
void Lua::WIWeb::GetBrowserViewSize(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::Push<Vector2i>(l,static_cast<::WIWeb*>(hPanel.get())->GetBrowserViewSize());
}
void Lua::WIWeb::CanGoBack(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,static_cast<::WIWeb*>(hPanel.get())->CanGoBack());
}
void Lua::WIWeb::CanGoForward(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,static_cast<::WIWeb*>(hPanel.get())->CanGoForward());
}
void Lua::WIWeb::GoBack(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->GoBack();
}
void Lua::WIWeb::GoForward(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->GoForward();
}
void Lua::WIWeb::HasDocument(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,static_cast<::WIWeb*>(hPanel.get())->HasDocument());
}
void Lua::WIWeb::IsLoading(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,static_cast<::WIWeb*>(hPanel.get())->IsLoading());
}
void Lua::WIWeb::Reload(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->Reload();
}
void Lua::WIWeb::ReloadIgnoreCache(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->ReloadIgnoreCache();
}
void Lua::WIWeb::StopLoad(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->StopLoad();
}
void Lua::WIWeb::Copy(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->Copy();
}
void Lua::WIWeb::Cut(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->Cut();
}
void Lua::WIWeb::Delete(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->Delete();
}
void Lua::WIWeb::Paste(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->Paste();
}
void Lua::WIWeb::Redo(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->Redo();
}
void Lua::WIWeb::SelectAll(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->SelectAll();
}
void Lua::WIWeb::Undo(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->Undo();
}

void Lua::WIWeb::SetZoomLevel(lua_State *l,WIWebHandle &hPanel,float lv)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIWeb*>(hPanel.get())->SetZoomLevel(lv);
}
void Lua::WIWeb::GetZoomLevel(lua_State *l,WIWebHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushNumber(l,static_cast<::WIWeb*>(hPanel.get())->GetZoomLevel());
}
