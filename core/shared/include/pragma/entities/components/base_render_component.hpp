// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_RENDER_COMPONENT_HPP__
#define __BASE_RENDER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/components/render_component_flags.hpp"

namespace pragma {
	class DLLNETWORK BaseRenderComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
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

#endif
