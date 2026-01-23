// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.lights.base_spot_vol;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseEnvLightSpotVolComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;

		ecs::BaseEntity *GetSpotlightTarget() const;

		void SetIntensityFactor(float intensityFactor);
		float GetIntensityFactor() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		virtual void SetSpotlightTarget(ecs::BaseEntity &ent);
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		float m_coneStartOffset = 0.f;
		float m_intensityFactor = 1.f;
		EntityHandle m_hSpotlightTarget = {};
		std::string m_kvSpotlightTargetName = "";
		NetEventId m_netEvSetSpotlightTarget = INVALID_NET_EVENT;
	};
};
