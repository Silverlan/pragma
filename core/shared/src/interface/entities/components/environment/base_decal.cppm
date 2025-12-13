// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.base_decal;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseEnvDecalComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
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
