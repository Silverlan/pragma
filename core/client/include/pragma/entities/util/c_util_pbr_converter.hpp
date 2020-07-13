/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_UTIL_PBR_CONVERTER_HPP__
#define __C_UTIL_PBR_CONVERTER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <sharedutils/util_parallel_job.hpp>
#include <pragma/entities/components/base_entity_component.hpp>
#include <memory>
#include <optional>
#include <thread>
#include <queue>
#include <atomic>
#include <mutex>
#include <unordered_set>

namespace prosper {class Texture; class IImage;};
namespace uimg {class ImageBuffer;};
namespace pragma
{
	namespace rendering::cycles {class Scene;};
	struct PBRAOBakeJob
	{
		PBRAOBakeJob(Model &mdl,Material &mat);
		util::WeakHandle<Model> hModel = {};
		MaterialHandle hMaterial = {};
		util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> job = {};
		bool isRunning = false;
		uint32_t width = 512;
		uint32_t height = 512;
		uint32_t samples = 512;
	};

	class DLLCLIENT CPBRConverterComponent final
		: public BaseEntityComponent
	{
	public:
		CPBRConverterComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		void GenerateAmbientOcclusionMaps(Model &mdl,uint32_t w=512,uint32_t h=512,uint32_t samples=512,bool rebuild=false);

		bool ConvertToPBR(CMaterial &matTraditional);
		void PollEvents();
	private:
		struct AmbientOcclusionInfo
		{
			// These values are a good compromise between quality and render time
			uint32_t width = 512;
			uint32_t height = 512;
			uint32_t samples = 512;
			bool rebuild = false;
		};
		struct ModelUpdateInfo
		{
			CallbackHandle cbOnMaterialsLoaded = {};
			bool updateMetalness = false;
			std::optional<AmbientOcclusionInfo> updateAmbientOcclusion = {};
		};
		void ConvertMaterialsToPBR(Model &mdl);
		void UpdateMetalness(Model &mdl);
		void UpdateMetalness(Model &mdl,CMaterial &mat);
		void UpdateAmbientOcclusion(Model &mdl,const AmbientOcclusionInfo &aoInfo={});
		void UpdateModel(Model &mdl,ModelUpdateInfo &updateInfo);
		void ScheduleModelUpdate(Model &mdl,bool updateMetalness,std::optional<AmbientOcclusionInfo> updateAOInfo={});

		void ProcessQueue();
		void WriteAOMap(Model &mdl,CMaterial &mat,uimg::ImageBuffer &imgBuffer,uint32_t w,uint32_t h) const;
		bool ShouldConvertMaterial(CMaterial &mat) const;
		bool IsPBR(CMaterial &mat) const;
		std::shared_ptr<prosper::Texture> ConvertSpecularMapToRoughness(prosper::Texture &specularMap);

		std::queue<PBRAOBakeJob> m_workQueue = {};
		std::unordered_set<std::string> m_convertedMaterials = {};

		CallbackHandle m_cbOnModelLoaded = {};
		CallbackHandle m_cbOnMaterialLoaded = {};
		std::unordered_map<Model*,ModelUpdateInfo> m_scheduledModelUpdates = {};
		void(*m_fCalcGeometryData)(const std::vector<Vector3>&,const std::vector<uint16_t>&,std::vector<float>*,std::vector<Vector3>*,uint32_t) = nullptr;
	};
};

class EntityHandle;
class DLLCLIENT CUtilPBRConverter
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
