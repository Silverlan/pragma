#include "stdafx_client.h"
#include "pragma/entities/func/c_func_kinematic.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/util/util_handled.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_kinematic,CFuncKinematic);

void CKinematicComponent::Initialize()
{
	BaseFuncKinematicComponent::Initialize();
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderMode(RenderMode::World);
}

Bool CKinematicComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvStartForward)
		StartForward();
	else if(eventId == m_netEvStartBackward)
		StartBackward();
	else
		return false;
	return true;
}

void CKinematicComponent::OnEntitySpawn()
{
	BaseFuncKinematicComponent::OnEntitySpawn();
	if(m_bInitiallyMoving == true)
		StartForward();
}

void CKinematicComponent::ReceiveData(NetPacket &packet)
{
	m_kvFirstNode = packet->ReadString();
	m_kvMoveSpeed = packet->Read<float>();
	m_bInitiallyMoving = packet->Read<bool>();
}
luabind::object CKinematicComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CKinematicComponentHandleWrapper>(l);}

////////////

void CFuncKinematic::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CKinematicComponent>();
}
