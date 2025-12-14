// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :assets.common;

bool pragma::asset::exists(const std::string &name, Type type) { return find_file(name, type).has_value(); }
std::optional<std::string> pragma::asset::determine_format_from_data(ufile::IFile &f, Type type)
{
	auto offset = f.Tell();
	std::array<char, 4> header {};
	f.Read(header.data(), header.size());
	f.Seek(offset);

	if(string::compare(header.data(), udm::HEADER_IDENTIFIER))
		return get_binary_udm_extension(type);
	else if(string::compare(header.data(), "WLD"))
		return FORMAT_MAP_LEGACY;
	else if(string::compare(header.data(), "WMD"))
		return FORMAT_MODEL_LEGACY;
	else if(string::compare(header.data(), "WPD"))
		return FORMAT_PARTICLE_SYSTEM_LEGACY;
	return get_ascii_udm_extension(type); // Assume it's the ASCII map format
}
std::optional<pragma::asset::Type> pragma::asset::determine_type_from_extension(const std::string_view &ext)
{
	std::string lext {ext};
	string::to_lower(lext);
	auto n = math::to_integral(Type::Count);
	for(auto i = decltype(n) {0u}; i < n; ++i) {
		auto type = static_cast<Type>(i);
		auto supportedExtensions = get_supported_extensions(type);
		auto it = std::find(supportedExtensions.begin(), supportedExtensions.end(), lext);
		if(it == supportedExtensions.end())
			continue;
		return type;
	}
	return {};
}
std::optional<std::string> pragma::asset::determine_format_from_filename(const std::string_view &fileName, Type type)
{
	std::string ext;
	if(ufile::get_extension(std::string {fileName}, &ext) == false)
		return {};
	string::to_lower(ext);
	auto supportedExtensions = get_supported_extensions(type);
	auto it = std::find(supportedExtensions.begin(), supportedExtensions.end(), ext);
	return (it != supportedExtensions.end()) ? *it : std::optional<std::string> {};
}
bool pragma::asset::matches_format(const std::string_view &format0, const std::string_view &format1) { return string::compare(format0.data(), format1.data(), false, math::min(format0.length(), format1.length())); }
pragma::util::Path pragma::asset::relative_path_to_absolute_path(const util::Path &relPath, Type type, const std::optional<std::string> &rootPath)
{
	auto r = (get_asset_root_directory(type) + std::string {'/'}) + relPath;
	if(rootPath.has_value())
		r = *rootPath + r;
	return r;
}
pragma::util::Path pragma::asset::absolute_path_to_relative_path(const util::Path &absPath, Type type)
{
	auto path = absPath;
	path.PopFront();
	return path;
}
std::optional<std::string> pragma::asset::get_udm_format_extension(Type type, bool binary)
{
	switch(type) {
	case Type::Model:
		return binary ? FORMAT_MODEL_BINARY : FORMAT_MODEL_ASCII;
	case Type::Map:
		return binary ? FORMAT_MAP_BINARY : FORMAT_MAP_ASCII;
	case Type::Material:
		return binary ? FORMAT_MATERIAL_BINARY : FORMAT_MATERIAL_ASCII;
	case Type::ParticleSystem:
		return binary ? FORMAT_PARTICLE_SYSTEM_BINARY : FORMAT_PARTICLE_SYSTEM_ASCII;
	case Type::ShaderGraph:
		return binary ? FORMAT_SHADER_GRAPH_BINARY : FORMAT_SHADER_GRAPH_ASCII;
	}
	static_assert(math::to_integral(Type::Count) == 7, "New asset type added, please update get_udm_format_extension");
	return {};
}
std::optional<std::string> pragma::asset::get_legacy_extension(Type type)
{
	switch(type) {
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
	switch(type) {
	case Type::Model:
		return FORMAT_MODEL_BINARY;
	case Type::Map:
		return FORMAT_MAP_BINARY;
	case Type::Material:
		return FORMAT_MATERIAL_BINARY;
	case Type::ParticleSystem:
		return FORMAT_PARTICLE_SYSTEM_BINARY;
	case Type::ShaderGraph:
		return FORMAT_SHADER_GRAPH_BINARY;
	}
	static_assert(math::to_integral(Type::Count) == 7, "New asset type added, please update get_binary_udm_extension");
	return {};
}
std::optional<std::string> pragma::asset::get_ascii_udm_extension(Type type)
{
	switch(type) {
	case Type::Model:
		return FORMAT_MODEL_ASCII;
	case Type::Map:
		return FORMAT_MAP_ASCII;
	case Type::Material:
		return FORMAT_MATERIAL_ASCII;
	case Type::ParticleSystem:
		return FORMAT_PARTICLE_SYSTEM_ASCII;
	case Type::ShaderGraph:
		return FORMAT_SHADER_GRAPH_ASCII;
	}
	static_assert(math::to_integral(Type::Count) == 7, "New asset type added, please update get_ascii_udm_extension");
	return {};
}
struct AssetFormatExtensionCache {
  public:
	struct FormatCache {
		std::vector<std::string> nativeFormats;
		std::vector<std::string> importFormats;
		std::vector<std::string> allFormats;
	};
	void Cache(pragma::asset::Type type, std::vector<std::string> nativeExts, std::vector<std::string> importExts)
	{
		FormatCache formatCache {};
		auto &allExts = formatCache.allFormats;
		allExts.reserve(nativeExts.size() + importExts.size());
		for(auto &ext : nativeExts)
			allExts.push_back(ext);
		for(auto &ext : importExts)
			allExts.push_back(ext);
		formatCache.nativeFormats = std::move(nativeExts);
		formatCache.importFormats = std::move(importExts);
		m_cache[pragma::math::to_integral(type)] = std::move(formatCache);
	}
	const std::vector<std::string> *GetCache(pragma::asset::Type type, pragma::asset::FormatType formatType) const
	{
		if(pragma::math::to_integral(type) >= m_cache.size() || m_cache[pragma::math::to_integral(type)].has_value() == false)
			return nullptr;
		auto &cache = *m_cache[pragma::math::to_integral(type)];
		switch(formatType) {
		case pragma::asset::FormatType::Native:
			return &cache.nativeFormats;
		case pragma::asset::FormatType::Import:
			return &cache.importFormats;
		case pragma::asset::FormatType::All:
			return &cache.allFormats;
		}
		return nullptr;
	}
  private:
	std::array<std::optional<FormatCache>, pragma::math::to_integral(pragma::asset::Type::Count)> m_cache;
};
static AssetFormatExtensionCache g_extCache;
void pragma::asset::update_extension_cache(Type type)
{
	switch(type) {
	case Type::Map:
		{
			g_extCache.Cache(type, {FORMAT_MAP_BINARY, FORMAT_MAP_ASCII, FORMAT_MAP_LEGACY}, {"bsp"});
			break;
		}
	case Type::Sound:
		{
			g_extCache.Cache(type, engine_info::get_supported_audio_formats(), {});
			break;
		}
	case Type::ParticleSystem:
		{
			g_extCache.Cache(type, {FORMAT_PARTICLE_SYSTEM_BINARY, FORMAT_PARTICLE_SYSTEM_ASCII, FORMAT_PARTICLE_SYSTEM_LEGACY}, {});
			break;
		}
	case Type::Texture:
		{
			auto &supportedFormats = MaterialManager::get_supported_image_formats();
			std::vector<std::string> extensions;
			extensions.reserve(supportedFormats.size());
			for(auto &format : supportedFormats)
				extensions.push_back(format.extension);
			g_extCache.Cache(type, std::move(extensions), {});
			break;
		}
	}

	auto *nw = Engine::Get()->GetClientState();
	if(!nw)
		nw = Engine::Get()->GetServerNetworkState();
	if(!nw)
		return;

	auto *assetManager = nw->GetAssetManager(type);
	if(!assetManager)
		return;
	auto &exts = assetManager->GetSupportedFormatExtensions();
	std::vector<std::string> nativeExts;
	std::vector<std::string> importExts;
	nativeExts.reserve(exts.size());
	importExts.reserve(exts.size());
	for(auto &extInfo : exts) {
		switch(extInfo.type) {
		case util::IAssetManager::FormatExtensionInfo::Type::Native:
			nativeExts.push_back(extInfo.extension);
			break;
		case util::IAssetManager::FormatExtensionInfo::Type::Import:
			importExts.push_back(extInfo.extension);
			break;
		}
	}
	nativeExts.shrink_to_fit();
	importExts.shrink_to_fit();
	g_extCache.Cache(type, std::move(nativeExts), std::move(importExts));
}
void pragma::asset::update_extension_cache()
{
	auto n = math::to_integral(Type::Count);
	for(auto i = decltype(n) {0u}; i < n; ++i)
		update_extension_cache(static_cast<Type>(i));
}
const std::vector<std::string> &pragma::asset::get_supported_extensions(Type type, FormatType formatType)
{
	auto *cache = g_extCache.GetCache(type, formatType);
	assert(cache);
	if(cache)
		return *cache;
	static std::vector<std::string> empty {};
	return empty;
}
std::string pragma::asset::get_normalized_path(const std::string &name, Type type)
{
	auto path = util::Path::CreateFile(name);
	path.Canonicalize();
	switch(type) {
	case Type::Model:
		//return ModelManager::GetNormalizedModelName(path.GetString());
	case Type::Map:
	case Type::Material:
	case Type::Texture:
	case Type::ParticleSystem:
	case Type::Sound:
	case Type::ShaderGraph:
		path.RemoveFileExtension(get_supported_extensions(type));
		break;
	}
	static_assert(math::to_integral(Type::Count) == 7, "New asset type added, please update get_normalized_path");
	return path.GetString();
}
bool pragma::asset::matches(const std::string &name0, const std::string &name1, Type type) { return string::compare(get_normalized_path(name0, type), get_normalized_path(name1, type), false); }
bool pragma::asset::remove_asset(const std::string &name, Type type)
{
	auto f = find_file(name, type);
	std::vector<std::string> deleted;
	while(f.has_value()) {
		auto it = std::find(deleted.begin(), deleted.end(), *f);
		if(it != deleted.end())
			return false;
		auto fullPath = std::string {get_asset_root_directory(type)} + '/' + *f;
		if(fs::remove_file(fullPath) == false)
			return false;
		deleted.push_back(*f);
		f = find_file(name, type);
	}
	return true;
}
std::optional<std::string> pragma::asset::find_file(const std::string &name, Type type, std::string *optOutFormat)
{
	auto normalizedName = get_normalized_path(name, type);
	switch(type) {
	case Type::Model:
	case Type::Map:
	case Type::Texture:
	case Type::ParticleSystem:
	case Type::Sound:
	case Type::ShaderGraph:
		{
			for(auto &ext : get_supported_extensions(type)) {
				auto nameWithExt = normalizedName + '.' + ext;
				if(fs::exists(std::string {get_asset_root_directory(type)} + "/" + nameWithExt)) {
					if(optOutFormat)
						*optOutFormat = ext;
					return nameWithExt;
				}
			}
			return {};
		}
	case Type::Material:
		{
			auto *sv = Engine::Get()->GetServerNetworkState();
			auto *cl = Engine::Get()->GetClientState();
			auto *nw = sv ? sv : cl; // Doesn't matter which one
			return nw ? nw->GetMaterialManager().FindAssetFilePath(name) : std::optional<std::string> {};
		}
	}
	static_assert(math::to_integral(Type::Count) == 7, "New asset type added, please update find_file");
	return {};
}
bool pragma::asset::is_loaded(NetworkState &nw, const std::string &name, Type type)
{
	switch(type) {
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
	case Type::ShaderGraph:
		return false; // Only client knows about shader graphs
	}
	static_assert(math::to_integral(Type::Count) == 7, "New asset type added, please update is_loaded");
	return false;
}

/////////////

void pragma::asset::ModelAssetWrapper::SetModel(Model &model) { m_model = model.shared_from_this(); }
pragma::asset::Model *pragma::asset::ModelAssetWrapper::GetModel() const { return m_model.get(); }

void pragma::asset::MaterialAssetWrapper::SetMaterial(material::Material &mat) { m_material = mat.GetHandle(); }
pragma::material::Material *pragma::asset::MaterialAssetWrapper::GetMaterial() const { return const_cast<material::Material *>(m_material.get()); }

void pragma::asset::AssetManager::RegisterImporter(const ImporterInfo &importerInfo, Type type, const ImportHandler &importHandler)
{
	Importer importer {};
	importer.info = importerInfo;
	importer.handler = importHandler;
	m_importers[math::to_integral(type)].push_back(importer);
}
void pragma::asset::AssetManager::RegisterExporter(const ExporterInfo &importerInfo, Type type, const ExportHandler &exportHandler)
{
	Exporter exporter {};
	exporter.info = importerInfo;
	exporter.handler = exportHandler;
	m_exporters[math::to_integral(type)].push_back(exporter);
}
std::unique_ptr<pragma::asset::IAssetWrapper> pragma::asset::AssetManager::ImportAsset(Game &game, Type type, ufile::IFile *f, const std::optional<std::string> &filePath, std::string *optOutErr) const
{
	auto fpath = filePath;
	if(f == nullptr && filePath.has_value()) {
		auto filePathNoExt = get_normalized_path(*filePath, type);
		ufile::remove_extension_from_filename(filePathNoExt);
		for(auto &importer : m_importers[math::to_integral(type)]) {
			for(auto &extInfo : importer.info.fileExtensions) {
				auto filePathWithExt = filePathNoExt + '.' + extInfo.first;
				auto f = fs::open_file(filePathWithExt, extInfo.second ? (fs::FileMode::Read | fs::FileMode::Binary) : fs::FileMode::Read);
				if(f == nullptr)
					continue;
				fpath = filePathWithExt;

				std::string err;
				fs::File fp {f};
				auto aw = importer.handler(game, fp, fpath, err);
				if(aw && aw->GetType() == type) {
					if(filePath.has_value()) {
						switch(type) {
						case Type::Model:
							{
								auto path = util::Path::CreateFile(*filePath);
								path.PopFront();
								auto *mdl = static_cast<ModelAssetWrapper &>(*aw).GetModel();
								if(mdl)
									mdl->ApplyPostImportProcessing();
								if(mdl && mdl->Save(game, util::CONVERT_PATH + "models/" + path.GetString(), err) == false)
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

	if(f) {
		auto pos = f->Tell();
		for(auto &importer : m_importers[math::to_integral(type)]) {
			std::string err;
			f->Seek(pos);
			auto aw = importer.handler(game, *f, fpath, err);
			if(aw && aw->GetType() == type) {
				auto path = util::Path::CreateFile(*filePath);
				// path.PopFront();
				auto *mdl = static_cast<ModelAssetWrapper &>(*aw).GetModel();
				if(mdl)
					mdl->ApplyPostImportProcessing();
				if(mdl && mdl->Save(game, util::CONVERT_PATH + "models/" + path.GetString(), err) == false)
					return nullptr;
				return aw;
			}
			if(optOutErr)
				*optOutErr = err;
		}
	}
	return nullptr;
}
bool pragma::asset::AssetManager::ExportAsset(Game &game, Type type, ufile::IFile &f, const IAssetWrapper &assetWrapper, std::string *optOutErr) const
{
	for(auto &exporter : m_exporters[math::to_integral(type)]) {
		std::string err;
		if(exporter.handler(game, f, assetWrapper, err))
			return true;
		if(optOutErr)
			*optOutErr = err;
	}
	return false;
}
