// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __FILTER_ENTITY_BASE_H__
#define __FILTER_ENTITY_BASE_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <string>
#include <vector>

class BaseEntity;
namespace pragma {
	class DLLNETWORK BaseFilterComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	  protected:
		std::string m_kvFilter;
	  public:
		virtual bool ShouldPass(BaseEntity &ent) const = 0;
	};

	class DLLNETWORK BaseFilterNameComponent : public BaseFilterComponent {
	  public:
		using BaseFilterComponent::BaseFilterComponent;
		virtual bool ShouldPass(BaseEntity &ent) const override;
	};

	class DLLNETWORK BaseFilterClassComponent : public BaseFilterComponent {
	  public:
		using BaseFilterComponent::BaseFilterComponent;
		virtual bool ShouldPass(BaseEntity &ent) const override;
	};
};

#endif
