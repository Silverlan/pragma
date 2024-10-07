/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __GAME_RESOURCES_HP__
#define __GAME_RESOURCES_HP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <optional>
#include <fsys/filesystem.h>

class NetworkState;
class Model;
namespace util {
	class Library;
	static const std::string IMPORT_PATH = "addons/imported/";
	static const std::string CONVERT_PATH = "addons/converted/";
	namespace impl {
		DLLNETWORK void *get_module_func(NetworkState *nw, const std::string &name);
	};
	DLLNETWORK std::shared_ptr<util::Library> initialize_external_archive_manager(NetworkState *nw);
	DLLNETWORK void close_external_archive_manager();
	DLLNETWORK std::optional<int32_t> get_mounted_game_priority(const std::string &game);
	DLLNETWORK void set_mounted_game_priority(const std::string &game, int32_t priority);

	DLLNETWORK bool port_nif_model(NetworkState *nw, const std::string &path, std::string mdlName);
	DLLNETWORK bool port_hl2_smd(NetworkState &nw, Model &mdl, VFilePtr &f, const std::string &animName, bool isCollisionMesh, std::vector<std::string> &outTextures);
	DLLNETWORK bool port_hl2_model(NetworkState *nw, const std::string &path, std::string mdlName);
	DLLNETWORK bool port_source2_model(NetworkState *nw, const std::string &path, std::string mdlName);
	DLLNETWORK bool port_hl2_particle(NetworkState *nw, const std::string &path);
	DLLNETWORK bool port_hl2_map(NetworkState *nw, const std::string &path);
	DLLNETWORK bool port_source2_map(NetworkState *nw, const std::string &path);
	DLLNETWORK bool port_file(NetworkState *nw, const std::string &path, const std::optional<std::string> &outputPath = {});
	DLLNETWORK bool port_sound_script(NetworkState *nw, const std::string &path);
};

#endif
