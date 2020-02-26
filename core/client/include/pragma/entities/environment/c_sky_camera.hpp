#ifndef __C_SKY_CAMERA_HPP__
#define __C_SKY_CAMERA_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/rendering/render_mesh_collection_handler.hpp"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma
{
	namespace rendering {struct CulledMeshData;};
	class DLLCLIENT CSkyCameraComponent final
		: public BaseEntityComponent
	{
	public:
		CSkyCameraComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		float GetSkyboxScale() const;
		const rendering::CulledMeshData &UpdateRenderMeshes(rendering::RasterizationRenderer &renderer,FRender renderFlags);

		rendering::RenderMeshCollectionHandler &GetRenderMeshCollectionHandler();
		const rendering::RenderMeshCollectionHandler &GetRenderMeshCollectionHandler() const;
	private:
		rendering::RenderMeshCollectionHandler m_renderMeshCollectionHandler = {};
		float m_skyboxScale = 1.f;
		CallbackHandle m_cbOnPreRender = {};
	};
};

class EntityHandle;
class DLLCLIENT CSkyCamera
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
