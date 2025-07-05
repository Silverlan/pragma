// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/environment/effects/env_sprite.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <algorithm>

using namespace pragma;

void BaseEnvSpriteComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "texture", false))
			m_spritePath = kvData.value;
		else if(ustring::compare<std::string>(kvData.key, "scale", false))
			m_size = ustring::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "bloom_scale", false))
			m_bloomScale = ustring::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "color", false))
			m_color = Color(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "render_mode", false))
			m_particleRenderMode = ustring::to_int(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "fade_in_time", false))
			m_tFadeIn = ustring::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "fade_out_time", false))
			m_tFadeOut = ustring::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
	ent.AddComponent("transform");
}
