// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.point.base_render_target;

export import :entities.components.base;

export namespace pragma {
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
