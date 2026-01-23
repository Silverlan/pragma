// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.lights.spot;

export import :entities.base_entity;
export import :entities.components.entity;
export import :math.mvp_bias;

export namespace pragma {
	class DLLCLIENT CLightSpotComponent final : public BaseEnvLightSpotComponent, public CBaseNetComponent, public math::MVPBias<1> {
	  public:
		CLightSpotComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override;
		virtual void SetConeStartOffset(float offset) override;
		virtual void OnEntitySpawn() override;
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void SetFieldAngleComponent(BaseFieldAngleComponent &c) override;
		void SetShadowDirty();

		void UpdateInnerConeAngle();
		void UpdateViewMatrices();
		void UpdateProjectionMatrix();
		void UpdateTransformMatrix();
	};
};

export class DLLCLIENT CEnvLightSpot : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
