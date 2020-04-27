/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LIGHT_MAP_RECEIVER_COMPONENT_HPP__
#define __C_LIGHT_MAP_RECEIVER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma
{
	class DLLCLIENT CLightMapReceiverComponent final
		: public BaseEntityComponent
	{
	public:
		static void SetupLightMapUvData(CBaseEntity &ent);
		using MeshIdx = uint32_t;
		using BufferIdx = uint32_t;

		CLightMapReceiverComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		const std::unordered_map<MeshIdx,std::vector<Vector2>> &GetMeshLightMapUvData() const;
		void AssignBufferIndex(MeshIdx meshIdx,BufferIdx bufIdx);
		std::optional<BufferIdx> FindBufferIndex(CModelSubMesh &mesh) const;
	protected:
		void UpdateModelMeshes();

		std::unordered_map<MeshIdx,std::vector<Vector2>> m_uvDataPerMesh {};
		std::unordered_map<MeshIdx,std::shared_ptr<ModelSubMesh>> m_meshes {};
		std::unordered_map<CModelSubMesh*,MeshIdx> m_meshToMeshIdx {};
		std::unordered_map<CModelSubMesh*,BufferIdx> m_meshToBufIdx {};
		std::string m_modelName; // Uvs are only valid for the model they were built with
		bool m_isModelBakedWithLightMaps = false;
	};
};

#endif
