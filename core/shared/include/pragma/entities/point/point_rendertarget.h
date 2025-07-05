// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __POINT_RENDERTARGET_H__
#define __POINT_RENDERTARGET_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <string>
#include "pragma/entities/components/basetoggle.h"

namespace pragma {
	class DLLNETWORK BasePointRenderTargetComponent : public BaseEntityComponent {
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
