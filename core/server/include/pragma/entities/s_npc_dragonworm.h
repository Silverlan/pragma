#ifndef __S_NPC_DRAGONWORM_H__
#define __S_NPC_DRAGONWORM_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/basenpcdragonworm.h"

namespace pragma
{
	class DLLSERVER SDragonWormComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	};
};

class DLLSERVER NPCDragonWorm
	: public SBaseEntity
{
public:
};

#endif