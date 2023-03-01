/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
		CLightPointComponent(BaseEntity &ent) : BaseEnvLightPointComponent(ent) {}
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
