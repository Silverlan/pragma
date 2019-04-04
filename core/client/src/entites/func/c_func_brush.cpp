#include "stdafx_client.h"
#include "pragma/entities/func/c_func_brush.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/c_rendermode.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_brush,CFuncBrush);

void CBrushComponent::Initialize()
{
	BaseFuncBrushComponent::Initialize();
	auto pRenderComponent = static_cast<CBaseEntity&>(GetEntity()).GetRenderComponent();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderMode(RenderMode::World);
}
void CBrushComponent::OnEntitySpawn()
{
	BaseFuncBrushComponent::OnEntitySpawn();
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	if(m_kvSolid)
	{
		auto pPhysComponent = ent.GetPhysicsComponent();
		if(pPhysComponent.valid())
			pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
		UpdateSurfaceMaterial(ent.GetNetworkState()->GetGameState());
	}
}
void CBrushComponent::ReceiveData(NetPacket &packet)
{
	m_kvSolid = packet->Read<bool>();
	m_kvSurfaceMaterial = packet->ReadString();
}
luabind::object CBrushComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CBrushComponentHandleWrapper>(l);}

////////

void CFuncBrush::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CBrushComponent>();
}
