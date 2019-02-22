#ifndef __BASEBOT_H__
#define __BASEBOT_H__

#include "pragma/networkdefinitions.h"

class BaseEntity;
namespace pragma
{
	class DLLNETWORK BaseBotComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	};
};

#endif
