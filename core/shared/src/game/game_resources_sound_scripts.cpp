/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/audio/alsound_type.h"
#include <se_script.hpp>
#include <sharedutils/util_file.h>

bool util::port_sound_script(NetworkState *nw,const std::string &path)
{
	static auto *ptrOpenArchiveFile = reinterpret_cast<void(*)(const std::string&,VFilePtr&)>(util::impl::get_module_func(nw,"open_archive_file"));
	if(ptrOpenArchiveFile == nullptr)
		return false;
	VFilePtr f = nullptr;
	ptrOpenArchiveFile(path,f);
	if(f == nullptr)
		return false;
	se::ScriptBlock root {};
	if(se::read_script(f,root) != se::ResultCode::Ok)
		return false;
	auto outPath = util::IMPORT_PATH +FileManager::GetCanonicalizedPath(path);
	if(ustring::substr(outPath,0,8) == std::string("scripts") +FileManager::GetDirectorySeparator())
		outPath = "scripts/sounds/" +outPath.substr(8);
	FileManager::CreatePath(ufile::get_path_from_filename(outPath).c_str());
	auto fOut = FileManager::OpenFile<VFilePtrReal>(outPath.c_str(),"w");
	if(fOut == nullptr)
		return false;
	for(auto &data : root.data)
	{
		if(data->IsBlock() == false)
			continue;
		auto &block = static_cast<se::ScriptBlock&>(*data);
		fOut->WriteString("\"" +block.identifier +"\"\n{\n");
		fOut->WriteString("\tplaysound\n\t{\n");

		auto bPlayGlobally = false;
		std::vector<std::string> sources;
		for(auto &v : block.data)
		{
			if(v->IsBlock() == true)
			{
				auto &subBlock = static_cast<se::ScriptBlock&>(*v);
				if(subBlock.identifier == "rndwave")
				{
					for(auto &v : subBlock.data)
					{
						auto &val = static_cast<se::ScriptValue&>(*v);
						if(val.identifier != "wave")
							continue;
						sources.push_back(val.value);
					}
				}
				continue;
			}
			auto &val = static_cast<se::ScriptValue&>(*v);
			if(val.identifier == "volume")
			{
				auto outVal = (val.value != "VOL_NORM") ? val.value : "1.0";
				fOut->WriteString("\t\t$float gain " +outVal +"\n");
			}
			else if(val.identifier == "pitch")
			{
				auto outVal = val.value;
				if(outVal == "PITCH_NORM")
					outVal = "1.0";
				else if(outVal == "PITCH_LOW")
					outVal = "0.95";
				else if(outVal == "PITCH_HIGH")
					outVal = "1.2";
				else
					outVal = std::to_string(util::to_float(outVal) /100.f);
				fOut->WriteString("\t\t$float pitch " +outVal +"\n");
			}
			else if(val.identifier == "soundlevel")
			{
				// TODO: Sound level = Attenuation
				// See https://developer.valvesoftware.com/wiki/Soundscripts
				if(val.identifier == "0")
				{
					bPlayGlobally = true;
					fOut->WriteString("\t\t$bool global 1\n");
				}
			}
			else if(val.identifier == "channel")
			{
				const std::unordered_map<std::string,ALSoundType> channelToType = {
					{"CHAN_AUTO",ALSoundType::Generic},
					{"CHAN_WEAPON",ALSoundType::Weapon},
					{"CHAN_VOICE",ALSoundType::Voice},
					{"CHAN_VOICE2",ALSoundType::Voice},
					{"CHAN_ITEM",ALSoundType::Physics},
					{"CHAN_BODY",ALSoundType::Effect},
					{"CHAN_VOICE_BASE",ALSoundType::Voice}
				};
				auto it = channelToType.find(val.value);
				if(it != channelToType.end())
					fOut->WriteString("\t\t$int type " +std::to_string(umath::to_integral(it->second)) +"\n");
			}
			else if(val.identifier == "wave")
				sources.push_back(val.value);
		}
		std::string mode = bPlayGlobally ? "stereo" : "mono";
		auto bLoop = false;
		auto bStream = false;
		if(sources.empty() == false)
		{
			for(auto &src : sources)
			{
				if(ustring::match(src,"*loop*") == true)
					fOut->WriteString("\t\t$bool loop 1\n");

				const std::string removePrefixes = "*#@><^()}$!?"; // Special prefixes that need to be removed
				while(src.empty() == false && removePrefixes.find(src.front()) != std::string::npos)
				{
					if(src.front() == '(' || src.front() == ')')
						mode = "both";
					else if(src.front() == '*')
						bStream = true;
					src.erase(src.begin());
				}
			}
			if(sources.size() == 1)
				fOut->WriteString("\t\t$string source \"" +sources.front() +"\"\n");
			else
			{
				fOut->WriteString("\t\tsource\n\t\t{\n");
				for(auto &src : sources)
					fOut->WriteString("\t\t\t\"" +src +"\"\n");
				fOut->WriteString("\t\t}\n");
			}
		}
		if(bStream == true)
			fOut->WriteString("\t\t$bool stream 1\n");
		fOut->WriteString("\t\t$string mode " +mode +"\n");
		fOut->WriteString("\t}\n}\n");
	}
	return true;
}

