// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_render;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseRenderComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		virtual void SetUnlit(bool b);
		virtual void SetCastShadows(bool b);
		bool IsUnlit() const;
		bool GetCastShadows() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		FRenderFlags m_renderFlags = FRenderFlags::CastShadows;
	};
};
