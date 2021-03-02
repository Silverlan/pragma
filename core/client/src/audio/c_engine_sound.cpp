/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include <sharedutils/util_pragma.hpp>
#include <pragma/util/util_game.hpp>
#include <alsoundsystem.hpp>
#include <alsound_coordinate_system.hpp>

extern DLLCLIENT CEngine *c_engine;

const al::SoundSystem *CEngine::GetSoundSystem() const {return const_cast<CEngine*>(this)->GetSoundSystem();}
al::SoundSystem *CEngine::GetSoundSystem() {return m_soundSystem.get();}

namespace sci
{
	class SoundSourceFactory
		: public al::SoundSourceFactory
	{
	public:
		virtual al::SoundSource *CreateSoundSource(al::SoundSystem &system,al::SoundBuffer &buffer,al::InternalSource *source) override {return new CALSound(c_engine->GetClientState(),system,buffer,source);}
		virtual al::SoundSource *CreateSoundSource(al::SoundSystem &system,al::Decoder &decoder,al::InternalSource *source) override {return new CALSound(c_engine->GetClientState(),system,decoder,source);}
	};
};

al::SoundSystem *CEngine::InitializeSoundEngine()
{
	Con::cout<<"Initializing sound engine..."<<Con::endl;
	m_soundSystem = al::SoundSystem::Create(util::pragma::units_to_metres(1.0));
	if(m_soundSystem == nullptr)
	{
		Con::cerr<<"WARNING: Unable to initialize sound engine!"<<Con::endl;
		return nullptr;
	}
	m_soundSystem->SetSoundSourceFactory(std::make_unique<sci::SoundSourceFactory>());
	al::set_world_scale(util::pragma::units_to_metres(1.0));
	return m_soundSystem.get();
}

al::PEffect CEngine::GetAuxEffect(const std::string &name)
{
	auto lname = name;
	ustring::to_lower(lname);
	auto it = m_auxEffects.find(lname);
	if(it == m_auxEffects.end())
		return nullptr;
	return it->second;
}

void CEngine::CloseSoundEngine() {m_soundSystem = nullptr;}

void CEngine::SetHRTFEnabled(bool b)
{
	auto *soundSys = GetSoundSystem();
	if(soundSys == nullptr)
		return;
	if(b == false)
		soundSys->DisableHRTF();
	else
	{
		soundSys->SetHRTF(0);
		if(soundSys->IsHRTFEnabled() == false)
			Con::cwar<<"WARNING: Unable to activate HRTF. Please make sure *.mhr-file is in correct directory!"<<Con::endl;
	}
}
