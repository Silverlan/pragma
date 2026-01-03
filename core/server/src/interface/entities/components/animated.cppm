// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.animated;

import :entities.components.entity;

export namespace pragma {
	namespace sAnimatedComponent {
		using namespace baseAnimatedComponent;
	}
	class DLLSERVER SAnimatedComponent final : public BaseAnimatedComponent, public SBaseNetComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		SAnimatedComponent(ecs::BaseEntity &ent) : BaseAnimatedComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;

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
