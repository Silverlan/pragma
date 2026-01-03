// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.lights.point;

export import :entities.base_entity;
export import :rendering.cube_map_side;
export import :math.mvp_bias;

export namespace pragma {
	class DLLCLIENT CLightPointComponent final : public BaseEnvLightPointComponent, public math::MVPBias<6> {
	  public:
		CLightPointComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		const std::array<std::vector<math::Plane>, 6u> &GetFrustumPlanes() const;
		const std::vector<math::Plane> &GetFrustumPlanes(rendering::CubeMapSide side) const;
	  protected:
		void UpdateProjectionMatrix();
		void UpdateFrustumPlanes();
		bool m_bSkipMatrixUpdate = false;
		// Frustum planes for each side of the cubemap
		std::array<std::vector<math::Plane>, 6u> m_frustumPlanes = {};
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void UpdateTransformationMatrix(unsigned int j) override;
		void SetShadowDirty();
		void UpdateViewMatrices();
	};
};

export class DLLCLIENT CEnvLightPoint : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
