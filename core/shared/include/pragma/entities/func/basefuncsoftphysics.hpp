/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEFUNCSOFTPHYSICS_HPP__
#define __BASEFUNCSOFTPHYSICS_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"

namespace pragma
{
	class DLLNETWORK BaseFuncSoftPhysicsComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		PhysSoftBodyInfo *GetSoftBodyInfo();
	protected:
		bool SetKeyValue(std::string key,std::string val);
		PhysSoftBodyInfo m_softBodyInfo = {};
	};
};

#endif
