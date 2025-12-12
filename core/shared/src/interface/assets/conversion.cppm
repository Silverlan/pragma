// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:assets.conversion;

export import :types;
export import pragma.filesystem;

export namespace pragma::util {
	DLLNETWORK const std::string IMPORT_PATH = "addons/imported/";
	DLLNETWORK const std::string CONVERT_PATH = "addons/converted/";
	namespace impl {
		DLLNETWORK void *get_module_func(pragma::NetworkState *nw, const std::string &name);
	};
	DLLNETWORK std::shared_ptr<pragma::util::Library> initialize_external_archive_manager(pragma::NetworkState *nw);
	DLLNETWORK void close_mount_external_library();
	DLLNETWORK void close_external_archive_manager();
	DLLNETWORK std::optional<int32_t> get_mounted_game_priority(const std::string &game);
	DLLNETWORK void set_mounted_game_priority(const std::string &game, int32_t priority);

	DLLNETWORK bool port_nif_model(pragma::NetworkState *nw, const std::string &path, std::string mdlName);
	DLLNETWORK bool port_hl2_smd(pragma::NetworkState &nw, pragma::asset::Model &mdl, VFilePtr &f, const std::string &animName, bool isCollisionMesh, std::vector<std::string> &outTextures);
	DLLNETWORK bool port_hl2_model(pragma::NetworkState *nw, const std::string &path, std::string mdlName);
	DLLNETWORK bool port_source2_model(pragma::NetworkState *nw, const std::string &path, std::string mdlName);
	DLLNETWORK bool port_hl2_particle(pragma::NetworkState *nw, const std::string &path);
	DLLNETWORK bool port_hl2_map(pragma::NetworkState *nw, const std::string &path);
	DLLNETWORK bool port_source2_map(pragma::NetworkState *nw, const std::string &path);
	DLLNETWORK bool port_file(pragma::NetworkState *nw, const std::string &path, const std::optional<std::string> &outputPath = {});
	DLLNETWORK bool port_sound_script(pragma::NetworkState *nw, const std::string &path);
};
