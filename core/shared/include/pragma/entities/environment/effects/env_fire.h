#ifndef __ENV_FIRE_H__
#define __ENV_FIRE_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <string>

namespace pragma
{
	class DLLNETWORK BaseEnvFireComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	protected:
		std::string m_fireType = "fire";
	};
};

#endif
