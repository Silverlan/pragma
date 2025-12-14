// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.steam.vdf;

pragma::util::MarkupFile::ResultCode pragma::util::steam::vdf::read_vdf_block(MarkupFile &mf, DataBlock &block, uint32_t depth)
{
	std::string key;
	auto r = MarkupFile::ResultCode::Ok;
	while(r == MarkupFile::ResultCode::Ok) {
		char token {};
		if((r = mf.ReadNextToken(token)) != MarkupFile::ResultCode::Ok)
			return r;
		if(token == '}') {
			if(depth == 0)
				return MarkupFile::ResultCode::Error;
			return r;
		}

		if((r = mf.ReadNextString(key)) != MarkupFile::ResultCode::Ok)
			return r;
		if((r = mf.ReadNextToken(token)) != MarkupFile::ResultCode::Ok)
			return r;
		string::to_lower(key);
		if(token == '{') {
			mf.IncrementFilePos();
			auto it = block.children.insert(std::make_pair(key, DataBlock {})).first;
			auto r = read_vdf_block(mf, it->second, depth + 1);
			if(r != MarkupFile::ResultCode::Ok)
				return r;
			if((r = mf.ReadNextToken(token)) != MarkupFile::ResultCode::Ok)
				return r;
			if(token != '}')
				return MarkupFile::ResultCode::Error;
			if(depth == 0)
				return r;
			mf.IncrementFilePos();
			continue;
		}
		std::string value;
		if((r = mf.ReadNextString(value)) != MarkupFile::ResultCode::Ok)
			return r;
		block.keyValues[key] = value;
	}
	return r;
}

bool pragma::util::steam::vdf::get_external_steam_locations(const std::string &steamRootPath, std::vector<std::string> &outExtLocations)
{
	auto f = fs::open_system_file((steamRootPath + "/steamapps/libraryfolders.vdf"), fs::FileMode::Read);
	if(f == nullptr)
		return false;
	auto lenContents = f->GetSize();

	DataStream dsContents {static_cast<uint32_t>(lenContents)};
	f->Read(dsContents->GetData(), lenContents);

	MarkupFile mf {dsContents};
	auto vdfData = pragma::util::make_shared<Data>();
	auto r = read_vdf_block(mf, vdfData->dataBlock);
	if(r != MarkupFile::ResultCode::Ok)
		return false;
	auto it = vdfData->dataBlock.children.find("libraryfolders");
	if(it == vdfData->dataBlock.children.end())
		return false;
	auto &libraryFolders = it->second;
	auto fAddPath = [&outExtLocations](std::string path) {
		string::replace(path, "\\\\", "/");
		if(path.empty() == false && path.back() == '/')
			path.pop_back();
		outExtLocations.push_back(path);
	};
	for(uint8_t i = 1; i <= 8; ++i) // 8 is supposedly the max number of external locations you can specify in steam
	{
		auto itKv = libraryFolders.keyValues.find(std::to_string(i));
		if(itKv != libraryFolders.keyValues.end())
			fAddPath(itKv->second);
		else {
			// Newer versions of Steam use a different format
			auto itChild = libraryFolders.children.find(std::to_string(i));
			if(itChild != libraryFolders.children.end()) {
				auto itMounted = itChild->second.keyValues.find("mounted");
				if(itMounted != itChild->second.keyValues.end() && itMounted->second == "0")
					continue;
				auto itPath = itChild->second.keyValues.find("path");
				if(itPath != itChild->second.keyValues.end())
					fAddPath(itPath->second);
			}
		}
	}
	return true;
}
