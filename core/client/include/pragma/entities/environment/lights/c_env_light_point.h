// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ENV_LIGHT_POINT_H__
#define __C_ENV_LIGHT_POINT_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/env_light_point.h"
#include "pragma/rendering/c_cubemapside.h"
#include <mathutil/plane.hpp>
#include <pragma/util/mvpbase.h>

namespace pragma {
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

class DLLCLIENT CEnvLightPoint : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
