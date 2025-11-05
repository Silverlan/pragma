// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.attachment;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SAttachmentComponent final : public BaseAttachmentComponent, public SBaseNetComponent {
	  public:
		SAttachmentComponent(pragma::ecs::BaseEntity &ent) : BaseAttachmentComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void SetAttachmentFlags(FAttachmentMode flags) override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
		virtual AttachmentData *SetupAttachment(pragma::ecs::BaseEntity *ent, const AttachmentInfo &attInfo) override;
	};
};
