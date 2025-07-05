// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ANIMATED_COMPONENT_HPP__
#define __S_ANIMATED_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_animated_component.hpp>

namespace pragma {
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

#endif
