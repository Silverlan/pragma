#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/wiluaskin.h"
#include <wgui/wibase.h>
#include "pragma/gui/wgui_luainterface.h"
#include <pragma/lua/lua_call.hpp>
#include <queue>

extern DLLCLIENT ClientState *client;
WILuaSkin::WILuaSkin(std::string id)
	: WISkin(id),m_lua(nullptr),m_rootClass(nullptr),m_refVars(-1)
{}
WILuaSkin::~WILuaSkin()
{
	if(m_refVars != -1)
		Lua::ReleaseReference(m_lua,m_refVars);
}

void WILuaSkin::Release(WIBase *el)
{
	WISkin::Release(el);
	std::vector<WIBase*> els;
	WIBase *parent = el;
	while(parent != nullptr)
	{
		els.push_back(parent);
		parent = parent->GetParent();
	}
	std::vector<WISkinClass*> classes = {&m_rootClass};
	for(auto i=els.size() -1;i!=size_t(-1);i--)
	{
		WIBase *el = els[i];
		auto numClasses = classes.size();
		for(size_t k=0;k<numClasses;k++)
			FindSkinClasses(el,classes[k]->classes,classes);
	}
	std::vector<WISkinClass*> elClasses;
	for(unsigned int i=0;i<classes.size();i++)
		FindSkinClasses(el,classes[i]->classes,elClasses);
	for(unsigned int i=0;i<elClasses.size();i++)
	{
		if(elClasses[i]->referenceRelease != -1)
		{
			auto refRelease = elClasses[i]->referenceRelease;
			Lua::CallFunction(m_lua,[this,refRelease,el](lua_State*) {
				Lua::PushRegistryValue(m_lua,refRelease);

				Lua::PushRegistryValue(m_lua,m_refVars);
				//WISkinClass *cl = elClasses[i];
				auto o = WGUILuaInterface::GetLuaObject(m_lua,el);
				o.push(m_lua);
				return Lua::StatusCode::Ok;
			},0);
		}
	}
}

void WILuaSkin::Initialize(WIBase *el)
{
	WISkin::Initialize(el);
	std::vector<WIBase*> els;
	WIBase *parent = el;
	while(parent != nullptr)
	{
		els.push_back(parent);
		parent = parent->GetParent();
	}
	std::vector<WISkinClass*> classes = {&m_rootClass};
	for(auto i=els.size() -1;i!=size_t(-1);i--)
	{
		WIBase *el = els[i];
		auto numClasses = classes.size();
		for(size_t k=0;k<numClasses;k++)
			FindSkinClasses(el,classes[k]->classes,classes);
	}
	std::vector<WISkinClass*> elClasses;
	for(unsigned int i=0;i<classes.size();i++)
		FindSkinClasses(el,classes[i]->classes,elClasses);
	for(unsigned int i=0;i<elClasses.size();i++)
	{
		if(elClasses[i]->referenceInitialize != -1)
		{
			auto refInit = elClasses[i]->referenceInitialize;
			Lua::CallFunction(m_lua,[this,refInit,el](lua_State*) {
				Lua::PushRegistryValue(m_lua,refInit);

				Lua::PushRegistryValue(m_lua,m_refVars);
				//WISkinClass *cl = elClasses[i];
				auto o = WGUILuaInterface::GetLuaObject(m_lua,el);
				o.push(m_lua);
				return Lua::StatusCode::Ok;
			},0);
		}
	}
}
void WILuaSkin::FindSkinClasses(WIBase *el,std::unordered_map<std::string,WISkinClass*> &classes,std::vector<WISkinClass*> &outClasses)
{
	WISkinClass *cl = FindSkinClass(el->GetClass(),classes);
	if(cl != nullptr)
		outClasses.push_back(cl);
	std::vector<std::string> &styleClasses = el->GetStyleClasses();
	for(unsigned int i=0;i<styleClasses.size();i++)
	{
		cl = FindSkinClass(styleClasses[i],classes);
		if(cl != nullptr)
			outClasses.push_back(cl);
	}
}
WISkinClass *WILuaSkin::FindSkinClass(const std::string &className,std::unordered_map<std::string,WISkinClass*> &classes)
{
	std::unordered_map<std::string,WISkinClass*>::iterator it = classes.find(className);
	if(it == classes.end())
		return NULL;
	return it->second;
}

void WILuaSkin::InitializeClasses()
{
	InitializeClasses(m_rootClass);
}

