// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.env_decal;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT DecalProjector {
		  public:
			struct DLLCLIENT MeshData {
				std::vector<geometry::ModelSubMesh *> subMeshes {};
				math::ScaledTransform pose = {};
			};
			DecalProjector(const Vector3 &pos, const Quat &rot, float size);
			const Vector3 &GetPos() const;
			const Quat &GetRotation() const;
			const math::Transform &GetPose() const;
			float GetSize() const;
			std::pair<Vector3, Vector3> GetAABB() const;

			bool GenerateDecalMesh(const std::vector<MeshData> &meshDatas, std::vector<math::Vertex> &outVerts, std::vector<uint16_t> &outTris);
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

			math::Transform m_pose = {};
			float m_size = 0.f;
		};

		class DLLCLIENT CDecalComponent final : public BaseEnvDecalComponent, public CBaseNetComponent {
		  public:
			CDecalComponent(ecs::BaseEntity &ent) : BaseEnvDecalComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;
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

	class DLLCLIENT CEnvDecal : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
