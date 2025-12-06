// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:entities.components.util_pbr_converter;

export import :entities.base_entity;
export import pragma.cmaterialsystem;

export namespace pragma {
	namespace rendering::cycles {
		class Scene;
	};
	struct PBRAOBakeJob {
		PBRAOBakeJob(pragma::Model &mdl, msys::Material &mat);
		util::WeakHandle<pragma::Model> hModel = {};
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
		CPBRConverterComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		virtual void OnTick(double dt) override;
		void GenerateAmbientOcclusionMaps(pragma::Model &mdl, uint32_t w = 512, uint32_t h = 512, uint32_t samples = 512, bool rebuild = false);
		void GenerateAmbientOcclusionMaps(pragma::ecs::BaseEntity &ent, uint32_t w = 512, uint32_t h = 512, uint32_t samples = 512, bool rebuild = false);

		bool ConvertToPBR(msys::CMaterial &matTraditional);
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
		void ConvertMaterialsToPBR(pragma::Model &mdl);
		void UpdateMetalness(pragma::Model &mdl);
		void UpdateMetalness(pragma::Model &mdl, msys::CMaterial &mat);
		void UpdateAmbientOcclusion(pragma::Model &mdl, const AmbientOcclusionInfo &aoInfo = {}, pragma::ecs::BaseEntity *optEnt = nullptr);
		void UpdateModel(pragma::Model &mdl, ModelUpdateInfo &updateInfo, pragma::ecs::BaseEntity *optEnt = nullptr);
		void ApplyMiscMaterialProperties(msys::CMaterial &mat, const physics::SurfaceMaterial &surfMat, const std::string &surfMatName);
		void ScheduleModelUpdate(pragma::Model &mdl, bool updateMetalness, std::optional<AmbientOcclusionInfo> updateAOInfo = {}, pragma::ecs::BaseEntity *optEnt = nullptr);

		void ProcessQueue();
		void WriteAOMap(pragma::Model &mdl, msys::CMaterial &mat, uimg::ImageBuffer &imgBuffer, uint32_t w, uint32_t h) const;
		bool ShouldConvertMaterial(msys::CMaterial &mat) const;
		bool IsPBR(msys::CMaterial &mat) const;
		std::shared_ptr<prosper::Texture> ConvertSpecularMapToRoughness(prosper::Texture &specularMap);

		std::queue<PBRAOBakeJob> m_workQueue = {};
		std::unordered_set<std::string> m_convertedMaterials = {};

		CallbackHandle m_cbOnModelLoaded = {};
		CallbackHandle m_cbOnMaterialLoaded = {};
		std::unordered_map<pragma::Model *, ModelUpdateInfo> m_scheduledModelUpdates = {};
	};
};

export class DLLCLIENT CUtilPBRConverter : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
