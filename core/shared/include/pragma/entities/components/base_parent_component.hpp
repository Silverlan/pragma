/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_PARENT_COMPONENT_HPP__
#define __BASE_PARENT_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

struct AttachmentData;
namespace pragma
{
	class BaseAttachableComponent;
	class DLLNETWORK BaseParentComponent
		: public BaseEntityComponent
	{
	public:
		virtual void Initialize() override;

		void UpdateChildAttachmentData();

		const std::vector<pragma::ComponentHandle<BaseAttachableComponent>> &GetChildren() const;
		std::vector<pragma::ComponentHandle<BaseAttachableComponent>> &GetChildren();
	protected:
		friend BaseAttachableComponent;
		BaseParentComponent(BaseEntity &ent);
		void AddChild(BaseAttachableComponent &ent);
		void RemoveChild(BaseAttachableComponent &ent);

		std::vector<pragma::ComponentHandle<BaseAttachableComponent>> m_children = {};
	};
};

#endif
