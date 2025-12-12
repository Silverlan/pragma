// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.point.base_render_target;

using namespace pragma;

void BasePointRenderTargetComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "material", false))
			m_kvMaterial = kvData.value;
		else if(pragma::string::compare<std::string>(kvData.key, "fov", false))
			m_kvFOV = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "refreshrate", false))
			m_kvRefreshRate = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "renderwidth", false))
			m_kvRenderWidth = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "renderheight", false))
			m_kvRenderHeight = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "nearz", false))
			m_kvNearZ = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "farz", false))
			m_kvFarZ = pragma::util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "depth", false))
			m_kvRenderDepth = pragma::util::to_int(kvData.value);
		else
			return pragma::util::EventReply::Unhandled;
		return pragma::util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
}
