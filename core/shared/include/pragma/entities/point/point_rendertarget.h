/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __POINT_RENDERTARGET_H__
#define __POINT_RENDERTARGET_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <string>
#include "pragma/entities/components/basetoggle.h"

namespace pragma
{
	class DLLNETWORK BasePointRenderTargetComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	protected:
		std::string m_kvMaterial;
		float m_kvFOV = 75.f;
		float m_kvRefreshRate = 0.f;
		float m_kvRenderWidth = 1024.f;
		float m_kvRenderHeight = 768.f;
		float m_kvNearZ = 0.f;
		float m_kvFarZ = 0.f;
		int m_kvRenderDepth = 1;
	};
};

#endif
