/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_TRANSFORM_COMPONENT_HPP__
#define __S_TRANSFORM_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>

namespace pragma
{
	class DLLSERVER STransformComponent final
		: public BaseTransformComponent,
		public SBaseNetComponent
	{
	public:
		STransformComponent(BaseEntity &ent) : BaseTransformComponent(ent) {}
		virtual void SetEyeOffset(const Vector3 &offset) override;
		virtual void SetScale(const Vector3 &scale) override;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

#endif
