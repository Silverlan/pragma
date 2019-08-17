#ifndef __C_UTIL_PBR_CONVERTER_HPP__
#define __C_UTIL_PBR_CONVERTER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_entity_component.hpp>
#include <memory>
#include <optional>
#include <thread>
#include <queue>
#include <atomic>
#include <mutex>

namespace prosper {class Texture; class Image;};
namespace pragma
{
	struct PBRConverterMaterialMeshData;
	struct PBRConverterModelData;
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

		void GenerateGeometryBasedTextures(Model &mdl);
	private:
		void StartThread();
		void EndThread();
		bool ShouldConvertMaterial(CMaterial &mat) const;
		std::shared_ptr<prosper::Texture> ConvertSpecularMapToRoughness(prosper::Texture &specularMap);
		void GenerateImageDataFromGeometryData(
			const PBRConverterMaterialMeshData &meshData,uint32_t resolution,
			std::function<void(uint16_t,uint16_t,uint16_t,const std::array<float,3>&,std::array<float,4>&)> applyPixelData,
			std::vector<std::array<float,4>> &outPixelData
		);

		std::thread m_worker = {};
		std::queue<std::shared_ptr<PBRConverterModelData>> m_workQueue = {};
		std::mutex m_workQueueMutex = {};
		std::queue<std::shared_ptr<PBRConverterModelData>> m_completeQueue = {};
		std::mutex m_completeQueueMutex = {};
		std::atomic<bool> m_running = false;
		std::atomic<bool> m_hasWork = false;

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
