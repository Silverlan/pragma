// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:audio.sound_script_events;

export import :audio.sound;
export import pragma.udm;

#undef CreateEvent

export namespace pragma::audio {
	class SoundScriptEvent;
	class SoundScriptManager;
	class DLLNETWORK SoundScriptEventContainer {
	  protected:
		std::vector<std::shared_ptr<SoundScriptEvent>> m_events;
		SoundScriptManager *m_manager;
		void InitializeEvents(udm::LinkedPropertyWrapper &prop);
	  public:
		SoundScriptEventContainer(SoundScriptManager *manager);
		virtual ~SoundScriptEventContainer();
		std::vector<std::shared_ptr<SoundScriptEvent>> &GetEvents();
		void PrecacheSounds();
		SoundScriptEvent *CreateEvent(std::string name);
		SoundScriptEvent *CreateEvent();
	};

	class DLLNETWORK SoundScriptValue {
	  private:
		float m_min;
		float m_max;
		bool m_bIsSet = false;
		void Initialize(float f);
		void Initialize(float min, float max);
	  public:
		SoundScriptValue(float f);
		SoundScriptValue(float min, float max);
		SoundScriptValue(udm::LinkedPropertyWrapper &prop);
		bool Load(const udm::LinkedPropertyWrapper &prop);
		float GetValue() const;
		bool IsSet() const;
		void SetSet(bool set) { m_bIsSet = set; }
	};

	class DLLNETWORK SSEBase;
	class DLLNETWORK SoundScriptEvent : public SoundScriptEventContainer {
	  public:
		SoundScriptEvent(SoundScriptManager *manager, float off = 0.f, bool bRepeat = false);
		virtual ~SoundScriptEvent();
		virtual void Initialize(udm::LinkedPropertyWrapper &prop);
		SSEBase *CreateEvent(double tStart);
		virtual void Precache();
		SoundScriptValue eventOffset;
		bool repeat;
	};

	class DLLNETWORK SSEBase {
	  public:
		friend SoundScriptEvent;
	  protected:
		SSEBase(SoundScriptEvent *event, double tStart, float eventOffset);
	  public:
		SoundScriptEvent *event;
		double timeCreated;
		float eventOffset;
	};

	class DLLNETWORK SSEPlaySound;
	class DLLNETWORK SSESound : public SSEBase {
	  public:
		friend SSEPlaySound;
	  protected:
		SSESound(std::shared_ptr<ALSound> sound, SSEPlaySound *event, double tStart, float eventOffset);
	  public:
		std::shared_ptr<ALSound> sound;
		ALSound *operator->();
	};

	class DLLNETWORK SSELua : public SoundScriptEvent {
	  public:
		SSELua(SoundScriptManager *manager, float off = 0.f, bool bRepeat = false) : SoundScriptEvent(manager, off, bRepeat) {}
		std::string name;
	};

	class DLLNETWORK SSEPlaySound : public SoundScriptEvent {
	  protected:
		virtual void PrecacheSound(const char *name);
		ALChannel GetChannel();
	  public:
		SSEPlaySound(SoundScriptManager *manager, float off = 0.f, bool bRepeat = false)
		    : SoundScriptEvent(manager, off, bRepeat), gain(1.f), pitch(1.f), offset(0.f), referenceDistance(1.f), rolloffFactor(1.f), minGain(0.f), maxGain(1.f), coneInnerAngle(360.f), coneOuterAngle(360.f), coneOuterGain(0.f), maxDistance(ALSOUND_DEFAULT_MAX_DISTANCE),
		      mode(ALChannel::Auto), startTime(0.f), endTime(0.f), fadeInTime(0.f), fadeOutTime(0.f), type(0.f)
		{
		}
		virtual ~SSEPlaySound() override {}
		std::vector<std::string> sources;
		bool loop = false;
		bool global = false;
		bool stream = false;
		SoundScriptValue type;
		SoundScriptValue gain;
		SoundScriptValue pitch;
		SoundScriptValue offset;
		SoundScriptValue referenceDistance;
		SoundScriptValue rolloffFactor;
		SoundScriptValue minGain;
		SoundScriptValue maxGain;
		SoundScriptValue coneInnerAngle;
		SoundScriptValue coneOuterAngle;
		SoundScriptValue coneOuterGain;
		SoundScriptValue startTime;
		SoundScriptValue endTime;
		SoundScriptValue fadeInTime;
		SoundScriptValue fadeOutTime;
		double maxDistance = ALSOUND_DEFAULT_MAX_DISTANCE;
		int position = 0;
		ALChannel mode = ALChannel::Mono;
		virtual void Initialize(udm::LinkedPropertyWrapper &prop) override;
		virtual SSESound *CreateSound(double tStart, const std::function<std::shared_ptr<ALSound>(const std::string &, ALChannel, ALCreateFlags)> &createSound);
		virtual void Precache() override;
	};
};
