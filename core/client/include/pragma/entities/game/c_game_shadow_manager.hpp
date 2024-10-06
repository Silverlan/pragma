/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_GAME_SHADOW_MANAGER_HPP__
#define __C_GAME_SHADOW_MANAGER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree.hpp"
#include <pragma/entities/components/base_entity_component.hpp>

namespace prosper {
	class PrimaryCommandBuffer;
	class RenderTarget;
	class DescriptorSetGroup;
	class Shader;
};
namespace Anvil {
	class DescriptorSet;
};
namespace pragma {
	enum class LightType : uint8_t;
};
namespace pragma {
	struct ShadowRenderInfo {
		const CBaseEntity *entity = nullptr;
		const CModelSubMesh *mesh = nullptr;
		uint32_t renderFlags = 0;
		Material *material = nullptr;
	};

	class ShaderShadow;
	class DLLCLIENT ShadowRenderer {
	  public:
		ShadowRenderer();
		ShadowRenderer(const ShadowRenderer &) = delete;
		ShadowRenderer &operator=(const ShadowRenderer &) = delete;
		enum class RenderResultFlags : uint8_t { None = 0u, TranslucentPending = 1u };

		void RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light);
	  private:
		struct OctreeCallbacks {
			std::function<bool(const OcclusionOctree<std::shared_ptr<ModelMesh>>::Node &)> nodeCallback;
			std::function<void(const CBaseEntity &, uint32_t)> entityCallback;
			std::function<void(const std::shared_ptr<ModelMesh> &)> meshCallback;
			std::function<void(const Model &, const CModelSubMesh &, uint32_t)> subMeshCallback;
		};
		struct LightSourceData {
			std::shared_ptr<prosper::IPrimaryCommandBuffer> drawCmd;
			pragma::CLightComponent *light;
			pragma::LightType type;
			Vector3 position;
			float radius;
		};
		void RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light, pragma::CLightComponent::ShadowMapType smType, pragma::LightType type, bool drawParticleShadows);
		bool UpdateShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light, pragma::CLightComponent::ShadowMapType smType);
		void UpdateWorldShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light);
		void UpdateEntityShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light);
		RenderResultFlags RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightComponent &light, uint32_t layerId, const Mat4 &depthMVP, pragma::ShaderShadow &shader, bool bTranslucent);
		void RenderCSMShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightDirectionalComponent &light, bool drawParticleShadows);

		std::vector<ShadowRenderInfo> m_shadowCasters = {};

		OctreeCallbacks m_octreeCallbacks = {};
		LightSourceData m_lightSourceData = {};
		util::WeakHandle<prosper::Shader> m_shader = {};
		util::WeakHandle<prosper::Shader> m_shaderTransparent = {};
		util::WeakHandle<prosper::Shader> m_shaderSpot = {};
		util::WeakHandle<prosper::Shader> m_shaderSpotTransparent = {};
		util::WeakHandle<prosper::Shader> m_shaderCSM = {};
		util::WeakHandle<prosper::Shader> m_shaderCSMTransparent = {};

		// Current entity when iterating entity meshes in an octree
		const CBaseEntity *m_currentEntity = nullptr;
		Model *m_currentModel = nullptr;
		uint32_t m_currentRenderFlags = 0;
	};

	class DLLCLIENT CShadowManagerComponent final : public BaseEntityComponent {
	  public:
		static CShadowManagerComponent *GetShadowManager();

		CShadowManagerComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;

		struct DLLCLIENT RenderTarget {
			std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
			uint32_t index = std::numeric_limits<uint32_t>::max();
		};
		using RtHandle = util::WeakHandle<std::weak_ptr<RenderTarget>>;
		using Priority = int64_t;
		enum class Type : uint32_t { Generic = 0, Cube };
		RtHandle RequestRenderTarget(Type type, uint32_t size, Priority priority = 0);
		void FreeRenderTarget(const RenderTarget &rt);
		void UpdatePriority(const RenderTarget &rt, Priority priority);
		prosper::IDescriptorSet *GetDescriptorSet();
		void ClearRenderTargets();
		ShadowRenderer &GetRenderer();
	  private:
		struct BufferSet {
			uint32_t limit = 0;

			struct BufferData {
				std::shared_ptr<RenderTarget> renderTarget = nullptr;
				std::shared_ptr<std::weak_ptr<RenderTarget>> renderTargetHandle = nullptr;
				Priority lastPriority = -1;
			};

			std::vector<BufferData> buffers;
		};
		BufferSet m_genericSet = {};
		BufferSet m_cubeSet = {};
		ShadowRenderer m_renderer = {};
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroup = nullptr;
		util::WeakHandle<prosper::Shader> m_whShadowShader = {};
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShadowRenderer::RenderResultFlags)

class DLLCLIENT CShadowManager : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
