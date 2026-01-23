// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.skybox;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CSkyboxComponent final : public BaseSkyboxComponent, public CBaseNetComponent {
		  public:
			CSkyboxComponent(ecs::BaseEntity &ent) : BaseSkyboxComponent(ent) {}
			virtual void Initialize() override;
			virtual void OnRemove() override;
			virtual void InitializeLuaObject(lua::State *l) override;

			virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
			virtual void ReceiveData(NetPacket &packet) override;
			virtual bool ShouldTransmitNetData() const override { return true; }

			virtual void SetSkyAngles(const EulerAngles &ang) override;
			const EulerAngles &GetSkyAngles() const;
			const Vector4 &GetRenderSkyAngles() const;

			void SetSkyMaterial(material::Material *mat);
			void ValidateMaterials();
		  private:
			bool CreateCubemapFromIndividualTextures(const std::string &materialPath, const std::string &postfix = "") const;
			CallbackHandle m_cbOnModelMaterialsLoaded = {};
			EulerAngles m_skyAngles;
			Vector4 m_renderSkyAngles;
		};
	};

	class DLLCLIENT CSkybox : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
