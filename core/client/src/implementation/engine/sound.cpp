// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :engine;
import :audio;

const pragma::audio::ISoundSystem *pragma::CEngine::GetSoundSystem() const { return const_cast<CEngine *>(this)->GetSoundSystem(); }
pragma::audio::ISoundSystem *pragma::CEngine::GetSoundSystem() { return m_soundSystem.get(); }

static std::pair<std::string, std::string> get_audio_api_path(std::string_view audioAPI)
{
	auto location = pragma::audio::get_audio_api_module_location(std::string {audioAPI});
	auto modulePath = pragma::util::get_normalized_module_path(location);
	return {std::move(location), std::move(modulePath)};
}

static std::expected<std::shared_ptr<pragma::util::Library>, std::string> load_audio_library_module(std::string_view audioAPI, std::string_view modulePath)
{
	std::string err;
	auto lib = pragma::util::load_library_module(std::string {modulePath}, pragma::util::get_default_additional_library_search_directories(std::string {modulePath}), {}, &err);
	if(!lib)
		return std::unexpected {err};
	return lib;
}

struct SoundSystemInfo {
	std::shared_ptr<pragma::audio::ISoundSystem> soundSystem;
	std::shared_ptr<pragma::util::Library> library;
};
static std::expected<SoundSystemInfo, std::string> load_audio_library(std::string_view audioAPI)
{
	auto [location, modulePath] = get_audio_api_path(audioAPI);
	auto lib = load_audio_library_module(audioAPI, modulePath);
	if(!lib)
		return std::unexpected {std::format("Failed to load audio library '{}': {}", modulePath, lib.error())};

	auto fInitAudioAPI = (*lib)->FindSymbolAddress<bool (*)(float, std::shared_ptr<pragma::audio::ISoundSystem> &, std::string &)>("initialize_audio_api");
	if(!fInitAudioAPI)
		return std::unexpected {std::format("Symbol 'initialize_audio_api' not found in audio library '{}'!", modulePath)};

	std::string errMsg;
	std::shared_ptr<pragma::audio::ISoundSystem> soundSystem;
	auto success = fInitAudioAPI(pragma::units_to_metres(1.f), soundSystem, errMsg);
	if(!success || !soundSystem)
		return std::unexpected {std::format("Failed to initialize audio library '{}': {}!", modulePath, errMsg)};
	return SoundSystemInfo {soundSystem, lib.value()};
}

pragma::audio::ISoundSystem *pragma::CEngine::InitializeSoundEngine()
{
	spdlog::info("Initializing sound engine...");

	auto &baseAudioAPI = GetAudioAPI();
	std::vector<std::string> fallbackApis {};
	if(baseAudioAPI != "soloud")
		fallbackApis.push_back("soloud");
	if(baseAudioAPI != "dummy")
		fallbackApis.push_back("dummy");
	for(auto &audioAPI : fallbackApis) {
		spdlog::info("Loading audio library '{}'...", audioAPI);
		auto res = load_audio_library(audioAPI);
		if(!res) {
			spdlog::warn("Failed to load audio library '{}': {}", audioAPI, res.error());
			continue;
		}
		auto &info = res.value();
		m_audioAPI = audioAPI;
		m_soundSystem = info.soundSystem;
		m_audioAPILib = info.library;
	}

	if(!m_soundSystem) {
		CriticalFailure("Failed to initialize sound system: No audio library could be loaded. See log messages for more information.");
		return nullptr;
	}

	m_soundSystem->SetSoundSourceFactory([](const audio::PSoundChannel &channel) -> audio::PSoundSource { return audio::CALSound::Create(get_cengine()->GetClientState(), channel); });
	audio::set_world_scale(units_to_metres(1.0));
	return m_soundSystem.get();
}

pragma::audio::PEffect pragma::CEngine::GetAuxEffect(const std::string &name)
{
	auto lname = name;
	string::to_lower(lname);
	auto it = m_auxEffects.find(lname);
	if(it == m_auxEffects.end())
		return nullptr;
	return it->second;
}

void pragma::CEngine::CloseSoundEngine() { m_soundSystem = nullptr; }

void pragma::CEngine::SetHRTFEnabled(bool b)
{
	auto *soundSys = GetSoundSystem();
	if(soundSys == nullptr)
		return;
	if(b == false)
		soundSys->DisableHRTF();
	else {
		soundSys->SetHRTF(0);
		if(soundSys->IsHRTFEnabled() == false)
			Con::CWAR << "Unable to activate HRTF. Please make sure *.mhr-file is in correct directory!" << Con::endl;
	}
}
