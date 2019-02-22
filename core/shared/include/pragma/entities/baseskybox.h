#ifndef __BASESKYBOX_H__
#define __BASESKYBOX_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <pragma/console/conout.h>

namespace pragma
{
	class DLLNETWORK BaseSkyboxComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	};
};

#endif
