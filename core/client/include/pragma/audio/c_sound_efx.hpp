// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SOUND_EFX_HPP__
#define __C_SOUND_EFX_HPP__

#include "pragma/clientdefinitions.h"
#include <vector>
#include <string>
#include <memory>

namespace udm {
	struct LinkedPropertyWrapper;
};
namespace al {
	class IEffect;
	DLLCLIENT std::shared_ptr<al::IEffect> create_aux_effect(const std::string &type, udm::LinkedPropertyWrapper &prop);
	DLLCLIENT std::shared_ptr<al::IEffect> create_aux_effect(const std::string &name, const std::string &type, udm::LinkedPropertyWrapper &prop);
	DLLCLIENT const std::vector<std::string> &get_aux_types();
};

#endif
