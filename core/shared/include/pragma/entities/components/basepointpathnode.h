#ifndef __POINT_PATH_NODE_H__
#define __POINT_PATH_NODE_H__

#include "pragma/networkdefinitions.h"
#include <string>

namespace pragma
{
	class DLLNETWORK BasePointPathNodeComponent
		: public BaseEntityComponent
	{
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
