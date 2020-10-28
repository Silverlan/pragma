/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LIGHT_MAP_COMPONENT_HPP__
#define __C_LIGHT_MAP_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace bsp {class File;};
namespace prosper {class IDynamicResizableBuffer;};
namespace util::bsp {struct LightMapInfo;};
namespace pragma
{
	class DLLCLIENT CLightMapComponent final
		: public BaseEntityComponent
	{
	public:
		static std::shared_ptr<prosper::IDynamicResizableBuffer> GenerateLightmapUVBuffers(std::vector<std::shared_ptr<prosper::IBuffer>> &outMeshLightMapUvBuffers);
		static std::shared_ptr<prosper::Texture> CreateLightmapTexture(uint32_t width,uint32_t height,const uint16_t *hdrPixelData);

		CLightMapComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		const std::shared_ptr<prosper::Texture> &GetLightMap() const;

		void InitializeLightMapData(
			const std::shared_ptr<prosper::Texture> &lightMap,
			const std::shared_ptr<prosper::IDynamicResizableBuffer> &lightMapUvBuffer,
			const std::vector<std::shared_ptr<prosper::IBuffer>> &meshUvBuffers
		);
		void SetLightMapAtlas(const std::shared_ptr<prosper::Texture> &lightMap);
		void ReloadLightMapData();

		prosper::IBuffer *GetMeshLightMapUvBuffer(uint32_t meshIdx) const;
		const std::vector<std::shared_ptr<prosper::IBuffer>> &GetMeshLightMapUvBuffers() const;
		std::vector<std::shared_ptr<prosper::IBuffer>> &GetMeshLightMapUvBuffers();

		void SetLightMapIntensity(float intensity);
		void SetLightMapExposure(float exp);
		float GetLightMapIntensity() const;
		float GetLightMapExposure() const;

		void ConvertLightmapToBSPLuxelData() const;

		void UpdateLightmapUvBuffers();
		std::shared_ptr<prosper::IDynamicResizableBuffer> GetGlobalLightMapUvBuffer() const;
	protected:
		std::shared_ptr<prosper::Texture> m_lightMapAtlas = nullptr;
		float m_lightMapIntensity = 1.f;
		float m_lightMapExposure = 0.f;

		// Contains the light map uv-buffer for each mesh of the world in the same order
		// they are in the model's mesh group
		std::vector<std::shared_ptr<prosper::IBuffer>> m_meshLightMapUvBuffers;
		std::shared_ptr<prosper::IDynamicResizableBuffer> m_meshLightMapUvBuffer = nullptr;
	};
};

#endif
