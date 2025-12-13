// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

#undef CreateEvent

export module pragma.shared:audio.sound_script_manager;

export import :audio.sound_script_events;

export namespace pragma::audio {
	class SoundScriptManager;
	class DLLNETWORK SoundScript : public SoundScriptEventContainer {
	  public:
		friend SoundScriptManager;
		SoundScript(SoundScriptManager *manager, const std::string &identifier);
		const std::string &GetIdentifier() const;
		virtual ~SoundScript() override;
	  protected:
		std::string m_identifier;
	};

	class DLLNETWORK SoundScriptManager {
	  protected:
		std::unordered_map<std::string, std::shared_ptr<SoundScript>> m_soundScripts;
		std::vector<std::string> m_soundScriptFiles;
		template<class TSoundScript>
		bool Load(const char *fname, std::vector<std::shared_ptr<SoundScript>> *scripts = nullptr)
		{
			return Load(fname, [this](const std::string &name) -> std::shared_ptr<SoundScript> { return util::make_shared<TSoundScript>(this, name); }, scripts);
		}
		bool Load(const char *fname, const std::function<std::shared_ptr<SoundScript>(const std::string &)> fCreateSoundScript, std::vector<std::shared_ptr<SoundScript>> *scripts = nullptr);
	  public:
		static const std::string &GetSoundScriptPath();
		SoundScriptManager();
		virtual ~SoundScriptManager();
		virtual bool Load(const char *fname, std::vector<std::shared_ptr<SoundScript>> *scripts = nullptr);
		SoundScript *FindScript(const char *name);
		void Clear();
		virtual SoundScriptEvent *CreateEvent(std::string name);
		SoundScriptEvent *CreateEvent();
		const std::unordered_map<std::string, std::shared_ptr<SoundScript>> &GetScripts() const;
		const std::vector<std::string> &GetSoundScriptFiles() const;
	};
};
