// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.func.base_brush;

using namespace pragma;

void BaseFuncBrushComponent::Initialize()
{
	BaseFuncSurfaceMaterialComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "solidity", false))
			m_kvSolid = util::to_boolean(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "surface_material", false))
			m_kvSurfaceMaterial = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("physics");
	ent.AddComponent("name");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent *>(whRenderComponent.get())->SetCastShadows(true);
	ent.AddComponent("model");
}
