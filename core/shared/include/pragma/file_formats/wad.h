// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
