// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <mathutil/plane.hpp>

export module pragma.client:entities.components.lights.point;

import :entities.base_entity;
import :rendering.cube_map_side;

export namespace pragma {
	class DLLCLIENT CLightPointComponent final : public BaseEnvLightPointComponent, public MVPBias<6> {
	  public:
		CLightPointComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		const std::array<std::vector<umath::Plane>, 6u> &GetFrustumPlanes() const;
		const std::vector<umath::Plane> &GetFrustumPlanes(CubeMapSide side) const;
	  protected:
		void UpdateProjectionMatrix();
		void UpdateFrustumPlanes();
		bool m_bSkipMatrixUpdate = false;
		// Frustum planes for each side of the cubemap
		std::array<std::vector<umath::Plane>, 6u> m_frustumPlanes = {};
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void UpdateTransformationMatrix(unsigned int j) override;
		void SetShadowDirty();
		void UpdateViewMatrices();
	};
};

export class DLLCLIENT CEnvLightPoint : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
