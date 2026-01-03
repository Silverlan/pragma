// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:audio.sound_script;

export import pragma.shared;
export import pragma.soundsystem;

#undef CreateEvent

export namespace pragma::audio {
	class DLLCLIENT CSSEPlaySound : public SSEPlaySound {
	  protected:
		std::shared_ptr<IEffect> m_dspEffect = nullptr;
		virtual void PrecacheSound(const char *name) override;
	  public:
		CSSEPlaySound(SoundScriptManager *manager);
		std::vector<std::shared_ptr<IEffect>> effects;
		virtual void Initialize(udm::LinkedPropertyWrapper &prop) override;
		virtual SSESound *CreateSound(double tStart, const std::function<std::shared_ptr<ALSound>(const std::string &, ALChannel, ALCreateFlags)> &createSound) override;
	};

	class DLLCLIENT CSoundScriptManager : public SoundScriptManager {
	  public:
		CSoundScriptManager();
		virtual ~CSoundScriptManager() override;
		virtual bool Load(const char *fname, std::vector<std::shared_ptr<SoundScript>> *scripts = nullptr) override;
		virtual SoundScriptEvent *CreateEvent(std::string name) override;
	};

	class DLLCLIENT CSoundScript : public SoundScript {
	  public:
		friend SoundScriptManager;
		friend CSoundScriptManager;
		CSoundScript(SoundScriptManager *manager, std::string identifier);
		virtual ~CSoundScript() override;
	};
};
