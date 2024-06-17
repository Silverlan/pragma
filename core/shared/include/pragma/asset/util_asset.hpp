/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_UTIL_ASSET_HPP__
#define __PRAGMA_UTIL_ASSET_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_path.hpp>
#include <material.h>
#include <memory>

class Game;
namespace ufile {
	struct IFile;
};
class Model;
class NetworkState;
namespace pragma::asset {
	static constexpr auto FORMAT_MAP_BINARY = "pmap_b";
	static constexpr auto FORMAT_MAP_ASCII = "pmap";
	static constexpr auto FORMAT_MAP_LEGACY = "wld";

	static constexpr auto FORMAT_MODEL_BINARY = "pmdl_b";
	static constexpr auto FORMAT_MODEL_ASCII = "pmdl";
	static constexpr auto FORMAT_MODEL_LEGACY = "wmd";

	static constexpr auto FORMAT_PARTICLE_SYSTEM_BINARY = "pptsys_b";
	static constexpr auto FORMAT_PARTICLE_SYSTEM_ASCII = "pptsys";
	static constexpr auto FORMAT_PARTICLE_SYSTEM_LEGACY = "wpt";

	static constexpr auto FORMAT_MATERIAL_BINARY = Material::FORMAT_MATERIAL_BINARY;
	static constexpr auto FORMAT_MATERIAL_ASCII = Material::FORMAT_MATERIAL_ASCII;
	static constexpr auto FORMAT_MATERIAL_LEGACY = Material::FORMAT_MATERIAL_LEGACY;

	enum class Type : uint8_t {
		Model = 0,
		Map,
		Material,
		Texture,
		Sound,
		ParticleSystem,

		Count
	};
	enum class FormatType : uint8_t { Native = 0, Import, All };
	enum class UDMFormat : uint8_t { Binary = 0, Ascii, NotAUdmFormat };
	DLLNETWORK bool exists(const std::string &name, Type type);
	DLLNETWORK bool matches(const std::string &name0, const std::string &name1, Type type);
	DLLNETWORK std::string get_normalized_path(const std::string &name, Type type);
	DLLNETWORK std::optional<std::string> find_file(const std::string &name, Type type, std::string *optOutFormat = nullptr);
	DLLNETWORK bool remove_asset(const std::string &name, Type type);
	DLLNETWORK bool is_loaded(NetworkState &nw, const std::string &name, Type type);
	DLLNETWORK const std::vector<std::string> &get_supported_extensions(Type type, FormatType formatType = FormatType::Native);
	DLLNETWORK void update_extension_cache(Type type);
	DLLNETWORK void update_extension_cache();
	DLLNETWORK std::optional<std::string> get_legacy_extension(Type type);
	DLLNETWORK std::optional<std::string> get_binary_udm_extension(Type type);
	DLLNETWORK std::optional<std::string> get_ascii_udm_extension(Type type);
	DLLNETWORK std::optional<std::string> determine_format_from_data(ufile::IFile &f, Type type);
	DLLNETWORK std::optional<std::string> determine_format_from_filename(const std::string_view &fileName, Type type);
	DLLNETWORK std::optional<Type> determine_type_from_extension(const std::string_view &ext);
	DLLNETWORK bool matches_format(const std::string_view &format0, const std::string_view &format1);
	DLLNETWORK util::Path relative_path_to_absolute_path(const util::Path &relPath, Type type, const std::optional<std::string> &rootPath = {});
	DLLNETWORK util::Path absolute_path_to_relative_path(const util::Path &absPath, Type type);
	DLLNETWORK std::optional<std::string> get_udm_format_extension(Type type, bool binary);
	constexpr const char *get_asset_root_directory(Type type)
	{
		switch(type) {
		case Type::Model:
			return "models";
		case Type::Map:
			return "maps";
		case Type::Material:
		case Type::Texture:
			return "materials";
		case Type::Sound:
			return "sounds";
		case Type::ParticleSystem:
			return "particles";
		}
		return "";
	}

