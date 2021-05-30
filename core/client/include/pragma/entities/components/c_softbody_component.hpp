/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SOFTBODY_COMPONENT_HPP__
#define __C_SOFTBODY_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_softbody_component.hpp>

namespace pragma
{
	class DLLCLIENT CSoftBodyComponent final
		: public BaseSoftBodyComponent
	{
	public:
		CSoftBodyComponent(BaseEntity &ent) : BaseSoftBodyComponent(ent) {}
		
		virtual void Initialize() override;
		void UpdateSoftBodyGeometry();
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		virtual bool InitializeSoftBodyData() override;
		virtual void ReleaseSoftBodyData() override;
	};
};

#endif
