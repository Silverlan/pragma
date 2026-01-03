// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.env_sky_camera;

export import :entities.base_entity;
export import :rendering.enums;
import :rendering.render_processor;
import :rendering.render_queue;

export {
	namespace pragma {
		class DLLCLIENT CSkyCameraComponent final : public BaseEntityComponent {
		  public:
			CSkyCameraComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
			virtual void Initialize() override;
			virtual void OnRemove() override;
			virtual void OnEntitySpawn() override;
			virtual void InitializeLuaObject(lua::State *l) override;
			void BuildSkyMeshRenderQueues(const CSceneComponent &scene, rendering::RenderFlags renderFlags, rendering::RenderMask renderMask, bool enableClipping, rendering::RenderQueue &outRenderQueue, rendering::RenderQueue &outTranslucentRenderQueue,
			  CRasterizationRendererComponent *optRasterizationRenderer = nullptr, bool waitForRenderQueues = true) const;

			float GetSkyboxScale() const;
		  private:
			struct SceneData {
				~SceneData();
				CallbackHandle onBuildRenderQueue = {};
				CallbackHandle onRendererChanged = {};
				CallbackHandle renderSkybox = {};
				CallbackHandle renderPrepass = {};
				CallbackHandle updateRenderBuffers = {};

				std::shared_ptr<rendering::RenderQueue> renderQueue = nullptr;
				std::shared_ptr<rendering::RenderQueue> renderQueueTranslucent = nullptr;
			};
			void UpdateScenes();
			void UpdateToggleState();
			void BindToShader(rendering::BaseRenderProcessor &processor) const;
			void UnbindFromShader(rendering::BaseRenderProcessor &processor) const;
			void BuildRenderQueues(const rendering::DrawSceneInfo &drawSceneInfo, SceneData &sceneData);

			float m_skyboxScale = 1.f;
			std::unordered_map<CSceneComponent::SceneIndex, std::shared_ptr<SceneData>> m_sceneData;
		};
	};

	class DLLCLIENT CSkyCamera : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
