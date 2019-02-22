#include "stdafx_shared.h"
#include "pragma/entities/func/basefuncbrush.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/components/base_render_component.hpp"

using namespace pragma;

void BaseFuncBrushComponent::Initialize()
{
	BaseFuncSurfaceMaterialComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"solidity",false))
			m_kvSolid = util::to_boolean(kvData.value);
		else if(ustring::compare(kvData.key,"surface_material",false))
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
		static_cast<BaseRenderComponent*>(whRenderComponent.get())->SetCastShadows(true);
	ent.AddComponent("model");
}
