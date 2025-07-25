// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENV_FOG_CONTROLLER_H__
#define __ENV_FOG_CONTROLLER_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/util/util_game.hpp"
#include <mathutil/color.h>

namespace pragma {
	class DLLNETWORK BaseEnvFogControllerComponent : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;

		virtual void SetFogStart(float start);
		virtual void SetFogEnd(float end);
		virtual void SetMaxDensity(float density);
		virtual void SetFogType(util::FogType type);

		float GetFogStart() const { return m_kvFogStart; }
		float GetFogEnd() const { return m_kvFogEnd; }
		float GetMaxDensity() const { return m_kvMaxDensity; }
		util::FogType GetFogType() const { return m_kvFogType; }
	  protected:
		float m_kvFogStart = 500.f;
		float m_kvFogEnd = 2'000.f;
		float m_kvMaxDensity = 1.f;
		util::FogType m_kvFogType = util::FogType::Linear;
	};
};

#endif
