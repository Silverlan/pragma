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
#include <material_manager2.hpp>
#include <sharedutils/util_path.hpp>
#include <udm.hpp>

extern DLLNETWORK Engine *engine;

bool pragma::asset::exists(const std::string &name,Type type)
{
	return find_file(name,type).has_value();
}
std::optional<std::string> pragma::asset::determine_format_from_data(VFilePtr f,Type type)
{
	auto offset = f->Tell();
	std::array<char,4> header {};
	f->Read(header.data(),header.size());
	f->Seek(offset);

	if(ustring::compare(header.data(),udm::HEADER_IDENTIFIER))
		return get_binary_udm_extension(type);
	else if(ustring::compare(header.data(),"WLD"))
		return FORMAT_MAP_LEGACY;
	else if(ustring::compare(header.data(),"WMD"))
		return FORMAT_MODEL_LEGACY;
	else if(ustring::compare(header.data(),"WPD"))
		return FORMAT_PARTICLE_SYSTEM_LEGACY;
	return get_ascii_udm_extension(type); // Assume it's the ASCII map format
}
std::optional<pragma::asset::Type> pragma::asset::determine_type_from_extension(const std::string_view &ext)
{
	std::string lext {ext};
	ustring::to_lower(lext);
	auto n = umath::to_integral(Type::Count);
	for(auto i=decltype(n){0u};i<n;++i)
	{
		auto type = static_cast<Type>(i);
		auto supportedExtensions = get_supported_extensions(type);
		auto it = std::find(supportedExtensions.begin(),supportedExtensions.end(),lext);
		if(it == supportedExtensions.end())
			continue;
		return type;
	}
	return {};
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
	case Type::ParticleSystem:
		return binary ? FORMAT_PARTICLE_SYSTEM_BINARY : FORMAT_PARTICLE_SYSTEM_ASCII;
	}
	return {};
}
std::optional<std::string> pragma::asset::get_legacy_extension(Type type)
{
	switch(type)
	{
	case Type::Model:
		return FORMAT_MODEL_LEGACY;
	case Type::Map:
		return FORMAT_MAP_LEGACY;
	case Type::Material:
		return FORMAT_MATERIAL_LEGACY;
	case Type::ParticleSystem:
		return FORMAT_PARTICLE_SYSTEM_LEGACY;
	}
	return {};
}
std::optional<std::string> pragma::asset::get_binary_udm_extension(Type type)
{
	switch(type)
	{
	case Type::Model:
		return FORMAT_MODEL_BINARY;
	case Type::Map:
		return FORMAT_MAP_BINARY;
	case Type::Material:
		return FORMAT_MATERIAL_BINARY;
	case Type::ParticleSystem:
		return FORMAT_PARTICLE_SYSTEM_BINARY;
	}
	return {};
}
std::optional<std::string> pragma::asset::get_ascii_udm_extension(Type type)
{
	switch(type)
	{
	case Type::Model:
		return FORMAT_MODEL_ASCII;
	case Type::Map:
		return FORMAT_MAP_ASCII;
	case Type::Material:
		return FORMAT_MATERIAL_ASCII;
	case Type::ParticleSystem:
		return FORMAT_PARTICLE_SYSTEM_ASCII;
	}
	return {};
}
std::vector<std::string> pragma::asset::get_supported_extensions(Type type,bool includeImportTypes)
{
	// Note: When attempting to find an asset, the Engine will take the order of these into account (i.e. extensions that come first will be prioritized)
	switch(type)
	{
	case Type::Model:
		if(includeImportTypes)
			return {FORMAT_MODEL_BINARY,FORMAT_MODEL_ASCII,FORMAT_MODEL_LEGACY,"mdl","vmdl_c","nif"}; // TODO: Grab import types from import manager
		return {FORMAT_MODEL_BINARY,FORMAT_MODEL_ASCII,FORMAT_MODEL_LEGACY};
	case Type::Map:
		if(includeImportTypes)
			return {FORMAT_MAP_BINARY,FORMAT_MAP_ASCII,FORMAT_MAP_LEGACY,"bsp"}; // TODO: Grab import types from import manager
		return {FORMAT_MAP_BINARY,FORMAT_MAP_ASCII,FORMAT_MAP_LEGACY};
	case Type::Material:
		if(includeImportTypes)
			return {FORMAT_MATERIAL_BINARY,FORMAT_MATERIAL_ASCII,FORMAT_MAP_LEGACY,"vmt","vmat_c"}; // TODO: Grab import types from import manager
		return {FORMAT_MATERIAL_BINARY,FORMAT_MATERIAL_ASCII,FORMAT_MAP_LEGACY};
	case Type::ParticleSystem:
		return {FORMAT_PARTICLE_SYSTEM_BINARY,FORMAT_PARTICLE_SYSTEM_ASCII,FORMAT_PARTICLE_SYSTEM_LEGACY};
	case Type::Sound:
		return engine_info::get_supported_audio_formats();
	case Type::Texture:
	{
		auto &supportedFormats = MaterialManager::get_supported_image_formats();
		std::vector<std::string> extensions;
		extensions.reserve(supportedFormats.size());
		for(auto &format : supportedFormats)
			extensions.push_back(format.extension);
		return extensions;
	}
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
		//return ModelManager::GetNormalizedModelName(path.GetString());
	case Type::Map:
	case Type::Material:
	case Type::Texture:
	case Type::ParticleSystem:
	case Type::Sound:
		path.RemoveFileExtension(get_supported_extensions(type));
		break;
	}
	return path.GetString();
}
bool pragma::asset::matches(const std::string &name0,const std::string &name1,Type type)
{
	return ustring::compare(get_normalized_path(name0,type),get_normalized_path(name1,type),false);
}
bool pragma::asset::remove_asset(const std::string &name,Type type)
{
	auto f = find_file(name,type);
	std::vector<std::string> deleted;
	while(f.has_value())
	{
		auto it = std::find(deleted.begin(),deleted.end(),*f);
		if(it != deleted.end())
			return false;
		auto fullPath = std::string{get_asset_root_directory(type)} +'/' +*f;
		if(FileManager::RemoveFile(fullPath.c_str()) == false)
			return false;
		deleted.push_back(*f);
		f = find_file(name,type);
	}
	return true;
}
std::optional<std::string> pragma::asset::find_file(const std::string &name,Type type,std::string *optOutFormat)
{
	auto normalizedName = get_normalized_path(name,type);
	switch(type)
	{
	case Type::Model:
	case Type::Map:
	case Type::Texture:
	case Type::ParticleSystem:
	case Type::Sound:
	{
		for(auto &ext : get_supported_extensions(type))
		{
			auto nameWithExt = normalizedName +'.' +ext;
			if(FileManager::Exists(std::string{get_asset_root_directory(type)} +"/" +nameWithExt))
			{
				if(optOutFormat)
					*optOutFormat = ext;
				return nameWithExt;
			}
		}
		return {};
	}
	case Type::Material:
	{
		auto *sv = engine->GetServerNetworkState();
		auto *cl = engine->GetClientState();
		auto *nw = sv ? sv : cl; // Doesn't matter which one
		return nw ? nw->GetMaterialManager().FindAssetFilePath(name) : std::optional<std::string>{};
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
		auto *asset = mdlManager.FindCachedAsset(name);
		return asset != nullptr;
	}
	case Type::Map:
		return false; // TODO
	case Type::Material:
		return nw.GetMaterialManager().FindCachedAsset(name);
	case Type::Sound:
	case Type::ParticleSystem:
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
	return nullptr;
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
