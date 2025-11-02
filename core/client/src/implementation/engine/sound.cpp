// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/logging.hpp"


module pragma.client;


import :engine;
import :audio;


const al::ISoundSystem *CEngine::GetSoundSystem() const { return const_cast<CEngine *>(this)->GetSoundSystem(); }
al::ISoundSystem *CEngine::GetSoundSystem() { return m_soundSystem.get(); }

al::ISoundSystem *CEngine::InitializeSoundEngine()
{
	spdlog::info("Initializing sound engine...");

	auto &audioAPI = GetAudioAPI();
	auto getAudioApiPath = [](const std::string &audioAPI, std::string &outLocation, std::string &outModulePath) {
		outLocation = pragma::audio::get_audio_api_module_location(audioAPI);
		outModulePath = util::get_normalized_module_path(outLocation);
	};
	auto loadAudioApiModule = [this, &getAudioApiPath](const std::string &renderAPI, std::string &outErr) -> bool {
		std::string location;
		std::string modulePath;
		getAudioApiPath(renderAPI, location, modulePath);
		m_audioAPILib = util::load_library_module(modulePath, util::get_default_additional_library_search_directories(modulePath), {}, &outErr);
		return (m_audioAPILib != nullptr);
	};
	std::string err;
	if(loadAudioApiModule(audioAPI, err) == false) {
		spdlog::warn("Failed to load default audio engine '{}': {}", audioAPI, err);
		// Fallback 1
		if(loadAudioApiModule("soloud", err) == false) {
			spdlog::warn("Failed to load 'soloud' fallback audio engine: ", err);
			// Fallback 2
			if(!loadAudioApiModule("dummy", err))
				spdlog::warn("Failed to load 'dummy' fallback audio engine: ", err);
		}
	}
	if(!m_audioAPILib)
		Con::crit << "No valid audio module found!" << Con::endl;
	auto lib = m_audioAPILib;
	std::string location;
	std::string modulePath;
	getAudioApiPath(audioAPI, location, modulePath);

	if(lib != nullptr) {
		spdlog::info("Loading audio module '{}'...", location);
		auto fInitAudioAPI = lib->FindSymbolAddress<bool (*)(float, std::shared_ptr<al::ISoundSystem> &, std::string &)>("initialize_audio_api");
		if(fInitAudioAPI == nullptr)
			err = "Symbol 'initialize_audio_api' not found in library '" + location + "'!";
		else {
			std::string errMsg;
			auto success = fInitAudioAPI(pragma::units_to_metres(1.f), m_soundSystem, errMsg);
			if(success == false)
				err = errMsg;
		}
	}
	else
		err = "Module '" + modulePath + "' not found!";
	if(m_soundSystem == nullptr)
		throw std::runtime_error {"Unable to load audio implementation library: " + err + "!"};
	m_soundSystem->SetSoundSourceFactory([](const al::PSoundChannel &channel) -> al::PSoundSource {
		return std::shared_ptr<CALSound> {new CALSound {pragma::get_cengine()->GetClientState(), channel}, [](CALSound *snd) {
			                                  snd->OnRelease();
			                                  delete snd;
		                                  }};
	});
	al::set_world_scale(pragma::units_to_metres(1.0));
	return m_soundSystem.get();
}

al::PEffect CEngine::GetAuxEffect(const std::string &name)
{
	auto lname = name;
	ustring::to_lower(lname);
	auto it = m_auxEffects.find(lname);
	if(it == m_auxEffects.end())
		return nullptr;
	return it->second;
}

void CEngine::CloseSoundEngine() { m_soundSystem = nullptr; }

void CEngine::SetHRTFEnabled(bool b)
{
	auto *soundSys = GetSoundSystem();
	if(soundSys == nullptr)
		return;
	if(b == false)
		soundSys->DisableHRTF();
	else {
		soundSys->SetHRTF(0);
		if(soundSys->IsHRTFEnabled() == false)
			Con::cwar << "Unable to activate HRTF. Please make sure *.mhr-file is in correct directory!" << Con::endl;
	}
}
