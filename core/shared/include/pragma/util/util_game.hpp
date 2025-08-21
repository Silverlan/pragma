// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_GAME_HPP__
#define __UTIL_GAME_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include <udm_types.hpp>
#include <sharedutils/util_pragma.hpp>
#include <sharedutils/util_parallel_job.hpp>
#include <sharedutils/util_ifile.hpp>
#include <sharedutils/util_path.hpp>
#include <mathutil/uvec.h>
#include <mathutil/vertex.hpp>
#include <vector>
#include <cinttypes>

class NetworkState;
class Game;
namespace udm {
	class Data;
	struct AssetData;
	struct LinkedPropertyWrapper;
};
namespace pragma {
	class BaseEntityComponent;
};
class VFilePtrInternal;
#include <sharedutils/util_hair.hpp>
namespace util {
	DLLNETWORK util::ParallelJob<std::vector<Vector2> &> generate_lightmap_uvs(NetworkState &nwState, uint32_t atlastWidth, uint32_t atlasHeight, const std::vector<umath::Vertex> &verts, const std::vector<uint32_t> &tris);
	DLLNETWORK std::shared_ptr<udm::Data> load_udm_asset(const std::string &fileName, std::string *optOutErr = nullptr);
	DLLNETWORK std::shared_ptr<udm::Data> load_udm_asset(std::unique_ptr<ufile::IFile> &&f, std::string *optOutErr = nullptr);

	DLLNETWORK void write_udm_entity(udm::LinkedPropertyWrapperArg udm, EntityHandle &hEnt);
	DLLNETWORK EntityHandle read_udm_entity(Game &game, udm::LinkedPropertyWrapperArg udm);
	DLLNETWORK EntityHandle read_udm_entity(::pragma::BaseEntityComponent &c, udm::LinkedPropertyWrapperArg udm);

	struct HairConfig;
	struct HairData;
	class HairFile {
	  public:
		static constexpr uint32_t PHAIR_VERSION = 1;
		static constexpr auto PHAIR_IDENTIFIER = "PHAIR";
		static std::shared_ptr<HairFile> Load(const udm::AssetData &data, std::string &outErr);
		static std::shared_ptr<HairFile> Create();
		bool Save(udm::AssetData &outData, std::string &outErr) const;
	  private:
		bool LoadFromAssetData(const udm::AssetData &data, std::string &outErr);
		util::HairConfig m_hairConfig;
		HairData m_hairData;
	};

	enum class FogType : uint8_t { Linear = 0, Exponential = 1, Exponential2 = 2 };

	DLLNETWORK std::optional<std::string> convert_udm_file_to_ascii(const std::string &fileName, std::string &outErr);
	DLLNETWORK std::optional<std::string> convert_udm_file_to_binary(const std::string &fileName, std::string &outErr);

	DLLNETWORK std::pair<std::vector<double>, std::vector<double>> generate_two_pass_gaussian_blur_coefficients(uint32_t radius, double sigma, bool linear = true, bool correction = true);

	DLLNETWORK util::Path get_user_data_dir();
	DLLNETWORK std::vector<util::Path> get_resource_dirs();

	DLLNETWORK bool show_notification(const std::string &summary, const std::string &body);
};

#endif
