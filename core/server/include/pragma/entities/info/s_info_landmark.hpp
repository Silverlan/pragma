/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_INFO_LANDMARK_HPP__
#define __S_INFO_LANDMARK_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/info/info_landmark.hpp>

namespace pragma
{
	class DLLSERVER SInfoLandmarkComponent final
		: public BaseInfoLandmarkComponent
	{
	public:
		SInfoLandmarkComponent(BaseEntity &ent) : BaseInfoLandmarkComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER InfoLandmark
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
