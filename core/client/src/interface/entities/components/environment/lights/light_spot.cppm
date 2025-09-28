// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.client:entities.components.lights.spot;

import :entities.base_entity;
import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CLightSpotComponent final : public BaseEnvLightSpotComponent, public CBaseNetComponent, public MVPBias<1> {
	  public:
		CLightSpotComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override;
		virtual void SetConeStartOffset(float offset) override;
		virtual void OnEntitySpawn() override;
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void SetFieldAngleComponent(BaseFieldAngleComponent &c) override;
		void SetShadowDirty();

		void UpdateInnerConeAngle();
		void UpdateViewMatrices();
		void UpdateProjectionMatrix();
		void UpdateTransformMatrix();
	};
};

export class DLLCLIENT CEnvLightSpot : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
