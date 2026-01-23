// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.util_pbr_converter;

export import :entities.base_entity;
export import pragma.cmaterialsystem;

export namespace pragma {
	namespace rendering::cycles {
		class Scene;
	};
	struct PBRAOBakeJob {
		PBRAOBakeJob(asset::Model &mdl, material::Material &mat);
		util::WeakHandle<asset::Model> hModel = {};
		material::MaterialHandle hMaterial = {};
		EntityHandle hEntity = {};
		util::ParallelJob<image::ImageLayerSet> job = {};
		bool isRunning = false;
		uint32_t width = 512;
		uint32_t height = 512;
		uint32_t samples = 512;
	};

	class DLLCLIENT CPBRConverterComponent final : public BaseEntityComponent {
	  public:
		CPBRConverterComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		virtual void OnTick(double dt) override;
		void GenerateAmbientOcclusionMaps(asset::Model &mdl, uint32_t w = 512, uint32_t h = 512, uint32_t samples = 512, bool rebuild = false);
		void GenerateAmbientOcclusionMaps(ecs::BaseEntity &ent, uint32_t w = 512, uint32_t h = 512, uint32_t samples = 512, bool rebuild = false);

		bool ConvertToPBR(material::CMaterial &matTraditional);
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
		void ConvertMaterialsToPBR(asset::Model &mdl);
		void UpdateMetalness(asset::Model &mdl);
		void UpdateMetalness(asset::Model &mdl, material::CMaterial &mat);
		void UpdateAmbientOcclusion(asset::Model &mdl, const AmbientOcclusionInfo &aoInfo = {}, ecs::BaseEntity *optEnt = nullptr);
		void UpdateModel(asset::Model &mdl, ModelUpdateInfo &updateInfo, ecs::BaseEntity *optEnt = nullptr);
		void ApplyMiscMaterialProperties(material::CMaterial &mat, const physics::SurfaceMaterial &surfMat, const std::string &surfMatName);
		void ScheduleModelUpdate(asset::Model &mdl, bool updateMetalness, std::optional<AmbientOcclusionInfo> updateAOInfo = {}, ecs::BaseEntity *optEnt = nullptr);

		void ProcessQueue();
		void WriteAOMap(asset::Model &mdl, material::CMaterial &mat, image::ImageBuffer &imgBuffer, uint32_t w, uint32_t h) const;
		bool ShouldConvertMaterial(material::CMaterial &mat) const;
		bool IsPBR(material::CMaterial &mat) const;
		std::shared_ptr<prosper::Texture> ConvertSpecularMapToRoughness(prosper::Texture &specularMap);

		std::queue<PBRAOBakeJob> m_workQueue = {};
		std::unordered_set<std::string> m_convertedMaterials = {};

		CallbackHandle m_cbOnModelLoaded = {};
		CallbackHandle m_cbOnMaterialLoaded = {};
		std::unordered_map<asset::Model *, ModelUpdateInfo> m_scheduledModelUpdates = {};
	};
};

export class DLLCLIENT CUtilPBRConverter : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
