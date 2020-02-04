#ifndef __C_ENV_DECAL_H__
#define __C_ENV_DECAL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/environment/effects/c_env_sprite.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/environment/env_decal.h>
#include <pragma/model/vertex.h>

namespace pragma
{
	class DLLCLIENT DecalProjector
	{
	public:
		DecalProjector(const Vector3 &pos,const Quat &rot,float size);
		const Vector3 &GetPos() const;
		const Quat &GetRotation() const;
		const physics::Transform &GetPose() const;
		float GetSize() const;
		std::pair<Vector3,Vector3> GetAABB() const;

		bool GenerateDecalMesh(const std::vector<ModelSubMesh*> &meshes,const physics::Transform &pose,std::vector<Vertex> &outVerts,std::vector<uint16_t> &outTris);
		void DebugDraw(float duration) const;
	private:
		struct VertexInfo
		{
			// Position in projector space
			Vector2 position = {};
			uint32_t originalMeshVertexIndex = std::numeric_limits<uint32_t>::max();
			std::optional<Vector2> barycentricCoordinates = {};
		};
		void GetOrthogonalBasis(Vector3 &forward,Vector3 &right,Vector3 &up) const;
		const std::array<Vector3,8> &GetProjectorCubePoints() const;
		std::pair<Vector3,Vector3> GetProjectorCubeBounds() const;
		std::vector<VertexInfo> CropTriangleVertsByLine(
			const Vector3 &v0,const Vector3 &v1,const Vector3 &v2,
			const std::vector<VertexInfo> &verts,const Vector2 &lineStart,const Vector2 &lineEnd
		);

		physics::Transform m_pose = {};
		float m_size = 0.f;
	};

	class DLLCLIENT CDecalComponent final
		: public BaseEnvDecalComponent,
		public CBaseNetComponent
	{
	public:
		CDecalComponent(BaseEntity &ent) : BaseEnvDecalComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override {return true;}

		DecalProjector GetProjector() const;
		bool ApplyDecal(const std::vector<ModelSubMesh*> &meshes,const pragma::physics::ScaledTransform &pose={});
	protected:
		bool ApplyDecal();
		bool ApplyDecal(DecalProjector &projector,const std::vector<ModelSubMesh*> &meshes,const pragma::physics::ScaledTransform &pose={});
	};
};

class DLLCLIENT CEnvDecal
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
