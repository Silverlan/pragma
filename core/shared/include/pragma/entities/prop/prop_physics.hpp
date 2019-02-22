#ifndef __PROP_PHYSICS_HPP__
#define __PROP_PHYSICS_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include <memory>

class BaseEntity;
namespace pragma
{
	class DLLNETWORK BasePropPhysicsComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	};
};

#endif
