/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_LIGHT_DIRECTIONAL_H__
#define __C_ENV_LIGHT_DIRECTIONAL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include <pragma/util/mvpbase.h>

namespace pragma
{
	class CShadowCSMComponent;
	class DLLCLIENT CLightDirectionalComponent final
		: public BaseEnvLightDirectionalComponent,
		public CBaseNetComponent,
		public MVPBias<1>
	{
	public:
		CLightDirectionalComponent(BaseEntity &ent) : BaseEnvLightDirectionalComponent(ent) {}
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		virtual void SetAmbientColor(const Color &color) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void OnEntitySpawn() override;

		CShadowCSMComponent *GetShadowMap();

		void UpdateFrustum(uint32_t frustumId);
		void UpdateFrustum();
		//void SetDirection(const Vector3 &dir);

		void ReloadShadowCommandBuffers();
		bool ShouldPass(uint32_t layer,const Vector3 &min,const Vector3 &max);
	protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void UpdateAmbientColor();
		void SetShadowDirty();

		ComponentHandle<CShadowCSMComponent> m_shadowMap = {};
		bool m_bShadowBufferUpdateScheduled;
		void RenderStaticWorldGeometry();
	};
};

class DLLCLIENT CEnvLightDirectional
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif