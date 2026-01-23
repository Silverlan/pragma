// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.lights.shadow_csm;

export import :entities.components.camera;
export import :entities.components.lights.light;

export namespace pragma {
	struct DLLCLIENT FrustumSplit {
		FrustumSplit();
		float neard;
		float fard;
	};

	struct DLLCLIENT Frustum {
		Frustum();
		FrustumSplit split = {};
		std::vector<Vector3> points;
		std::vector<math::Plane> planes;
		Vector3 bounds[2] = {{}, {}};
		Mat4 projection = umat::identity();
		Mat4 viewProjection = umat::identity();
		Vector3 center = {};
		bounding_volume::OBB obb = {};
		bounding_volume::AABB aabb = {};
		Vector3 obbCenter = {}; // Also center of aabb
		float radius = 0.f;
	};

	class DLLCLIENT CShadowCSMComponent final : public BaseEntityComponent {
	  public:
		static constexpr uint32_t MAX_CASCADE_COUNT = 4;

		CShadowCSMComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		void ReloadDepthTextures();
		bool ShouldUpdateLayer(uint32_t layerId) const;
		Mat4 &GetProjectionMatrix(unsigned int layer);
		Mat4 &GetViewProjectionMatrix(unsigned int layer);
		void SetFrustumUpdateCallback(const std::function<void(void)> &f);
		void SetSplitCount(unsigned int numSplits);
		void SetMaxDistance(float dist);
		float GetMaxDistance();
		unsigned int GetSplitCount();
		void UpdateFrustum(uint32_t splitId, CCameraComponent &cam, const Mat4 &matView, const Vector3 &dir);
		void UpdateFrustum(CCameraComponent &cam, const Mat4 &matView, const Vector3 &dir);
		float *GetSplitFarDistances();
		Frustum *GetFrustumSplit(unsigned int splitId);

		const Mat4 &GetStaticPendingViewProjectionMatrix(uint32_t layer) const;
		const std::shared_ptr<prosper::RenderTarget> &GetStaticPendingRenderTarget() const;
		void RenderBatch(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, BaseEnvLightDirectionalComponent &light);

		std::shared_ptr<prosper::IFramebuffer> GetFramebuffer(rendering::ShadowMapType smType, uint32_t layer = 0) const;
		prosper::IRenderPass *GetRenderPass(rendering::ShadowMapType smType) const;
		const std::shared_ptr<prosper::RenderTarget> &GetRenderTarget(rendering::ShadowMapType smType) const;

		prosper::Texture *GetDepthTexture() const;
		prosper::Texture *GetDepthTexture(rendering::ShadowMapType smType) const;

		void FreeRenderTarget();

		uint32_t GetLayerCount() const;
		bool IsDynamicValid() const;
		uint64_t GetLastUpdateFrameId() const;
		void SetLastUpdateFrameId(uint64_t id);
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		void DestroyTextures();
		uint32_t m_layerCount = 0;
		std::array<bool, MAX_CASCADE_COUNT> m_layerUpdate;
		std::array<Mat4, MAX_CASCADE_COUNT> m_vpMatrices;

		struct EntityInfo {
			EntityInfo() = default;
			EntityInfo(ecs::CBaseEntity *ent) : hEntity(ent->GetHandle()) {}
			EntityHandle hEntity = {};
			std::queue<std::weak_ptr<geometry::ModelMesh>> meshes;
			bool bAlreadyPassed = false;
		};
		struct TranslucentEntityInfo {
			EntityHandle hEntity = {};
			std::queue<std::weak_ptr<geometry::ModelSubMesh>> subMeshes;
			;
		};
		struct CascadeMeshInfo {
			std::vector<EntityInfo> entityMeshes;
			std::vector<TranslucentEntityInfo> translucentMeshes;
		};
		struct TextureSet {
			TextureSet();
			std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
		};
		struct {
			TextureSet staticTextureSet;
			std::array<Mat4, MAX_CASCADE_COUNT> prevVpMatrices;
			std::array<Mat4, MAX_CASCADE_COUNT> prevProjectionMatrices;
			Vector4 prevSplitDistances = {};
			std::array<CascadeMeshInfo, MAX_CASCADE_COUNT> meshes;
		} m_pendingInfo;

		util::WeakHandle<prosper::Shader> m_whShaderCsm = {};
		util::WeakHandle<prosper::Shader> m_whShaderCsmTransparent = {};
		std::vector<Frustum> m_frustums = {};
		std::vector<float> m_fard = {};
		std::function<void(void)> m_onFrustumUpdated;
		unsigned int m_numSplits = 0u;
		float m_maxDistance = 0.f;
		uint64_t m_lastFrameUpdate = std::numeric_limits<uint64_t>::max();
		std::array<TextureSet, 2> m_textureSets; // 0 = Used for static geometry (Rarely moving and not animated); 1 = Used for dynamic geometry

		void UpdateSplitDistances(float nd, float fd);
		void InitializeTextureSet(TextureSet &set, rendering::ShadowMapType smType);
		void InitializeDepthTextures(uint32_t size);
	};
};
