// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.lights.directional;

export import :entities.components.lights.shadow_csm;
export import :math.mvp_bias;

export namespace pragma {
	class DLLCLIENT CLightDirectionalComponent final : public BaseEnvLightDirectionalComponent, public CBaseNetComponent, public math::MVPBias<1> {
	  public:
		CLightDirectionalComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void SetAmbientColor(const Color &color) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override;
		virtual void OnEntitySpawn() override;

		CShadowCSMComponent *GetShadowMap();

		void UpdateFrustum(uint32_t frustumId);
		void UpdateFrustum();
		//void SetDirection(const Vector3 &dir);

		void ReloadShadowCommandBuffers();
		bool ShouldPass(uint32_t layer, const Vector3 &min, const Vector3 &max);
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void UpdateAmbientColor();
		void SetShadowDirty();

		ComponentHandle<CShadowCSMComponent> m_shadowMap = {};
		bool m_bShadowBufferUpdateScheduled;
		void RenderStaticWorldGeometry();
	};
};

export class DLLCLIENT CEnvLightDirectional : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
