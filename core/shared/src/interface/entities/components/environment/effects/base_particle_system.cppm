// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <string>

export module pragma.shared:entities.components.environment.effects.base_particle_system;

export import :entities.components.base;

#define SF_PARTICLE_SYSTEM_CONTINUOUS 2048
#define SF_PARTICLE_SYSTEM_REMOVE_ON_COMPLETE 4096

export namespace pragma {
	class DLLNETWORK BaseEnvParticleSystemComponent : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
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
