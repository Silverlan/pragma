/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __MAPINFO_H__
#define __MAPINFO_H__

#include <string>
struct MapInfo
{
	std::string name;
	std::string fileName;
	std::string md5;
};

#endif