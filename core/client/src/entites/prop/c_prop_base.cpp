/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/prop/c_prop_base.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

void CPropComponent::Initialize()
{
	BasePropComponent::Initialize();

	/*BindEvent(CRenderComponent::EVENT_SHOULD_DRAW,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &shouldDrawData = static_cast<CEShouldDraw&>(evData.get());
		auto &ent = GetEntity();
		auto pTrComponent = ent.GetTransformComponent();
		auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3{};
		if((m_sqrMaxVisibleDist == 0.0 || static_cast<double>(uvec::length_sqr(shouldDrawData.camOrigin -pos)) <= m_sqrMaxVisibleDist) == false)
		{
			shouldDrawData.shouldDraw = CEShouldDraw::ShouldDraw::No;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto physType = UpdatePhysicsType(&ent);
		auto &mdl = static_cast<CEOnModelChanged&>(evData.get()).model;
		if(mdl.get() == nullptr || !ent.IsSpawned() || physType == PHYSICSTYPE::NONE)
			return;
		BasePropComponent::InitializePhysics(physType);
	});*/
}
luabind::object CPropComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CPropComponentHandleWrapper>(l);}
void CPropComponent::ReceiveData(NetPacket &packet)
{
	m_kvMass = packet->Read<float>();
}

void CPropComponent::OnEntitySpawn()
{
	BasePropComponent::OnEntitySpawn();
	auto physType = UpdatePhysicsType(&GetEntity());
	BasePropComponent::Setup(physType,m_propMoveType);
}
