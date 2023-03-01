/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __NAVSYSTEM_H__
#define __NAVSYSTEM_H__

#include "pragma/networkdefinitions.h"
#include <udm_types.hpp>
#include <mathutil/glmutil.h>

class Game;
class rcContext;
struct rcHeightfield;
struct rcCompactHeightfield;
struct rcContourSet;
struct rcPolyMesh;
struct rcPolyMeshDetail;
typedef unsigned int dtPolyRef;
class dtNavMesh;
class dtNavMeshQuery;

class RcNavMesh;
class DLLNETWORK RcPathResult {
  public:
	std::vector<uint32_t> path;
	RcNavMesh &navMesh;
	uint32_t pathCount;
	Vector3 start;
	Vector3 end;
	bool GetNode(uint32_t nodeId, const Vector3 &closest, Vector3 &node) const;
	RcPathResult(RcNavMesh &pNavMesh, const std::shared_ptr<dtNavMeshQuery> &pQuery, Vector3 &pStart, Vector3 &pEnd, unsigned int numResults);
  private:
	std::shared_ptr<dtNavMeshQuery> query;
};

class DLLNETWORK RcNavMesh {
  public:
	RcNavMesh(const std::shared_ptr<rcPolyMesh> &polyMesh, const std::shared_ptr<rcPolyMeshDetail> &polyMeshDetail, const std::shared_ptr<dtNavMesh> &navMesh);
	dtNavMesh &GetNavMesh();
	rcPolyMesh &GetPolyMesh();
	rcPolyMeshDetail &GetPolyMeshDetail();
  private:
	std::shared_ptr<rcPolyMesh> m_polyMesh;
	std::shared_ptr<rcPolyMeshDetail> m_polyMeshDetail;
	std::shared_ptr<dtNavMesh> m_navMesh;
};

namespace udm {
	struct AssetData;
};

namespace pragma {
	namespace nav {
		static constexpr uint32_t PNAV_VERSION = 1;
		static constexpr auto PNAV_IDENTIFIER = "PNAV";
		static constexpr auto PNAV_EXTENSION_BINARY = "pnav_b";
		static constexpr auto PNAV_EXTENSION_ASCII = "pnav";
		enum class PolyFlags : uint16_t {
			None = 0u,
			Walk = 1u,
			Swim = Walk << 1u,
			Door = Swim << 1u,
			Jump = Door << 1u,
			Disabled = Jump << 1u,

			All = std::numeric_limits<uint16_t>::max()
		};
		struct DLLNETWORK ConvexArea {
			std::vector<Vector3> verts;
			uint8_t area = 0u;
		};
		struct DLLNETWORK Config {
			enum class PartitionType : uint32_t { Watershed, Monotone, Layers };

			Config() = default;
			Config(float walkableRadius, float characterHeight, float maxClimbHeight, float walkableSlopeAngle);
			float walkableRadius = 0.f;
			float characterHeight = 0.f;
			float maxClimbHeight = 0.f;
			float walkableSlopeAngle = 0.f;
			float maxEdgeLength = 0.f;
			float maxSimplificationError = 1.3f;
			int32_t minRegionSize = 8;
			int32_t mergeRegionSize = 20;

			float cellSize = 0.f;
			float cellHeight = 0.f;
			float vertsPerPoly = 6.f;
			float sampleDetailDist = 60.f;
			float sampleDetailMaxError = 1.f;
			PartitionType partitionType = PartitionType::Watershed;
		};
		DLLNETWORK std::shared_ptr<RcNavMesh> generate(Game &game, const Config &config, std::string *err = nullptr);
		DLLNETWORK std::shared_ptr<RcNavMesh> generate(Game &game, const Config &config, const BaseEntity &ent, std::string *err = nullptr);
		DLLNETWORK std::shared_ptr<RcNavMesh> generate(Game &game, const Config &config, const std::vector<Vector3> &verts, const std::vector<int32_t> &indices, const std::vector<ConvexArea> *areas = nullptr, std::string *err = nullptr);
		DLLNETWORK std::shared_ptr<RcNavMesh> load(Game &game, const std::string &fname, Config &outConfig);
		class DLLNETWORK Mesh {
		  public:
			template<class TMesh>
			static std::shared_ptr<TMesh> Create(const std::shared_ptr<RcNavMesh> &rcMesh, const Config &config);
			template<class TMesh>
			static std::shared_ptr<TMesh> Load(Game &game, const std::string &fname);
			static std::shared_ptr<Mesh> Create(const std::shared_ptr<RcNavMesh> &rcMesh, const Config &config);
			static std::shared_ptr<Mesh> Load(Game &game, const std::string &fname);

			std::shared_ptr<RcPathResult> FindPath(const Vector3 &start, const Vector3 &end);
			bool RayCast(const Vector3 &start, const Vector3 &end, Vector3 &hit);
			bool Save(Game &game, udm::AssetDataArg outData, std::string &outErr);
			bool Save(Game &game, const std::string &fileName, std::string &outErr);

			const Config &GetConfig() const;

			const std::shared_ptr<RcNavMesh> &GetRcNavMesh() const;
			std::shared_ptr<RcNavMesh> &GetRcNavMesh();
		  protected:
			friend DLLNETWORK std::shared_ptr<RcNavMesh> load(Game &game, const std::string &fname, Config &outConfig);
			Mesh(const std::shared_ptr<RcNavMesh> &rcMesh, const Config &config);
			Mesh() = default;
			bool LoadFromAssetData(Game &game, const udm::AssetData &data, std::string &outErr);
			bool FindNearestPoly(const Vector3 &pos, dtPolyRef &ref);
		  private:
			std::shared_ptr<RcNavMesh> m_rcMesh;
			Config m_config = {};
		};
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::nav::PolyFlags);

template<class TMesh>
std::shared_ptr<TMesh> pragma::nav::Mesh::Create(const std::shared_ptr<RcNavMesh> &rcMesh, const Config &config)
{
	return std::shared_ptr<TMesh>(new TMesh(rcMesh, config));
}
template<class TMesh>
std::shared_ptr<TMesh> pragma::nav::Mesh::Load(Game &game, const std::string &fname)
{
	Config config;
	auto rcMesh = load(game, fname, config);
	return (rcMesh != nullptr) ? Create<TMesh>(rcMesh, config) : nullptr;
}

#endif
