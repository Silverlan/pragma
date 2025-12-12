// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.server:core.resource_watcher;

export import pragma.shared;

export {
	class DLLSERVER ESResourceWatcherCallbackType : public EResourceWatcherCallbackType {
	  public:
		using EResourceWatcherCallbackType::EResourceWatcherCallbackType;

		static const ESResourceWatcherCallbackType NavMesh;
		static const ESResourceWatcherCallbackType Count;
	  protected:
		enum class E : uint32_t { NavMesh = pragma::math::to_integral(EResourceWatcherCallbackType::E::Count), Count };
	};

	class DLLSERVER SResourceWatcherManager : public ResourceWatcherManager {
	  protected:
		virtual void OnResourceChanged(const pragma::util::Path &rootPath, const pragma::util::Path &path, const std::string &ext) override;
	  public:
		using ResourceWatcherManager::ResourceWatcherManager;
	};
};
