// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:assets.conversion;

export import :types;
export import pragma.filesystem;

export namespace pragma::util {
#ifdef WINDOWS_CLANG_COMPILER_FIX
	DLLNETWORK const std::string &IMPORT_PATH {
		static std::string str = "addons/imported/";
		return str;
	}
	DLLNETWORK const std::string &CONVERT_PATH {
		static std::string str = "addons/converted/";
		return str;
	}
#else
	DLLNETWORK const std::string IMPORT_PATH = "addons/imported/";
	DLLNETWORK const std::string CONVERT_PATH = "addons/converted/";
#endif
	namespace impl {
		DLLNETWORK void *get_module_func(NetworkState *nw, const std::string &name);
	};
	DLLNETWORK std::shared_ptr<Library> initialize_external_archive_manager(NetworkState *nw);
	DLLNETWORK void close_mount_external_library();
	DLLNETWORK void close_external_archive_manager();
	DLLNETWORK std::optional<int32_t> get_mounted_game_priority(const std::string &game);
	DLLNETWORK void set_mounted_game_priority(const std::string &game, int32_t priority);

	DLLNETWORK bool port_nif_model(NetworkState *nw, const std::string &path, std::string mdlName);
	DLLNETWORK bool port_hl2_smd(NetworkState &nw, asset::Model &mdl,fs::VFilePtr &f, const std::string &animName, bool isCollisionMesh, std::vector<std::string> &outTextures);
	DLLNETWORK bool port_hl2_model(NetworkState *nw, const std::string &path, std::string mdlName);
	DLLNETWORK bool port_source2_model(NetworkState *nw, const std::string &path, std::string mdlName);
	DLLNETWORK bool port_hl2_particle(NetworkState *nw, const std::string &path);
	DLLNETWORK bool port_hl2_map(NetworkState *nw, const std::string &path);
	DLLNETWORK bool port_source2_map(NetworkState *nw, const std::string &path);
	DLLNETWORK bool port_file(NetworkState *nw, const std::string &path, const std::optional<std::string> &outputPath = {});
	DLLNETWORK bool port_sound_script(NetworkState *nw, const std::string &path);
};
