/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __INFO_LANDMARK_HPP__
#define __INFO_LANDMARK_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	class DLLNETWORK BaseInfoLandmarkComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	};
};

#endif
