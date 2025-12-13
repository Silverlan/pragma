// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.triggers.base_teleport;

using namespace pragma;

void BaseTriggerTeleportComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "target", false))
			m_target = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("model");
	ent.AddComponent("touch");
}

util::EventReply BaseTriggerTeleportComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseTouchComponent::EVENT_ON_START_TOUCH) {
		if(m_target.empty())
			return util::EventReply::Unhandled;
		auto &ent = GetEntity();
		auto *game = ent.GetNetworkState()->GetGameState();
		std::vector<ecs::BaseEntity *> targetCandidates;

		ecs::EntityIterator it {*game};
		it.AttachFilter<EntityIteratorFilterEntity>(m_target);
		for(auto *ent : it)
			targetCandidates.push_back(ent);

		if(targetCandidates.empty())
			return util::EventReply::Unhandled;
		auto *entTarget = targetCandidates[math::random(0, targetCandidates.size() - 1)];
		auto ptrTrComponent = ent.GetTransformComponent();
		auto ptrTrComponentTgt = entTarget->GetTransformComponent();
		if(ptrTrComponent && ptrTrComponentTgt) {
			ptrTrComponent->SetPosition(ptrTrComponentTgt->GetPosition());
			if(ent.GetSpawnFlags() & math::to_integral(SpawnFlags::FaceTargetDirectionOnTeleport))
				ptrTrComponent->SetAngles(ptrTrComponentTgt->GetAngles());
		}
	}
	return util::EventReply::Unhandled;
}
