// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.audio.dsp.base;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SBaseSoundDspComponent : public BaseEnvSoundDspComponent, public SBaseNetComponent {
		  public:
			virtual void Initialize() override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void SetGain(float gain) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		  protected:
			using BaseEnvSoundDspComponent::BaseEnvSoundDspComponent;
		};

		class DLLSERVER SSoundDspComponent final : public SBaseSoundDspComponent {
		  public:
			SSoundDspComponent(ecs::BaseEntity &ent) : SBaseSoundDspComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER EnvSoundDsp : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
