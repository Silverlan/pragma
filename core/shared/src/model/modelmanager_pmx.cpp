// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/model/modelmanager.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/asset/util_asset.hpp"

pragma::asset::PmxFormatHandler::PmxFormatHandler(util::IAssetManager &assetManager) : AssetManagerFormatHandler {assetManager} {}
bool pragma::asset::PmxFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	auto &state = static_cast<ModelManager &>(GetAssetManager()).GetNetworkState();
	auto *game = state.GetGameState();
	if(!game)
		return false;
	std::string err;
	if(!state.InitializeLibrary("mount_external/pr_mount_external", &err, game->GetLuaState())) {
		m_error = "Failed to load module 'mount_external/pr_mount_external'!";
		return false;
	}
	return AssetManagerFormatHandler::Import(outputPath, outFilePath);
}
