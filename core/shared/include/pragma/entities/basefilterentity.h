/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __FILTER_ENTITY_BASE_H__
#define __FILTER_ENTITY_BASE_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <string>
#include <vector>

class BaseEntity;
namespace pragma
{
	class DLLNETWORK BaseFilterComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	protected:
		std::string m_kvFilter;
	public:
		virtual bool ShouldPass(BaseEntity &ent) const=0;
	};

	class DLLNETWORK BaseFilterNameComponent
		: public BaseFilterComponent
	{
	public:
		using BaseFilterComponent::BaseFilterComponent;
		virtual bool ShouldPass(BaseEntity &ent) const override;
	};

	class DLLNETWORK BaseFilterClassComponent
		: public BaseFilterComponent
	{
	public:
		using BaseFilterComponent::BaseFilterComponent;
		virtual bool ShouldPass(BaseEntity &ent) const override;
	};
};

#endif
