/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/modelmanager.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/asset/util_asset.hpp"
#include "pragma/lua/libraries/limport.hpp"

pragma::asset::AssimpFormatHandler::AssimpFormatHandler(util::IAssetManager &assetManager) : util::IImportAssetFormatHandler {assetManager} {}
bool pragma::asset::AssimpFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	std::string err;
	auto result = Lua::import::import_model_asset(static_cast<ModelManager &>(GetAssetManager()).GetNetworkState(), outputPath, outFilePath, err);
	if(!result)
		m_error = std::move(err);
	return result;
}
