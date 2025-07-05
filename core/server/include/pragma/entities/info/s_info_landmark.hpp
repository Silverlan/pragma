// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_INFO_LANDMARK_HPP__
#define __S_INFO_LANDMARK_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/info/info_landmark.hpp>

namespace pragma {
	class DLLSERVER SInfoLandmarkComponent final : public BaseInfoLandmarkComponent {
	  public:
		SInfoLandmarkComponent(BaseEntity &ent) : BaseInfoLandmarkComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER InfoLandmark : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
