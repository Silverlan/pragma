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
#include "pragma/game/game_resources.hpp"
#include <sharedutils/util_path.hpp>

extern DLLNETWORK Engine *engine;

bool pragma::asset::exists(NetworkState &nw,const std::string &name,Type type)
{
	return find_file(nw,name,type).has_value();
}
std::optional<std::string> pragma::asset::determine_format_from_data(VFilePtr &f,Type type)
{
	auto offset = f->Tell();
	std::array<char,4> header {};
	f->Read(header.data(),header.size());
	f->Seek(offset);
	if(ustring::compare(header.data(),"PMDL"))
		return FORMAT_MAP_BINARY;
	else if(ustring::compare(header.data(),"WLD"))
		return FORMAT_MAP_LEGACY;
	return FORMAT_MAP_ASCII; // Assume it's the ASCII map format
}
std::optional<std::string> pragma::asset::determine_format_from_filename(const std::string_view &fileName,Type type)
{
	std::string ext;
	if(ufile::get_extension(std::string{fileName},&ext) == false)
		return {};
	ustring::to_lower(ext);
	auto supportedExtensions = get_supported_extensions(type);
	auto it = std::find(supportedExtensions.begin(),supportedExtensions.end(),ext);
	return (it != supportedExtensions.end()) ? *it : std::optional<std::string>{};
}
bool pragma::asset::matches_format(const std::string_view &format0,const std::string_view &format1)
{
	return ustring::compare(format0.data(),format1.data(),false,umath::min(format0.length(),format1.length()));
}
util::Path pragma::asset::relative_path_to_absolute_path(const util::Path &relPath,Type type,const std::optional<std::string> &rootPath)
{
	auto r = (get_asset_root_directory(type) +std::string{'/'}) +relPath;
	if(rootPath.has_value())
		r = *rootPath +r;
	return r;
}
util::Path pragma::asset::absolute_path_to_relative_path(const util::Path &absPath,Type type)
{
	auto path = absPath;
	path.PopFront();
	return path;
}
std::optional<std::string> pragma::asset::get_udm_format_extension(Type type,bool binary)
{
	switch(type)
	{
	case Type::Model:
		return binary ? FORMAT_MODEL_BINARY : FORMAT_MODEL_ASCII;
	case Type::Map:
		return binary ? FORMAT_MAP_BINARY : FORMAT_MAP_ASCII;
	case Type::Material:
		return binary ? FORMAT_MATERIAL_BINARY : FORMAT_MATERIAL_ASCII;
	}
	return {};
}
std::vector<std::string> pragma::asset::get_supported_extensions(Type type)
{
	// Note: When attempting to find an asset, the Engine will take the order of these into account (i.e. extensions that come first will be prioritized)
	switch(type)
	{
	case Type::Model:
		return {FORMAT_MODEL_BINARY,FORMAT_MODEL_ASCII,FORMAT_MODEL_LEGACY};
	case Type::Map:
		return {FORMAT_MAP_BINARY,FORMAT_MAP_ASCII,FORMAT_MAP_LEGACY};
	case Type::Material:
		return {/*FORMAT_MATERIAL_BINARY,FORMAT_MATERIAL_ASCII,*/FORMAT_MAP_LEGACY};
	case Type::Sound:
	case Type::Texture:
	{
		auto &supportedFormats = MaterialManager::get_supported_image_formats();
		std::vector<std::string> extensions;
		extensions.reserve(supportedFormats.size());
		for(auto &format : supportedFormats)
			extensions.push_back(format.extension);
		return extensions;
	}
	case Type::ParticleSystem:
		// TODO
		break;
	}
	return {};
}
std::string pragma::asset::get_normalized_path(const std::string &name,Type type)
{
	auto path = util::Path::CreateFile(name);
	path.Canonicalize();
	switch(type)
	{
	case Type::Model:
		return ModelManager::GetNormalizedModelName(path.GetString());
	case Type::Map:
	case Type::Material:
	case Type::Texture:
		path.RemoveFileExtension(get_supported_extensions(type));
		break;
	case Type::Sound:
	case Type::ParticleSystem:
		// TODO
		break;
	}
	return path.GetString();
}
bool pragma::asset::matches(const std::string &name0,const std::string &name1,Type type)
{
	return ustring::compare(get_normalized_path(name0,type),get_normalized_path(name1,type),false);
}
std::optional<std::string> pragma::asset::find_file(NetworkState &nw,const std::string &name,Type type,std::string *optOutFormat)
{
	auto normalizedName = get_normalized_path(name,type);
	switch(type)
	{
	case Type::Model:
	{
		for(auto &ext : get_supported_extensions(type))
		{
			auto nameWithExt = normalizedName +'.' +ext;
			if(FileManager::Exists("models/" +nameWithExt))
			{
				if(optOutFormat)
					*optOutFormat = ext;
				return nameWithExt;
			}
		}
		return {};
	}
	case Type::Map:
	{
		for(auto &ext : get_supported_extensions(type))
		{
			auto nameWithExt = normalizedName +'.' +ext;
			if(FileManager::Exists("maps/" +nameWithExt))
			{
				if(optOutFormat)
					*optOutFormat = ext;
				return nameWithExt;
			}
		}
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
			{
				if(optOutFormat)
					*optOutFormat = format.extension;
				return extFileName;
			}
		}
		return {};
	}
	case Type::ParticleSystem:
	{
		auto normName = name;
		std::string ext;
		if(ufile::get_extension(normName,&ext) == false)
		{
			if(optOutFormat)
				*optOutFormat = "wpt";
			normName += ".wpt";
		}
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
	case Type::Map:
		return false; // TODO
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
							if(mdl && mdl->Save(game,::util::CONVERT_PATH +"models/" +path.GetString(),err) == false)
								return nullptr;
							break;
						}
						case Type::Map:
							break; // TODO
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
