/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SKY_CAMERA_HPP__
#define __C_SKY_CAMERA_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/rendering/render_mesh_collection_handler.hpp"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma
{
	class ShaderGameWorld;
	namespace rendering {struct BaseRenderProcessor; class RenderQueue;};
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
	private:
		void BindToShader(pragma::rendering::BaseRenderProcessor &processor);
		void UnbindFromShader(pragma::rendering::BaseRenderProcessor &processor);
		float m_skyboxScale = 1.f;
		CallbackHandle m_cbOnBuildRenderQueue = {};
		CallbackHandle m_cbPostRenderSkybox = {};
		std::shared_ptr<pragma::rendering::RenderQueue> m_renderQueue = nullptr;
		std::shared_ptr<pragma::rendering::RenderQueue> m_renderQueueTranslucent = nullptr;
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
