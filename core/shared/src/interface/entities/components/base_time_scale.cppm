// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

#include <cinttypes>
#include <optional>
#include <functional>
#include <memory>
#include "pragma/lua/core.hpp"

export module pragma.shared:entities.components.base_time_scale;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseTimeScaleComponent : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		virtual void Initialize() override;

		virtual void SetTimeScale(float timeScale);
		float GetTimeScale() const;
		const util::PFloatProperty &GetTimeScaleProperty() const;
		float GetEffectiveTimeScale() const;

		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseTimeScaleComponent(pragma::ecs::BaseEntity &ent);
		pragma::NetEventId m_netEvSetTimeScale = pragma::INVALID_NET_EVENT;
		util::PFloatProperty m_timeScale = nullptr;
	};
};
