// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"




export module pragma.client:entities.components.env_sky_camera;

import :entities.base_entity;
import :rendering.enums;
import :rendering.render_processor;
import :rendering.render_queue;

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
