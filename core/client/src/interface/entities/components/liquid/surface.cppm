// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:entities.components.liquid_surface;

export import :entities.components.renderer;
export import :entities.components.surface;

export namespace pragma {
	struct DLLCLIENT WaterScene {
		~WaterScene();
		pragma::util::TWeakSharedHandle<pragma::CSceneComponent> sceneReflection = {};
		std::shared_ptr<prosper::IBuffer> settingsBuffer = nullptr;

		// Fog
		std::shared_ptr<prosper::IBuffer> fogBuffer = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> fogDescSetGroup = nullptr;

		std::shared_ptr<prosper::IDescriptorSetGroup> descSetGroupTexEffects = nullptr;

		std::vector<CallbackHandle> hPostProcessing = {};
		CallbackHandle hComponentCreationCallback = {};

		std::shared_ptr<prosper::Texture> texScene = nullptr;
		std::shared_ptr<prosper::Texture> texSceneDepth = nullptr;

		pragma::util::PFloatProperty waterScale = nullptr;
		pragma::util::PFloatProperty waveStrength = nullptr;
		pragma::util::PFloatProperty waveSpeed = nullptr;
		pragma::util::PFloatProperty reflectiveIntensity = nullptr;
	};

	class DLLCLIENT CLiquidSurfaceComponent final : public BaseLiquidSurfaceComponent, public CBaseNetComponent {
	  public:
		CLiquidSurfaceComponent(pragma::ecs::BaseEntity &ent) : BaseLiquidSurfaceComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnRemove() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void ReceiveData(NetPacket &packet) override;

		void ClearWaterScene();
		const WaterScene &GetWaterScene() const;
		prosper::IDescriptorSet *GetEffectDescriptorSet() const;
		msys::CMaterial *GetWaterMaterial() const;
		void InitializeWaterScene(const Vector3 &refPos, const Vector3 &planeNormal, const Vector3 &waterAabbMin, const Vector3 &waterAabbMax);
		void InitializeWaterScene(const WaterScene &scene);
		bool IsWaterSceneValid() const;
	  private:
		virtual void OnEntitySpawn() override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void RenderPostProcessingOverlay(const pragma::rendering::DrawSceneInfo &drawSceneInfo);
		void InitializeRenderData();
		void InitializeRenderer(pragma::CRendererComponent &component);

		EntityHandle m_hEntUnderwater {};
		pragma::ComponentHandle<CSurfaceComponent> m_surfaceComponent;
		std::unique_ptr<WaterScene> m_waterScene = nullptr;
		pragma::util::WeakHandle<prosper::Shader> m_shaderPpWater {};
		pragma::math::geometry::PlaneSide m_curCameraSurfaceSide = pragma::math::geometry::PlaneSide::OnPlane;
		std::pair<Vector3, Vector3> m_waterAabbBounds = {};
		bool m_renderDataInitialized = false;
	};
};
