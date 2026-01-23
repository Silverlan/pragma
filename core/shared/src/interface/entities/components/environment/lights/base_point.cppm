// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.lights.base_point;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseEnvLightPointComponent : public BaseEntityComponent {
	  public:
		static Candela CalcIntensityAtPoint(const Vector3 &lightPos, Candela intensity, const Vector3 &point, std::optional<float> radius = {});

		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		float CalcDistanceFalloff(const Vector3 &point) const;
	  protected:
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
	};
};
