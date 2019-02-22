#ifndef __ENV_LIGHT_POINT_H__
#define __ENV_LIGHT_POINT_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <pragma/definitions.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/entities/environment/lights/env_light.h"
#include <string>

namespace pragma
{
	class DLLNETWORK BaseEnvLightPointComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	};
};

#endif