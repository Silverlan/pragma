/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_RENDER_COMPONENT_HPP__
#define __BASE_RENDER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/components/render_component_flags.hpp"

namespace pragma
{
	class DLLNETWORK BaseRenderComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void SetUnlit(bool b);
		virtual void SetCastShadows(bool b);
		bool IsUnlit() const;
		bool GetCastShadows() const;

		virtual void Save(udm::LinkedPropertyWrapper &udm) override;
		virtual void Load(udm::LinkedPropertyWrapper &udm,uint32_t version) override;
	protected:
		FRenderFlags m_renderFlags = FRenderFlags::CastShadows;
	};
};

#endif
