// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.audio.base_sound_scape;

using namespace pragma;

void BaseEnvSoundScapeComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "soundscape", false))
			m_kvSoundScape = kvData.value;
		else if(pragma::string::compare<std::string>(kvData.key, "radius", false))
			m_kvRadius = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "position", false)) {
			auto id = pragma::util::to_int(kvData.key.substr(8, kvData.key.length()).c_str());
			m_positions.insert(std::unordered_map<unsigned int, std::string>::value_type(id, kvData.value));
		}
		else
			return pragma::util::EventReply::Unhandled;
		return pragma::util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
}
