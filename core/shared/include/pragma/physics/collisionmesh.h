/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __COLLISIONMESH_H__
#define __COLLISIONMESH_H__

#include "pragma/networkdefinitions.h"
#include <udm_types.hpp>
#include <mathutil/glmutil.h>
#include "pragma/model/modelupdateflags.hpp"
#include <vector>
#include <memory>

class Game;
struct PhysSoftBodyInfo;
class ModelSubMesh;
class Model;
namespace pragma {
	enum class Axis : uint8_t;
};
namespace pragma::physics {
	class IShape;
};
namespace udm {
	struct AssetData;
	using Version = uint32_t;
};
class DLLNETWORK CollisionMesh : public std::enable_shared_from_this<CollisionMesh> {
  public:
	static constexpr auto PCOL_IDENTIFIER = "PCOL";
	static constexpr udm::Version PCOL_VERSION = 1;
#pragma pack(push, 1)
	struct DLLNETWORK SoftBodyAnchor {
		enum class Flags : uint8_t { None = 0u, Rigid = 1u, DisableCollisions = Rigid << 1u };
		uint16_t vertexIndex = std::numeric_limits<uint16_t>::max();
		uint32_t boneId = std::numeric_limits<uint32_t>::max();
		float influence = 1.f;
		Flags flags = Flags::None;

		bool operator==(const SoftBodyAnchor &other) const;
		bool operator!=(const SoftBodyAnchor &other) const { return !operator==(other); }
	};
#pragma pack(pop)
	CollisionMesh(const CollisionMesh &other);
	bool operator==(const CollisionMesh &other) const;
	bool operator!=(const CollisionMesh &other) const { return !operator==(other); }
  private:
	CollisionMesh(Game *game);

	//void PhysSoftBody::AppendAnchor(uint32_t nodeId,PhysRigidBody &body,const Vector3 &localPivot,bool bDisableCollision,float influence)
	struct DLLNETWORK SoftBodyInfo {
		SoftBodyInfo();
		std::weak_ptr<ModelSubMesh> subMesh = {};
		std::vector<uint32_t> triangles; // Triangles of sub-mesh to use of soft-body physics
		std::shared_ptr<PhysSoftBodyInfo> info = nullptr;
		std::vector<SoftBodyAnchor> anchors;
		bool operator==(const SoftBodyInfo &other) const;
		bool operator!=(const SoftBodyInfo &other) const { return !operator==(other); }
	};

	std::shared_ptr<SoftBodyInfo> m_softBodyInfo = nullptr;

	Game *m_game = nullptr;
	std::vector<Vector3> m_vertices;
	std::vector<uint16_t> m_triangles;
	std::vector<int> m_surfaceMaterials;
	Vector3 m_min = {};
	Vector3 m_max = {};
	Vector3 m_origin = {};
	float m_mass = 0.f;
	std::shared_ptr<pragma::physics::IShape> m_shape = nullptr;
	bool m_bConvex = true;
	int m_boneID = -1;
	int m_surfaceMaterialId = 0;
	Vector3 m_centerOfMass = {};
	double m_volume = 0.0;
	util::Uuid m_uuid;
	void ClipAgainstPlane(const Vector3 &n, double d, CollisionMesh &clippedMesh);
	bool LoadFromAssetData(Game &game, Model &mdl, const udm::AssetData &data, std::string &outErr);
  public:
	static std::shared_ptr<CollisionMesh> Create(Game *game);
	static std::shared_ptr<CollisionMesh> Create(const CollisionMesh &other);
	static std::shared_ptr<CollisionMesh> Load(Game &game, Model &mdl, const udm::AssetData &data, std::string &outErr);
	std::shared_ptr<pragma::physics::IShape> CreateShape(const Vector3 &scale = {1.f, 1.f, 1.f}) const;
	void SetBoneParent(int boneID);
	int GetBoneParent() const;
	void CalculateBounds();
	void Validate();
	void GetAABB(Vector3 *min, Vector3 *max) const;
	void SetAABB(Vector3 &min, Vector3 &max);
	void SetOrigin(const Vector3 &origin);
	const Vector3 &GetOrigin() const;
	Vector3 &GetOrigin();
	std::shared_ptr<pragma::physics::IShape> GetShape();
	bool IntersectAABB(Vector3 *min, Vector3 *max);
	void UpdateShape();
	void ClearShape();
	void SetConvex(bool bConvex);
	bool IsConvex() const;
	std::vector<Vector3> &GetVertices();
	std::vector<int> &GetSurfaceMaterials();
	void SetSurfaceMaterial(int id);
	void SetSurfaceMaterial(const std::string &surfMat);
	int GetSurfaceMaterial() const;
	void SetMass(float mass);
	float GetMass() const;
	void Update(ModelUpdateFlags flags = ModelUpdateFlags::AllData);
	void AddVertex(const Vector3 &v);
	void Rotate(const Quat &rot);
	void Translate(const Vector3 &t);
	void Scale(const Vector3 &scale);
	void Mirror(pragma::Axis axis);
	void Centralize();

	const util::Uuid &GetUuid() const;
	void SetUuid(const util::Uuid &uuid);

	// Triangles can be empty if the collision point-cloud was never triangulated
	const std::vector<uint16_t> &GetTriangles() const;
	std::vector<uint16_t> &GetTriangles();
	void CalculateVolumeAndCom();
	const Vector3 &GetCenterOfMass() const;
	void SetCenterOfMass(const Vector3 &com);
	double GetVolume() const;
	void SetVolume(double vol);

	void ClipAgainstPlane(const Vector3 &n, double d, CollisionMesh &clippedMeshA, CollisionMesh &clippedMeshB);

	void SetSoftBody(bool b);
	bool IsSoftBody() const;
	ModelSubMesh *GetSoftBodyMesh() const;
	void SetSoftBodyMesh(ModelSubMesh &mesh);
	const std::vector<uint32_t> *GetSoftBodyTriangles() const;
	std::vector<uint32_t> *GetSoftBodyTriangles();
	PhysSoftBodyInfo *GetSoftBodyInfo() const;
	bool AddSoftBodyAnchor(uint16_t vertIdx, uint32_t boneIdx, SoftBodyAnchor::Flags flags = SoftBodyAnchor::Flags::None, float influence = 1.f, uint32_t *anchorIdx = nullptr);
	void RemoveSoftBodyAnchor(uint32_t anchorIdx);
	void ClearSoftBodyAnchors();
	const std::vector<SoftBodyAnchor> *GetSoftBodyAnchors() const;
	std::vector<SoftBodyAnchor> *GetSoftBodyAnchors();

	bool Save(Game &game, Model &mdl, udm::AssetDataArg outData, std::string &outErr);
};
REGISTER_BASIC_BITWISE_OPERATORS(CollisionMesh::SoftBodyAnchor::Flags);

DLLNETWORK std::ostream &operator<<(std::ostream &out, const CollisionMesh &o);

#endif
