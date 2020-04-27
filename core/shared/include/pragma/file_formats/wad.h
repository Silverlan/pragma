/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __WAD_H__
#define __WAD_H__
#include "pragma/networkdefinitions.h"
#include "pragma/file_formats/wdf.h"
#include "pragma/model/animation/animation.h"

// Currently unused
//#define WAD_VERSION 0x0001

class DLLNETWORK FWAD
	: FWDF
{
private:
	
public:
	std::shared_ptr<Animation> ReadData(unsigned short version,VFilePtr f);
	std::shared_ptr<Animation> Load(unsigned short version,const char *animation);
};

#endif