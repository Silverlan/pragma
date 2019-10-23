#ifndef __C_UTIL_PBR_CONVERTER_HPP__
#define __C_UTIL_PBR_CONVERTER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <sharedutils/util_parallel_job.hpp>
#include <pragma/model/model_handle.hpp>
#include <pragma/entities/components/base_entity_component.hpp>
#include <memory>
#include <optional>
#include <thread>
#include <queue>
#include <atomic>
#include <mutex>
#include <unordered_set>

namespace prosper {class Texture; class Image;};
namespace util {class ImageBuffer;};
namespace pragma
{
	namespace rendering::cycles {class Scene;};
	struct PBRAOBakeJob
	{
		PBRAOBakeJob(Model &mdl,Material &mat);
		ModelHandle hModel = {};
		MaterialHandle hMaterial = {};
		util::ParallelJob<std::shared_ptr<util::ImageBuffer>> job = {};
		bool isRunning = false;
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

		bool ConvertToPBR(CMaterial &matTraditional);
		void PollEvents();
	private:
		static bool IsSurfaceMaterialMetal(std::string surfMat);
		void ConvertMaterialsToPBR(Model &mdl);
		void UpdateMetalness(Model &mdl);
		void UpdateMetalness(Model &mdl,CMaterial &mat);
		void UpdateAmbientOcclusion(Model &mdl);

		void ProcessQueue();
		void WriteAOMap(Model &mdl,CMaterial &mat,util::ImageBuffer &imgBuffer,uint32_t w,uint32_t h) const;
		void ApplyAOMap(CMaterial &mat,const std::string &aoName) const;
		bool ShouldConvertMaterial(CMaterial &mat) const;
		bool IsPBR(CMaterial &mat) const;
		std::shared_ptr<prosper::Texture> ConvertSpecularMapToRoughness(prosper::Texture &specularMap);

		std::queue<PBRAOBakeJob> m_workQueue = {};
		std::unordered_set<std::string> m_convertedMaterials = {};

		CallbackHandle m_cbOnModelLoaded = {};
		CallbackHandle m_cbOnMaterialLoaded = {};
		std::unordered_map<Model*,CallbackHandle> m_onModelMaterialsLoadedCallbacks = {};
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
