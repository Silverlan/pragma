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
std::unique_ptr<pragma::asset::IAssetWrapper> pragma::asset::AssetManager::ImportAsset(Type type,VFilePtr f,const std::optional<std::string> &filePath,std::string *optOutErr) const
{
	auto fpath = filePath;
	if(f == nullptr && filePath.has_value())
	{
		auto filePathNoExt = *filePath;
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
				auto aw = importer.handler(f,fpath,err);
				if(aw && aw->GetType() == type)
					return aw;
				if(optOutErr)
					*optOutErr = err;
			}
		}
		return nullptr;
	}

	for(auto &importer : m_importers[umath::to_integral(type)])
	{
		std::string err;
		auto aw = importer.handler(f,fpath,err);
		if(aw && aw->GetType() == type)
			return aw;
		if(optOutErr)
			*optOutErr = err;
	}
	return false;
}
bool pragma::asset::AssetManager::ExportAsset(Type type,VFilePtrReal f,const IAssetWrapper &assetWrapper,std::string *optOutErr) const
{
	for(auto &exporter : m_exporters[umath::to_integral(type)])
	{
		std::string err;
		if(exporter.handler(f,assetWrapper,err))
			return true;
		if(optOutErr)
			*optOutErr = err;
	}
	return false;
}
