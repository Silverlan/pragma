/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_REFLECTION_PROBE_HPP__
#define __C_ENV_REFLECTION_PROBE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "sharedutils/util_parallel_job.hpp"
#include <pragma/entities/components/base_entity_component.hpp>

namespace prosper {
	class Texture;
	class DescriptorSetGroup;
};
namespace uimg {
	struct ImageLayerSet;
};
namespace pragma {
	class CSceneComponent;
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
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		static void BuildAllReflectionProbes(Game &game, bool rebuild = false);
		static void BuildReflectionProbes(Game &game, std::vector<CReflectionProbeComponent *> &probes, bool rebuild = false);
		static prosper::IDescriptorSet *FindDescriptorSetForClosestProbe(const CSceneComponent &scene, const Vector3 &origin, float &outIntensity);

		CReflectionProbeComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		bool CaptureIBLReflectionsFromScene(const std::vector<BaseEntity *> *optEntityList = nullptr, bool renderJob = false);
		bool GenerateIBLReflectionsFromEnvMap(const std::string &envMapFileName);
		bool GenerateIBLReflectionsFromCubemap(prosper::Texture &cubemap);
		bool LoadIBLReflectionsFromFile();
		bool SaveIBLReflectionsToFile();
		const rendering::IBLData *GetIBLData() const;
		prosper::IDescriptorSet *GetIBLDescriptorSet();
		bool GenerateFromEquirectangularImage(uimg::ImageBuffer &imgBuf);

		float GetIBLStrength() const;
		void SetIBLStrength(float iblStrength);

		UpdateStatus UpdateIBLData(bool rebuild = false);
		bool RequiresRebuild() const;
		std::string GetLocationIdentifier() const;

		std::string GetCubemapIBLMaterialFilePath() const;
		void SetCubemapIBLMaterialFilePath(const std::string &path);
	  private:
		static std::shared_ptr<prosper::IImage> CreateCubemapImage();
		Material *LoadMaterial(bool &outIsDefault);

		void InitializeDescriptorSet();
		void ClearDescriptorSet();
		void ClearIblData();
		util::ParallelJob<uimg::ImageLayerSet> CaptureRaytracedIBLReflectionsFromScene(uint32_t width, uint32_t height, const Vector3 &camPos, const Quat &camRot, float nearZ, float farZ, umath::Degree fov, float exposure, const std::vector<BaseEntity *> *optEntityList = nullptr,
		  bool renderJob = false);
		bool FinalizeCubemap(prosper::IImage &imgCubemap);
		std::string GetCubemapIBLMaterialPath() const;
		std::string GetCubemapIdentifier() const;
		std::unique_ptr<rendering::IBLData> m_iblData = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_iblDsg = nullptr;

		struct RaytracingJobManager {
			RaytracingJobManager(CReflectionProbeComponent &probe);
			~RaytracingJobManager();
			util::ParallelJob<uimg::ImageLayerSet> job = {};
			std::shared_ptr<uimg::ImageBuffer> m_equirectImageBuffer = nullptr;
			CReflectionProbeComponent &probe;
			void StartNextJob();
			void Finalize();
		};
		std::unique_ptr<RaytracingJobManager> m_raytracingJobManager = nullptr;
		StateFlags m_stateFlags = StateFlags::RequiresRebuild;

		std::string m_srcEnvMap = "";
		std::string m_iblMat = "";
		std::optional<float> m_strength = {};
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CReflectionProbeComponent::StateFlags)

class DLLCLIENT CEnvReflectionProbe : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
