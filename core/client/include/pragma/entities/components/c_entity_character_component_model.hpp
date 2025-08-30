// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ENTITY_CHARACTER_COMPONENT_MODEL_HPP__
#define __C_ENTITY_CHARACTER_COMPONENT_MODEL_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

class CBaseEntity;
namespace pragma {
	class DLLCLIENT CEntityCharacterComponentModel : public BaseEntityComponent {
	  public:
		CEntityCharacterComponentModel(CBaseEntity &ent);
		virtual void Initialize() override;
	  protected:
		// COMPONENT TODO
		//virtual void ApplyAnimationBlending(AnimationSlotInfo &animInfo,double tDelta) override;
	};
};

#endif
