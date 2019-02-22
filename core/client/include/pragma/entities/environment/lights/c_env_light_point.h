#ifndef __C_ENV_LIGHT_POINT_H__
#define __C_ENV_LIGHT_POINT_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/env_light_point.h"
#include <pragma/util/mvpbase.h>

namespace pragma
{
	class DLLCLIENT CLightPointComponent final
		: public BaseEnvLightPointComponent,public MVPBias<6>
	{
	public:
		CLightPointComponent(BaseEntity &ent) : BaseEnvLightPointComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		void UpdateProjectionMatrix();
		void UpdateFrustumPlanes();
		bool m_bSkipMatrixUpdate = false;
		// Frustum planes for each side of the cubemap
		std::array<std::vector<Plane>,6u> m_frustumPlanes = {};
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void UpdateTransformationMatrix(unsigned int j) override;
		void UpdateViewMatrices();
	};
};

class DLLCLIENT CEnvLightPoint
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif