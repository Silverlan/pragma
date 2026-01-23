// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.lights.base_directional;

export import :entities.components.base;
export import pragma.util;

export namespace pragma {
	class DLLNETWORK BaseEnvLightDirectionalComponent : public BaseEntityComponent {
	  public:
		static Candela CalcIntensityAtPoint(Candela intensity, const Vector3 &point);

		BaseEnvLightDirectionalComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void SetAmbientColor(const Color &color);
		const Color &GetAmbientColor() const;
		const util::PColorProperty &GetAmbientColorProperty() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		util::PColorProperty m_ambientColor = nullptr;
		Float m_maxExposure = 8.f;
		NetEventId m_netEvSetAmbientColor = INVALID_NET_EVENT;
	};
};
