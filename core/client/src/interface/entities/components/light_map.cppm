// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.light_map;

export import :rendering.cycles;
export import :rendering.lightmap_data_cache;
export import pragma.prosper;
import source_engine.bsp;

export namespace pragma {
	struct DLLCLIENT LightmapBakeSettings {
		std::optional<uint32_t> width {};
		std::optional<uint32_t> height {};
		std::optional<rendering::cycles::SceneInfo::ColorTransform> colorTransform {};
		float exposure = 1.f;
		float skyStrength = 0.3f;
		float globalLightIntensityFactor = 1.f;
		std::string sky = "skies/dusk379.hdr";
		uint32_t samples = 1'225;
		bool denoise = true;
		bool createAsRenderJob = false;
		bool rebuildUvAtlas = false;
	};
	class DLLCLIENT CLightMapComponent final : public BaseEntityComponent {
	  public:
		enum class Texture : uint32_t {
			DiffuseDirectMap = 0,
			DiffuseIndirectMap,
			DiffuseMap,
			DominantDirectionMap,

			Count
		};
		static spdlog::logger &LOGGER;
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);
		static std::shared_ptr<prosper::IDynamicResizableBuffer> GenerateLightmapUVBuffers(std::vector<std::shared_ptr<prosper::IBuffer>> &outMeshLightMapUvBuffers);
		static std::shared_ptr<prosper::Texture> CreateLightmapTexture(image::ImageBuffer &imgBuf);
		static bool BakeLightmaps(const LightmapBakeSettings &bakeSettings);
		static bool ImportLightmapAtlas(fs::VFilePtr f);
		static bool ImportLightmapAtlas(const std::string &path);
		static bool ImportLightmapAtlas(image::ImageBuffer &imgBuf);

		CLightMapComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntitySpawn() override;
		const std::shared_ptr<prosper::Texture> &GetLightMap() const;
		const std::shared_ptr<prosper::Texture> &GetDirectionalLightMap() const;

		void InitializeLightMapData(const std::shared_ptr<prosper::Texture> &lightMap, const std::shared_ptr<prosper::IDynamicResizableBuffer> &lightMapUvBuffer, const std::vector<std::shared_ptr<prosper::IBuffer>> &meshUvBuffers,
		  const std::shared_ptr<prosper::Texture> &directionalLightmap = nullptr, bool keepCurrentTextures = false);
		void SetLightMapAtlas(const std::shared_ptr<prosper::Texture> &lightMap);
		const std::shared_ptr<prosper::Texture> &GetLightMapAtlas() const;
		void SetDirectionalLightMapAtlas(const std::shared_ptr<prosper::Texture> &lightMap);
		const std::shared_ptr<prosper::Texture> &GetDirectionalLightMapAtlas() const;
		bool HasValidLightMap() const;
		void ReloadLightMapData();

		prosper::IBuffer *GetMeshLightMapUvBuffer(uint32_t meshIdx) const;
		const std::vector<std::shared_ptr<prosper::IBuffer>> &GetMeshLightMapUvBuffers() const;
		std::vector<std::shared_ptr<prosper::IBuffer>> &GetMeshLightMapUvBuffers();

		void SetLightMapExposure(float exp);
		float GetLightMapExposure() const;
		float CalcLightMapPowExposurePow() const;
		const util::PFloatProperty &GetLightMapExposureProperty() const { return m_lightMapExposure; }

		void ConvertLightmapToBSPLuxelData() const;

		void UpdateLightmapUvBuffers();
		std::shared_ptr<prosper::IDynamicResizableBuffer> GetGlobalLightMapUvBuffer() const;

		const rendering::LightmapDataCache *GetLightmapDataCache() const;
		void SetLightmapDataCache(rendering::LightmapDataCache *cache);

		const std::shared_ptr<prosper::Texture> &GetTexture(Texture tex) const;
		void SetLightMapMaterial(const std::string &matName);
		const std::string &GetLightMapMaterialName() const;
	  protected:
		void InitializeFromMaterial();

		std::string m_lightMapMaterialName;
		material::MaterialHandle m_lightMapMaterial;
		std::array<std::shared_ptr<prosper::Texture>, math::to_integral(Texture::Count)> m_textures;

		util::PFloatProperty m_lightMapExposure = nullptr;
		std::shared_ptr<rendering::LightmapDataCache> m_lightmapDataCache;

		// Contains the light map uv-buffer for each mesh of the world in the same order
		// they are in the model's mesh group
		std::vector<std::shared_ptr<prosper::IBuffer>> m_meshLightMapUvBuffers;
		std::shared_ptr<prosper::IDynamicResizableBuffer> m_meshLightMapUvBuffer = nullptr;
	};
};
