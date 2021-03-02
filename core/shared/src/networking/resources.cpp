/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networking/resources.h"
#include <sharedutils/util_file.h>

static const std::string aExtensions[] = {
	"cache",
	"wav",
	"mp3",
	"ogg",
	"dds",
	"wmd",
	"wad",
	"txt",
	"gls",
	"clua",
	"wld",
	"txt",
	"wmi"
};
static std::vector<std::string> extensions(aExtensions,aExtensions +sizeof(aExtensions) /sizeof(aExtensions[0]));

static const std::string aFolders[] = {
	"cache",
	"maps",
	"models",
	"sounds",
	"materials",
	"shaders",
	"scripts",
	"data",
	"lua"
};
static std::vector<std::string> folders(aFolders,aFolders +sizeof(aFolders) /sizeof(aFolders[0]));

DLLNETWORK bool IsValidResource(std::string res)
{
	if(!ufile::compare_extension(res,&extensions))
		return false;
	if(!ufile::compare_directory(res,&folders))
		return false;
	return true;
}