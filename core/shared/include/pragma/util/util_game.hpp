/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __UTIL_GAME_HPP__
#define __UTIL_GAME_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include <udm_types.hpp>
#include <sharedutils/util_pragma.hpp>
#include <sharedutils/util_parallel_job.hpp>
#include <mathutil/uvec.h>
#include <mathutil/vertex.hpp>
#include <vector>
#include <cinttypes>

class NetworkState;
class Game;
namespace udm {class Data; struct AssetData; struct LinkedPropertyWrapper;};
namespace pragma {class BaseEntityComponent;};
class VFilePtrInternal;
#include <sharedutils/util_hair.hpp>
namespace util
{
	DLLNETWORK util::ParallelJob<std::vector<Vector2>&> generate_lightmap_uvs(NetworkState &nwState,uint32_t atlastWidth,uint32_t atlasHeight,const std::vector<umath::Vertex> &verts,const std::vector<uint32_t> &tris);
	DLLNETWORK std::shared_ptr<udm::Data> load_udm_asset(const std::string &fileName,std::string *optOutErr=nullptr);
	DLLNETWORK std::shared_ptr<udm::Data> load_udm_asset(std::shared_ptr<VFilePtrInternal> f,std::string *optOutErr=nullptr);

	DLLNETWORK void write_udm_entity(udm::LinkedPropertyWrapperArg udm,EntityHandle &hEnt);
	DLLNETWORK EntityHandle read_udm_entity(Game &game,udm::LinkedPropertyWrapperArg udm);
	DLLNETWORK EntityHandle read_udm_entity(::pragma::BaseEntityComponent &c,udm::LinkedPropertyWrapperArg udm);
	
	struct HairConfig;
	struct HairData;
	class HairFile
	{
	public:
		static constexpr uint32_t PHAIR_VERSION = 1;
		static constexpr auto PHAIR_IDENTIFIER = "PHAIR";
		static std::shared_ptr<HairFile> Load(const udm::AssetData &data,std::string &outErr);
		static std::shared_ptr<HairFile> Create();
		bool Save(udm::AssetData &outData,std::string &outErr) const;
	private:
		bool LoadFromAssetData(const udm::AssetData &data,std::string &outErr);
		util::HairConfig m_hairConfig;
		HairData m_hairData;
	};
};

#endif
