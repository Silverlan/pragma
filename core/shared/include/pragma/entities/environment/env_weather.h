#ifndef __ENV_WEATHER_H__
#define __ENV_WEATHER_H__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	class DLLNETWORK BaseEnvWeatherComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
	};
};

#endif
