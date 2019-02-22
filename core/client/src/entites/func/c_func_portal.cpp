#include "stdafx_client.h"
#include "pragma/entities/func/c_func_portal.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/scene/camera.h"
#include "pragma/c_engine.h"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_portal,CFuncPortal);

void CFuncPortalComponent::Initialize()
{
	BaseFuncPortalComponent::Initialize();
	auto pRenderComponent = static_cast<CBaseEntity&>(GetEntity()).GetRenderComponent();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderMode(RenderMode::World);
}
luabind::object CFuncPortalComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CFuncPortalComponentHandleWrapper>(l);}

////////////

void CFuncPortal::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFuncPortalComponent>();
}
