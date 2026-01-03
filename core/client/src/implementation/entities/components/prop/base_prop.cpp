// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.prop_base;
using namespace pragma;

void CPropComponent::Initialize()
{
	BasePropComponent::Initialize();

	/*BindEvent(CRenderComponent::EVENT_SHOULD_DRAW,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &shouldDrawData = static_cast<CEShouldDraw&>(evData.get());
		auto &ent = GetEntity();
		auto pTrComponent = ent.GetTransformComponent();
		auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3{};
		if((m_sqrMaxVisibleDist == 0.0 || static_cast<double>(uvec::length_sqr(shouldDrawData.camOrigin -pos)) <= m_sqrMaxVisibleDist) == false)
		{
			shouldDrawData.shouldDraw = CEShouldDraw::ShouldDraw::No;
			return pragma::util::EventReply::Handled;
		}
		return pragma::util::EventReply::Unhandled;
	});
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto physType = UpdatePhysicsType(&ent);
		auto &mdl = static_cast<CEOnModelChanged&>(evData.get()).model;
		if(mdl.get() == nullptr || !ent.IsSpawned() || physType == pragma::physics::PhysicsType::None)
			return;
		BasePropComponent::InitializePhysics(physType);
	});*/
}
void CPropComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CPropComponent::ReceiveData(NetPacket &packet) { m_kvMass = packet->Read<float>(); }

void CPropComponent::OnEntitySpawn()
{
	BasePropComponent::OnEntitySpawn();
	auto physType = UpdatePhysicsType(&GetEntity());
	Setup(physType, m_propMoveType);
}
