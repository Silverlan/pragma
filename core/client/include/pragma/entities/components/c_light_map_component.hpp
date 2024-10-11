/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LIGHT_MAP_COMPONENT_HPP__
#define __C_LIGHT_MAP_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/raytracing/cycles.hpp"
#include <pragma/entities/components/base_entity_component.hpp>

namespace prosper {
	class IDynamicResizableBuffer;
};
namespace util::bsp {
	struct LightMapInfo;
};
namespace spdlog {
	class logger;
};
namespace pragma {
	struct LightmapDataCache;
	class DLLCLIENT CLightMapComponent final : public BaseEntityComponent {
	  public:
		enum class Texture : uint32_t {
			DiffuseDirectMap = 0,
			DiffuseIndirectMap,
			DiffuseMap,
			DominantDirectionMap,

			Count
		};
		struct DLLCLIENT LightmapBakeSettings {
			std::optional<uint32_t> width {};
			std::optional<uint32_t> height {};
			std::optional<pragma::rendering::cycles::SceneInfo::ColorTransform> colorTransform {};
			float exposure = 1.f;
			float skyStrength = 0.3f;
			float globalLightIntensityFactor = 1.f;
			std::string sky = "skies/dusk379.hdr";
			uint32_t samples = 1'225;
			bool denoise = true;
			bool createAsRenderJob = false;
			bool rebuildUvAtlas = false;
		};
		static spdlog::logger &LOGGER;
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		static std::shared_ptr<prosper::IDynamicResizableBuffer> GenerateLightmapUVBuffers(std::vector<std::shared_ptr<prosper::IBuffer>> &outMeshLightMapUvBuffers);
		static std::shared_ptr<prosper::Texture> CreateLightmapTexture(uimg::ImageBuffer &imgBuf);
		static bool BakeLightmaps(const LightmapBakeSettings &bakeSettings);
		static bool ImportLightmapAtlas(VFilePtr f);
		static bool ImportLightmapAtlas(const std::string &path);
		static bool ImportLightmapAtlas(uimg::ImageBuffer &imgBuf);

		CLightMapComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
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

		const LightmapDataCache *GetLightmapDataCache() const;
		void SetLightmapDataCache(LightmapDataCache *cache);

		const std::shared_ptr<prosper::Texture> &GetTexture(Texture tex) const;
		void SetLightMapMaterial(const std::string &matName);
		const std::string &GetLightMapMaterialName() const;
	  protected:
		void InitializeFromMaterial();

		std::string m_lightMapMaterialName;
		msys::MaterialHandle m_lightMapMaterial;
		std::array<std::shared_ptr<prosper::Texture>, umath::to_integral(Texture::Count)> m_textures;

		util::PFloatProperty m_lightMapExposure = nullptr;
		std::shared_ptr<LightmapDataCache> m_lightmapDataCache;

		// Contains the light map uv-buffer for each mesh of the world in the same order
		// they are in the model's mesh group
		std::vector<std::shared_ptr<prosper::IBuffer>> m_meshLightMapUvBuffers;
		std::shared_ptr<prosper::IDynamicResizableBuffer> m_meshLightMapUvBuffer = nullptr;
	};
};

#endif
