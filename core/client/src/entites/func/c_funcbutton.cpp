#include "stdafx_client.h"
#include "pragma/entities/func/c_funcbutton.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_button,CFuncButton);

void CButtonComponent::Initialize()
{
	BaseFuncButtonComponent::Initialize();
	auto pRenderComponent = static_cast<CBaseEntity&>(GetEntity()).GetRenderComponent();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderMode(RenderMode::World);
}
luabind::object CButtonComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CButtonComponentHandleWrapper>(l);}

//////////

void CFuncButton::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CButtonComponent>();
}
