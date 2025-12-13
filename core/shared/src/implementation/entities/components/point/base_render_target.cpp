// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.point.base_render_target;

using namespace pragma;

void BasePointRenderTargetComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "material", false))
			m_kvMaterial = kvData.value;
		else if(pragma::string::compare<std::string>(kvData.key, "fov", false))
			m_kvFOV = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "refreshrate", false))
			m_kvRefreshRate = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "renderwidth", false))
			m_kvRenderWidth = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "renderheight", false))
			m_kvRenderHeight = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "nearz", false))
			m_kvNearZ = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "farz", false))
			m_kvFarZ = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "depth", false))
			m_kvRenderDepth = util::to_int(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
}
