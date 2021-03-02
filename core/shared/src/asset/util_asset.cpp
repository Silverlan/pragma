/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/asset/util_asset.hpp"
#include "pragma/networkstate/networkstate.h"
#include "pragma/model/modelmanager.h"
#include <sharedutils/util_path.hpp>

extern DLLNETWORK Engine *engine;

bool pragma::asset::exists(NetworkState &nw,const std::string &name,Type type)
{
	return find_file(nw,name,type).has_value();
}
std::string pragma::asset::get_normalized_path(const std::string &name,Type type)
{
	switch(type)
	{
	case Type::Model:
		return ModelManager::GetNormalizedModelName(name);
	case Type::Material:
	case Type::Sound:
	case Type::Texture:
	case Type::ParticleSystem:
		// TODO
		break;
	}
	return name;
}
bool pragma::asset::matches(const std::string &name0,const std::string &name1,Type type)
{
	switch(type)
	{
	case Type::Model:
	{
		auto normName0 = ModelManager::GetNormalizedModelName(name0);
		auto normName1 = ModelManager::GetNormalizedModelName(name1);
		return ustring::compare(normName0,normName1,false);
	}
	case Type::Material:
	case Type::Sound:
	case Type::Texture:
	case Type::ParticleSystem:
		// TODO
		break;
	}
	return name0 == name1;
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

/////////////

void pragma::asset::ModelAssetWrapper::SetModel(Model &model) {m_model = model.shared_from_this();}
Model *pragma::asset::ModelAssetWrapper::GetModel() const {return m_model.get();}

void pragma::asset::MaterialAssetWrapper::SetMaterial(Material &mat) {m_material = mat.GetHandle();}
Material *pragma::asset::MaterialAssetWrapper::GetMaterial() const {return m_material.get();}

void pragma::asset::AssetManager::RegisterImporter(const ImporterInfo &importerInfo,Type type,const ImportHandler &importHandler)
{
	Importer importer {};
	importer.info = importerInfo;
	importer.handler = importHandler;
	m_importers[umath::to_integral(type)].push_back(importer);
}
void pragma::asset::AssetManager::RegisterExporter(const ExporterInfo &importerInfo,Type type,const ExportHandler &exportHandler)
{
	Exporter exporter {};
	exporter.info = importerInfo;
	exporter.handler = exportHandler;
	m_exporters[umath::to_integral(type)].push_back(exporter);
}
std::unique_ptr<pragma::asset::IAssetWrapper> pragma::asset::AssetManager::ImportAsset(Game &game,Type type,VFilePtr f,const std::optional<std::string> &filePath,std::string *optOutErr) const
{
	auto fpath = filePath;
	if(f == nullptr && filePath.has_value())
	{
		auto filePathNoExt = pragma::asset::get_normalized_path(*filePath,type);
		ufile::remove_extension_from_filename(filePathNoExt);
		for(auto &importer : m_importers[umath::to_integral(type)])
		{
			for(auto &ext : importer.info.fileExtensions)
			{
				auto filePathWithExt = filePathNoExt +'.' +ext;
				auto f = FileManager::OpenFile(filePathWithExt.c_str(),"rb");
				if(f == nullptr)
					continue;
				fpath = filePathWithExt;

				std::string err;
				auto aw = importer.handler(game,f,fpath,err);
				if(aw && aw->GetType() == type)
				{
					if(filePath.has_value())
					{
						switch(type)
						{
						case Type::Model:
						{
							auto path = util::Path::CreateFile(*filePath);
							path.PopFront();
							auto *mdl = static_cast<pragma::asset::ModelAssetWrapper&>(*aw).GetModel();
							if(mdl)
								mdl->SaveLegacy(&game,path.GetString(),"addons/converted/");
							break;
						}
						case Type::Material:
							break; // TODO
						case Type::ParticleSystem:
							break; // TODO
						case Type::Sound:
							break; // TODO
						case Type::Texture:
							break; // TODO
						}
					}
					return aw;
				}
				if(optOutErr)
					*optOutErr = err;
			}
		}
		return nullptr;
	}

	for(auto &importer : m_importers[umath::to_integral(type)])
	{
		std::string err;
		auto aw = importer.handler(game,f,fpath,err);
		if(aw && aw->GetType() == type)
			return aw;
		if(optOutErr)
			*optOutErr = err;
	}
	return false;
}
bool pragma::asset::AssetManager::ExportAsset(Game &game,Type type,VFilePtrReal f,const IAssetWrapper &assetWrapper,std::string *optOutErr) const
{
	for(auto &exporter : m_exporters[umath::to_integral(type)])
	{
		std::string err;
		if(exporter.handler(game,f,assetWrapper,err))
			return true;
		if(optOutErr)
			*optOutErr = err;
	}
	return false;
}
