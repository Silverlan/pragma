/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_PARTICLE_SYSTEM_H__
#define __ENV_PARTICLE_SYSTEM_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/components/basetoggle.h"
#include "pragma/entities/baseentity_handle.h"
#include <string>

#define SF_PARTICLE_SYSTEM_CONTINUOUS 2048
#define SF_PARTICLE_SYSTEM_REMOVE_ON_COMPLETE 4096

namespace pragma
{
	class DLLNETWORK BaseEnvParticleSystemComponent
		: public BaseEntityComponent
	{
	public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember);
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void SetRemoveOnComplete(bool b);
		virtual void SetContinuous(bool b);
		virtual void SetParticleFile(const std::string &fileName);
		const std::string &GetParticleFile() const;
		void SetParticleSystem(const std::string &ptName);
		const std::string &GetParticleSystem() const;
		bool IsContinuous() const;
		bool GetRemoveOnComplete() const;
		virtual void OnEntitySpawn() override;
	protected:
		void UpdateRemoveOnComplete();

		std::string m_particleName;
		std::string m_particleFile;
		bool m_bRemoveOnComplete = false;
	};
};

#endif
