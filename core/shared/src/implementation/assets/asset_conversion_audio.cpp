// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <ostream>

#include <unordered_set>

#include <functional>
#include <map>

#include <cinttypes>
#include <optional>
#include <vector>
#include <cstring>
#include <unordered_map>
#include <cmath>
#include "pragma/lua/core.hpp"

#include <memory>
#include <string>

module pragma.shared;

import :assets.conversion;
import se_script;

bool util::port_sound_script(NetworkState *nw, const std::string &path)
{
	static auto *ptrOpenArchiveFile = reinterpret_cast<void (*)(const std::string &, VFilePtr &, const std::optional<std::string> &)>(util::impl::get_module_func(nw, "open_archive_file"));
	if(ptrOpenArchiveFile == nullptr)
		return false;
	VFilePtr f = nullptr;
	ptrOpenArchiveFile(path, f, {});
	if(f == nullptr)
		return false;
	source_engine::script::ScriptBlock root {};
	if(source_engine::script::read_script(f, root) != source_engine::script::ResultCode::Ok)
		return false;
	auto outPath = util::IMPORT_PATH + FileManager::GetCanonicalizedPath(path);
	if(ustring::substr(outPath, 0, 8) == std::string("scripts") + FileManager::GetDirectorySeparator())
		outPath = "scripts/sounds/" + outPath.substr(8);
	ufile::remove_extension_from_filename(outPath, std::array<std::string, 1> {"txt"});
	outPath += ".udm";
	FileManager::CreatePath(ufile::get_path_from_filename(outPath).c_str());

	auto udmData = udm::Data::Create();
	auto outData = udmData->GetAssetData();
	auto udm = *outData;
	for(auto &data : root.data) {
		if(data->IsBlock() == false)
			continue;
		auto &block = static_cast<source_engine::script::ScriptBlock &>(*data);
		auto udmBlock = udm[block.identifier];
		auto udmEvents = udmBlock.AddArray("events", 1);
		auto udmEvent = udmEvents[0];
		udmEvent["type"] = "playsound";

		auto bPlayGlobally = false;
		std::vector<std::string> sources;
		for(auto &v : block.data) {
			if(v->IsBlock() == true) {
				auto &subBlock = static_cast<source_engine::script::ScriptBlock &>(*v);
				if(subBlock.identifier == "rndwave") {
					for(auto &v : subBlock.data) {
						auto &val = static_cast<source_engine::script::ScriptValue &>(*v);
						if(val.identifier != "wave")
							continue;
						sources.push_back(val.value);
					}
				}
				continue;
			}
			auto &val = static_cast<source_engine::script::ScriptValue &>(*v);
			if(val.identifier == "volume") {
				auto outVal = (val.value != "VOL_NORM") ? val.value : "1.0";
				udmEvent["gain"] = outVal;
			}
			else if(val.identifier == "pitch") {
				auto outVal = val.value;
				if(outVal == "PITCH_NORM")
					outVal = "1.0";
				else if(outVal == "PITCH_LOW")
					outVal = "0.95";
				else if(outVal == "PITCH_HIGH")
					outVal = "1.2";
				else
					outVal = std::to_string(util::to_float(outVal) / 100.f);
				udmEvent["pitch"] = outVal;
			}
			else if(val.identifier == "soundlevel") {
				// TODO: Sound level = Attenuation
				// See https://developer.valvesoftware.com/wiki/Soundscripts
				if(val.identifier == "0") {
					bPlayGlobally = true;
					udmEvent["global"] = true;
				}
			}
			else if(val.identifier == "channel") {
				const std::unordered_map<std::string, pragma::audio::ALSoundType> channelToType
				  = {{"CHAN_AUTO", pragma::audio::ALSoundType::Generic}, {"CHAN_WEAPON", pragma::audio::ALSoundType::Weapon}, {"CHAN_VOICE", pragma::audio::ALSoundType::Voice}, {"CHAN_VOICE2", pragma::audio::ALSoundType::Voice}, {"CHAN_ITEM", pragma::audio::ALSoundType::Physics}, {"CHAN_BODY", pragma::audio::ALSoundType::Effect}, {"CHAN_VOICE_BASE", pragma::audio::ALSoundType::Voice}};
				auto it = channelToType.find(val.value);
				if(it != channelToType.end())
					udmEvent["type"] = udm::enum_to_string(it->second);
			}
			else if(val.identifier == "wave")
				sources.push_back(val.value);
		}
		std::string mode = bPlayGlobally ? "stereo" : "mono";
		auto bLoop = false;
		auto bStream = false;
		if(sources.empty() == false) {
			for(auto &src : sources) {
				if(ustring::match(src, "*loop*") == true)
					udmEvent["loop"] = true;

				const std::string removePrefixes = "*#@><^()}$!?"; // Special prefixes that need to be removed
				while(src.empty() == false && removePrefixes.find(src.front()) != std::string::npos) {
					if(src.front() == '(' || src.front() == ')')
						mode = "both";
					else if(src.front() == '*')
						bStream = true;
					src.erase(src.begin());
				}
			}
			if(sources.size() == 1)
				udmEvent["source"] = sources.front();
			else
				udmEvent["source"] = sources;
		}
		if(bStream == true)
			udmEvent["stream"] = true;
		udmEvent["mode"] = mode;
	}

	auto fOut = FileManager::OpenFile<VFilePtrReal>(outPath.c_str(), "w");
	if(fOut == nullptr)
		return false;
	return udmData->SaveAscii(fOut, udm::AsciiSaveFlags::Default);
}
