/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __RAYTRACES_H__
#define __RAYTRACES_H__

#include <mathutil/glmutil.h>
#include "pragma/physics/transform.hpp"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/physics/physobj.h"
#include <sharedutils/util_weak_handle.hpp>
#include <memory>

enum class RayCastFlags : uint32_t
{
	None = 0u,
	ReportHitPosition = 1u,
	ReportHitNormal = ReportHitPosition<<1u,
	ReportHitUV = ReportHitNormal<<1u,
	ReportAllResults = ReportHitUV<<1u,
	ReportAnyResult = ReportAllResults<<1u,
	ReportBackFaceHits = ReportAnyResult<<1u,
	Precise = ReportBackFaceHits<<1u,

	IgnoreDynamic = Precise<<1u,
	IgnoreStatic = IgnoreDynamic<<1u,
	InvertFilter = IgnoreStatic<<1u,

	Default = ReportHitPosition | ReportHitNormal | ReportHitUV
};
REGISTER_BASIC_BITWISE_OPERATORS(RayCastFlags);

enum class RayCastHitType : uint8_t
{
	None = 0,
	Touch,
	Block
};

namespace pragma::physics {class IConvexShape; class IRayCastFilterCallback;};
class DLLNETWORK TraceData
{
public:
	TraceData();
	TraceData(const TraceData &other);
	RayCastFlags GetFlags() const;
	const pragma::physics::Transform &GetSource() const;
	const pragma::physics::Transform &GetTarget() const;
	Vector3 GetSourceOrigin() const;
	Quat GetSourceRotation() const;
	Vector3 GetTargetOrigin() const;
	Quat GetTargetRotation() const;
	Vector3 GetDirection() const;
	float GetDistance() const;
	CollisionMask GetCollisionFilterMask() const;
	CollisionMask GetCollisionFilterGroup() const;
	void SetShape(const pragma::physics::IConvexShape &shape);
	const pragma::physics::IConvexShape *GetShape() const;
	void SetSource(const Vector3 &origin);
	void SetSourceRotation(const Quat &rot);
	void SetSource(const pragma::physics::Transform &t);
	void SetTarget(const Vector3 &target);
	void SetTargetRotation(const Quat &rot);
	void SetTarget(const pragma::physics::Transform &t);
	void SetRotation(const Quat &rot);
	void SetFlags(RayCastFlags flags);
	void SetCollisionFilterMask(CollisionMask mask);
	void SetCollisionFilterGroup(CollisionMask group);
	void SetFilter(const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &filter);
	void SetFilter(BaseEntity &ent);
	void SetFilter(std::vector<EntityHandle> &&ents);
	void SetFilter(PhysObj &phys);
	void SetFilter(pragma::physics::ICollisionObject &colObj);
	void SetFilter(
		const std::function<RayCastHitType(pragma::physics::IShape&,pragma::physics::IRigidBody&)> &preFilter,
		const std::function<RayCastHitType(pragma::physics::IShape&,pragma::physics::IRigidBody&)> &postFilter=nullptr
	);
	const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &GetFilter() const;
	bool HasFlag(RayCastFlags flag) const;
protected:
	bool HasTarget() const;
private:
	pragma::physics::Transform m_tStart;
	pragma::physics::Transform m_tEnd;

	RayCastFlags m_flags;
	bool m_bHasTarget = false;
	CollisionMask m_filterMask;
	CollisionMask m_filterGroup;
	std::shared_ptr<pragma::physics::IRayCastFilterCallback> m_filter = nullptr;
	util::WeakHandle<pragma::physics::IConvexShape> m_shape = {};
};

class ModelMesh;
class ModelSubMesh;
class Material;
class PhysObj;
struct DLLNETWORK TraceResult
{
	TraceResult()
	{}
	TraceResult(const TraceData &data);
	RayCastHitType hitType = RayCastHitType::None;
	EntityHandle entity = {};
	util::TWeakSharedHandle<pragma::physics::ICollisionObject> collisionObj = {};
	std::weak_ptr<pragma::physics::IShape> shape = {};
	PhysObjHandle physObj = {};
	float fraction = 0.f;

	float distance = 0.f;
	Vector3 normal = {};
	Vector3 position = {};
	Vector3 startPosition = Vector3{};

	void GetMeshes(ModelMesh **mesh,ModelSubMesh **subMesh);
	Material *GetMaterial();
	bool GetMaterial(std::string &mat);
private:
	struct MeshInfo
	{
		std::vector<std::shared_ptr<ModelMesh>> meshes;
		ModelMesh *mesh = nullptr;
		ModelSubMesh *subMesh = nullptr;
	};
	std::shared_ptr<MeshInfo> m_meshInfo = nullptr;
	void InitializeMeshes();
};

#endif
