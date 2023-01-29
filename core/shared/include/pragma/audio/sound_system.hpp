/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SOUND_SYSTEM_HPP__
#define __SOUND_SYSTEM_HPP__

#include "pragma/clientdefinitions.h"
#include <sharedutils/util_shared_handle.hpp>
#include <mathutil/uvec.h>

namespace pragma::audio {
	class IListener;
	class DLLCLIENT ISoundSystem {
	  public:
		enum class GlobalEffectFlag : uint32_t { None = 0, RelativeSounds = 1, WorldSounds = RelativeSounds << 1, All = RelativeSounds | WorldSounds };
		enum class Type : uint32_t { Buffer = 0, Decoder = 1 };

		static std::shared_ptr<ISoundSystem> Create(const std::string &deviceName, float metersPerUnit = 1.f);
		static std::shared_ptr<ISoundSystem> Create(float metersPerUnit = 1.f);
		virtual ~ISoundSystem() = default;
	  public:
		template<class TEfxProperties>
		PEffect CreateEffect(const TEfxProperties &props);
		AuxiliaryEffectSlot *CreateAuxiliaryEffectSlot();
		void FreeAuxiliaryEffectSlot(AuxiliaryEffectSlot *slot);

		// It's the caller's responsibility to destroy all shared pointer instances before the sound system is destroyed
		PSoundSource CreateSource(SoundBuffer &buffer);
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
		PSoundSource CreateSource(Decoder &decoder);
#endif
		PSoundSource CreateSource(const std::string &name, bool bStereo, Type type = Type::Buffer);
		const std::vector<PSoundSource> &GetSources() const;
		std::vector<PSoundSource> &GetSources();
		void StopSounds();

		std::vector<SoundBuffer *> GetBuffers() const;

		const IListener &GetListener() const;
		IListener &GetListener();

		SoundBuffer *LoadSound(const std::string &path, bool bConvertToMono = false, bool bAsync = true);
		SoundBuffer *GetBuffer(const std::string &path, bool bStereo = true);
		PDecoder CreateDecoder(const std::string &path, bool bConvertToMono = false);

		bool IsSupported(ChannelConfig channels, SampleType type) const;

		float GetDopplerFactor() const;
		void SetDopplerFactor(float factor);

		float GetSpeedOfSound() const;
		void SetSpeedOfSound(float speed);

		DistanceModel GetDistanceModel() const;
		void SetDistanceModel(DistanceModel mdl);

		std::string GetDeviceName() const;
		void PauseDeviceDSP();
		void ResumeDeviceDSP();

		// HRTF
		std::vector<std::string> GetHRTFNames() const;
		std::string GetCurrentHRTF() const;
		bool IsHRTFEnabled() const;
		void SetHRTF(uint32_t id);
		void DisableHRTF();

		uint32_t AddGlobalEffect(Effect &effect, GlobalEffectFlag flags = GlobalEffectFlag::All, const Effect::Params &params = Effect::Params());
		void RemoveGlobalEffect(Effect &effect);
		void RemoveGlobalEffect(uint32_t slotId);
		void SetGlobalEffectParameters(Effect &effect, const Effect::Params &params);
		void SetGlobalEffectParameters(uint32_t slotId, const Effect::Params &params);
		uint32_t GetMaxAuxiliaryEffectsPerSource() const;

		void SetSoundSourceFactory(std::unique_ptr<SoundSourceFactory> factory);

		void Update();

		uint32_t GetAudioFrameSampleCount() const;
		void SetAudioFrameSampleCount(uint32_t size);
	  private:
		IListener m_listener;
		std::vector<PSoundSource> m_sources;

		struct BufferCache {
			PSoundBuffer mono = nullptr;
			PSoundBuffer stereo = nullptr;
			bool fileSourceMonoOnly = false;
		};
		std::unordered_map<std::string, BufferCache> m_buffers;
		std::vector<EffectHandle> m_effects;
		std::vector<PAuxiliaryEffectSlot> m_effectSlots;
		std::unique_ptr<SoundSourceFactory> m_soundSourceFactory = nullptr;

		uint32_t m_audioFrameSampleCount = 1'024;

		struct DLLCLIENT GlobalEffect {
			struct DLLCLIENT SoundInfo {
				~SoundInfo();
				SoundSourceHandle source = {};
				CallbackHandle relativeCallback = {};
				uint32_t slotId = std::numeric_limits<uint32_t>::max();
			};
			Effect *effect = nullptr;
			GlobalEffectFlag flags = GlobalEffectFlag::None;
			Effect::Params params = {};
			std::vector<std::shared_ptr<SoundInfo>> sourceInfo;
		};
		std::uint32_t m_nextGlobalEffectId = 0;
		std::queue<uint32_t> m_freeGlobalEffectIds;
		std::unordered_map<uint32_t, GlobalEffect> m_globalEffects;
		PSoundSource InitializeSource(SoundSource *source);
		void RemoveGlobalEffect(GlobalEffect &globalEffect);
		void SetGlobalEffectParameters(GlobalEffect &globalEffect, const Effect::Params &params);
		void ApplyGlobalEffect(SoundSource &source, GlobalEffect &globalEffect);
		void ApplyGlobalEffects(SoundSource &source);
		PEffect CreateEffect();

		float m_speedOfSound = 343.3f;
		float m_dopplerFactor = 1.f;
		DistanceModel m_distanceModel = DistanceModel::LinearClamped;

		// Steam Audio
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
		bool m_bSteamAudioEnabled = false;
		std::shared_ptr<ipl::Context> m_iplContext = nullptr;
		std::shared_ptr<ipl::Scene> m_iplScene = nullptr;
		std::unordered_map<std::string, std::shared_ptr<ipl::AudioDataBuffer>> m_audioBuffers;
#endif
	};
};
#pragma warning(pop)

#endif
