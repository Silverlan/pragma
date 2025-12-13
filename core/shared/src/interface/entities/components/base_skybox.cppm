// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_skybox;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseSkyboxComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;

		virtual void SetSkyAngles(const EulerAngles &ang);
		const EulerAngles &GetSkyAngles() const;
	  protected:
		EulerAngles m_skyAngles = {};
		NetEventId m_netEvSetSkyAngles = INVALID_NET_EVENT;
	};
};
