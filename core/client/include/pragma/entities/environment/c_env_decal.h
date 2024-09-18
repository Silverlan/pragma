/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_DECAL_H__
#define __C_ENV_DECAL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/environment/effects/c_env_sprite.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/environment/env_decal.h>
#include <mathutil/vertex.hpp>

namespace pragma {
	class DLLCLIENT DecalProjector {
	  public:
		struct DLLCLIENT MeshData {
			std::vector<ModelSubMesh *> subMeshes {};
			umath::ScaledTransform pose = {};
		};
		DecalProjector(const Vector3 &pos, const Quat &rot, float size);
		const Vector3 &GetPos() const;
		const Quat &GetRotation() const;
		const umath::Transform &GetPose() const;
		float GetSize() const;
		std::pair<Vector3, Vector3> GetAABB() const;

		bool GenerateDecalMesh(const std::vector<MeshData> &meshDatas, std::vector<umath::Vertex> &outVerts, std::vector<uint16_t> &outTris);
		void DebugDraw(float duration) const;
	  private:
		struct VertexInfo {
			// Position in projector space
			Vector2 position = {};
			uint32_t originalMeshVertexIndex = std::numeric_limits<uint32_t>::max();
			std::optional<Vector2> barycentricCoordinates = {};
		};
		void GetOrthogonalBasis(Vector3 &forward, Vector3 &right, Vector3 &up) const;
		const std::array<Vector3, 8> &GetProjectorCubePoints() const;
		std::pair<Vector3, Vector3> GetProjectorCubeBounds() const;
		std::vector<VertexInfo> CropTriangleVertsByLine(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const std::vector<VertexInfo> &verts, const Vector2 &lineStart, const Vector2 &lineEnd);

		umath::Transform m_pose = {};
		float m_size = 0.f;
	};

	class DLLCLIENT CDecalComponent final : public BaseEnvDecalComponent, public CBaseNetComponent {
	  public:
		CDecalComponent(BaseEntity &ent) : BaseEnvDecalComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		virtual void SetSize(float size) override;
		virtual void SetMaterial(const std::string &mat) override;

		DecalProjector GetProjector() const;
		bool ApplyDecal(const std::vector<DecalProjector::MeshData> &meshes);
		bool ApplyDecal();

		void DebugDraw(float duration) const;
		virtual void OnTick(double dt) override;
	  protected:
		bool ApplyDecal(DecalProjector &projector, const std::vector<DecalProjector::MeshData> &meshes);

		bool m_decalDirty = true;
	};
};

class DLLCLIENT CEnvDecal : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
