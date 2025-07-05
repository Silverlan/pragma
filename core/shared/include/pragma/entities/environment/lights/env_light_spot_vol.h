// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENV_LIGHT_SPOT_VOL_H__
#define __ENV_LIGHT_SPOT_VOL_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/components/basetoggle.h"
#include <mathutil/color.h>
#include <mathutil/umath.h>
#include <string>

namespace pragma {
	class DLLNETWORK BaseEnvLightSpotVolComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;

		BaseEntity *GetSpotlightTarget() const;

		void SetIntensityFactor(float intensityFactor);
		float GetIntensityFactor() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		virtual void SetSpotlightTarget(BaseEntity &ent);
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		float m_coneStartOffset = 0.f;
		float m_intensityFactor = 1.f;
		EntityHandle m_hSpotlightTarget = {};
		std::string m_kvSpotlightTargetName = "";
		pragma::NetEventId m_netEvSetSpotlightTarget = pragma::INVALID_NET_EVENT;
	};
};

#endif
