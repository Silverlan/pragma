// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.effects.base_smoke_trail;

using namespace pragma;

void BaseEnvSmokeTrailComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "speed", false))
			m_speed = string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "distance", false))
			m_distance = string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "min_sprite_size", false))
			m_minSpriteSize = string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "max_sprite_size", false))
			m_maxSpriteSize = string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "material", false))
			m_material = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
}
