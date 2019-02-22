#include "stdafx_client.h"
#include "pragma/entities/components/c_bsp_component.hpp"

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

luabind::object CBSPComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CBSPComponentHandleWrapper>(l);}
void CBSPComponent::Initialize()
{
	BaseEntityComponent::Initialize();

}
void CBSPComponent::InitializeBSPTree(bsp::File &bsp)
{
	// TODO
	//auto bspTree = BSPTree::Create(bsp);

}
