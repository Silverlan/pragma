#ifndef __C_ENV_LIGHT_DIRECTIONAL_H__
#define __C_ENV_LIGHT_DIRECTIONAL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include <pragma/util/mvpbase.h>

namespace pragma
{
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
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void OnEntitySpawn() override;

		void UpdateFrustum(uint32_t frustumId);
		void UpdateFrustum();
		//void SetDirection(const Vector3 &dir);

		void ReloadShadowCommandBuffers();
		//const std::vector<Vulkan::SwapCommandBuffer> &GetShadowCommandBuffers() const; // prosper TODO
		//const Vulkan::SwapCommandBuffer *GetShadowCommandBuffer(uint32_t layer) const; // prosper TODO
		bool ShouldPass(uint32_t layer,const Vector3 &min,const Vector3 &max);
	protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void UpdateAmbientColor();

		bool m_bShadowBufferUpdateScheduled;
		//std::vector<Vulkan::SwapCommandBuffer> m_cmdShadowBuffers; // prosper TODO
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