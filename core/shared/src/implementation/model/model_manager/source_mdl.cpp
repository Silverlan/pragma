// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.model_manager;

pragma::asset::SourceMdlFormatHandler::SourceMdlFormatHandler(util::IAssetManager &assetManager) : IImportAssetFormatHandler {assetManager} {}
bool pragma::asset::SourceMdlFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	auto &nw = static_cast<ModelManager &>(GetAssetManager()).GetNetworkState();
	auto path = static_cast<ModelManager &>(GetAssetManager()).GetRootDirectory().GetString() + ufile::get_path_from_filename(outputPath);
	auto fileName = ufile::get_file_from_filename(outputPath);
	outFilePath = outputPath;
	return pragma::util::port_hl2_model(&nw, path, fileName);
}

/////////

pragma::asset::Source2VmdlFormatHandler::Source2VmdlFormatHandler(util::IAssetManager &assetManager) : IImportAssetFormatHandler {assetManager} {}
bool pragma::asset::Source2VmdlFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	auto &nw = static_cast<ModelManager &>(GetAssetManager()).GetNetworkState();
	auto path = static_cast<ModelManager &>(GetAssetManager()).GetRootDirectory().GetString() + ufile::get_path_from_filename(outputPath);
	auto fileName = ufile::get_file_from_filename(outputPath);
	outFilePath = outputPath;
	return pragma::util::port_source2_model(&nw, path, fileName);
}

/////////

pragma::asset::NifFormatHandler::NifFormatHandler(util::IAssetManager &assetManager) : IImportAssetFormatHandler {assetManager} {}
bool pragma::asset::NifFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	auto &nw = static_cast<ModelManager &>(GetAssetManager()).GetNetworkState();
	return pragma::util::port_nif_model(&nw, outputPath, outFilePath);
}

/////////

pragma::asset::AssetManagerFormatHandler::AssetManagerFormatHandler(util::IAssetManager &assetManager) : IImportAssetFormatHandler {assetManager} {}
bool pragma::asset::AssetManagerFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	auto *game = static_cast<ModelManager &>(GetAssetManager()).GetNetworkState().GetGameState();
	if(!game)
		return false;
	std::string err;
	if(!get_engine()->GetAssetManager().ImportAsset(*game, Type::Model, m_file.get(), outputPath, &err)) {
		m_error = std::move(err);
		return false;
	}
	return true;
}
