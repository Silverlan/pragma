// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:core.resource_watcher;

export import pragma.shared;

export namespace pragma::util {
	class DLLSERVER ESResourceWatcherCallbackType : public EResourceWatcherCallbackType {
	  public:
		using EResourceWatcherCallbackType::EResourceWatcherCallbackType;

		static const ESResourceWatcherCallbackType NavMesh;
		static const ESResourceWatcherCallbackType Count;
	  protected:
		enum class E : uint32_t { NavMesh = math::to_integral(EResourceWatcherCallbackType::E::Count), Count };
	};

	class DLLSERVER SResourceWatcherManager : public ResourceWatcherManager {
	  protected:
		virtual void OnResourceChanged(const Path &rootPath, const Path &path, const std::string &ext) override;
	  public:
		using ResourceWatcherManager::ResourceWatcherManager;
	};
};
