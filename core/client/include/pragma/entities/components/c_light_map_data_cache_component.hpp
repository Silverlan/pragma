/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LIGHT_MAP_DATA_CACHE_COMPONENT_HPP__
#define __C_LIGHT_MAP_DATA_CACHE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma
{
	struct LightmapDataCache;
	class DLLCLIENT CLightMapDataCacheComponent final
		: public BaseEntityComponent
	{
	public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember);
		CLightMapDataCacheComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;

		void SetLightMapDataCache(const std::string &cachePath);
		const std::string &GetLightMapDataCache() const;
		void ReloadCache();
		void InitializeUvBuffers();
	protected:
		std::shared_ptr<LightmapDataCache> m_lightmapDataCache = nullptr;
		std::string m_lightmapDataCacheFile;
	};
};

#endif
