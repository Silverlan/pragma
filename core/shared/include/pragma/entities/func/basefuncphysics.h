/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

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
