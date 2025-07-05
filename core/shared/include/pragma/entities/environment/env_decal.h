// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENV_DECAL_H__
#define __ENV_DECAL_H__

#include "pragma/networkdefinitions.h"
#include <string>

namespace pragma {
	class DLLNETWORK BaseEnvDecalComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;

		virtual void SetSize(float size);
		float GetSize() const;
		virtual void SetMaterial(const std::string &mat);
		const std::string &GetMaterial() const;
		bool GetStartDisabled() const;
	  protected:
		float m_size = 1.f;
		std::string m_material = "";
		bool m_startDisabled = false;
	};
};

#endif
