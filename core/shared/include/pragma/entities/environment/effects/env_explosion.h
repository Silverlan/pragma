#ifndef __ENV_EXPLOSION_H__
#define __ENV_EXPLOSION_H__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	class DLLNETWORK BaseEnvExplosionComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void Explode();
	};
};

#endif