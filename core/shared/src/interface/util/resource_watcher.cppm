// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.resource_watcher;

export import :networking.util;
export import :types;
export import pragma.materialsystem;

export namespace pragma::util {
	class DLLNETWORK EResourceWatcherCallbackType : public ExtensibleEnum {
	  public:
		// declare the enum first so it can be used in following declarations
		enum class E : uint32_t { Model = 0u, Material, Texture, Map, SoundScript, Sound, Count };

		// public factory that accepts the enum type (no cast needed at call site)
		static EResourceWatcherCallbackType createFromEnum(E e) noexcept { return EResourceWatcherCallbackType(static_cast<uint32_t>(e)); }
	  protected:
		// forwarding ctor that calls the base ctor (base ctor is protected)
		// do NOT mark constexpr if the base ctor is not constexpr
		explicit EResourceWatcherCallbackType(uint32_t v) noexcept : ExtensibleEnum(v) {}
	};
	namespace eResourceWatcherCallbackType {
		CLASS_ENUM_COMPAT const EResourceWatcherCallbackType Model;
		CLASS_ENUM_COMPAT const EResourceWatcherCallbackType Material;
		CLASS_ENUM_COMPAT const EResourceWatcherCallbackType Texture;
		CLASS_ENUM_COMPAT const EResourceWatcherCallbackType Map;
		CLASS_ENUM_COMPAT const EResourceWatcherCallbackType SoundScript;
		CLASS_ENUM_COMPAT const EResourceWatcherCallbackType Sound;
		CLASS_ENUM_COMPAT const EResourceWatcherCallbackType Count;
	}
};

export {
	namespace std {
		template<>
		struct hash<pragma::util::EResourceWatcherCallbackType> {
			std::size_t operator()(const pragma::util::EResourceWatcherCallbackType &object) const { return object.Hash(); }
		};
	}
}

export namespace pragma::util {
	class DLLNETWORK ResourceWatcherManager {
	public:
		using TypeHandler = std::function<void(const Path &, const std::string &)>;
		ResourceWatcherManager(NetworkState *nw);
		bool MountDirectory(const std::string &path, bool bAbsolutePath = false);
		void Poll();

		// All file changes will be ignored as long as the watcher is locked.
		// It can be locked multiple times and has to be unlocked the same amount of
		// times to resume watching
		void Lock();
		void Unlock();
		ScopeGuard ScopeLock();
		bool IsLocked() const;
		CallbackHandle AddChangeCallback(EResourceWatcherCallbackType type, const std::function<void(std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>)> &fcallback);
		void RegisterTypeHandler(const std::string &ext, const TypeHandler &handler);
	protected:
		NetworkState *m_networkState = nullptr;
		uint32_t m_lockedCount = 0;
		std::recursive_mutex m_watcherMutex;
		void OnResourceChanged(const Path &rootPath, const Path &path);
		void ReloadMaterial(const std::string &path);
		virtual void OnMaterialReloaded(const std::string &path, const std::unordered_set<asset::Model *> &modelMap) {}
		virtual void OnResourceChanged(const Path &rootPath, const Path &path, const std::string &ext);
		virtual void GetWatchPaths(std::vector<std::string> &paths);
		virtual void ReloadTexture(const std::string &path);
		void CallChangeCallbacks(EResourceWatcherCallbackType type, const std::string &path, const std::string &ext);
	private:
		std::unordered_map<EResourceWatcherCallbackType, std::vector<CallbackHandle>> m_callbacks;
		std::unordered_map<std::string, std::function<void()>> m_watchFiles;
		std::vector<std::shared_ptr<fs::DirectoryWatcherCallback>> m_watchers;
		std::unordered_map<std::string, TypeHandler> m_typeHandlers;
		std::shared_ptr<fs::DirectoryWatcherManager> m_watcherManager;
	};
}
