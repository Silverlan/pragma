// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.model_manager;

pragma::asset::AssimpFormatHandler::AssimpFormatHandler(util::IAssetManager &assetManager) : IImportAssetFormatHandler {assetManager} {}
bool pragma::asset::AssimpFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	std::string err;
	auto result = Lua::import::import_model_asset(static_cast<ModelManager &>(GetAssetManager()).GetNetworkState(), outputPath, outFilePath, err);
	if(!result)
		m_error = std::move(err);
	return result;
}
