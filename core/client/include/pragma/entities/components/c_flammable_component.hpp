/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_FLAMMABLE_EMITTER_COMPONENT_HPP__
#define __C_FLAMMABLE_EMITTER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_flammable_component.hpp>

namespace pragma
{
	class CParticleSystemComponent;
	class DLLCLIENT CFlammableComponent final
		: public BaseFlammableComponent,
		public CBaseNetComponent
	{
	public:
		CFlammableComponent(BaseEntity &ent) : BaseFlammableComponent(ent) {}
		virtual void Initialize() override;
		virtual util::EventReply Ignite(float duration,BaseEntity *attacker=nullptr,BaseEntity *inflictor=nullptr) override;
		virtual void Extinguish() override;
		virtual void OnTick(double dt) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	protected:
		void UpdateFlameParticlePositions();
		struct DLLCLIENT IgniteInfo
		{
			IgniteInfo();
			~IgniteInfo();
			struct Particle
			{
				Particle(pragma::CParticleSystemComponent &pt,uint32_t boneId=0);
				Particle(pragma::CParticleSystemComponent &pt,const Vector3 &offset);
				util::WeakHandle<CParticleSystemComponent> hParticle;
				uint32_t boneId;
				Vector3 offset;
			};
			void Clear();
			std::shared_ptr<ALSound> sound = nullptr;
			std::vector<Particle> flameParticles;
		} m_igniteInfo = {};
	};
};

#endif
