// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:util.resource_watcher;

export import pragma.shared;

export namespace pragma::util {
	class DLLCLIENT ECResourceWatcherCallbackType : public EResourceWatcherCallbackType {
	  public:
		using EResourceWatcherCallbackType::EResourceWatcherCallbackType;

		static const ECResourceWatcherCallbackType Shader;
		static const ECResourceWatcherCallbackType ParticleSystem;
		static const ECResourceWatcherCallbackType Count;
	  protected:
		enum class E : uint32_t { Shader = math::to_integral(EResourceWatcherCallbackType::E::Count), ParticleSystem, Count };
	};

	class DLLCLIENT CResourceWatcherManager : public ResourceWatcherManager {
	  protected:
		virtual void OnMaterialReloaded(const std::string &path, const std::unordered_set<asset::Model *> &modelMap) override;
		virtual void OnResourceChanged(const Path &rootPath, const Path &path, const std::string &ext) override;
		virtual void ReloadTexture(const std::string &path) override;
		virtual void GetWatchPaths(std::vector<std::string> &paths) override;
	  public:
		CResourceWatcherManager(NetworkState *nw);
	};
};
