/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __POINT_PATH_NODE_H__
#define __POINT_PATH_NODE_H__

#include "pragma/networkdefinitions.h"
#include <string>

namespace pragma {
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

#endif
