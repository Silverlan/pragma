#ifndef __BASE_TRIGGER_REMOVE_HPP__
#define __BASE_TRIGGER_REMOVE_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_handle.h"
#include <vector>

namespace pragma
{
	class DLLNETWORK BaseTriggerRemoveComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	};
};

#endif
