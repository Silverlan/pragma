/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/asset/util_asset.hpp"
#include "pragma/networkstate/networkstate.h"
#include "pragma/model/modelmanager.h"

extern DLLENGINE Engine *engine;

bool pragma::asset::exists(NetworkState &nw,const std::string &name,Type type)
{
	return find_file(nw,name,type).has_value();
}
std::optional<std::string> pragma::asset::find_file(NetworkState &nw,const std::string &name,Type type)
{
	switch(type)
	{
	case Type::Model:
	{
		auto normName = ModelManager::GetNormalizedModelName(name);
		if(FileManager::Exists("models/" +normName))
			return normName;
		return {};
	}
	case Type::Material:
		return nw.GetMaterialManager().FindMaterialPath(name);
	case Type::Sound:
		return {}; // TODO
	case Type::Texture:
	{
		for(auto &format : MaterialManager::get_supported_image_formats())
		{
			auto extFileName = name +'.' +format.extension;
			if(FileManager::Exists("materials/" +extFileName))
				return extFileName;
		}
		return {};
	}
	case Type::ParticleSystem:
	{
		auto normName = name;
		std::string ext;
		if(ufile::get_extension(normName,&ext) == false)
			normName += ".wpt";
		if(FileManager::Exists("particles/" +normName) == false)
			return {};
		return normName;
	}
	}
	return {};
}
bool pragma::asset::is_loaded(NetworkState &nw,const std::string &name,Type type)
{
	switch(type)
	{
	case Type::Model:
	{
		auto &mdlManager = nw.GetModelManager();
		return mdlManager.FindCachedModel(name);
	}
	case Type::Material:
		return nw.GetMaterialManager().FindMaterial(name);
	case Type::Sound:
		return false; // TODO
	case Type::Texture:
		return false; // Only client knows about textures
	}
	return false;
}
