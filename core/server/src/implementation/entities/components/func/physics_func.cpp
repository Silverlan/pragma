// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.func.physics;

using namespace pragma;

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

physics::PhysObj *SFuncPhysicsComponent::InitializePhysics()
{
	if(m_bClientsidePhysics == true)
		return nullptr;
	return BaseFuncPhysicsComponent::InitializePhysics();
}

/////////////

void SFuncPhysicsComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FuncPhysics::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFuncPhysicsComponent>();
}
