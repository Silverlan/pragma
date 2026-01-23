// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.core;

export import :entities.base_entity_handle;

export import :types;
export import pragma.udm;
import util_zip;

export {
	namespace pragma::util {
		DLLNETWORK ParallelJob<std::vector<Vector2> &> generate_lightmap_uvs(NetworkState &nwState, uint32_t atlastWidth, uint32_t atlasHeight, const std::vector<math::Vertex> &verts, const std::vector<uint32_t> &tris);
		DLLNETWORK std::shared_ptr<udm::Data> load_udm_asset(const std::string &fileName, std::string *optOutErr = nullptr);
		DLLNETWORK std::shared_ptr<udm::Data> load_udm_asset(std::unique_ptr<ufile::IFile> &&f, std::string *optOutErr = nullptr);

		DLLNETWORK void write_udm_entity(udm::LinkedPropertyWrapperArg udm, EntityHandle &hEnt);
		DLLNETWORK EntityHandle read_udm_entity(Game &game, udm::LinkedPropertyWrapperArg udm);
		DLLNETWORK EntityHandle read_udm_entity(BaseEntityComponent &c, udm::LinkedPropertyWrapperArg udm);

		class HairFile {
		  public:
			static constexpr uint32_t PHAIR_VERSION = 1;
			static constexpr auto PHAIR_IDENTIFIER = "PHAIR";
			static std::shared_ptr<HairFile> Load(const udm::AssetData &data, std::string &outErr);
			static std::shared_ptr<HairFile> Create();
			bool Save(udm::AssetData &outData, std::string &outErr) const;
		  private:
			bool LoadFromAssetData(const udm::AssetData &data, std::string &outErr);
			HairConfig m_hairConfig;
			HairData m_hairData;
		};

		DLLNETWORK std::optional<std::string> convert_udm_file_to_ascii(const std::string &fileName, std::string &outErr);
		DLLNETWORK std::optional<std::string> convert_udm_file_to_binary(const std::string &fileName, std::string &outErr);

		DLLNETWORK std::pair<std::vector<double>, std::vector<double>> generate_two_pass_gaussian_blur_coefficients(uint32_t radius, double sigma, bool linear = true, bool correction = true);

		DLLNETWORK Path get_user_data_dir();
		DLLNETWORK std::vector<Path> get_resource_dirs();

		DLLNETWORK bool show_notification(const std::string &summary, const std::string &body);
		DLLNETWORK bool add_file_to_zip_archive(uzip::ZIPFile &zipArchive, const std::string &fileName, const void *data, uint64_t size, bool bOverwrite = true);
	};
};
