// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.effects.base_smoke_trail;

using namespace pragma;

void BaseEnvSmokeTrailComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "speed", false))
			m_speed = pragma::string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "distance", false))
			m_distance = pragma::string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "min_sprite_size", false))
			m_minSpriteSize = pragma::string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "max_sprite_size", false))
			m_maxSpriteSize = pragma::string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "material", false))
			m_material = kvData.value;
		else
			return pragma::util::EventReply::Unhandled;
		return pragma::util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
}
