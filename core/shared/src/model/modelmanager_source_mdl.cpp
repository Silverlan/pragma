/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/model/modelmanager.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/asset/util_asset.hpp"

pragma::asset::SourceMdlFormatHandler::SourceMdlFormatHandler(util::IAssetManager &assetManager) : util::IImportAssetFormatHandler {assetManager} {}
bool pragma::asset::SourceMdlFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	auto &nw = static_cast<ModelManager &>(GetAssetManager()).GetNetworkState();
	auto path = static_cast<ModelManager &>(GetAssetManager()).GetRootDirectory().GetString() + ufile::get_path_from_filename(outputPath);
	auto fileName = ufile::get_file_from_filename(outputPath);
	outFilePath = outputPath;
	return util::port_hl2_model(&nw, path, fileName);
}

/////////

pragma::asset::Source2VmdlFormatHandler::Source2VmdlFormatHandler(util::IAssetManager &assetManager) : util::IImportAssetFormatHandler {assetManager} {}
bool pragma::asset::Source2VmdlFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	auto &nw = static_cast<ModelManager &>(GetAssetManager()).GetNetworkState();
	auto path = static_cast<ModelManager &>(GetAssetManager()).GetRootDirectory().GetString() + ufile::get_path_from_filename(outputPath);
	auto fileName = ufile::get_file_from_filename(outputPath);
	outFilePath = outputPath;
	return util::port_source2_model(&nw, path, fileName);
}

/////////

pragma::asset::NifFormatHandler::NifFormatHandler(util::IAssetManager &assetManager) : util::IImportAssetFormatHandler {assetManager} {}
bool pragma::asset::NifFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	auto &nw = static_cast<ModelManager &>(GetAssetManager()).GetNetworkState();
	return util::port_nif_model(&nw, outputPath, outFilePath);
}

/////////

pragma::asset::AssetManagerFormatHandler::AssetManagerFormatHandler(util::IAssetManager &assetManager) : util::IImportAssetFormatHandler {assetManager} {}
bool pragma::asset::AssetManagerFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	auto *game = static_cast<ModelManager &>(GetAssetManager()).GetNetworkState().GetGameState();
	if(!game)
		return false;
	std::string err;
	if(!pragma::get_engine()->GetAssetManager().ImportAsset(*game, pragma::asset::Type::Model, m_file.get(), outputPath, &err)) {
		m_error = std::move(err);
		return false;
	}
	return true;
}
