// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.env_reflection_probe;

import :entities.components.scene;

export {
	namespace pragma {
		namespace rendering {
			struct DLLCLIENT IBLData {
				IBLData(const std::shared_ptr<prosper::Texture> &irradianceMap, const std::shared_ptr<prosper::Texture> &prefilterMap, const std::shared_ptr<prosper::Texture> &brdfMap);
				std::shared_ptr<prosper::Texture> irradianceMap;
				std::shared_ptr<prosper::Texture> prefilterMap;
				std::shared_ptr<prosper::Texture> brdfMap;
				float strength = 1.f;
			};
		};
		class DLLCLIENT CReflectionProbeComponent final : public BaseEntityComponent {
		  public:
			enum class UpdateStatus : uint8_t { Initial = 0, Pending, Complete, Failed };
			enum class StateFlags : uint8_t { None = 0u, BakingFailed = 1u, RequiresRebuild = BakingFailed << 1u };
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
			static void BuildAllReflectionProbes(Game &game, bool rebuild = false);
			static void BuildReflectionProbes(Game &game, std::vector<CReflectionProbeComponent *> &probes, bool rebuild = false);
			static prosper::IDescriptorSet *FindDescriptorSetForClosestProbe(const CSceneComponent &scene, const Vector3 &origin, float &outIntensity);

			CReflectionProbeComponent(ecs::BaseEntity &ent);
			virtual ~CReflectionProbeComponent() override;
			virtual void Initialize() override;
			virtual void OnEntitySpawn() override;
			virtual void OnRemove() override;
			virtual void InitializeLuaObject(lua::State *l) override;
			bool CaptureIBLReflectionsFromScene(const std::vector<ecs::BaseEntity *> *optEntityList = nullptr, bool renderJob = false);
			bool GenerateIBLReflectionsFromEnvMap(const std::string &envMapFileName);
			bool GenerateIBLReflectionsFromCubemap(prosper::Texture &cubemap);
			bool LoadIBLReflectionsFromFile();
			bool SaveIBLReflectionsToFile();
			const rendering::IBLData *GetIBLData() const;
			prosper::IDescriptorSet *GetIBLDescriptorSet();
			bool GenerateFromEquirectangularImage(image::ImageBuffer &imgBuf);

			float GetIBLStrength() const;
			void SetIBLStrength(float iblStrength);

			UpdateStatus UpdateIBLData(bool rebuild = false);
			bool RequiresRebuild() const;
			std::string GetLocationIdentifier() const;

			std::string GetCubemapIBLMaterialFilePath() const;
			void SetCubemapIBLMaterialFilePath(const std::string &path);
		  private:
			static std::shared_ptr<prosper::IImage> CreateCubemapImage();
			material::Material *LoadMaterial(bool &outIsDefault);

			void InitializeDescriptorSet();
			void ClearDescriptorSet();
			void ClearIblData();
			util::ParallelJob<image::ImageLayerSet> CaptureRaytracedIBLReflectionsFromScene(uint32_t width, uint32_t height, const Vector3 &camPos, const Quat &camRot, float nearZ, float farZ, math::Degree fov, float exposure, const std::vector<ecs::BaseEntity *> *optEntityList = nullptr,
			  bool renderJob = false);
			bool FinalizeCubemap(prosper::IImage &imgCubemap);
			std::string GetCubemapIBLMaterialPath() const;
			std::string GetCubemapIdentifier() const;
			std::unique_ptr<rendering::IBLData> m_iblData = nullptr;
			std::shared_ptr<prosper::IDescriptorSetGroup> m_iblDsg = nullptr;

			struct RaytracingJobManager {
				RaytracingJobManager(CReflectionProbeComponent &probe);
				~RaytracingJobManager();
				util::ParallelJob<image::ImageLayerSet> job = {};
				std::shared_ptr<image::ImageBuffer> m_equirectImageBuffer = nullptr;
				CReflectionProbeComponent &probe;
				void StartNextJob();
				void Finalize();
			};
			std::unique_ptr<RaytracingJobManager> m_raytracingJobManager;
			StateFlags m_stateFlags = StateFlags::RequiresRebuild;

			std::string m_srcEnvMap = "";
			std::string m_iblMat = "";
			std::optional<float> m_strength = {};
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::CReflectionProbeComponent::StateFlags)

	class DLLCLIENT CEnvReflectionProbe : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
