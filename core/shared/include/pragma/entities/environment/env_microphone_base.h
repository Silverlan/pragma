#ifndef __ENV_MICROPHONE_BASE_H__
#define __ENV_MICROPHONE_BASE_H__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	class DLLNETWORK BaseEnvMicrophoneComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
	};
};

#endif