#ifndef __BASEFUNCPHYSICS_H__
#define __BASEFUNCPHYSICS_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/func/basefuncsurfacematerial.hpp"

namespace pragma
{
	class DLLNETWORK BaseFuncPhysicsComponent
		: public BaseFuncSurfaceMaterialComponent
	{
	public:
		using BaseFuncSurfaceMaterialComponent::BaseFuncSurfaceMaterialComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	protected:
		virtual PhysObj *InitializePhysics();

		float m_kvMass = 0.f;
		bool m_bClientsidePhysics = false;
	};
};

#endif
