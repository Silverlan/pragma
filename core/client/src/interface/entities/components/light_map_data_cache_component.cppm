// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.light_map_data_cache;

export import :rendering.lightmap_data_cache;
export import pragma.shared;

export namespace pragma {
	class DLLCLIENT CLightMapDataCacheComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		CLightMapDataCacheComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;

		void SetLightMapDataCachePath(const std::string &cachePath);
		const std::string &GetLightMapDataCachePath() const;
		const std::shared_ptr<rendering::LightmapDataCache> &GetLightMapDataCache() const;
		void ReloadCache();
		void InitializeUvBuffers();

		virtual void OnTick(double dt) override;
	  protected:
		std::shared_ptr<rendering::LightmapDataCache> m_lightmapDataCache = nullptr;
		std::string m_lightmapDataCacheFile;
		bool m_lightmapDataCacheDirty = false;
	};
};
