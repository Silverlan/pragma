// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.effects.base_sprite;

using namespace pragma;

void BaseEnvSpriteComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "texture", false))
			m_spritePath = kvData.value;
		else if(pragma::string::compare<std::string>(kvData.key, "scale", false))
			m_size = pragma::string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "bloom_scale", false))
			m_bloomScale = pragma::string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "color", false))
			m_color = Color(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "render_mode", false))
			m_particleRenderMode = pragma::string::to_int(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "fade_in_time", false))
			m_tFadeIn = pragma::string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "fade_out_time", false))
			m_tFadeOut = pragma::string::to_float(kvData.value);
		else
			return pragma::util::EventReply::Unhandled;
		return pragma::util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
	ent.AddComponent("transform");
}
