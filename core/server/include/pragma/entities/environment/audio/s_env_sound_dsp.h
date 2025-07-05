// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ENV_SOUND_DSP_H__
#define __S_ENV_SOUND_DSP_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/audio/env_sound_dsp.h"
#include "pragma/entities/components/s_entity_component.hpp"

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
		SSoundDspComponent(BaseEntity &ent) : SBaseSoundDspComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvSoundDsp : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
