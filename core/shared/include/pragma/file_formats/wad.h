/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __WAD_H__
#define __WAD_H__
#include "pragma/networkdefinitions.h"
#include "pragma/model/animation/animation.hpp"

namespace ufile {
	struct IFile;
};
class DLLNETWORK FWAD {
  private:
  public:
	std::shared_ptr<pragma::animation::Animation> ReadData(unsigned short version, ufile::IFile &f);
	std::shared_ptr<pragma::animation::Animation> Load(unsigned short version, const char *animation);
};

#endif
