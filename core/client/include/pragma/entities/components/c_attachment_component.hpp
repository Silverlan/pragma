/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ATTACHMENT_COMPONENT_HPP__
#define __C_ATTACHMENT_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_attachment_component.hpp>

namespace pragma {
	class DLLCLIENT CAttachmentComponent final : public BaseAttachmentComponent, public CBaseNetComponent {
	  public:
		CAttachmentComponent(BaseEntity &ent) : BaseAttachmentComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		virtual void ReceiveData(NetPacket &packet) override;
		virtual void UpdateViewAttachmentOffset(BaseEntity *ent, pragma::BaseCharacterComponent &pl, Vector3 &pos, Quat &rot, Bool bYawOnly = false) const override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};

#endif
