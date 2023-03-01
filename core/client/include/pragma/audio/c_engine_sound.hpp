/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENGINE_SOUND_HPP__
#define __C_ENGINE_SOUND_HPP__

#include "pragma/c_engine.h"

template<class TEfxProperties>
std::shared_ptr<al::IEffect> CEngine::CreateAuxEffect(const std::string &name, const TEfxProperties &props)
{
	auto lname = name;
	ustring::to_lower(lname);
	auto effect = GetAuxEffect(lname);
	if(effect != nullptr)
		return effect;
	auto *soundSys = GetSoundSystem();
	if(soundSys == nullptr)
		return nullptr;
	try {
		effect = soundSys->CreateEffect(props);
	}
	catch(const std::runtime_error &e) {
		Con::cwar << "Unable to create auxiliary effect '" << name << "': " << e.what() << Con::endl;
		return nullptr;
	}
	m_auxEffects.insert(decltype(m_auxEffects)::value_type(name, effect));
	return effect;
}

#endif
