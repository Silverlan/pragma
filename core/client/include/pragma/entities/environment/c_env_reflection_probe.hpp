#ifndef __C_ENV_REFLECTION_PROBE_HPP__
#define __C_ENV_REFLECTION_PROBE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_entity_component.hpp>

namespace prosper {class Texture; class DescriptorSetGroup;};
namespace pragma
{
	namespace rendering
	{
		struct DLLCLIENT IBLData
		{
			IBLData(const std::shared_ptr<prosper::Texture> &irradianceMap,const std::shared_ptr<prosper::Texture> &prefilterMap,const std::shared_ptr<prosper::Texture> &brdfMap);
			std::shared_ptr<prosper::Texture> irradianceMap;
			std::shared_ptr<prosper::Texture> prefilterMap;
			std::shared_ptr<prosper::Texture> brdfMap;
			float strength = 1.f;
		};
	};
	class DLLCLIENT CReflectionProbeComponent final
		: public BaseEntityComponent
	{
	public:
		enum class UpdateStatus : uint8_t
		{
			Initial = 0,
			Pending,
			Complete,
			Failed
		};
		enum class StateFlags : uint8_t
		{
			None = 0u,
			BakingFailed = 1u,
			RequiresRebuild = BakingFailed<<1u
		};
		static void BuildAllReflectionProbes(Game &game,bool rebuild=false);
		static void BuildReflectionProbes(Game &game,std::vector<CReflectionProbeComponent*> &probes,bool rebuild=false);
		static Anvil::DescriptorSet *FindDescriptorSetForClosestProbe(Scene &scene,const Vector3 &origin,float &outIntensity);

		CReflectionProbeComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		bool CaptureIBLReflectionsFromScene();
		bool GenerateIBLReflectionsFromEnvMap(const std::string &envMapFileName);
		bool GenerateIBLReflectionsFromCubemap(prosper::Texture &cubemap);
		bool LoadIBLReflectionsFromFile();
		bool SaveIBLReflectionsToFile();
		const rendering::IBLData *GetIBLData() const;
		Anvil::DescriptorSet *GetIBLDescriptorSet();

		float GetIBLStrength() const;

		UpdateStatus UpdateIBLData(bool rebuild=false);
		bool RequiresRebuild() const;
	private:
		static std::shared_ptr<prosper::Image> CreateCubemapImage();
		Material *LoadMaterial(bool &outIsDefault);

		void InitializeDescriptorSet();
		util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> CaptureRaytracedIBLReflectionsFromScene(
			uint32_t width,uint32_t height,uint32_t layerIndex,
			const Vector3 &camPos,const Quat &camRot,float nearZ,float farZ,umath::Degree fov
		);
		bool FinalizeCubemap(prosper::Image &imgCubemap);
		std::string GetCubemapIBLMaterialPath() const;
		std::string GetCubemapIBLMaterialFilePath() const;
		std::string GetCubemapIdentifier() const;
		std::unique_ptr<rendering::IBLData> m_iblData = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> m_iblDsg = nullptr;

		struct RaytracingJobManager
		{
			RaytracingJobManager(CReflectionProbeComponent &probe);
			~RaytracingJobManager();
			std::array<util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>>,6> jobs = {};
			std::array<std::shared_ptr<uimg::ImageBuffer>,6> m_layerImageBuffers = {};
			CReflectionProbeComponent &probe;
			uint32_t m_nextJobIndex = 0u;
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

class EntityHandle;
class DLLCLIENT CEnvReflectionProbe
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
