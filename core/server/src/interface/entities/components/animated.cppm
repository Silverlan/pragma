// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"


export module pragma.server.entities.components.animated;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SAnimatedComponent final : public BaseAnimatedComponent, public SBaseNetComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		SAnimatedComponent(BaseEntity &ent) : BaseAnimatedComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;

		using BaseAnimatedComponent::PlayAnimation;
		using BaseAnimatedComponent::PlayLayeredAnimation;
		using BaseAnimatedComponent::StopLayeredAnimation;
		virtual void PlayAnimation(int animation, FPlayAnim flags = FPlayAnim::Default) override;
		virtual void StopLayeredAnimation(int slot) override;
		virtual void PlayLayeredAnimation(int slot, int animation, FPlayAnim flags = FPlayAnim::Default) override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};
