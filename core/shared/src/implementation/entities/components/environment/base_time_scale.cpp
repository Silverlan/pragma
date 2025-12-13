// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.base_time_scale;

using namespace pragma;

void BaseEnvTimescaleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "timescale", false))
			m_kvTimescale = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "inner_radius", false))
			m_kvInnerRadius = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "outer_radius", false))
			m_kvOuterRadius = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
