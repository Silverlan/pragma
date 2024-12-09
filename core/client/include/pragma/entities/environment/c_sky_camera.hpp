/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SKY_CAMERA_HPP__
#define __C_SKY_CAMERA_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/rendering/render_mesh_collection_handler.hpp"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma {
	class ShaderGameWorld;
	namespace rendering {
		struct BaseRenderProcessor;
		class LightingStageRenderProcessor;
		class RenderQueue;
	};
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

#endif
