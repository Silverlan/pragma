// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.triggers.base_teleport;

using namespace pragma;

void BaseTriggerTeleportComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "target", false))
			m_target = kvData.value;
		else
			return pragma::util::EventReply::Unhandled;
		return pragma::util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("model");
	ent.AddComponent("touch");
}

pragma::util::EventReply BaseTriggerTeleportComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == pragma::util::EventReply::Handled)
		return pragma::util::EventReply::Handled;
	if(eventId == baseTouchComponent::EVENT_ON_START_TOUCH) {
		if(m_target.empty())
			return pragma::util::EventReply::Unhandled;
		auto &ent = GetEntity();
		auto *game = ent.GetNetworkState()->GetGameState();
		std::vector<pragma::ecs::BaseEntity *> targetCandidates;

		pragma::ecs::EntityIterator it {*game};
		it.AttachFilter<EntityIteratorFilterEntity>(m_target);
		for(auto *ent : it)
			targetCandidates.push_back(ent);

		if(targetCandidates.empty())
			return pragma::util::EventReply::Unhandled;
		auto *entTarget = targetCandidates[pragma::math::random(0, targetCandidates.size() - 1)];
		auto ptrTrComponent = ent.GetTransformComponent();
		auto ptrTrComponentTgt = entTarget->GetTransformComponent();
		if(ptrTrComponent && ptrTrComponentTgt) {
			ptrTrComponent->SetPosition(ptrTrComponentTgt->GetPosition());
			if(ent.GetSpawnFlags() & pragma::math::to_integral(SpawnFlags::FaceTargetDirectionOnTeleport))
				ptrTrComponent->SetAngles(ptrTrComponentTgt->GetAngles());
		}
	}
	return pragma::util::EventReply::Unhandled;
}
