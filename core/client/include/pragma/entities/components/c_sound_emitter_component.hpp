/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SOUND_EMITTER_COMPONENT_HPP__
#define __C_SOUND_EMITTER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_sound_emitter_component.hpp>

namespace pragma
{
	class DLLCLIENT CSoundEmitterComponent final
		: public BaseSoundEmitterComponent,
		public CBaseNetComponent
	{
	public:
		CSoundEmitterComponent(BaseEntity &ent) : BaseSoundEmitterComponent(ent) {}
		// Sounds
		virtual std::shared_ptr<ALSound> CreateSound(std::string sndname,ALSoundType type) override;
		virtual std::shared_ptr<ALSound> EmitSound(std::string sndname,ALSoundType type,float gain,float pitch=1.f) override;
		void AddSound(std::shared_ptr<ALSound> snd); // Only to be used from netmessage

		virtual void PrecacheSounds() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	protected:
		virtual void MaintainSounds() override;
		virtual bool ShouldRemoveSound(ALSound &snd) const override;
	};
};

#endif
