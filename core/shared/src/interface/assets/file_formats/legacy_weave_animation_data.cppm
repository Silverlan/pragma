// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:assets.file_formats.legacy_weave_animation_data;

import :model.animation;

export class DLLNETWORK FWAD {
  private:
  public:
	std::shared_ptr<pragma::animation::Animation> ReadData(unsigned short version, ufile::IFile &f);
	std::shared_ptr<pragma::animation::Animation> Load(unsigned short version, const char *animation);
};
