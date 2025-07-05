// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ENV_SOUND_DSP_H__
#define __C_ENV_SOUND_DSP_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/environment/audio/env_sound_dsp.h>
#include <alsound_source.hpp>

namespace pragma {
	class DLLCLIENT CBaseSoundDspComponent : public BaseEnvSoundDspComponent, public CBaseNetComponent {
	  public:
		virtual ~CBaseSoundDspComponent() override;
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual Bool ReceiveNetEvent(UInt32 eventId, NetPacket &p) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;
	  protected:
		CBaseSoundDspComponent(BaseEntity &ent) : BaseEnvSoundDspComponent(ent) {}

		bool m_bAffectRelative = false;
		bool m_bApplyGlobal = false;
		bool m_bAllWorldSounds = false;
		bool m_bAllSounds = false;
		std::shared_ptr<al::IEffect> m_dsp = nullptr;
		std::vector<std::pair<al::SoundSourceHandle, uint32_t>> m_affectedSounds;
		ALSoundType m_types = ALSoundType::All & ~ALSoundType::GUI;

		ALSoundType GetTargetSoundTypes() const;
		std::vector<std::pair<al::SoundSourceHandle, uint32_t>>::iterator FindSoundSource(al::SoundSource &src);
		void UpdateSoundSource(al::SoundSource &src, float gain);
		void DetachSoundSource(al::SoundSource &src);
		void DetachAllSoundSources();
	};

	class DLLCLIENT CSoundDspComponent final : public CBaseSoundDspComponent {
	  public:
		CSoundDspComponent(BaseEntity &ent) : CBaseSoundDspComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CEnvSoundDsp : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
