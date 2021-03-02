/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENTITY_CHARACTER_COMPONENT_MODEL_HPP__
#define __C_ENTITY_CHARACTER_COMPONENT_MODEL_HPP__

#include <pragma/entities/components/base_entity_component.hpp>

class CBaseEntity;
namespace pragma
{
	class DLLCLIENT CEntityCharacterComponentModel
		: public BaseEntityComponent
	{
	public:
		CEntityCharacterComponentModel(CBaseEntity &ent);
		virtual void Initialize() override;
	protected:
		// COMPONENT TODO
		//virtual void ApplyAnimationBlending(AnimationSlotInfo &animInfo,double tDelta) override;
	};
};

#endif
