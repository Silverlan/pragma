#include "stdafx_client.h"
#include "pragma/entities/trigger/c_trigger_touch.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(trigger_touch,CTriggerTouch);

extern CGame *c_game;
void CTouchComponent::Initialize()
{
	BaseTouchComponent::Initialize();
}
void CTouchComponent::OnEntitySpawn()
{
	BaseTouchComponent::OnEntitySpawn();
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	if(ent.IsClientsideOnly() == true)
	{
		auto pPhysComponent = ent.GetPhysicsComponent();
		if(pPhysComponent.valid())
			pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
	}
	auto pRenderComponent = static_cast<CBaseEntity&>(GetEntity()).GetRenderComponent();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderMode(RenderMode::World);
}
luabind::object CTouchComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CTouchComponentHandleWrapper>(l);}

/////////////

void CTriggerTouch::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CTouchComponent>();
}
