// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.lights.spot_vol;

export import :entities.base_entity;
export import :entities.components.entity;
import :model.mesh;

export namespace pragma {
	class DLLCLIENT CLightSpotVolComponent final : public BaseEnvLightSpotVolComponent, public CBaseNetComponent {
	  public:
		CLightSpotVolComponent(ecs::BaseEntity &ent) : BaseEnvLightSpotVolComponent(ent) {}
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;
	  protected:
		float CalcEndRadius() const;
		uint32_t CalcSegmentCount() const;
		bool UpdateMeshData();
		void InitializeVolumetricLight();
		virtual bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		std::vector<std::shared_ptr<geometry::CModelSubMesh>> m_subMeshes;
		std::shared_ptr<asset::Model> m_model;
		material::MaterialHandle m_material;
	};
};

export class DLLCLIENT CEnvLightSpotVol : public pragma::ecs::CBaseEntity {
	virtual void Initialize() override;
};
