/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/modelmanager.h"

pragma::asset::SourceMdlFormatHandler::SourceMdlFormatHandler(util::IAssetManager &assetManager)
	: util::IImportAssetFormatHandler{assetManager}
{}
bool pragma::asset::SourceMdlFormatHandler::Import(const std::string &outputPath,std::string &outFilePath)
{
	// TODO
	return false;
}
