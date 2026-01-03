// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.game_shadow_manager;

import :rendering.shaders;

export namespace pragma {
	class CLightComponent;
	struct ShadowRenderInfo {
		const ecs::CBaseEntity *entity = nullptr;
		const geometry::CModelSubMesh *mesh = nullptr;
		uint32_t renderFlags = 0;
		material::Material *material = nullptr;
	};

	class DLLCLIENT ShadowRenderer {
	  public:
		ShadowRenderer();
		ShadowRenderer(const ShadowRenderer &) = delete;
		ShadowRenderer &operator=(const ShadowRenderer &) = delete;
		enum class RenderResultFlags : uint8_t { None = 0u, TranslucentPending = 1u };

		void RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light);
	  private:
		struct OctreeCallbacks {
			std::function<bool(const OcclusionOctree<std::shared_ptr<geometry::ModelMesh>>::Node &)> nodeCallback;
			std::function<void(const ecs::CBaseEntity &, uint32_t)> entityCallback;
			std::function<void(const std::shared_ptr<geometry::ModelMesh> &)> meshCallback;
			std::function<void(const asset::Model &, const geometry::CModelSubMesh &, uint32_t)> subMeshCallback;
		};
		struct LightSourceData {
			std::shared_ptr<prosper::IPrimaryCommandBuffer> drawCmd;
			CLightComponent *light;
			LightType type;
			Vector3 position;
			float radius;
		};
		void RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light, rendering::ShadowMapType smType, LightType type, bool drawParticleShadows);
		bool UpdateShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light, rendering::ShadowMapType smType);
		void UpdateWorldShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light);
		void UpdateEntityShadowCasters(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light);
		RenderResultFlags RenderShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CLightComponent &light, uint32_t layerId, const Mat4 &depthMVP, ShaderShadow &shader, bool bTranslucent);
		void RenderCSMShadows(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, BaseEnvLightDirectionalComponent &light, bool drawParticleShadows);

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
		const ecs::CBaseEntity *m_currentEntity = nullptr;
		asset::Model *m_currentModel = nullptr;
		uint32_t m_currentRenderFlags = 0;
	};

	class DLLCLIENT CShadowManagerComponent final : public BaseEntityComponent {
	  public:
		static CShadowManagerComponent *GetShadowManager();

		CShadowManagerComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
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
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::ShadowRenderer::RenderResultFlags)
}

export class DLLCLIENT CShadowManager : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
