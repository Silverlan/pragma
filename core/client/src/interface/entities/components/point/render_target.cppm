// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.point_render_target;

export import :entities.base_entity;
export import :entities.components.camera;
export import :entities.components.entity;
export import :rendering.enums;
export import pragma.cmaterialsystem;

export {
	namespace pragma {
		class DLLCLIENT CRenderTargetComponent final : public BasePointRenderTargetComponent, public CBaseNetComponent {
		  public:
			CRenderTargetComponent(ecs::BaseEntity &ent) : BasePointRenderTargetComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		  protected:
			void SetRenderSize(Vector2 &size);
			void SetRenderSize(float w, float h);
			Vector2 GetRenderSize();
			void GetRenderSize(float *w, float *h);

			void SetRenderMaterial(material::Material *mat);
			material::Material *GetRenderMaterial();
			void SetRenderMaterial(std::string mat);

			void SetRefreshRate(float f);
			float GetRefreshRate();

			void SetRenderFOV(float fov);
			float GetRenderFOV();

			void SetRenderDepth(unsigned int depth);
			unsigned int GetRenderDepth();

			void Spawn();
			unsigned int GetTextureBuffer();
			void Render(rendering::SceneRenderPass renderMode);

			std::unique_ptr<material::Texture> m_texture = nullptr;
			material::Material *m_matRender = nullptr;
			float m_refreshRate = 0.f;
			double m_tLastRefresh;
			unsigned int m_curDepth = 0u;
			util::WeakHandle<CCameraComponent> m_cam = {};
		};
	};

	class DLLCLIENT CPointRenderTarget : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