	struct DLLNETWORK IAssetWrapper {
		virtual ~IAssetWrapper() = default;
		virtual Type GetType() const = 0;
	};

	struct DLLNETWORK ModelAssetWrapper : public IAssetWrapper {
		virtual Type GetType() const override { return Type::Model; }
		void SetModel(Model &model);
		Model *GetModel() const;
	  private:
		std::shared_ptr<Model> m_model = nullptr;
	};

	struct DLLNETWORK MaterialAssetWrapper : public IAssetWrapper {
		virtual Type GetType() const override { return Type::Material; }
		void SetMaterial(Material &mat);
		Material *GetMaterial() const;
	  private:
		msys::MaterialHandle m_material = nullptr;
	};

	class DLLNETWORK AssetManager {
	  public:
		struct DLLNETWORK ImporterInfo {
			std::string name;
			std::string description;
			std::vector<std::pair<std::string, bool /* binary */>> fileExtensions;
		};
		using ExporterInfo = ImporterInfo;
		using ImportHandler = std::function<std::unique_ptr<IAssetWrapper>(Game &, ufile::IFile &, const std::optional<std::string> &, std::string &)>;
		using ExportHandler = std::function<bool(Game &, ufile::IFile &, const IAssetWrapper &, std::string &)>;
		void RegisterImporter(const ImporterInfo &importerInfo, Type type, const ImportHandler &importHandler);
		void RegisterExporter(const ExporterInfo &importerInfo, Type type, const ExportHandler &exportHandler);
		std::unique_ptr<IAssetWrapper> ImportAsset(Game &game, Type type, ufile::IFile *f, const std::optional<std::string> &filePath = {}, std::string *optOutErr = nullptr) const;
		bool ExportAsset(Game &game, Type type, ufile::IFile &f, const IAssetWrapper &assetWrapper, std::string *optOutErr = nullptr) const;

		uint32_t GetImporterCount(Type type) const { return m_importers[umath::to_integral(type)].size(); }
		uint32_t GetExporterCount(Type type) const { return m_exporters[umath::to_integral(type)].size(); }
		const ImporterInfo *GetImporterInfo(Type type, uint32_t idx) const
		{
			auto &importers = m_importers[umath::to_integral(type)];
			return (idx < importers.size()) ? &importers.at(idx).info : nullptr;
		}
		const ExporterInfo *GetExporterInfo(Type type, uint32_t idx) const
		{
			auto &exporters = m_exporters[umath::to_integral(type)];
			return (idx < exporters.size()) ? &exporters.at(idx).info : nullptr;
		}
	  private:
		struct DLLNETWORK Importer {
			ImporterInfo info;
			ImportHandler handler;
		};
		struct DLLNETWORK Exporter {
			ExporterInfo info;
			ExportHandler handler;
		};
		std::array<std::vector<Importer>, umath::to_integral(Type::Count)> m_importers;
		std::array<std::vector<Exporter>, umath::to_integral(Type::Count)> m_exporters;
	};

	struct DLLNETWORK VtfInfo {
		enum class Format : uint32_t {
			Bc1 = 0,
			Bc1a,
			Bc2,
			Bc3,
			R8G8B8A8_UNorm,
			R8G8_UNorm,
			R16G16B16A16_SFloat,
			R32G32B32A32_SFloat,
			A8B8G8R8_UNorm_Pack32,
			B8G8R8A8_UNorm,

			Count
		};
		enum class Flags : uint32_t { None = 0u, Srgb = 1u, NormalMap = Srgb << 1u, GenerateMipmaps = NormalMap << 1u };
		Format inputFormat = Format::R8G8B8A8_UNorm;
		Format outputFormat = Format::Bc1;
		Flags flags = Flags::None;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::asset::VtfInfo::Flags);

#endif