void WILuaSkin::InitializeClasses(WISkinClass &cl)
{
	int table = Lua::GetStackTop(m_lua);
	Lua::PushNil(m_lua); /* 1 */
	while(Lua::GetNextPair(m_lua,table) != 0) /* 2 */
	{
		if(Lua::IsTable(m_lua,-1) && Lua::IsString(m_lua,-2))
		{
			std::string key = Lua::CheckString(m_lua,-2);
			WISkinClass *subClass = new WISkinClass(m_lua);
			cl.classes.insert(std::unordered_map<std::string,WISkinClass*>::value_type(key,subClass));
			Lua::PushString(m_lua,"Initialize"); /* 3 */
			Lua::GetTableValue(m_lua,-2); /* 3 */
			if(Lua::IsFunction(m_lua,-1))
			{
				int fcReference = Lua::CreateReference(m_lua); /* 2 */
				subClass->referenceInitialize = fcReference;
			}
			else
				Lua::Pop(m_lua,1); /* 2 */

			Lua::PushString(m_lua,"Release"); /* 3 */
			Lua::GetTableValue(m_lua,-2); /* 3 */
			if(Lua::IsFunction(m_lua,-1))
			{
				int fcReference = Lua::CreateReference(m_lua); /* 2 */
				subClass->referenceRelease = fcReference;
			}
			else
				Lua::Pop(m_lua,1); /* 2 */

			Lua::PushString(m_lua,"children"); /* 3 */
			Lua::GetTableValue(m_lua,-2); /* 3 */
			if(Lua::IsTable(m_lua,-1))
				InitializeClasses(*subClass); /* 3 */
			Lua::Pop(m_lua,1); /* 2 */
		}
		Lua::Pop(m_lua,1); /* 1 */
	} /* 0 */
}

void WILuaSkin::InitializeBase(WILuaSkin *base)
{
	if(base == nullptr)
		return;
	Lua::PushRegistryValue(m_lua,m_refVars); /* 1 */
	int t = Lua::GetStackTop(m_lua);
	Lua::PushRegistryValue(m_lua,base->m_refVars); /* 2 */
	int tBase = Lua::GetStackTop(m_lua);
	Lua::PushNil(m_lua); /* 3 */
	while(Lua::GetNextPair(m_lua,tBase) != 0) /* 4 */
	{
		Lua::PushValue(m_lua,-2); /* 5 */
		Lua::GetTableValue(m_lua,t); /* 5 */
		if(!Lua::IsSet(m_lua,-1))
		{
			Lua::Pop(m_lua,1); /* 4 */
			Lua::PushValue(m_lua,-2); /* 5 */
			Lua::PushValue(m_lua,-2); /* 6 */
			Lua::SetTableValue(m_lua,t); /* 4 */
		}
		else
			Lua::Pop(m_lua,1); /* 4 */
		Lua::Pop(m_lua,1); /* 3 */
	} /* 2 */
	Lua::Pop(m_lua,2); /* 0 */
	InitializeBaseClass(base->m_rootClass,m_rootClass);
}

void WILuaSkin::InitializeBaseClass(WISkinClass &base,WISkinClass &cl)
{
	std::unordered_map<std::string,WISkinClass*>::iterator it;
	for(it=base.classes.begin();it!=base.classes.end();it++)
	{
		std::unordered_map<std::string,WISkinClass*>::iterator itThis = cl.classes.find(it->first);
		if(itThis == cl.classes.end())
			cl.classes.insert(std::unordered_map<std::string,WISkinClass*>::value_type(it->first,it->second->Copy()));
		else
			InitializeBaseClass(*it->second,*itThis->second);
	}
}

void WILuaSkin::Initialize(lua_State *l,Settings &settings)
{
	m_lua = l;
	m_rootClass.lua = l;
	m_refVars = settings.vars;
	Lua::PushRegistryValue(l,settings.skin); /* 1 */
	InitializeClasses();
	Lua::Pop(l,1); /* 0 */
	Lua::ReleaseReference(l,settings.skin);
	InitializeBase(settings.base);
}

//////////////////////////////////

WISkinClass::WISkinClass(lua_State *l)
	: referenceInitialize(-1),referenceRelease(-1),lua(l)
{}
WISkinClass::~WISkinClass()
{
	if(referenceInitialize != -1)
		Lua::ReleaseReference(lua,referenceInitialize);
	if(referenceRelease != -1)
		Lua::ReleaseReference(lua,referenceRelease);
	std::unordered_map<std::string,WISkinClass*>::iterator it;
	for(it=classes.begin();it!=classes.end();it++)
		delete it->second;
}
WISkinClass *WISkinClass::Copy()
{
	WISkinClass *other = new WISkinClass(lua);

	Lua::PushRegistryValue(lua,referenceInitialize);
	other->referenceInitialize = Lua::CreateReference(lua);

	Lua::PushRegistryValue(lua,referenceRelease);
	other->referenceRelease = Lua::CreateReference(lua);

	std::unordered_map<std::string,WISkinClass*>::iterator it;
	for(it=classes.begin();it!=classes.end();it++)
	{
		WISkinClass *copy = it->second->Copy();
		classes.insert(std::unordered_map<std::string,WISkinClass*>::value_type(it->first,copy));
	}
	return other;
}