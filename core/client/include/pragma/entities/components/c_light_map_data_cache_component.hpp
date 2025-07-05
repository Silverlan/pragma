// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LIGHT_MAP_DATA_CACHE_COMPONENT_HPP__
#define __C_LIGHT_MAP_DATA_CACHE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma {
	struct LightmapDataCache;
	class DLLCLIENT CLightMapDataCacheComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		CLightMapDataCacheComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;

		void SetLightMapDataCachePath(const std::string &cachePath);
		const std::string &GetLightMapDataCachePath() const;
		const std::shared_ptr<LightmapDataCache> &GetLightMapDataCache() const;
		void ReloadCache();
		void InitializeUvBuffers();

		virtual void OnTick(double dt) override;
	  protected:
		std::shared_ptr<LightmapDataCache> m_lightmapDataCache = nullptr;
		std::string m_lightmapDataCacheFile;
		bool m_lightmapDataCacheDirty = false;
	};
};

#endif
