// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:audio.aux_util;

export import pragma.soundsystem;
export import pragma.udm;
export import std;

export namespace pragma::audio {
	DLLCLIENT std::shared_ptr<IEffect> create_aux_effect(const std::string &type, udm::LinkedPropertyWrapper &prop);
	DLLCLIENT std::shared_ptr<IEffect> create_aux_effect(const std::string &name, const std::string &type, udm::LinkedPropertyWrapper &prop);
	DLLCLIENT const std::vector<std::string> &get_aux_types();
};
