// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_path.hpp>
#include <material.h>
#include <memory>

export module pragma.shared:assets.common;

export class NetworkState;
export class Model;
export namespace pragma::asset {
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

	static constexpr auto FORMAT_SHADER_GRAPH_BINARY = "psg_b";
	static constexpr auto FORMAT_SHADER_GRAPH_ASCII = "psg";

	enum class Type : uint8_t {
		Model = 0,
		Map,
		Material,
		Texture,
		Sound,
		ParticleSystem,
		ShaderGraph,

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
	DLLNETWORK ::util::Path relative_path_to_absolute_path(const ::util::Path &relPath, Type type, const std::optional<std::string> &rootPath = {});
	DLLNETWORK ::util::Path absolute_path_to_relative_path(const ::util::Path &absPath, Type type);
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
		case Type::ShaderGraph:
			return "scripts/shader_data/graphs/object";
		}
		static_assert(umath::to_integral(Type::Count) == 7, "New asset type added, please update get_asset_root_directory");
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
};
