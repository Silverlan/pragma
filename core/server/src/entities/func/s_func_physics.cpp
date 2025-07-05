// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/func/s_func_physics.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/physics/movetypes.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_physics, FuncPhysics);

void SFuncPhysicsComponent::Initialize()
{
	BaseFuncPhysicsComponent::Initialize();
	if(m_bClientsidePhysics == true)
		static_cast<SBaseEntity &>(GetEntity()).SetSynchronized(false);
}

void SFuncPhysicsComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_kvMass);
	packet->WriteString(m_kvSurfaceMaterial);
}

PhysObj *SFuncPhysicsComponent::InitializePhysics()
{
	if(m_bClientsidePhysics == true)
		return nullptr;
	return BaseFuncPhysicsComponent::InitializePhysics();
}

/////////////

void SFuncPhysicsComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FuncPhysics::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFuncPhysicsComponent>();
}
