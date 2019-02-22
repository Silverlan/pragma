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
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void SetRemoveOnComplete(bool b);
		virtual void SetContinuous(bool b);
		virtual void SetParticleFile(const std::string &fileName);
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
