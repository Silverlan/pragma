// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <pragma/entities/components/base_entity_component.hpp>

export module pragma.client.entities.components:env_sky_camera;

export {
	namespace pragma {
		class DLLCLIENT CSkyCameraComponent final : public BaseEntityComponent {
		public:
			CSkyCameraComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
			virtual void Initialize() override;
			virtual void OnRemove() override;
			virtual void OnEntitySpawn() override;
			virtual void InitializeLuaObject(lua_State *l) override;
			void BuildSkyMeshRenderQueues(const pragma::CSceneComponent &scene, RenderFlags renderFlags, pragma::rendering::RenderMask renderMask, bool enableClipping, rendering::RenderQueue &outRenderQueue, rendering::RenderQueue &outTranslucentRenderQueue,
			pragma::CRasterizationRendererComponent *optRasterizationRenderer = nullptr, bool waitForRenderQueues = true) const;

			float GetSkyboxScale() const;
		private:
			struct SceneData {
				~SceneData();
				CallbackHandle onBuildRenderQueue = {};
				CallbackHandle onRendererChanged = {};
				CallbackHandle renderSkybox = {};
				CallbackHandle renderPrepass = {};
				CallbackHandle updateRenderBuffers = {};

				std::shared_ptr<pragma::rendering::RenderQueue> renderQueue = nullptr;
				std::shared_ptr<pragma::rendering::RenderQueue> renderQueueTranslucent = nullptr;
			};
			void UpdateScenes();
			void UpdateToggleState();
			void BindToShader(pragma::rendering::BaseRenderProcessor &processor) const;
			void UnbindFromShader(pragma::rendering::BaseRenderProcessor &processor) const;
			void BuildRenderQueues(const util::DrawSceneInfo &drawSceneInfo, SceneData &sceneData);

			float m_skyboxScale = 1.f;
			std::unordered_map<pragma::CSceneComponent::SceneIndex, std::shared_ptr<SceneData>> m_sceneData;
		};
	};

	class DLLCLIENT CSkyCamera : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
