/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/point/point_rendertarget.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

void BasePointRenderTargetComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"material",false))
			m_kvMaterial = kvData.value;
		else if(ustring::compare(kvData.key,"fov",false))
			m_kvFOV = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"refreshrate",false))
			m_kvRefreshRate = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"renderwidth",false))
			m_kvRenderWidth = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"renderheight",false))
			m_kvRenderHeight = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"nearz",false))
			m_kvNearZ = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"farz",false))
			m_kvFarZ = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"depth",false))
			m_kvRenderDepth = util::to_int(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
}
