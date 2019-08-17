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
		};
	};
	class DLLCLIENT CReflectionProbeComponent final
		: public BaseEntityComponent
	{
	public:
		static void BuildAllReflectionProbes(Game &game);

		CReflectionProbeComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		bool CaptureIBLReflectionsFromScene();
		bool GenerateIBLReflectionsFromEnvMap(const std::string &envMapFileName);
		bool LoadIBLReflectionsFromFile(const std::string &fileName);
		bool SaveIBLReflectionsToFile(const std::string &fileName);
		const rendering::IBLData *GetIBLData() const;
		Anvil::DescriptorSet *GetIBLDescriptorSet();
	private:
		void InitializeDescriptorSet();
		std::unique_ptr<rendering::IBLData> m_iblData = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> m_iblDsg = nullptr;
	};
};

class EntityHandle;
class DLLCLIENT CEnvReflectionProbe
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
