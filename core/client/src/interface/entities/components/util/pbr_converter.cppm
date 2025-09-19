// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <sharedutils/util_parallel_job.hpp>
#include <pragma/entities/components/base_entity_component.hpp>
#include "image/prosper_texture.hpp"
#include "util_image_buffer.hpp"
#include <cmaterial.h>
#include <memory>
#include <optional>
#include <thread>
#include <queue>
#include <atomic>
#include <mutex>
#include <unordered_set>

export module pragma.client.entities.components.util_pbr_converter;

export namespace pragma {
	namespace rendering::cycles {
		class Scene;
	};
	struct PBRAOBakeJob {
		PBRAOBakeJob(Model &mdl, Material &mat);
		util::WeakHandle<Model> hModel = {};
		msys::MaterialHandle hMaterial = {};
		EntityHandle hEntity = {};
		util::ParallelJob<uimg::ImageLayerSet> job = {};
		bool isRunning = false;
		uint32_t width = 512;
		uint32_t height = 512;
		uint32_t samples = 512;
	};

	class DLLCLIENT CPBRConverterComponent final : public BaseEntityComponent {
	  public:
		CPBRConverterComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		virtual void OnTick(double dt) override;
		void GenerateAmbientOcclusionMaps(Model &mdl, uint32_t w = 512, uint32_t h = 512, uint32_t samples = 512, bool rebuild = false);
		void GenerateAmbientOcclusionMaps(BaseEntity &ent, uint32_t w = 512, uint32_t h = 512, uint32_t samples = 512, bool rebuild = false);

		bool ConvertToPBR(CMaterial &matTraditional);
		void PollEvents();
	  private:
		struct AmbientOcclusionInfo {
			// These values are a good compromise between quality and render time
			//haha very funny clang
			AmbientOcclusionInfo() {};
			AmbientOcclusionInfo(uint32_t width, uint32_t height, uint32_t samples, bool rebuild) : width(width), height(height), samples(samples), rebuild(rebuild) {};
			uint32_t width = 512;
			uint32_t height = 512;
			uint32_t samples = 512;
			bool rebuild = false;
		};
		struct ModelUpdateInfo {
			CallbackHandle cbOnMaterialsLoaded = {};
			bool updateMetalness = false;
			std::optional<AmbientOcclusionInfo> updateAmbientOcclusion = {};
		};
		void ConvertMaterialsToPBR(Model &mdl);
		void UpdateMetalness(Model &mdl);
		void UpdateMetalness(Model &mdl, CMaterial &mat);
		void UpdateAmbientOcclusion(Model &mdl, const AmbientOcclusionInfo &aoInfo = {}, BaseEntity *optEnt = nullptr);
		void UpdateModel(Model &mdl, ModelUpdateInfo &updateInfo, BaseEntity *optEnt = nullptr);
		void ApplyMiscMaterialProperties(CMaterial &mat, const SurfaceMaterial &surfMat, const std::string &surfMatName);
		void ScheduleModelUpdate(Model &mdl, bool updateMetalness, std::optional<AmbientOcclusionInfo> updateAOInfo = {}, BaseEntity *optEnt = nullptr);

		void ProcessQueue();
		void WriteAOMap(Model &mdl, CMaterial &mat, uimg::ImageBuffer &imgBuffer, uint32_t w, uint32_t h) const;
		bool ShouldConvertMaterial(CMaterial &mat) const;
		bool IsPBR(CMaterial &mat) const;
		std::shared_ptr<prosper::Texture> ConvertSpecularMapToRoughness(prosper::Texture &specularMap);

		std::queue<PBRAOBakeJob> m_workQueue = {};
		std::unordered_set<std::string> m_convertedMaterials = {};

		CallbackHandle m_cbOnModelLoaded = {};
		CallbackHandle m_cbOnMaterialLoaded = {};
		std::unordered_map<Model *, ModelUpdateInfo> m_scheduledModelUpdates = {};
	};
};

export class DLLCLIENT CUtilPBRConverter : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
