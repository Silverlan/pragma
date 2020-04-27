/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __ENV_FIRE_H__
#define __ENV_FIRE_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <string>

namespace pragma
{
	class DLLNETWORK BaseEnvFireComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	protected:
		std::string m_fireType = "fire";
	};
};

#endif
