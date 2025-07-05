// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SOFTBODY_COMPONENT_HPP__
#define __C_SOFTBODY_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_softbody_component.hpp>

namespace pragma {
	class DLLCLIENT CSoftBodyComponent final : public BaseSoftBodyComponent {
	  public:
		CSoftBodyComponent(BaseEntity &ent) : BaseSoftBodyComponent(ent) {}

		virtual void Initialize() override;
		void UpdateSoftBodyGeometry();
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual bool InitializeSoftBodyData() override;
		virtual void ReleaseSoftBodyData() override;
	};
};

#endif
