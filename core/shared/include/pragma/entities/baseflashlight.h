#ifndef __BASEFLASHLIGHT_H__
#define __BASEFLASHLIGHT_H__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	class DLLNETWORK BaseFlashlightComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	};
};

#endif
