// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.point.base_path_node;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BasePointPathNodeComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;

		BasePointPathNodeComponent *GetNextNode();
		BasePointPathNodeComponent *GetPreviousNode();
		virtual void OnEntitySpawn() override;
	  protected:
		std::string m_kvNextNode;
		util::WeakHandle<BasePointPathNodeComponent> m_nextNode = {};
		util::WeakHandle<BasePointPathNodeComponent> m_previousNode = {};
		void SetPreviousNode(BasePointPathNodeComponent *node);
	};
};
