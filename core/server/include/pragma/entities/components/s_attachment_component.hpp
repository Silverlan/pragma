/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_ATTACHMENT_COMPONENT_HPP__
#define __S_ATTACHMENT_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_attachment_component.hpp>

namespace pragma {
	class DLLSERVER SAttachmentComponent final : public BaseAttachmentComponent, public SBaseNetComponent {
	  public:
		SAttachmentComponent(BaseEntity &ent) : BaseAttachmentComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void SetAttachmentFlags(FAttachmentMode flags) override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
		virtual AttachmentData *SetupAttachment(BaseEntity *ent, const AttachmentInfo &attInfo) override;
	};
};

#endif
