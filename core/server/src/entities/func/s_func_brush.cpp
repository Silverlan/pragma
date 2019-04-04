#include "stdafx_server.h"
#include "pragma/entities/func/s_func_brush.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_brush,FuncBrush);

extern DLLSERVER SGame *s_game;

void SBrushComponent::Initialize()
{
	BaseFuncBrushComponent::Initialize();
}
void SBrushComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(!m_kvSolid)
		return;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
	UpdateSurfaceMaterial(s_game);
}
void SBrushComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<bool>(m_kvSolid);
	packet->WriteString(m_kvSurfaceMaterial);
}

luabind::object SBrushComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SBrushComponentHandleWrapper>(l);}

void FuncBrush::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SBrushComponent>();
}
