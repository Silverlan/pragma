// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:audio.aux_util;

export import pragma.soundsystem;
export import pragma.udm;
export import std;

export namespace al {
	DLLCLIENT std::shared_ptr<al::IEffect> create_aux_effect(const std::string &type, udm::LinkedPropertyWrapper &prop);
	DLLCLIENT std::shared_ptr<al::IEffect> create_aux_effect(const std::string &name, const std::string &type, udm::LinkedPropertyWrapper &prop);
	DLLCLIENT const std::vector<std::string> &get_aux_types();
};
