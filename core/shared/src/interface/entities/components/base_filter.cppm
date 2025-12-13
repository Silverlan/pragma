// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_filter;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseFilterComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	  protected:
		std::string m_kvFilter;
	  public:
		virtual bool ShouldPass(ecs::BaseEntity &ent) const = 0;
	};

	class DLLNETWORK BaseFilterNameComponent : public BaseFilterComponent {
	  public:
		using BaseFilterComponent::BaseFilterComponent;
		virtual bool ShouldPass(ecs::BaseEntity &ent) const override;
	};

	class DLLNETWORK BaseFilterClassComponent : public BaseFilterComponent {
	  public:
		using BaseFilterComponent::BaseFilterComponent;
		virtual bool ShouldPass(ecs::BaseEntity &ent) const override;
	};
};
