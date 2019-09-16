#ifndef __C_LIGHT_MAP_COMPONENT_HPP__
#define __C_LIGHT_MAP_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace bsp {class File;};
namespace prosper {class DynamicResizableBuffer;};
namespace pragma
{
	class DLLCLIENT CLightMapComponent final
		: public BaseEntityComponent
	{
	public:
		static std::shared_ptr<prosper::Texture> LoadLightMap(pragma::level::BSPInputData &bspInputData);
		static std::shared_ptr<prosper::DynamicResizableBuffer> LoadLightMapUvBuffers(const std::vector<std::vector<Vector2>> &meshUvData,std::vector<std::shared_ptr<prosper::Buffer>> &outMeshLightMapUvBuffers);

		CLightMapComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		const std::shared_ptr<prosper::Texture> &GetLightMap() const;

		void InitializeLightMapData(
			const std::shared_ptr<prosper::Texture> &lightMap,
			const std::shared_ptr<prosper::DynamicResizableBuffer> &lightMapUvBuffer,
			const std::vector<std::shared_ptr<prosper::Buffer>> &meshUvBuffers
		);

		prosper::Buffer *GetMeshLightMapUvBuffer(uint32_t meshIdx) const;
		const std::vector<std::shared_ptr<prosper::Buffer>> &GetMeshLightMapUvBuffers() const;
		std::vector<std::shared_ptr<prosper::Buffer>> &GetMeshLightMapUvBuffers();

		// Reads the uv coordinates from the buffer
		void ReadLightmapUvCoordinates(std::vector<std::vector<Vector2>> &uvs) const;
		std::shared_ptr<prosper::DynamicResizableBuffer> GetGlobalLightMapUvBuffer() const;
	protected:
		std::shared_ptr<prosper::Texture> m_lightMapAtlas = nullptr;

		// Contains the light map uv-buffer for each mesh of the world in the same order
		// they are in the model's mesh group
		std::vector<std::shared_ptr<prosper::Buffer>> m_meshLightMapUvBuffers;
		std::shared_ptr<prosper::DynamicResizableBuffer> m_meshLightMapUvBuffer = nullptr;
	};
};

#endif
